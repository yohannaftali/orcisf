# Change History

Dated, chronological record of notable changes to this project. Current
architecture and working rules live in [`AGENTS.md`](AGENTS.md) — this file is
history only; don't duplicate current-state description here.

---

## 1998–1999 — Original thesis project

- **1998-07-15** — First line of source code written (per the header comment in
  [`ORCISF.cpp`](Optimasi%20Beton/Source/ORCISF.cpp)): start of the Tugas Akhir
  (undergraduate thesis) "Optimasi Beton Bertulang Pada Struktur Portal Ruang"
  by Yohan Naftali (7712/TS), Program Studi Teknik Sipil, Universitas Atma Jaya
  Yogyakarta, advised by Dr. Ir. FX. Nurwadji Wibowo, M.Sc. and Ir. Ch. Arief
  Sudibyo.
- **1999-03-08** — Thesis seminar/defense held in Yogyakarta ("Diseminarkan Di
  Yogyakarta Pada Tanggal 8 Maret 1999", per `ORCISF.cpp`).
- **1999 (Jun–Jul)** — Bulk of the `.hpp` modules under `Optimasi Beton/Source/`
  written/dated (structural mechanics, RC design, and Flexible Polyhedron
  optimizer modules).
- **1999-08-10** — v0.01 release: `BacaSaya.txt` (user manual) and the compiled
  `orcisf.exe`/`cw3230.dll` binary dated to this day.
- **1999-08-23** — `orcisf.ico` added.

## 2006 — Follow-up

- **2006** — `Optimasi Beton/Optimasi Struktur Beton.doc` created (file
  timestamp 2006-09-10); binary inspection in 2026 found it has no
  recoverable text or embedded objects — its intended content, if any, is
  lost. A recompiled build (`orcisf.exe` + `cw3230.dll`) with its own
  `aplikasi.*` example dataset was archived under
  `Optimasi Beton/Example/2006/`.

## 2026-08-09 — Repository documented and set up for AI-assisted work

- Repository initialized on GitHub (`yohannaftali/orcisf`), with the existing
  `Optimasi Beton/` archive (source, binary, examples, thesis manuscript) as
  the initial commit.
- Added `.claude/skills/planner` and `.claude/skills/coder` (GitHub-issue
  planning/implementation skills) and `CLAUDE.md` pointing every agent at
  `AGENTS.md`.
- Authored `AGENTS.md` (architecture/working-rules reference), this
  `CHANGE_HISTORY.md`, and expanded `README.md`, based on a full read-through
  of the legacy source (`Optimasi Beton/Source/*.hpp`), the original user
  manual (`BacaSaya.txt`), and the repository layout — documenting the
  structural-analysis + "Flexible Polyhedron" optimization architecture, the
  `.inp`/`.isd`/.../`.opt` file-format convention, and the reserved-but-empty
  `src/` directory for any future modernization work.

## 2026-08-09 — LaTeX transcription of the thesis manuscript

- Added `Optimasi Beton/Teori/latex/`: a LaTeX build of the actual thesis
  (`Teori/Isi/BAB I.doc`–`BAB V.doc`, `Intisari.doc`, `Cover/*.doc`,
  `Daftar/*.doc`), requested as "a LaTeX version of `Optimasi Struktur
  Beton.doc`". That specific file turned out to be empty on inspection (see
  the 2006 entry above), so the transcription targets the real thesis
  content instead, per the user's choice when offered that option.
- Text was extracted with `antiword`; formulas embedded as MathType/
  Equation Editor OLE objects (undetectable as plain text) were
  reconstructed by cross-referencing the surrounding prose against the
  actual C++ implementation in `Source/*.hpp` — see
  `Teori/latex/LEGACY_TRANSCRIPTION_NOTES.md` for the full methodology and
  a breakdown of what's verbatim vs. reconstructed vs. unrecoverable
  (figures; Tabel 2-1's stiffness matrix, rebuilt from the cited Weaver &
  Gere reference instead).
- Compiled and verified with Tectonic 0.17.0 (downloaded standalone, no
  system LaTeX install available) — `main.pdf`, 73 pages, no errors.
  `main.pdf` is checked in alongside the `.tex` sources.

## 2026-08-09 — GUI port planned: epic #1 + sub-issues #2–#9 created

- Scoped the modern cross-platform GUI port of ORCISF (requested: 3D
  CAD-style structure/load input with drag-and-drop editing, live progress
  and detailed per-iteration logging, user-selectable core count for the
  optimization run, per-member RC reinforcement drawings, PDF/legacy-text
  export, and rendering any `Example/*` folder in 3D) into GitHub issues
  under `yohannaftali/orcisf`: epic **#1**, sub-issues **#2** (build/stack
  scaffold), **#3** (headless engine port), **#4** (multi-threaded
  optimization core), **#5** (3D viewport + example-folder loader), **#6**
  (interactive 3D editor), **#7** (load-input GUI), **#8** (reinforcement
  detailing drawings), **#9** (PDF/text export).
- Researched the GUI/rendering stack per the user's request (wxWidgets vs.
  Dear ImGui + GLFW) and recommended **Dear ImGui (docking) + GLFW +
  OpenGL3 + ImGuizmo (3D manipulation) + ImPlot (charts) +
  nativefiledialog-extended (native file dialogs) + libharu/HPDF (PDF) +
  CMake/vcpkg** — immediate-mode UI drawn in the same GL context as the 3D
  scene is the standard, actively-maintained pattern for CAD-style
  drag-and-drop 3D editors; wxWidgets would need bespoke 3D-gizmo
  integration for the app's core interaction pattern. Full rationale is in
  issue #1's body.
- No implementation existed yet at this point — `src/` was still empty;
  these issues were the plan, to be picked up (e.g. via the `coder` skill)
  one at a time. (Issue #2 was implemented the same day — see next entry.)

## 2026-08-09 — #2: `src/` CMake + Dear ImGui build scaffold

- Implemented issue #2's acceptance criteria: `src/` now has a working
  GLFW + OpenGL3 + Dear ImGui (docking) window (`app/main.cpp`,
  `app/Application.{h,cpp}`) with a docked Viewport/Properties/Log layout
  and a placeholder menu-bar toolbar (`gui/Toolbar.{h,cpp}`,
  `gui/ViewportPanel.{h,cpp}`, `gui/PropertiesPanel.{h,cpp}`,
  `gui/LogPanel.{h,cpp}` — the log panel is functionally real, the other
  three are inert placeholders for #5/#6/#7).
- Dependencies (`imgui` w/ `docking-experimental`+`glfw-binding`+
  `opengl3-binding`, `glfw3`, `imguizmo`, `implot`,
  `nativefiledialog-extended`, `libharu`) declared in `src/vcpkg.json`
  (manifest mode, pinned `builtin-baseline`) — every `find_package`/target
  name in `src/CMakeLists.txt` was verified against the actual
  microsoft/vcpkg port files (not assumed), since a wrong target name
  would silently break every downstream issue's build.
- `src/CMakePresets.json` defines `{windows,macos,linux}-{debug,release}`
  presets against `$env{VCPKG_ROOT}`; `.github/workflows/build-src.yml`
  matrix-builds all three OSes via `lukka/run-vcpkg` + plain
  `cmake --preset`/`cmake --build --preset` on every push/PR touching
  `src/**`.
- **Local validation — actually built and ran, not just configured:** no
  MSVC was on `PATH` and no system CMake/Ninja/vcpkg existed in this
  environment, so `cmake`+`ninja` were obtained via `pip` and `vcpkg` was
  bootstrapped from source (a local-only `x64-mingw-dynamic` triplet
  override in git-ignored `src/CMakeUserPresets.json`, which does not
  affect the committed `x64-windows`/CI configuration). All 6 dependencies
  built from source against the MinGW UCRT64 toolchain already present
  (~5.3 min); this also confirmed every `find_package`/target name in
  `src/CMakeLists.txt` is correct, since vcpkg prints each dependency's
  actual CMake usage snippet after building it (`implot::implot`,
  `unofficial::libharu::hpdf`, `nfd::nfd` — all matched). One real bug was
  caught and fixed: `Application.cpp` defined
  `IMGUI_DEFINE_MATH_OPERATORS` *after* `#include <imgui.h>` instead of
  before, which `imgui_internal.h` rejects with a `#error`. After the fix,
  `orcisf_gui.exe` built cleanly and ran for 5s without crashing or
  printing any GLFW/OpenGL errors (window + GL context + ImGui + docking
  layout all initialize correctly). CI (Windows/macOS/Linux, MSVC on
  Windows) still separately validates the other two OSes and the
  MSVC/x64-windows configuration.
- `AGENTS.md` updated: `src/` section now documents the actual scaffold
  structure (not just the plan), repository-layout tree, GitHub Workflow
  CI note, and Validation section build instructions.

## 2026-08-09 — #3: headless engine port (structural analysis, RC design, optimizer)

- Ported every legacy analysis/design/optimization file
  (`Struktur.hpp`, `Solver.hpp`, `Pembebanan.hpp`, `ELEMEN.HPP`,
  `Balok.hpp`, `Kolom.hpp`, `Kendala.hpp`, `Polyhedron.hpp`,
  `Pengacakan.hpp`, `Penormalan.hpp`, `Telusur.hpp`, `Baru.hpp`,
  `Pengurutan.hpp`, `Diskritisasi.hpp`, and the read/write halves of
  `InOut.hpp`/`CETAK.HPP`) into `src/engine/` as a headless C++20 static
  library (`orcisf_engine`) with no console/GUI dependency, plus a
  headless validation CLI (`orcisf_cli`, `info`/`equilibrium`/`optimize`
  subcommands). `orcisf_gui` now links `orcisf_engine` (nothing calls into
  it from the GUI yet — that's #4–#9).
- Every legacy global from `Variabel.hpp` became one field on an owned
  `StructureData` context struct passed by reference, keeping the original
  Indonesian identifiers and 1-based array indexing on purpose (for
  line-by-line checkability against the legacy source, per the file's own
  header comment) — satisfies #3's "RAII, no bare globals" criterion
  without changing any numeric formula.
- Two categories of not-obviously-correct legacy behavior were identified
  by close reading and preserved deliberately (documented in
  `CostConstraint.h` and `LegacyIO.h`'s header comments, and summarized in
  `src/engine/README.md`'s "Deliberate deviations" section): (1)
  `Kendala_Harga()` adjusts a candidate column's stirrup spacing/side
  length in place but *not* a beam's, unlike the optimizer's own
  first-generation/end-of-generation evaluation path, which adjusts both;
  (2) `cetak_akhir()`'s `.str` output reflects whichever population slot's
  geometry is "frozen" at call time (not necessarily the best), while its
  `.opt`/`.kdl` sections explicitly use the best (`JSTD-1`) structure. Both
  are reproduced exactly, not "fixed."
- One genuine, justified simplification: `KendalaHarga()` (called
  potentially thousands of times per generation during the search) no
  longer re-reads input files or re-runs the structural analysis on every
  call the way the legacy `Kendala_Harga()` did — those calls were
  idempotent after the first one (same frozen geometry every time within a
  search), so skipping them changes nothing numerically while being far
  faster and safe to eventually parallelize (issue #4).
- Random-number generation (`std::mt19937`, statistically equivalent) is
  **not** bit-reproducible against Borland's `randomize()`/`random()` —
  documented as a fundamental, unavoidable limit (the original wasn't
  reproducible run-to-run either, since it seeds from wall-clock time).
- New detailed per-generation calculation log (`<dataset>.log.txt`,
  superset of `.his`): every generation's full population (index, fitness,
  kendala, harga, and every beam/column design-variable vector), not just
  a one-line summary per shrink/convergence event.
- Progress/cancellation interface: `ProgressCallback` (generation, best
  fitness/harga/kendala, elapsed time, converged flag) polled once per
  generation, plus an `std::atomic<bool>` cancel token checked between
  generations — the GUI can now show live progress without polling files
  (once #4/#5 wire it up).
- **Validation** (no Borland toolchain exists to diff against the original
  binary, and its output isn't reproducible anyway — see above):
  1. `orcisf_cli info` against `Example/Data01/GEDUNG` reproduces every
     field of the checked-in `GEDUNG.INP` exactly.
  2. `orcisf_cli equilibrium` (uniform mid-range section on every member,
     full analysis, check that support reactions balance applied loads +
     self-weight — a physics-based check independent of RNG/history)
     passed on all 8 bundled dataset folders (`Data01`, `Data02+3`,
     `Data02x2`, `Data03x2`, `Data04+3`, `Apl1-1`, `Apl2-1`, `Apl3-1`):
     residual ≤0.22 N against totals of ~0.9–1.6 million N.
  3. `orcisf_cli optimize` end-to-end runs cross-checked against the
     thesis text: `Data01` (the Harsoyo validation case) reports
     `JVD=168 JSTD=171`, matching `Teori/Isi/BAB IV.doc` exactly; `Apl1-1`
     reports total beam load `36200 N/m`, exactly BAB IV's stated
     `35 kN/m` applied load plus computed self-weight. All 5 datasets
     tested converge to a fully constraint-satisfying design (`kendala=0`)
     with monotonically decreasing cost.
  4. **Incident during validation, self-corrected:** the first `optimize`
     run pointed output at the real `Example/Data01/` folder; since
     Windows filenames are case-insensitive, `GEDUNG.opt` collided with
     and overwrote the checked-in `gedung.opt`/`.str`/`.kdl`/`.inf`/`.his`
     reference files. Caught immediately via `git status`, restored with
     `git restore` (no data loss — git had the originals), and all further
     validation was run against scratch copies outside the repo. Flagged
     prominently in `engine/README.md` and `AGENTS.md` so the next agent
     doesn't repeat it.
- Not in scope for #3 (see `src/engine/README.md`): real multi-threaded
  population evaluation (#4 — `OptimizationOptions::worker_threads` exists
  as a documented no-op for now), and wiring any of this into the actual
  GUI (#4–#9).

## 2026-08-09 — #4: multi-threaded optimization core + GUI run/cancel panel

- `Optimizer.cpp`'s two hot loops now fork-join across
  `options.worker_threads - 1` extra `std::thread`s per call, each with its
  own private `StructureData` clone (required for correctness — every
  analysis/design/constraint function mutates shared scratch fields like
  `SFF`/`SM`/`AM` on whichever `StructureData` it's given):
  `EvaluatePopulationParallel` (initial `JSTD`-candidate population, once
  per run) and `CariBaruParallel` (`cari_baru()`'s trial search, every
  generation — the loop that actually dominates wall-clock time). A new
  `RunOnLanes` helper partitions an index range across lanes (lane 0 = the
  calling thread) and returns the per-lane ranges used, so callers can
  merge worker results back precisely.
- **Performance fix found and applied during this work:** the first
  `CariBaruParallel` implementation synced each worker via a full
  `StructureData = StructureData` copy (~14MB) every generation, which
  measured ~3x *slower* than single-threaded on the bundled `Data01`
  dataset (8 threads: 2.6s vs. 1 thread: 0.9s for 2000 generations) —
  per-generation copy cost vastly exceeded the actual trial-search work.
  Fixed with `SyncSearchState()` (copies only the ~10 JVD-sized fields a
  trial actually reads: `TM`/`TS`/`arah`/`no_TS_terjauh`/`var_b_jelek`/
  `var_k_jelek`) plus a `kMinTrialsPerWorker` threshold that falls back to
  the identical sequential `CariBaru()` when there isn't enough work per
  lane to justify thread-spawn overhead. Re-verified after the fix:
  1-thread and 8-thread timings at parity, and bit-identical
  `POPULATION_DUMP` output for the same `rng_seed` (see
  `engine/README.md`'s new Threading determinism section for the exact
  methodology — including re-running the diff with the fallback threshold
  temporarily forced to 1, to prove the parallel merge path itself is
  correct and not just the sequential fallback).
- `TrialCount()` (trials for a given generation) is almost always 1–6 for
  the bundled `Data01` dataset (M=21 members), rarely spiking to ~65 early
  in a run — meaning the bundled example datasets are too small for
  `worker_threads>1` to show meaningful wall-clock speedup here; this is a
  property of the example structures, not a threading-design flaw. The
  architecture is sized for larger structures where per-candidate
  analysis+design cost is large enough to amortize thread overhead.
- New GUI panel `src/gui/RunPanel.{h,cpp}`: dataset path + cost/design
  parameter fields, a worker-thread-count slider (default
  `hardware_concurrency() - 1`, satisfying #4's settings-control
  criterion), Run/Cancel buttons, and a live progress bar (generation,
  best fitness/harga/kendala, elapsed time, converged flag). Runs
  `engine::RunFullOptimization` on a background `std::thread` so the ImGui
  frame loop never blocks; the engine's progress callback (called on that
  worker thread) writes into a mutex-guarded `ProgressInfo` that `Draw()`
  (UI thread) reads back each frame — the only cross-thread state shared.
  Cancel sets an `std::atomic<bool>` checked both by the progress
  callback's return value and by `RunFullOptimization`'s `cancel` pointer.
  Wired into `Application` (new dock alongside Properties) with its log
  sink forwarding to `LogPanel`.
- Not yet addressed (left for a future pass, not blocking #4's core
  acceptance criteria): a dedicated ThreadSanitizer CI run (MinGW doesn't
  support `-fsanitize=thread`; the empirical determinism proof above is
  the substitute for now) and finer-grained mid-dispatch cancellation
  (cancellation is currently checked between generations, same as before
  #4, not mid-`RunOnLanes`).
