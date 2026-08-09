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

**⚠️ Always point the CLI/GUI at a scratch copy of a dataset, not the
checked-in `Optimasi Beton/Example/` archive** — output filenames are
case-*insensitive* on Windows (`GEDUNG.opt` and `gedung.opt` are the same
file), so running against the real archive folder will silently overwrite
the checked-in reference `.opt`/`.str`/`.kdl`/`.inf`/`.his` files.

## Building / running the CLI

```sh
cd src
cmake --preset windows-release   # or macos-release / linux-release
cmake --build --preset windows-release
./build/windows-release/engine/orcisf_cli info "path/to/Example/Data01/GEDUNG"
./build/windows-release/engine/orcisf_cli equilibrium "path/to/Example/Data01/GEDUNG"
./build/windows-release/engine/orcisf_cli optimize "path/to/scratch-copy/GEDUNG" 250000 5000 50 50 1e10 500 3 1
```
