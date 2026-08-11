# orcisf_engine

Headless C++ port of the legacy analysis/design/optimization engine
(`Optimasi Beton/Source/*.hpp`) — direct-stiffness 3D space-frame analysis,
RC beam/column design per SK SNI T-15-1991-03, and the "Flexible Polyhedron"
direct-search optimizer. No console I/O, no GUI dependency; the GUI (`src/`)
and the headless `orcisf_cli` tool both link against it.

Tracked as GitHub issue [#3](https://github.com/yohannaftali/orcisf/issues/3).

## Layout

```
engine/
├── CMakeLists.txt
├── include/engine/        # public headers (one per legacy source file, see below)
├── src/                    # implementations
└── tools/orcisf_cli.cpp    # headless validation/batch CLI (info/equilibrium/optimize)
```

| This port | Legacy source | Notes |
|---|---|---|
| `StructureData.h` | `Variabel.hpp` | Every legacy global as one owned (RAII) context struct passed by reference instead of touched via globals. Same field names/types/1-based indexing on purpose — see the file's own header comment. |
| `StiffnessSolver.{h,cpp}` | `Solver.hpp` | `banfac`/`bansol` → `BandedFactorize`/`BandedSolve`. Throws `UnstableStructureError` instead of printing+`exit(1)`. |
| `StructuralAnalysis.{h,cpp}` | `Struktur.hpp`, `ELEMEN.HPP`, the computational half of `Pembebanan.hpp` | `struktur()`/`inersia()`/`kekakuan_batang()`/`hasil()`/`periksa_batang()`/`indeks_batang()`/`isi_matrik_kekakuan()`/`beban()`/`berat_sendiri()`/`isi_elemen_balok()`/`elemen_lapangan()`/`elemen_tumpuan()`/`isi_elemen_kolom()`. |
| `BeamDesign.{h,cpp}` | `Balok.hpp` | `class balok`'s constructor → `DesignBeam()`; `lendutan()` → `Lendutan()`. |
| `ColumnDesign.{h,cpp}` | `Kolom.hpp` | `class kolom`'s constructor → `DesignColumn()`, including the false-position biaxial-capacity search. |
| `CostConstraint.{h,cpp}` | `Kendala.hpp` | `Kendala_Harga()` → `KendalaHarga()`. **Deliberately** does not re-run the analysis on every call (see the header comment) — a documented, justified deviation, not a fidelity gap. |
| `Optimizer.{h,cpp}` | `Polyhedron.hpp`, `Pengacakan.hpp`, `Penormalan.hpp`, `Telusur.hpp`, `Baru.hpp`, `Pengurutan.hpp`, `Diskritisasi.hpp` | `RunOptimization()` is the `optimasi()` driver; every other legacy file in this list has a matching internal (anonymous-namespace) function in `Optimizer.cpp`. |
| `LegacyIO.{h,cpp}` | `InOut.hpp` (read half), `Pembebanan.hpp` (read half), `CETAK.HPP` | Reads the legacy `.inp/.isd/.idl/.ijl/.ids/.ijs/.bbn` format exactly (plain whitespace-token ASCII); writes `.opt/.str/.kdl/.inf` reproducing `cetak_akhir()`, including its specific choice of which population slot's geometry the `.str` section reflects (see the function's header comment). |
| `Engine.{h,cpp}` | -- (new) | Facade: `RunFullOptimization()` / `LoadDatasetForViewing()`. Also writes the new detailed per-generation log (`<generic>.log.txt`, superset of `.his`). |
| `Discretization.h` | `Diskritisasi.hpp` | `konversi()` → `Konversi()` — see the file's comment for why this isn't just `std::round`. |

## Deliberate deviations from the legacy source (and why)

1. **`KendalaHarga()` doesn't re-read files or re-run the structural
   analysis on every call.** The legacy `Kendala_Harga()` called
   `baca_data()`/`baca_beban()`/`inersia()`/`struktur()` at the top of
   *every single invocation* (called potentially thousands of times per
   generation during `cari_baru()`'s search) — but always against the same
   frozen `no_struktur`, so those calls were idempotent after the first
   one. Re-running them here would only waste time and break thread-safety
   for future parallel evaluation (issue #4); the *result* is identical.
2. **Randomness is not bit-reproducible against the legacy binary.** The
   original used Borland's `randomize()` (seeded from wall-clock time) +
   `random(n)` — not reproducible even by re-running the *original* binary,
   let alone a modern reimplementation. `Optimizer.cpp` uses
   `std::mt19937` for the same *statistical* behavior (uniform in `[0,n)`).
   This means a full optimization run's exact final `.opt` numbers will
   differ from any specific historical run — see "Validation" below for
   what was checked instead.
3. **Multi-threading is issue #4's scope, not #3's.**
   `OptimizationOptions::worker_threads` exists as a forward-looking knob
   but `RunOptimization()` is currently single-threaded (correctness
   first). Making the per-generation population evaluation loop safe to
   parallelize needs each worker to own its own `StructureData` scratch
   state (`SFF`/`SM`/`SMRT`/`AM`/etc. are all mutated in place) — that's
   the actual work of #4.

Two more are **not** deviations — they're real characteristics of the
1999 algorithm, preserved on purpose (see `CostConstraint.h`'s header
comment for the first, `LegacyIO.h`'s `WriteFinalResults` comment for the
second):
- `KendalaHarga()` adaptively tightens a *column's* stirrup spacing/side
  length but not a beam's, while the optimizer's own first-generation/
  end-of-generation evaluation path (`EvaluateCandidateFull()` in
  `Optimizer.cpp`) adjusts both.
- `cetak_akhir()`/`WriteFinalResults()`'s `.str` section reflects whichever
  population slot's geometry happens to be "frozen" at the time it's
  called (not necessarily the best), while its `.opt`/`.kdl` sections
  explicitly switch to the best (`JSTD-1`) structure.

## Re-optimize from last best (issue #16)

`OptimizationOptions::seed_from_previous_best` + `seed_var_b`/`seed_var_k`
let a caller continue refining an already-decent design instead of always
starting `optimasi()`'s initial population fully random. This is a new
engine capability, not a legacy-fidelity deviation (the original program
had no such option).

- **Where it plugs in:** `AcakVariabel()` (the port of `Pengacakan.hpp`'s
  `acak_variabel()`) normally does `Randomisasi()` (fully random) for
  population slot 0, `CariStrukturAwal()` (all-max-index reference design)
  for slot 1, and `Randomisasi()` for every other slot. When a valid seed
  is supplied, slot 0 is populated by a plain copy (`SeedStrukturAwal()`)
  instead of `Randomisasi()` -- everything else about the initial
  population (slot 1's reference design, slots 2..JSTD-1's randomization)
  is unchanged.
- **Validity check lives in the engine, not just the GUI**: the seed is
  only used if its size exactly matches `12*jum_balok` / `5*jum_kolom` for
  *this* run (computed by `PrepareOptimization()`, which must run before
  `RunOptimization()` per its existing precondition). A mismatch (e.g. the
  dataset's geometry was edited between the seeded run and this one,
  changing which/how many members classify as beams vs. columns) silently
  falls back to a normal random slot 0 -- never a crash or out-of-bounds
  write. The GUI (`RunPanel`) additionally only *offers* the option after
  a real completed run against the dataset currently being run, but the
  engine doesn't trust that alone.
- **Determinism/threading (issue #4) unaffected:** seeding replaces one
  RNG draw (`Randomisasi()` for slot 0) with a plain array copy, entirely
  within the single-threaded population-generation step that runs before
  any worker threads are created. `worker_threads` still only changes
  wall-clock time, never the numeric result, whether or not seeding is
  used.
- **Why the seeded design's cost can't get worse:** the seeded slot 0 is
  evaluated by the same `EvaluateCandidateFull()` every other initial
  slot goes through (including beam/column adaptive stirrup-tightening,
  which can only improve or preserve a design's fitness, never degrade
  it), then the whole population is sorted by fitness. The search loop
  afterward (`GantiBaru`/`Penyusutan`) only ever replaces the worst slot
  with something better or shrinks toward the current best -- it never
  discards a fitter candidate already in the population. So the final
  best (`fitstr[JSTD-1]` after the last `Sort()`) is always >= the
  evaluated fitness of the seeded design, meaning the re-optimized
  result's cost is never worse than the seed's, given equal constraint
  satisfaction (`kendala`) -- this was reasoned through analytically (see
  Validation below for why it wasn't exercised with a real run).

## Validation

No Borland C++ 5.02 toolchain exists to run the original binary
side-by-side in this environment (and its output wouldn't be
bit-reproducible anyway, per point 2 above), so validation targets what
*can* be checked rigorously:

1. **File reading is exact.** `orcisf_cli info <dataset>` against
   `Example/Data01/GEDUNG` reproduces every field in the checked-in
   `GEDUNG.INP` (`M=21 NJ=16 NR=60 NRJ=16`, `E=1.96615e+10`, joint
   coordinates, member topology, discrete-table counts) verbatim.
2. **The stiffness solver is independently correct.** `orcisf_cli
   equilibrium <dataset>` assigns a uniform mid-range section to every
   member, runs a full analysis, and checks that global vertical
   equilibrium holds (sum of support reactions balances applied joint
   loads + beam self-weight) — a physics-based check that doesn't depend
   on reproducing any historical run. Checked against all 5 bundled
   dataset families (`Data01`, `Data02+3`, `Data02x2`, `Data03x2`,
   `Data04+3`, `Apl1-1`, `Apl2-1`, `Apl3-1`): residual is ≤0.22 N against
   totals of ~0.9–1.6 million N (float32 rounding noise, not error).
3. **End-to-end runs are sane and cross-checked against the thesis text.**
   `orcisf_cli optimize` on `Data01` (Harsoyo validation case) reports
   `JVD=168 JSTD=171` — matching `Teori/Isi/BAB IV.doc` exactly ("168
   variabel desain dan 171 struktur desain"). On `Apl1-1` (thesis's
   "Aplikasi Program Optimasi" case), the reported total beam load is
   `36200 N/m` — exactly `35000` (BAB IV's stated `35 kN/m` applied load)
   `+ 1200` (computed self-weight), confirming both the input parsing and
   the self-weight calculation. All 5 tested datasets converge to a
   fully constraint-satisfying design (`kendala=0`) with monotonically
   decreasing cost and complete in well under a second for 30 generations.
4. **Output files are well-formed**, matching the legacy `.opt`/`.kdl`
   layout (per-member dimensions, reinforcement, cost, and constraint
   breakdown) — see `LegacyIO.cpp`.

**Issue #16 (re-optimize from last best) was empirically verified** with a
standalone scratch program (not checked in) linking directly against
`orcisf_engine`, against a scratch copy of `Example/Data01`:
1. A deliberately-truncated baseline run (`j_iterasi_mak=3`, unlikely to
   have converged) finished at `harga=3.02672e+07 kendala=0`.
2. A second run seeded from that best design (`seed_from_previous_best=true`,
   `j_iterasi_mak=50`, a *different* `rng_seed` for the rest of the
   population) finished at `harga=2.46519e+07 kendala=0` -- lower cost,
   confirming both that the seed actually took effect and that continuing
   the search from it improves on the truncated starting point (the
   "cost never gets worse than the seed" argument above, now confirmed
   empirically rather than only analytically).
3. Re-running the seeded case with `worker_threads=4` produced a
   bit-identical result (`fitstr`/`hargastr`/`kendalastr` all equal) to
   the `worker_threads=1` seeded run for the same `rng_seed` -- confirms
   issue #4's determinism guarantee holds with seeding active too, per
   #16's explicit acceptance criterion.
A run with an already-converged baseline (`j_iterasi_mak=50` for both
runs) produced an *identical* result on the second run rather than a
further improvement -- expected, not a bug: there's nothing left to
improve once the baseline already found the same optimum the seeded
search would converge to.

**⚠️ Always point the CLI/GUI at a scratch copy of a dataset, not the
checked-in `Optimasi Beton/Example/` archive** — output filenames are
case-*insensitive* on Windows (`GEDUNG.opt` and `gedung.opt` are the same
file), so running against the real archive folder will silently overwrite
the checked-in reference `.opt`/`.str`/`.kdl`/`.inf`/`.his` files.

### Threading determinism (issue #4)

`OptimizationOptions::worker_threads` controls how many `std::thread`s the
optimizer's two hot loops (`EvaluatePopulationParallel`, `CariBaruParallel`
in `Optimizer.cpp`) fork per call. Numeric output must be identical
regardless of `worker_threads` for a fixed `rng_seed` — more threads only
changes wall-clock time. Verified via `orcisf_cli optimize`'s
`POPULATION_DUMP` section (every population slot's fitness/kendala/harga +
full design-variable vector), diffed between a `worker_threads=1` and a
`worker_threads=8` run against the same scratch dataset copy and the same
`rng_seed`:

```sh
orcisf_cli optimize <scratch-dataset> 250000 5000 50 50 1e10 200 3 1 1 12345 1 > run_1t.out
orcisf_cli optimize <scratch-dataset> 250000 5000 50 50 1e10 200 3 1 8 12345 1 > run_8t.out
diff run_1t.out run_8t.out   # only the "worker_threads=" label line should differ
```

`CariBaruParallel` falls back to the sequential `CariBaru()` below
`kMinTrialsPerWorker` trials per lane (avoids thread-spawn/sync overhead
that isn't worth paying — see `Optimizer.cpp`'s comment above the
constant). On the bundled `Data01` (`M=21`), `TrialCount()` is almost
always 1–6 (rare spikes to ~65 early in a run), so most generations take
that fallback and never actually exercise the parallel merge/reduce code
path. **The diff above alone does not prove the parallel path is
correct** if it only ever hit the fallback — when validating a change to
`CariBaruParallel`/`SyncSearchState`, temporarily force `kMinTrialsPerWorker`
down to `1`, rebuild, and re-run the same diff to confirm the merge logic
itself (not just the fallback) reproduces sequential output exactly, then
revert the constant. This was done for the initial #4 implementation:
bit-identical `POPULATION_DUMP` output with the parallel path forced on
for every generation.

Because `TrialCount()` is small for every dataset bundled in this repo,
wall-clock speedup from `worker_threads>1` is negligible-to-nonexistent
here (thread-spawn overhead roughly cancels out the saved work) — this is
a property of the bundled example structures being modest (M≈21 members),
not a flaw in the threading design. The architecture (private per-worker
`StructureData`, targeted `SyncSearchState` instead of a full struct copy,
minimum-work threshold) is sized for larger structures where a single
candidate's analysis+design cost is large enough to amortize thread
overhead.

## Building / running the CLI

```sh
cd src
cmake --preset windows-release   # or macos-release / linux-release
cmake --build --preset windows-release
./build/windows-release/engine/orcisf_cli info "path/to/Example/Data01/GEDUNG"
./build/windows-release/engine/orcisf_cli equilibrium "path/to/Example/Data01/GEDUNG"
./build/windows-release/engine/orcisf_cli optimize "path/to/scratch-copy/GEDUNG" 250000 5000 50 50 1e10 500 3 1
```
