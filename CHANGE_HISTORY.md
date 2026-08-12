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

## 2026-08-10 — #5: 3D viewport + example-folder loader

- New `src/gui/viewport/` subsystem: `Math3D.h` (minimal Vec3/Mat4, no GLM
  dependency), `Camera.{h,cpp}` (orbit camera: target/distance/yaw/pitch,
  mouse-drag orbit/pan, scroll zoom, arrow-key orbit + keyboard zoom),
  `SceneModel.{h,cpp}` (builds a render-ready joints/members/restraints
  snapshot from `engine::StructureData` + an optional `MemberResult` list;
  `PickMember()` does ray-vs-segment picking, pure geometry with no GL
  involved), `SceneRenderer.{h,cpp}` (owns an offscreen FBO + a flat-lit
  shader + a unit-cube mesh reused for both member boxes and joint/
  restraint markers, renders into a texture `ViewportPanel` displays via
  `ImGui::Image`).
- New engine addition `src/engine/{include/engine,src}/MemberResults.h/.cpp`:
  `ComputeMemberResults()` mirrors `WriteFinalResults()`'s per-member
  design-calculation loop (`PeriksaBatang`/`IsiElemenBalokFields`/
  `ElemenLapangan`/`ElemenTumpuan`/`DesignBeam`/`IsiElemenKolomFields`/
  `DesignColumn`) but captures the results (dimensions, demand/capacity,
  per-check `kendala`) into a `std::vector<MemberResult>` instead of
  writing text — feeds the viewport's dimension-scaled/constraint-colored
  member rendering and the Properties panel's per-member data display.
  `orcisf_cli optimize` now also prints a `MEMBER_RESULTS` section using
  it (verified sane output against `Apl1-1`: 4 beams + 4 columns, correct
  dimensions/cost, `kendala=0` for a converged run).
- `ViewportPanel`/`PropertiesPanel` got their first real implementation
  (previously issue #2 placeholders): orbit/pan/zoom camera + click-to-pick
  a member (drag-vs-click distinguished by accumulated mouse-delta
  magnitude, not just mouse-up), Properties panel shows the selected
  member's dimensions and, once an optimization has run, demand/capacity
  per check region + pass/fail coloring. `Toolbar`'s "File > Open
  Folder..." is wired to a native folder dialog (NFD) that scans the
  chosen folder for a case-insensitive `.inp` file to resolve the generic
  dataset path, then calls `engine::LoadDatasetForViewing` (geometry only)
  or, after a `RunPanel` run completes, `Application` calls
  `ComputeMemberResults` on the finished `StructureData` and rebuilds the
  scene with full results.
- Added the `gl3w` vcpkg dependency (core-profile OpenGL function loader,
  initialized once in `main.cpp` after `glfwMakeContextCurrent`) — chosen
  over hand-rolling ~40 function pointers or a heavier loader; every other
  new file includes `<GL/gl3w.h>` per-translation-unit as gl3w requires.
- **Validation, and what could not be automated:** no Borland/legacy
  binary exists to compare 3D rendering against (it never had one), and
  this environment can't script clicks through a native OS folder dialog
  or a live GLFW window reliably (Windows' foreground-focus protections
  block a background process from stealing focus from the terminal/
  browser). What *was* verified: (1) `PickMember()`'s ray-vs-segment math
  against a standalone synthetic-scene test program (not checked in) —
  correct hits/misses for straight and angled rays; (2) the full load →
  build-scene → render pipeline end-to-end via a temporary environment-
  variable-gated debug hook in `Application`'s constructor (removed before
  commit), screenshotted while running: correctly renders `Apl1-1`'s 4
  columns with orange restraint markers at the base joints, in the right
  docked-panel layout; (3) `MEMBER_RESULTS` CLI output sanity-checked
  against `Apl1-1` (see above). Camera drag/orbit/pan and the live
  Open-Folder/Run→viewport-refresh flow are implemented but not
  interactively exercised end-to-end in this environment — reasoned
  through carefully but flagged here for anyone who *can* click through it.
- **Unrelated environment bug found and fixed while testing:** launching
  `orcisf_gui.exe` failed with "Entry Point Not Found: clock_gettime64...
  libstdc++-6.dll" — root cause was `C:\Program Files\PostgreSQL\17\bin\`
  (also on this machine's PATH) shipping its own, ABI-incompatible
  `libwinpthread-1.dll` that could shadow the MinGW toolchain's matching
  copy depending on PATH order, breaking `libstdc++-6.dll`'s own runtime
  dependency resolution. Fixed generally (not just worked around) by
  having `CMakeLists.txt` copy the compiler's own runtime DLLs next to
  `orcisf_gui.exe` post-build on MinGW (see `AGENTS.md`'s `gui/viewport/`
  section) — makes the app directory's copies win regardless of PATH,
  which is also just correct practice for distributing a MinGW-built exe.
  No-op on the MSVC/x64-windows triplet CI uses.

## 2026-08-10 — #6: interactive 3D structure editor

- New `src/gui/editor/` subsystem: `Selection.h` (`SelectionKind`
  {None,Joint,Member} + `EditorOptions` for connect-mode/snap-to-grid,
  shared by ViewportPanel/PropertiesPanel/Toolbar), `EditableStructure.{h,cpp}`
  (add/move/delete joint, add/delete member, a single fixed/free restraint
  toggle per joint, `Validate()`), `UndoStack.{h,cpp}` (geometry-only
  snapshots — `X`/`Y`/`Z`/`JRL`/`JJ`/`JK`/`IA` sized to the current `NJ`/`M`,
  not a full ~14MB `StructureData` copy, the same lesson issue #4 already
  learned about that).
- `DeleteJoint()` cascades (deletes every member touching the joint first)
  and compacts every array so 1-based indices stay contiguous with no
  gaps — the same invariant every other legacy array in this port relies
  on. Verified with a standalone test program (not checked in): a 4-joint
  square, deleting one joint, checking the cascading member deletion +
  index remapping + an undo/redo round-trip all landed correctly.
- `SceneModel` gained `PickJoint()` (ray-vs-sphere, sibling to #5's
  `PickMember()`) so a click in the viewport can select a joint in
  preference to a member.
- `ViewportPanel` gained an ImGuizmo translate gizmo for the selected
  joint (undo-snapshotted exactly once per drag, on the `IsUsing()`
  false→true edge, not every frame) and connect-mode clicking (first
  joint click picks a start point, second click on a different joint adds
  a member and stays in connect mode for chaining). `PropertiesPanel`
  gained numeric X/Y/Z entry + a restrained checkbox for a selected joint,
  Delete Joint/Member buttons, and an always-visible validation-issue
  list. `Toolbar` gained an Edit menu (Undo/Redo, Add Joint, Connect
  Joints, Snap to Grid + grid-size slider). `Application` now owns the
  one canonical editable `StructureData` (`loaded_sd_`) that both the
  Open-Folder load path and the editor operate on; any edit invalidates
  previously-computed `MemberResult` data (member ids can shift on
  delete), so the scene rebuilds geometry-only (no results/colors) after
  the first edit, until a fresh optimization run repopulates them.
- Explicitly out of scope per #6's acceptance criteria (not attempted):
  wiring edited in-memory geometry into `RunPanel`'s "Run" (it still
  always re-reads a dataset from disk by path) — no save/export-to-`.inp`
  exists yet either. Natural follow-up for #7 or a dedicated issue.
- **Validation:** joint/member picking, numeric position entry (confirmed
  live-updating the 3D view and the connected members' geometry), the
  Restrained checkbox, Delete Joint, Add Joint (confirmed it defaults to
  the scene's bounding-sphere center), Undo (confirmed it exactly restored
  prior geometry and cleared selection), and the Validation panel (showed
  "No issues -- OK to run" throughout, matching the unit-tested
  zero-issue case) were all exercised end-to-end via synthesized Win32
  mouse/keyboard input (with `AttachThreadInput` to work around Windows'
  foreground-focus protections blocking a background process from
  stealing focus) and screenshotted to confirm. `EditableStructure`'s
  core logic (add/move/delete cascading + compaction, `Validate()`
  catching coincident joints and zero-length members, undo/redo) was also
  unit-tested standalone before GUI wiring. **Not exercised:** an actual
  click-drag on the ImGuizmo gizmo's translate arrows (sub-pixel
  axis-handle targeting was judged too unreliable to script reliably) —
  the gizmo was confirmed to render at the correct position, and its
  output feeds the identical `MoveJoint()` call the verified numeric-entry
  path already exercises, so remaining risk is isolated to ImGuizmo's own
  hit-testing.

## 2026-08-10 — #7: load (pembebanan) input GUI

- Studied the legacy `.bbn` format precisely (`Pembebanan.hpp`'s
  `load_data()`/`baca_beban()`) before designing anything: it only ever
  supports two load categories -- a uniform distributed load on a member
  (`W`, transverse/gravity-sense, with its 12 fixed-end-force components
  `AML` derived from `W` and the member's length via a fixed formula) and
  a 6-DOF generalized action at a joint (`AJ`, "arah 1..6" =
  Fx,Fy,Fz,Mx,My,Mz -- also how a "point load" or "wind/lateral load" is
  represented, there's no separate type for those in the format). The GUI
  deliberately mirrors exactly these two categories, not more, so
  `WriteLoads()` can always round-trip whatever the GUI creates.
- New engine functions `engine::ReadLoadsRaw()`/`engine::WriteLoads()`
  (`LegacyIO.h/.cpp`): `ReadLoadsRaw()` parses `.bbn` without `ReadLoads()`'s
  `BeratSendiri()` self-weight side effect (tolerates a missing file, for
  a brand-new dataset with no loads yet); `WriteLoads()` writes the exact
  legacy format. Critical invariant documented in both the code and
  `AGENTS.md`: the file never includes self-weight, so `WriteLoads()` must
  only ever be given raw (self-weight-free) `W`/`AJ` -- verified with a
  standalone round-trip test (`ReadDataset`+`ReadLoadsRaw` →
  `WriteLoads` → `ReadLoadsRaw` again) against a real scratch copy of
  `Example/Apl1-1`: every `W`/`AML`/`AJ` value bit-identical, including
  the real 35000 N/m beam loads; also confirmed `ReadLoadsRaw()` tolerates
  a missing `.bbn`.
- `EditableStructure` (#6) extended with `SetMemberLoad()`/`ClearMemberLoad()`
  (recomputes the 12 `AML` fixed-end-forces from the member's *current*
  length, so a load set before a geometry edit stays correct) and
  `SetJointLoad()`/`ClearJointLoad()`; `AddJoint()`/`AddMember()`/
  `DeleteJoint()`/`DeleteMember()` now also initialize/compact `W`/`AML`
  (per member) and `AJ` (per joint) alongside the geometry fields they
  already handled, and `UndoStack`'s `GeometrySnapshot` now captures them
  too -- unit-tested standalone (UDL fixed-end-force formula correctness,
  undo/redo covering loads, load state correctly surviving/compacting
  through a member delete) before GUI wiring.
- `SceneModel` gained `MemberLoadVisual`/`JointLoadVisual` (populated from
  `sd`'s raw `W`/`AJ` -- callers must have loaded via `ReadLoadsRaw()`, not
  `ReadLoads()`, or self-weight would show up as a phantom "user load").
  `SceneRenderer` gained `DrawArrow()`/`DrawLoads()`: fixed-visual-length
  arrow glyphs (magnitudes span orders of magnitude across real datasets,
  so glyph length isn't scaled by them -- the exact number is always in
  the schedule table/Properties), moments shown as a plain marker rather
  than a directional glyph.
- `ViewportPanel` gained load-placement-mode clicking (`EditorOptions::load_mode`):
  clicking a member/joint while in that mode sets a default load (5000 N/m,
  or (0,-10000,0,0,0,0) N downward) which the user then fine-tunes.
  `PropertiesPanel` gained load fields (member `W`; joint `Fx..Mz`) +
  Clear Load. New `src/gui/LoadsPanel.{h,cpp}` (docked panel, issue #7's
  "load-schedule panel (table)" criterion): one row per member/joint with
  a nonzero raw load, inline-editable, row click syncs `Selection`.
  `Toolbar` gained a Loads menu (Select/Add Member Load/Add Joint Load)
  and `File > Save Loads (.bbn)`.
- `Application::OnOpenFolderRequested()` switched from
  `engine::LoadDatasetForViewing()` (which calls `ReadLoads()`) to
  `ReadDataset()`+`ReadLoadsRaw()` directly, so the editable in-memory
  structure always holds raw loads. `OnRunResult()` (a real optimization
  run, which *does* go through self-weight-inflating `ReadLoads()`)
  explicitly zeroes `W`/`AML`/`AJ` before treating that run's
  `StructureData` as editable -- there's no cheap way to separate a run's
  self-weight-inflated joint actions back into "raw user load" per joint
  (a joint's self-weight contribution can come from multiple columns
  sharing it), so post-run load editing starts from a clean slate rather
  than risk showing the user numbers they never entered.
- **Validation:** the full workflow was exercised end-to-end in this
  environment against a real scratch copy of `Example/Apl1-1`
  (screenshotted at each step, via the same synthesized-Win32-input
  approach as #6): existing `.bbn` loads (35000 N/m × 4 beams) loaded,
  rendered (visible arrow glyphs), and matched `LoadsPanel`'s table
  exactly; placing a new joint load via Loads > Add Joint Load + a
  viewport click updated Properties and `LoadsPanel` in sync; **File >
  Save Loads (.bbn) was actually clicked, and the resulting file was read
  back from disk and diffed against expectations** -- it contained both
  the original 4 member loads and the newly-placed joint load, exactly
  matching what the GUI showed (a real end-to-end read → edit → write →
  verify-on-disk round trip, not just the standalone engine-level test
  above). One scripting mishap during this process is worth recording:
  a mis-clicked menu coordinate opened the native "Open Folder" dialog
  instead of "Save Loads" — caught immediately (window title changed to
  "Select Folder"), dismissed with Escape, no data lost, re-verified the
  correct menu item's position via a screenshot before retrying.

## 2026-08-10 — #8: reinforcement detailing drawings per beam/column

- Extended `engine::MemberResult` (issue #5's addition) with reinforcement
  fields -- beam: `lap_dia_tarik`/`lap_n_tarik`/`lap_dia_tekan`/`lap_n_tekan`
  and the `tum_*` equivalents, plus `stirrup_dia`/`stirrup_spacing`/
  `cover_mm`; column: `col_dia`/`col_n_dia`/`col_bar_spacing`/`stirrup_*`/
  `cover_mm`. No new engine computation was needed -- `IsiElemenBalokFields()`/
  `IsiElemenKolomFields()` (already called by `ComputeMemberResults()`)
  were already setting these legacy fields as a side effect of computing
  dimensions; #8 just captures them too. `orcisf_cli optimize`'s
  `MEMBER_RESULTS` section now also prints them, sanity-checked against
  `Example/Apl1-1`: realistic bar counts/diameters/stirrup spacing for
  every beam and column (e.g. a 500×500 column: `12D25`, 100mm spacing,
  D10-300 stirrups).
- New `src/gui/detailing/DetailingLayout.{h,cpp}`: pure geometry (zero
  ImGui/OpenGL dependency), builds a `DetailingDrawing` (concrete outline,
  `RebarCircle` bar positions, `DimensionLabel` text, local mm coordinates)
  from a `MemberResult` -- deliberately separated from rendering so issue
  #9's PDF export can reuse the exact same layout computation with a
  different renderer (HPDF instead of ImDrawList) instead of recomputing
  bar positions. Beam tension bars are placed on the correct physical face
  per region: lapangan (midspan, sagging/positive moment) tension at the
  bottom, tumpuan (support, hogging/negative moment) tension at the top --
  the standard RC convention, and exactly why the legacy format tracks two
  independent reinforcement sets per beam. Column bars are spread evenly
  around all four sides given only `N_DIA` (bars per side), producing
  `4*N_DIA-4` unique bars (corners shared between adjacent sides),
  matching `WriteFinalResults()`'s display formula exactly. Both were
  unit-tested standalone (bar-in-bounds, correct tension/compression face
  per region, correct column bar count for a known `N_DIA`) before GUI wiring.
- New `src/gui/DetailingPanel.{h,cpp}`: docked "Detailing" tab (alongside
  Viewport) rendering whichever member is selected via `ImDrawList`
  (concrete rect, stirrup rect, bar circles color-coded tension/
  compression, text labels for B/H/cover/stirrup/bar callouts). Beams show
  two side-by-side sections (Tumpuan first, then Lapangan, matching
  `WriteFinalResults()`'s text-output order); columns show one. View-only,
  no editing capability (not called for by #8's acceptance criteria).
- **Validation:** the full rendering pipeline was verified interactively
  in this environment against a real completed optimization run of
  `Example/Apl1-1` (screenshotted): a column's drawing (12D25 bars in the
  correct evenly-spaced arrangement, correct spacing/cover/stirrup labels)
  and a beam's drawing (both Tumpuan and Lapangan sections, correct
  tension-face flip between them, all labels) were confirmed to exactly
  match that same run's `orcisf_cli`/`MEMBER_RESULTS` numeric output --
  not just internally consistent, but numerically identical to the
  independently-computed CLI values. One cosmetic issue was caught and
  fixed during this pass: a Unicode filled-circle glyph used for the
  tension/compression color legend rendered as "?" (not in ImGui's
  default font's glyph range) -- replaced with a small `ImDrawList`-drawn
  colored square instead, which doesn't depend on font glyph coverage.

## 2026-08-10 — #9: PDF + legacy-text export of results (epic #1 complete)

- New `engine::WriteStructureFile()`/`WriteDiscreteTables()` (`LegacyIO.h/.cpp`):
  mirror `ReadStructureFile()`/`ReadDiscreteTables()` exactly in reverse,
  completing the round-trip started by issue #7's `WriteLoads()`.
  `WriteStructureFile()` recomputes `NRJ`/`NR`/`ND`/`N` from the current
  `sd.JRL` (a joint counts as restrained if any of its 6 DOF flags is 1)
  rather than trusting `sd`'s own fields, since the #6/#7 editor doesn't
  maintain those derived counts -- a dataset built or edited entirely in
  the GUI still exports a self-consistent `.inp`. Verified with a
  standalone round-trip test against a real dataset: every field
  (including the recomputed `NRJ`/`NR`/`ND`/`N`) came back bit-identical
  after a write + re-read.
- New `src/report/` module (namespace `orcisf::report` -- not `export`,
  a reserved C++ word since C++98): `TextExport.{h,cpp}` writes the full
  legacy file set (`.inp`/`.isd`/`.idl`/`.ijl`/`.ids`/`.ijs`/`.bbn` always;
  `.opt`/`.str`/`.kdl`/`.inf` + copies of `.his`/`.log.txt` from the run's
  original location if a completed, not-since-edited run exists --
  per-generation history can't be regenerated from the final
  `StructureData` alone). `PdfExport.{h,cpp}` builds a multi-page HPDF
  report (cover/input summary, a per-member dimensions/cost/constraint
  table, and one landscape detailing page per member) by calling
  `gui::BuildDetailingDrawing()` (issue #8) directly and drawing the
  identical layout with HPDF calls instead of `ImDrawList` calls --
  exactly the reuse issue #8's layout/render split was built for. HPDF's
  C callback-based error handling uses the standard `setjmp`/`longjmp`
  pattern (documented as a known, accepted tradeoff, not something to
  "fix").
- `Application` now tracks `has_run_results_`/`current_results_`: true
  only when `loaded_sd_` is exactly what a completed run produced (no
  edit since -- `var_b`/`var_k`/`fitstr` etc. still valid, matching
  `WriteFinalResults()`'s own precondition), gating "Export PDF..." and
  the `.opt`/`.str`/`.kdl`/`.inf`/`.his`/`.log.txt` part of "Export
  Text...". `Toolbar` gained "Export PDF..."/"Export Text..." (File
  menu), wired to real native NFD dialogs (Save dialog with a `.pdf`
  filter; folder picker, reusing the loaded dataset's own base filename
  in the chosen destination).
- **Validation:** `WriteStructureFile()`/`WriteDiscreteTables()`'s
  round-trip and `WritePdfReport()`/`WriteTextExport()` were first
  exercised standalone against a real completed run of `Example/Apl1-1` --
  the generated PDF was opened and screenshotted page-by-page (cover,
  a per-member summary table exactly matching `MEMBER_RESULTS`, and a
  detailing page with correct Tumpuan/Lapangan sections matching issue
  #8's ImGui rendering exactly); text export produced all 12 files
  (11 legacy extensions + `.log.txt`) at plausible sizes. **The full GUI
  path was then separately verified through the actual native dialogs**
  (not simulated): clicking "Export PDF..." opened a real Chrome "Save
  As" dialog pre-filled with `report.pdf` and a `*.pdf` filter, producing
  a file byte-identical in size to the standalone test; clicking "Export
  Text..." opened a real "Select Folder" dialog (navigated via its
  address bar) and produced the same 12-file set. The round-trip
  criterion was checked by re-reading the exported dataset with
  `orcisf_cli info`/`equilibrium`: identical geometry/topology to the
  original, and a perfect (0.0) equilibrium residual. One unrelated
  mishap during this pass: a screenshot command with an uncorrected
  window-relative click offset accidentally captured an unrelated
  desktop window; noticed immediately, nothing in that window was acted
  on or described, and the correct window was re-targeted for the retry.
- **This closes epic #1** (issues #2–#9). See `AGENTS.md`'s "Epic #1
  complete" note for what that does and doesn't claim -- acceptance
  criteria met and verified where this environment allows, not a
  feature-complete replacement of the legacy program; known simplifications
  (per-DOF restraint editing, arrowed CAD dimension lines, no direct
  edited-geometry-to-Run path without a save/reload) are documented in
  each issue's section of `AGENTS.md` rather than repeated here.

## 2026-08-10 — CI build-src.yml failing on Linux and Windows

- Issue #10 created on GitHub: "fix(src): CI build-src.yml failing on
  Linux and Windows targets".
- Scope: `.github/workflows/build-src.yml`
- Labels: bug, infra
- A notification claimed CI "only works for Windows, fails for Linux and
  macOS" -- checked the actual latest run
  (https://github.com/yohannaftali/orcisf/actions/runs/31344377690,
  commit a0796f3) before filing, which showed the opposite pattern for two
  of the three targets: `macos-latest` passed, while `ubuntu-latest`
  failed at the Configure step and `windows-latest` failed at the Build
  step. Detailed logs weren't accessible (403, needs admin/write access)
  so root cause isn't identified yet -- filed with job/step-level status
  only.
- **Root causes found and fixed, issue closed.** Pulled full job logs via
  the GitHub API (needed a repo token; the earlier 403 was because the
  first attempt used no auth) and identified two independent problems:
  - **Linux (Configure):** vcpkg's `pthread-stubs` port (a transitive
    dependency, pulled in via GLFW's X11 deps) needs `autoconf`/
    `autoconf-archive`/`automake`/`libtool` from the system package
    manager to build via autotools; `build-src.yml`'s "Install Linux
    GL/X11 dev packages" step didn't install them. Fixed in one commit,
    which then surfaced a second autotools port needing the same
    treatment one level further down the dependency graph: `libxcrypt`
    additionally needs `libltdl-dev`. Both are now installed in that step.
  - **Windows (Build/link):** every `.cpp` compiled, but linking
    `orcisf_gui.exe` failed with dozens of `undefined reference to
    ImGui::...` errors. Root cause: the Windows job never set up an MSVC
    environment, so CMake's Ninja generator auto-detected
    `C:\mingw64\bin\c++.exe` (present on the `windows-latest` runner) as
    the compiler instead of `cl.exe` -- linking MinGW/Itanium-ABI object
    files against the `x64-windows` vcpkg triplet's MSVC-ABI import
    libraries. `CMakeLists.txt` already assumed MSVC/x64-windows in CI
    (see its `MINGW`-guarded post-build DLL-copy comment) but nothing in
    the workflow actually enforced it. Fixed by adding
    `ilammy/msvc-dev-cmd` and forcing `CC=CXX=cl` before configuring.
  - Windows was independently verified with a full local build on this
    machine (VS Build Tools 2022's bundled `cl`/`cmake`/`ninja`, vcpkg
    checked out at the same pinned commit CI uses) -- both `orcisf_gui.exe`
    and `orcisf_cli.exe` linked and ran. Local Linux verification via WSL
    was attempted but abandoned (`sudo` required an interactive password
    with no TTY available); the Linux fix was verified via CI directly
    instead, across two iterations (pthread-stubs, then libxcrypt).
  - Final CI run (https://github.com/yohannaftali/orcisf/actions/runs/31375100535,
    commit 6aff4b4): all three matrix targets (`windows-latest`,
    `ubuntu-latest`, `macos-latest`) passed. Issue #10 closed.

## 2026-08-10 — File > New Data action + Open Folder... rename

- Issue #11 created on GitHub: "feat(src): add File > New Data action,
  rename Open Folder... to Open Data...".
- Scope: `src/gui/Toolbar.{h,cpp}`, `src/app/Application.{h,cpp}`
- Labels: enhancement
- Currently `File` only has "Open Folder..." (loads an existing dataset);
  there's no in-app way to start a blank dataset from scratch. #11 adds a
  `New Data` action and relabels the existing item to `Open Data...` so
  the two read as a create/load pair.
- **#11 expanded (same day):** clarified that point/member/restraint
  entry mostly already exists (issue #6's `gui/editor/`) and just needs
  to work starting from a blank `New Data` dataset -- not new machinery.
  Added a genuinely new requirement: a `.inf`-equivalent preview/export
  (`Koordinat Titik Kumpul (m)` / `Informasi Batang` / `Pengekang Titik
  Kumpul`, English aliases acceptable) generated directly from the
  in-GUI-edited dataset at any time, since today `.inf` is only ever
  produced by `engine::WriteFinalResults()` as a byproduct of a
  completed run (#9). Units stay meters-only per the legacy format;
  cm/mm support explicitly deferred to a future issue, not attempted
  here.

## 2026-08-10 — orcisf_gui.exe opens an extra console window on Windows

- Issue #12 created on GitHub: "fix(src): orcisf_gui.exe opens an extra
  console window on Windows".
- Scope: `src/CMakeLists.txt`
- Labels: bug
- User-reported: launching `orcisf_gui.exe` also opens a terminal window
  that can't be closed independently -- closing it kills the GUI too.
  Root cause identified by inspection (not yet locally reproduced on
  this machine): `add_executable(orcisf_gui ...)` in `src/CMakeLists.txt`
  doesn't pass `WIN32`, so it links `/SUBSYSTEM:CONSOLE` on MSVC/Windows
  (the toolchain #10 fixed CI to actually use), allocating a console at
  startup. `orcisf_cli`'s own target is unaffected and should stay a
  console app.
- **Investigated whether this also affects macOS/Linux (comment on
  #12):** no. `WIN32` is a Windows-only CMake keyword (silently ignored
  by non-Windows generators), and `src/CMakeLists.txt` has no
  `MACOSX_BUNDLE` set either, so macOS/Linux builds are already plain
  terminal-launched binaries with no packaging step -- out of scope for
  #12, would need a separate future issue for `.app`/`.desktop`
  packaging if ever wanted.
- **Fixed:** added `WIN32` to `add_executable(orcisf_gui ...)` in
  `src/CMakeLists.txt`, switching MSVC/MinGW to `/SUBSYSTEM:WINDOWS`.
  This alone broke the MSVC build (`WIN32` subsystem's default CRT
  startup expects `WinMain`, but `main.cpp` intentionally keeps a
  portable plain `main()`) with `LNK2019: unresolved external symbol
  WinMain`; fixed by adding `target_link_options(orcisf_gui PRIVATE
  /ENTRY:mainCRTStartup)` under the existing `if(MSVC)` block, telling
  the linker to keep using the C entry point (MinGW's CRT does this
  automatically with `WIN32` + a plain `main()`, no equivalent flag
  needed there). Also addressed the issue's "don't silently lose
  startup diagnostics" acceptance criterion: `main.cpp`'s two fatal
  startup-error `fprintf(stderr, ...)` call sites (GLFW init failure,
  gl3w init failure) now go through a small `LogStartupError()` helper
  that writes to both `stderr` (harmless no-op without a console) and
  an `orcisf_gui_startup.log` file next to the executable.
  - Verified on this machine with a real MSVC build (VS Build Tools
    2022, vcpkg at the pinned commit): confirmed via
    `Get-CimInstance Win32_Process` that the running `orcisf_gui.exe`
    has no `conhost.exe` child process (no console), and that force-
    closing the app's process leaves nothing else running (matching
    the "closing the window fully exits" criterion -- couldn't test
    the exact "click the window's own close button" interaction in
    this environment, but the process-exit behavior is the same
    either way since there's only one process).
  - Files: `src/CMakeLists.txt`, `src/app/main.cpp`

## 2026-08-11 — #11 expanded: title/material form + auto-calculated params

- **#11 expanded again:** added GUI form-input acceptance criteria for
  dataset-level scalar properties -- all fields already exist on
  `engine::StructureData` (verified against `Optimasi Beton/Source/
  Variabel.hpp`/`CETAK.HPP`), this is purely a missing GUI form, no new
  engine fields needed:
  - `ISN` (Title / "Informasi nama struktur")
  - `E`/`G` (Young's Modulus / Shear Modulus, N/m^2, "Modulus
    Elastisitas aksial" / "Modulus Geser")
  - `FC`/`FY`/`FYS` (f'c / fy / fyt, MPa -- note legacy identifier for
    stirrup strength is `FYS`, not `FYT`)
  - Read-only auto-calculated: `M`/`N`/`NJ`/`NR`/`NRJ` (members/DOF/
    joints/support restraints/restrained joints), explicitly pointed at
    reusing `engine::WriteStructureFile()`'s existing `NRJ`/`NR`/`ND`/`N`
    re-derivation from `sd.JRL` rather than recomputing it a second way
  - Pinned the exact `.inf` section order/labels from `CETAK.HPP` lines
    26-83 (title -> Parameter Struktur -> Properti Elemen Material ->
    coordinates -> members -> restraints) as the target format for #11's
    `.inf`-preview acceptance criterion.

## 2026-08-11 — fix(src): #11 implemented (New Data, title/material form, .inf preview)

- Implemented against issue #11's expanded acceptance criteria:
  - `File > New Data` (new `Application::OnNewDataRequested()`, reuses
    the existing blank-`StructureData` path `OnAddJointRequested()`
    already had); `Open Folder...` relabeled `Open Data...`.
  - `PropertiesPanel`'s new "General" section (shown when nothing is
    selected): direct-edit form for `ISN`/`E`/`G`/`FC`/`FY`/`FYS`
    (all pre-existing `StructureData` fields, no engine changes needed)
    via a new `EditableStructure::Sd()` mutable accessor, plus read-only
    auto-calculated `M`/DOF/`NJ`/`NR`/`NRJ`.
  - New `engine::ComputeRestraintSummary()` (`LegacyIO.h`/`.cpp`):
    extracted `WriteStructureFile()`'s NRJ/NR/ND/N-from-JRL logic into a
    shared function (also returns the restrained-joint list, replacing
    `sd.T_K`), now used by `WriteStructureFile()`, the new
    `WriteInfPreview()`, and the Properties panel's read-only fields.
  - New `engine::WriteInfPreview()`: writes the six .inf "input echo"
    sections (title/params/material/coordinates/members/restraints, per
    `CETAK.HPP` lines 26-83) from the live in-memory dataset, independent
    of a completed run -- wired to a new `File > Export INF Preview...`
    menu item (NFD save dialog).
  - Point/member/restraint editing itself needed no new code -- issue #6
    already covers it and works the same whether the dataset came from
    `Open Data...` or `New Data`.
- **Verified:** full GUI build succeeds (MSVC/vcpkg, same toolchain as
  #10/#12); app launches and runs without crashing with the new code.
  `WriteInfPreview()` was verified standalone against a real dataset
  (`Example/Apl1-1`): read it, wrote a fresh `.inf` to a scratch path,
  diffed all six sections against the checked-in `aplikasi.inf` --
  numerically identical (only whitespace/column-width formatting
  differs). **Not verified:** interactively clicking the new menu items
  or typing into the new General-section fields in the live GUI --
  reasoned through and engine-level tested instead, not exercised
  end-to-end with synthesized input the way issue #6/#7 were.
- Files: `src/engine/include/engine/LegacyIO.h`,
  `src/engine/src/LegacyIO.cpp`, `src/gui/editor/EditableStructure.h`,
  `src/gui/PropertiesPanel.cpp`, `src/gui/Toolbar.h`, `src/gui/Toolbar.cpp`,
  `src/app/Application.h`, `src/app/Application.cpp`

## 2026-08-11 — feat(src): New Data asks for a save location; add Save/Save As...

- Follow-up to #11 (still open, no separate issue filed -- this extends
  the same `File` menu work): `New Data` now prompts for a folder + base
  filename via NFD (`Application::PromptForGenericPath()`, reusing the
  `NFD_SaveDialogU8_With` pattern `Export PDF...`/`Export INF
  Preview...` already used) and immediately writes the initial file set
  there via `report::WriteTextExport()`, instead of leaving a blank
  in-memory structure with no path until the user happened to hit Save.
  Cancelling the dialog leaves whatever was previously loaded untouched.
- Added `File > Save` (writes the full legacy input set --
  `.inp`/`.isd`/`.idl`/`.ijl`/`.ids`/`.ijs`/`.bbn`, plus
  `.opt`/`.str`/`.kdl`/`.inf` if a completed run exists -- back to the
  current path; falls back to Save As if there isn't one yet) and
  `File > Save As...` (always prompts for a new location, updates the
  tracked path). Both are thin wrappers around the already-existing
  `report::WriteTextExport()` (issue #9) -- no new engine/export code
  needed, this was purely a missing menu entry point plus the New Data
  prompt.
- **Verified:** full GUI build succeeds; app launches/runs without
  crashing with the new menu items and callbacks wired in.
  `report::WriteTextExport()` itself was already verified in #9's own
  pass (see that dated entry). **Not verified:** clicking through the
  actual New Data/Save/Save As... dialogs interactively in this
  environment.
- Files: `src/app/Application.h`, `src/app/Application.cpp`,
  `src/gui/Toolbar.h`, `src/gui/Toolbar.cpp`
- Committed as `6ca2d51` with "Closes #11" in the message -- GitHub
  auto-closed #11 on push to `main` as a side effect (not a separate
  explicit close action). Worth noting since the interactive-GUI
  verification gap above was flagged at the same time; reopen if that
  gap turns out to matter.

## 2026-08-11 — Interactive verification of the New Data flow; Ctrl+S fix

- Closed the "not verified interactively" gap flagged in the two entries
  above: drove `orcisf_gui.exe` with synthesized mouse/keyboard input
  (Win32 `SetCursorPos`/`mouse_event`, DPI-aware -- `SetProcessDPIAware()`
  was required for `SetCursorPos` coordinates to match actual screen
  pixels, a per-monitor-DPI machine otherwise silently misplaces every
  click) and screenshots, end to end:
  - `File` menu opens with all items correctly labeled and enabled/
    disabled as coded (`Save`/`Save As...`/exports greyed out with
    nothing loaded).
  - `New Data` opened the real native Save-As dialog (pre-filled
    `struktur.inp`, `ORCISF Dataset (*.inp)` filter); typed a scratch
    path, saved -- app logged "Started a new structure at: ..." and all
    7 legacy files actually appeared on disk.
  - The new `PropertiesPanel` "General" section rendered correctly
    (Title/E/G/f'c/fy/fyt + auto-calculated Structural Parameters, all
    zero for the blank structure).
  - Typed into the Title field, tabbed out, clicked `File > Save` --
    confirmed the edit round-tripped to the `.inp` file on disk.
- **Bug found during this pass and fixed:** `Ctrl+S` was display-only.
  `ImGui::MenuItem`'s shortcut-text parameter is cosmetic -- it does not
  bind the key chord, and no separate keyboard handler existed. Fixed by
  adding `ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)` in
  `Application::OnFrame()`, gated on the same `can_export_text` condition
  the menu item itself uses. Re-verified with the actual key chord (not
  a menu click): typed a new title, pressed Ctrl+S, confirmed the `.inp`
  file changed and the log line read "Saved: ...".
- **Environment hazard hit mid-pass (not this project's bug):** a
  separate, live browser-automation session repeatedly stole foreground
  focus from `orcisf_gui.exe`, and on one retry a `SetForegroundWindow`
  call silently failed, sending a `Ctrl+A` + typed path + Enter into
  VS Code's file explorer instead (it opened several `Example/Data04+3`
  files as tabs -- no edits, `git status` confirmed no repo files were
  touched). Recovered by re-verifying `GetForegroundWindow() == target`
  before every input sequence from that point on, and by getting exact
  click coordinates via `GetClientRect`/`ClientToScreen` instead of
  eyeballing screenshot pixel positions (which was itself unreliable --
  small cropped screenshots appear to get silently resized for display,
  breaking manual pixel math; the fix was cropping at 2x with
  nearest-neighbor scaling and reading text directly off the crop rather
  than computing from perceived proportions).
- Files: `src/app/Application.cpp` (Ctrl+S fix only -- the verification
  itself touched no code)

## 2026-08-11 — Epic #13: more user-friendly GUI

- Issue #13 (epic) created on GitHub, split into 5 sub-issues per this
  repo's existing epic #1 pattern:
  - #14 configurable icon toolbar below the menu bar (Office/Adobe/
    AutoCAD-style)
  - #15 Default/Design/Optimization view-layout presets (extends
    `Application::BuildDockspace()`, currently one fixed layout)
  - #16 re-optimize using the last best result -- flagged explicitly as
    *not* a pure GUI change: the ported `optimasi()` driver always
    randomizes its full initial population, so this needs an
    engine-level way to seed a population slot from a previous best
    design, not just new RunPanel UI
  - #17 "Regenerate Seed" button next to `RunPanel`'s existing
    "RNG seed (0 = random)" field
  - #18 AutoCAD-style step-by-step guidance while adding joints/members/
    loads (extends the existing single-line `TextColored` hints in
    `Toolbar::Draw()` for connect-mode/load-placement-mode)
- Scope: `src/gui/`, `src/app/Application.cpp`, `src/engine/` (#16 only)
- Labels: enhancement

## 2026-08-11 — feat(src): #14 implemented (icon toolbar)

- New `src/gui/IconToolbar.{h,cpp}`: an icon-button row docked below the
  main menu bar with New Data/Open Data/Save/Undo/Redo/Add Joint/Connect
  Joints/Run, each reusing the exact callback the equivalent `File`/`Edit`
  menu item already calls (no duplicated logic). Delivered as a fixed
  curated set rather than full drag-to-reorder customization, per #14's
  acceptance criteria allowing that scope-down.
- Icons are hand-drawn `ImDrawList` primitives (lines/shapes), not an
  icon font -- avoids a `vcpkg.json`/dependency change, matches this
  project's existing preference for small hand-rolled GUI code.
- `Application::OnFrame()` manually shrinks
  `ImGui::GetMainViewport()->WorkPos`/`WorkSize` by
  `IconToolbar::kHeight` after drawing the toolbar (a plain window, not
  `BeginMainMenuBar()`, so it doesn't auto-reserve dockspace room the
  way the menu bar does) so panels don't render underneath it.
- `RunPanel` gained public `CanRun()`/`TriggerRun()` so the toolbar's Run
  button can start a run through the exact same gated `StartRun()` path
  the panel's own button uses, without exposing `dataset_path_` or
  duplicating the run-start logic.
- **Verified interactively** in this environment (synthesized Win32
  input + screenshots): all 8 icons render with correct enabled/disabled
  dimming; hover tooltips work; clicking "Add Joint" started a blank
  structure and added joint 1 (Properties panel updated correctly,
  Save/Undo icons became enabled immediately after, Redo/Run correctly
  stayed disabled).
- Files: `src/gui/IconToolbar.h` (new), `src/gui/IconToolbar.cpp` (new),
  `src/gui/RunPanel.h`, `src/gui/RunPanel.cpp`, `src/app/Application.h`,
  `src/app/Application.cpp`, `src/CMakeLists.txt`

## 2026-08-11 — feat(src): #15 implemented (view-layout presets)

- `Toolbar`'s View menu (previously disabled placeholder items) now has
  working `Default`/`Design`/`Optimization` entries, checkmarking the
  active one and firing a new `SetOnViewLayout(ViewLayoutPreset)`
  callback.
- `Application::BuildDockspace()` now rebuilds the dock layout whenever
  the active preset changes (previously ran once at startup only, via
  `dockspace_initialized_` alone). Each preset is its own
  `BuildDefaultLayout()`/`BuildDesignLayout()`/`BuildOptimizationLayout()`
  free function:
  - Default: unchanged from the original single fixed layout (Viewport/
    Detailing center, Properties+Run Optimization tabbed right,
    Loads+Log tabbed bottom) -- no regression, per the issue's explicit
    requirement.
  - Design: Viewport/Detailing stay large; Properties gets its own
    dedicated column (no longer sharing with Run Optimization); Loads
    gets a wider bottom strip; Run Optimization+Log move to a smaller
    tabbed corner (still reachable, not hidden).
  - Optimization: Run Optimization+Log get the large primary area;
    Viewport/Detailing and Properties/Loads move to tabbed pairs in a
    secondary column.
- No panel is ever fully hidden by any preset -- all six panels are
  docked somewhere in every layout, just resized/re-tabbed.
- **Verified interactively** in this environment: all three presets
  render with the exact arrangement described above; the View menu's
  checkmark correctly tracks the active preset; switching Design ->
  Optimization -> back to Default restores the original layout exactly.
  Also confirmed switching presets doesn't touch loaded dataset/
  selection/undo state (added a joint, cycled through all three
  presets, joint/selection/undo history all persisted).
- Files: `src/gui/Toolbar.h`, `src/gui/Toolbar.cpp`,
  `src/app/Application.h`, `src/app/Application.cpp`

## 2026-08-11 — feat(src): #17 implemented (Regenerate Seed button)

- Added a "Regenerate Seed" button next to `RunPanel`'s existing "RNG
  seed (0 = random)" field, generating a new value via
  `std::random_device`.
- **Bug found and fixed during interactive verification:** the field
  round-trips `rng_seed_` (`unsigned int`) through `ImGui::InputInt`
  (signed `int`). A raw `std::random_device` draw can exceed `INT_MAX`,
  which displayed as a negative number -- and the field's own commit
  path (`std::max(0, rng_seed_i)`) would have silently clamped that
  negative display back to `0` (the "always random" sentinel) on the
  next edit. Fixed by generating with
  `std::uniform_int_distribution<int>(1, INT_MAX)` instead, so the
  value is always positive and displays correctly.
- **Verified interactively** in this environment: clicked the button
  twice in the running app, confirmed two different valid positive
  seed values appeared in the field each time (the first attempt, before
  the fix, showed `-1306594150` -- caught by actually reading the
  field, not just by code review).
- Files: `src/gui/RunPanel.cpp`

## 2026-08-11 — Custom borderless window chrome + modern theme

- Issue #19 created on GitHub: "feat(src): custom borderless window
  chrome + modern ImGui theme (cross-platform)".
- Scope: `src/app/main.cpp`, new `CustomTitleBar`/`Theme` files
- Labels: enhancement
- User request (via /planner) was an elaborate, code-generation-style
  spec for removing OS title bars on Windows/macOS/Linux and replacing
  them with a custom ImGui title bar + drag/resize + a premium dark
  theme. Filed as a tracked issue rather than generating code directly,
  per this project's convention. Flagged prominently in the issue body:
  this app uses **GLFW**, which doesn't expose native window-proc/
  delegate hooks by default -- correctly implementing native drag/
  resize/snap on each platform requires pulling raw native handles
  (`glfwGetWin32Window()`/`glfwGetCocoaWindow()`/`glfwGetX11Window()`)
  and subclassing/hooking outside GLFW's own supported API, which GLFW's
  own docs caution against doing carelessly. Wayland support on Linux is
  explicitly called out as needing a feasibility spike first (Wayland
  compositors don't allow client-side window positioning the way X11/
  Win32/Cocoa do) -- may need to scope down to "X11 only" for Linux.
- **#19 revised (same day) to a platform-agnostic-first rollout:** split
  into Phase 0 (ships identically on all three platforms using only
  GLFW's own cross-platform API -- `GLFW_DECORATED=false`, the ImGui
  title bar, `ApplyModernTheme()`, and a basic `glfwSetWindowPos`-based
  drag that works everywhere but isn't OS-native) and Phase 1+ (each
  platform's native drag/resize/snap integration, Windows first, shipped
  as independent non-blocking follow-up work; Wayland stays on Phase 0's
  fallback indefinitely if the feasibility spike finds it infeasible).
  Rationale: get the modern *look* live on every platform immediately
  instead of blocking the whole issue on native per-platform systems
  work landing everywhere first.
- **#19 corrected (same day): Phase 0 is not equally functional on all
  three platforms, only equally visual.** Phase 0's drag mechanism
  (`glfwSetWindowPos` per-frame) is a no-op on Wayland -- Wayland
  compositors don't allow client-side window positioning at all, unlike
  X11/Win32/Cocoa. Added a requirement to detect the Wayland backend at
  runtime (`glfwGetPlatform() == GLFW_PLATFORM_WAYLAND`) and surface the
  limitation instead of shipping a window that silently can't be moved,
  and tightened the "verified" acceptance criteria to require actually
  confirming drag works (or is a documented no-op on Wayland) rather
  than just confirming the code compiles/runs per platform.

## 2026-08-11 — feat(src): #19 Phase 0 implemented (borderless chrome + theme)

- `main.cpp`: `glfwWindowHint(GLFW_DECORATED, GLFW_FALSE)` before window
  creation; `ImGui::StyleColorsDark()` replaced with new
  `orcisf::app::ApplyModernTheme()`.
- New `src/app/Theme.{h,cpp}`: dark-slate ImGui style override --
  `WindowRounding`/`FrameRounding` (~10/6), thin borders, generous
  padding, a single desaturated-blue accent color used consistently.
- New `src/app/CustomTitleBar.{h,cpp}`: app title, drag-to-move, and
  Minimize/Maximize/Close, hand-drawn `ImDrawList` icons (no icon font,
  matching `IconToolbar`'s existing approach). Drag is a per-frame
  mouse-delta nudge (`glfwSetWindowPos`), a no-op on Wayland by design
  (detected via `glfwGetPlatform()`), surfaced via a disabled-looking
  drag zone + tooltip rather than failing silently -- Phase 1 (native
  per-platform drag/resize/snap) is separate follow-up work already
  scoped in issue #19.
- **Architecture finding, not a style choice:** the title bar had to be
  drawn *inside* `Toolbar`'s existing `BeginMainMenuBar()`/
  `EndMainMenuBar()` block via a new `Toolbar::SetTitleBarDrawer()`
  hook, not as its own top-of-viewport ImGui window. A standalone
  window (using the same `WorkPos`-shrink stacking trick `IconToolbar`
  uses to sit *below* the menu bar) rendered zero visible pixels when
  placed *above* it -- confirmed with a debug fill color that never
  appeared on screen. `ImGui::BeginMainMenuBar()` always claims the
  true top of the viewport regardless of what was already drawn there,
  so the later-submitted menu bar painted over it completely every
  frame.
- `Application` gained a raw `GLFWwindow*` member (`SetWindow()`, called
  once from `main.cpp`) -- the one deliberate exception to "only
  `main.cpp` touches GLFW types" this port has followed since #2; stored
  opaquely and only ever forwarded to `CustomTitleBar::Draw()`.
- **Bug found and fixed during verification:** the three title-bar
  buttons used default `ImGui::SameLine()` between them, which picks up
  `ApplyModernTheme()`'s nonzero `style.ItemSpacing` and silently widens
  the button cluster past the computed available width -- pushing Close
  off the window's actual right edge (invisible/unclickable). Fixed
  with explicit `SameLine(x, 0.0f)` zero-spacing between the three
  buttons.
- **Verified interactively** in this environment: borderless window
  confirmed (no OS title bar), title text renders correctly in the menu
  bar row, all three buttons render with correct icons including the
  Maximize<->Restore swap, clicking Maximize actually maximized the
  window (confirmed via a full-screen screenshot) and clicking it again
  restored it. The drag mechanism was confirmed working emphatically --
  during testing, an in-progress click sequence produced a large
  real window-position delta that moved the live window off-screen
  (`GetWindowRect` showed negative X coordinates); recovered with a
  plain Win32 `SetWindowPos` call (not a code change) to bring it back
  on screen, with no other window/file affected. macOS/Linux were not
  interactively verified in this environment (no access to those
  platforms) -- verify via a real build on each before treating Phase 0
  as fully done there.
- Files: `src/app/CustomTitleBar.h` (new), `src/app/CustomTitleBar.cpp`
  (new), `src/app/Theme.h` (new), `src/app/Theme.cpp` (new),
  `src/app/main.cpp`, `src/app/Application.h`, `src/app/Application.cpp`,
  `src/gui/Toolbar.h`, `src/gui/Toolbar.cpp`, `src/CMakeLists.txt`

## 2026-08-11 — fix(src): #19 drag jitter

- **User-reported bug, same day as the Phase 0 landing:** dragging the
  window "glittered/shook" instead of moving smoothly.
- Root cause: the original drag implementation accumulated
  `ImGui::GetIO().MouseDelta` per frame and applied it via
  `glfwSetWindowPos()`. That creates a feedback oscillation --
  `glfwSetWindowPos()` moves the window but not the physical cursor, so
  the cursor's *window-relative* position (what GLFW's mouse callback,
  and therefore ImGui's `MouseDelta`, actually reports) silently shifts
  by the exact opposite of the move on the very next frame. This is a
  known pitfall of the naive MouseDelta-drag pattern with GLFW.
- Fix: `CustomTitleBar` now captures an absolute screen-cursor position
  once at drag start (`ImGui::IsItemActivated()`, new
  `dragging_`/`drag_start_cursor_screen_*_`/`drag_start_window_*_`
  members) and every frame recomputes
  `glfwGetWindowPos() + glfwGetCursorPos()` fresh (both always-current,
  not event-cached), applying the window position as an offset from that
  fixed anchor rather than an accumulated delta -- no feedback path.
- **Verified** with a scripted small-step drag (mouse down, 10-20 small
  cursor moves, window position logged after each step): perfectly
  linear/monotonic movement matching the cursor exactly, zero
  oscillation, in both directions and through negative (partially
  off-screen) window X coordinates.
- Files: `src/app/CustomTitleBar.h`, `src/app/CustomTitleBar.cpp`

## 2026-08-11 — feat(src): #18 implemented (AutoCAD-style Add Joint + guidance)

- Per explicit user decision (issue #18 flagged this as needing
  confirmation before implementing): **"Add Joint" is now a
  click-to-place mode**, not an instant single-shot action -- matching
  Connect Joints/the load-placement modes' existing click-driven
  pattern. `Toolbar`'s Edit-menu item and `IconToolbar`'s button are now
  checkable toggles (`EditorOptions::add_joint_mode`, new field);
  `Application::OnAddJointRequested()` only bootstraps a blank structure
  if nothing's loaded, the actual placement is a new branch in
  `ViewportPanel::HandlePicking()` that ray-casts the click against the
  horizontal plane through the camera's orbit target and calls
  `editable->AddJoint()` there directly, respecting `snap_to_grid`. Like
  Connect Joints, stays active after each placement.
- Added AutoCAD-command-line-style step-by-step status text in
  `Toolbar.cpp` for all four modes: Add Joint, Connect Joints (now
  distinguishes "click the first joint" vs. "click the second joint"
  based on `connect_first_joint`), and both load-placement modes.
- **Real bug found and fixed during interactive verification:**
  `ViewportPanel::Draw()` showed a static "No dataset loaded" placeholder
  -- instead of the interactive 3D image widget -- whenever the scene had
  0 joints/members, with no distinction from "nothing loaded at all".
  That's exactly the state right after bootstrapping a blank structure
  for Add Joint mode, so the very first click had nothing to click on --
  a hard dead end. Caught by actually driving the app: Properties panel
  stayed at "Number of joints: 0" after a click that should have placed
  one. Fixed by gating the placeholder on `!editable && scene.Empty()`
  instead of `scene.Empty()` alone.
- **Verified interactively:** toggling Add Joint mode (button highlight,
  tooltip, status text all update); clicking empty viewport space placed
  a joint exactly at the click position (Properties panel showed
  "Joint 1" with correct position); mode stayed active -- a second click
  placed "Joint 2" without re-opening any menu. Connect Joints' new
  first-click-vs-second-click text was code-reviewed but not re-verified
  live (unrelated focus-stealing interference interrupted that specific
  check; the code mirrors an already-proven state check pattern).
- Files: `src/gui/editor/Selection.h`, `src/gui/ViewportPanel.cpp`,
  `src/gui/Toolbar.cpp`, `src/gui/IconToolbar.cpp`,
  `src/app/Application.cpp`

## 2026-08-11 — Epic #13 close/reopen correction

- Mistakenly closed epic #13 with a comment claiming all 5 sub-issues
  were done, including #16 (re-optimize using the last best result) --
  #16 was never implemented; it was explicitly deferred earlier the same
  day in favor of #18 due to its larger scope (needs real engine-level
  changes to the optimizer's initial population, not just GUI). Caught
  immediately by re-checking #16's actual state via the API before
  reporting completion. Reopened #13 and posted a correction comment.
  Status: #14/#15/#17/#18 closed, #16 open, #13 open pending #16.

## 2026-08-11 — Epic #20: joints/members list, 2D plane-locked drawing, UCS icon

- Created epic **#20** and three sub-issues, per the user's bundled
  feature request (via `/planner`): a Joints/Members list panel with
  delete-with-cascade-warning, AutoCAD-style 2D plane-locked drawing
  (X-Y/X-Z/Y-Z orthographic views with an adjustable offset), and a UCS
  icon overlay in the viewport.
  - **#21** — feat(src): add a Joints/Members list panel (editable,
    delete-with-cascade-warning). Reuses `LoadsPanel`'s existing
    inline-edit/row-click-syncs-Selection pattern; new warning-before-
    cascade-delete requirement in front of `EditableStructure::DeleteJoint()`,
    which already cascades silently.
  - **#22** — feat(src): 2D plane-locked drawing (X-Y/X-Z/Y-Z orthographic
    views + adjustable offset). Extends #18's single-plane ray-intersection
    logic in `ViewportPanel::HandlePicking()`; requires adding an
    orthographic projection mode to `Camera` (currently perspective-orbit
    only).
  - **#23** — feat(src): UCS icon overlay in the viewport. Design note
    recommends a small `ImDrawList` overlay (axis directions projected
    through `Camera::ViewMatrix()`'s rotation), matching this project's
    existing hand-drawn-icon style (`IconToolbar.cpp`), not a 3D gizmo mesh.
  - **Already exists, called out in the epic body so it isn't duplicated**:
    drag-to-move a joint (ImGuizmo gizmo, issue #6) and cascade-delete on
    joint removal (`EditableStructure::DeleteJoint()`) both already work;
    only the confirmation warning is new scope (#21).
  - Epic #20's body was PATCHed after sub-issue creation to link the real
    issue numbers, following the #1/#13 epic pattern.

## 2026-08-11 — feat(src): implement issue #21 (Joints/Members list panel)

- Added `src/gui/JointsMembersPanel.{h,cpp}`: a "Joints/Members" panel,
  docked alongside "Loads" in all three view-layout presets, listing
  every joint (X/Y/Z inline-editable, restraint toggle, delete) and
  member (joint A/B, delete) in the loaded dataset. Reuses `LoadsPanel`'s
  row-select/inline-edit pattern; joint moves go through the same
  `EditableStructure::MoveJoint()` the gizmo/Properties panel already use.
- Deleting a joint with connected members now shows a confirmation modal
  (which members will be deleted) before calling
  `EditableStructure::DeleteJoint()` -- that function's cascade-delete
  behavior itself is unchanged (pre-existing, see `gui/editor/` notes);
  only the warning is new, and only from this panel.
- Wired into `src/app/Application.{h,cpp}` (new panel member, dock
  windows, `OnFrame()` draw call) and `src/CMakeLists.txt` (new source
  file).
- No local `cmake`/build toolchain was available in this environment this
  session -- the change was reviewed against `LoadsPanel.cpp`/
  `PropertiesPanel.cpp`'s exact patterns and `EditableStructure.h`'s
  actual API, but **not compiled or interactively run**. CI is the first
  real build check. Issue #21 status set to `ready-for-review`, not
  `done`, until that's confirmed.

## 2026-08-11 — feat(src): implement issue #22 (2D plane-locked drawing)

- Added `Camera::SetViewPlane()`/`IsOrthographic()` (`gui/viewport/
  Camera.{h,cpp}`): switches between the perspective/orbit camera and an
  orthographic view locked to X-Y/X-Z/Y-Z, bypassing yaw/pitch via two
  new private fixed-basis members to avoid a real gimbal-lock case
  (top-down XZ view: `Forward() == WorldUp()` degenerates the usual
  `Cross(Forward(), WorldUp())` right-vector formula).
- Added `Mat4::Orthographic()` (`gui/viewport/Math3D.h`).
- Added `ViewPlane` enum + `EditorOptions::view_plane`/`plane_offset_xy`/
  `plane_offset_xz`/`plane_offset_yz` (`gui/editor/Selection.h`) -- each
  plane's offset is remembered independently across switches.
- `Toolbar`'s new "View Plane" menu selects the plane and edits the
  active plane's offset (typeable `InputFloat` + `SliderFloat`).
  `ViewportPanel::Draw()` draws a read-only "Plane X-Y is at Z = ..."
  overlay at the bottom-left of the 3D image, and syncs the camera's
  plane/offset every frame.
- `ViewportPanel::HandlePicking()`'s Add Joint branch now places points
  exactly on the locked plane (exploiting that `ScreenRay()`'s direction
  in ortho mode already *is* the locked axis) and force-sets the locked
  coordinate to the exact configured offset rather than trusting ray-math
  floating point; grid-snap only snaps the two free axes. Orbit
  (mouse-drag and arrow-key) is disabled while a plane is locked; pan/
  zoom still work. `ImGuizmo::SetOrthographic()` now follows
  `camera_.IsOrthographic()` instead of being hardcoded false.
- **A real bug was caught by hand-deriving the up-vector cross product**
  (no build toolchain available to run it): the Y-Z elevation plane's
  right-vector was initially wrong-signed, which would have rendered
  that view upside-down (Y is this project's vertical axis). Fixed
  before committing.
- No local `cmake` toolchain was available this session -- not compiled
  or interactively run. CI is the first real build check. Issue #22
  status set to `ready-for-review`, not `done`, until confirmed.

## 2026-08-11 — feat(src): implement issue #23 (UCS icon overlay)

- Added `DrawUcsIcon()` (`gui/ViewportPanel.cpp`, anonymous namespace): a
  small 3-axis (X red/Y green/Z blue, labeled) UCS indicator drawn on
  `ImGui::GetForegroundDrawList()`, always visible bottom-left of the
  viewport in every camera mode (free/perspective and #22's plane-locked
  orthographic views alike). Projected via dot products against
  `Camera::Right()`/`Forward()` (reusing the existing up-vector
  derivation `Pan()`/`ScreenRay()` already use), not a full matrix
  multiply or a new 3D mesh in `SceneRenderer`.
- Never creates an ImGui item, so it can't intercept mouse input meant
  for orbit/pan/click-to-place -- true by construction (draw-list-only).
- Reserves a fixed area in the viewport's bottom-left corner so it
  doesn't overlap issue #22's plane-offset readout, which now starts
  past it.
- This completes all three of epic #20's sub-issues (#21, #22, #23).
  Epic #20 itself is left open pending explicit user confirmation to
  close, per this project's usual caution around auto-closing epics.
- No local `cmake` toolchain was available this session -- not compiled
  or interactively run. CI is the first real build check. Issue #23
  status set to `ready-for-review`, not `done`, until confirmed.

## 2026-08-11 — Epic status recheck (/planner): closed #1 and #20

- Re-synced all 23 tracked issues' actual state against `AGENTS.md`. Found
  #21/#22/#23 already `closed` on GitHub (auto-closed by their "Closes
  #N" commit trailers) but still recorded as `ready-for-review` --
  corrected.
- **Epic #1** (port ORCISF to a modern GUI): all sub-issues #2-#9 closed
  -- closed with a summary comment.
- **Epic #20** (joints/members list, 2D plane-locked drawing, UCS icon):
  all sub-issues #21/#22/#23 closed -- closed with a summary comment.
- **Epic #13** (more user-friendly GUI) intentionally left open -- #16
  (re-optimize using the last best result) is still open (deferred
  earlier for needing real engine-level optimizer changes, not just
  GUI), so #13 isn't actually done yet.

## 2026-08-11 — feat(src): implement issue #16 (re-optimize from last best)

- Added `OptimizationOptions::seed_from_previous_best`/`seed_var_b`/
  `seed_var_k` (`engine/include/engine/Optimizer.h`): when set and their
  size matches the run's `12*jum_balok`/`5*jum_kolom` exactly,
  `Optimizer.cpp`'s `AcakVariabel()` seeds population slot 0 with the
  given design (`SeedStrukturAwal()`) instead of randomizing it. A size
  mismatch silently falls back to a normal random slot 0 -- validated in
  the engine itself, not only the GUI.
- Documented in `engine/README.md`'s new "Re-optimize from last best"
  section: where it plugs into `AcakVariabel()`, why it doesn't affect
  issue #4's threading determinism (seeding replaces one RNG draw before
  any worker threads exist), and an analytical argument for why the
  re-optimized cost can never be worse than the seed's.
- `gui/RunPanel.{h,cpp}`: new "Re-optimize from last best result"
  checkbox, enabled only when a completed (not cancelled/errored) run's
  result exists for the *exact* dataset path currently entered
  (`has_result_`/`result_dataset_path_`). `StartRun()` extracts the
  previous best design (population slot `JSTD-1`) from `result_sd_`
  before it gets overwritten by the new run.
- No local `cmake` toolchain was available this session -- not compiled
  or interactively run; the "cost never worse" claim is reasoned
  analytically, not empirically confirmed. CI is the first real build
  check. Issue #16 status set to `ready-for-review`, not `done`, until a
  real run confirms the acceptance criterion. Epic #13 can be revisited
  for closure once that's confirmed.

## 2026-08-11 — Build toolchain located; #16 compiled and empirically verified

- Found a usable local build toolchain in this environment after all
  (CMake at `C:\Qt\Tools\CMake_64\bin\cmake.exe`, MSVC via VS2022
  BuildTools at `C:\Program Files (x86)\Microsoft Visual Studio\2022\
  BuildTools`, Ninja generator) -- earlier sessions this project
  incorrectly reported "no cmake toolchain available"; it was present
  but not on the default `PATH`/shell profile.
- Built both `orcisf_cli` and `orcisf_gui` from the existing
  `windows-release` preset build directory -- **all source changed in
  this session's #16/#21/#22/#23 work compiled cleanly** (only warnings,
  no errors): `Optimizer.{h,cpp}`, `RunPanel.{h,cpp}`, `Toolbar.cpp`,
  `ViewportPanel.cpp`, `Camera.{h,cpp}`, `Math3D.h`, `Selection.h`,
  `JointsMembersPanel.{h,cpp}`. (One link failure along the way was a
  leftover `orcisf_gui.exe` process from an earlier session holding the
  file open -- killed it, not a code issue.)
- **Empirically verified issue #16's acceptance criteria** with a
  standalone scratch program (not checked in) linked against
  `orcisf_engine`, run against a scratch copy of `Example/Data01`: a
  truncated baseline run (`harga=3.02672e+07`) followed by a seeded
  re-optimization (`harga=2.46519e+07`) showed a real cost improvement,
  confirming the seed took effect; re-running the seeded case with
  `worker_threads=4` produced a bit-identical result to
  `worker_threads=1`, confirming issue #4's determinism guarantee holds
  with seeding active. Full numbers in `engine/README.md`.
- `AGENTS.md`'s #16/#21/#22/#23 sections updated from "not compiled, no
  toolchain available" to reflect what was actually verified now that a
  toolchain exists. #16's `## Tracked Issues` status confirmed `closed`
  (already auto-closed via its commit's `Closes #16` trailer, now backed
  by a real empirical check rather than only analytical reasoning).

## 2026-08-11 — Closed epic #13; rewrote README.md with a GUI tutorial

- Closed epic **#13** (more user-friendly GUI) -- all five sub-issues
  (#14/#15/#16/#17/#18) are closed, #16 now backed by a real empirical
  verification (see the entry above) rather than only analytical
  reasoning.
- **Rewrote `README.md`**: it previously described `src/` as "reserved,
  currently empty," which had been stale since epic #1 landed. Added a
  full "GUI application tutorial" section (getting the app, the window
  layout, building/editing geometry, applying loads, running the
  optimizer including re-optimize-from-last-best, inspecting results,
  exporting), updated the repository-contents tree, and added a
  "Releases" section pointing at prebuilt binaries.
- **`AGENTS.md`**: added "Epic #13 complete"/"Epic #20 complete" summary
  notes (mirroring the existing "Epic #1 complete" one), documented the
  local Windows build toolchain location (CMake/MSVC BuildTools/Ninja --
  not on default `PATH`, an earlier session incorrectly reported none
  available) in the Validation section, and added a "Releases" note to
  the GitHub Workflow section (alpha versioning scheme: `v0.0.x-alpha`).
- **Published the first GitHub Release**: tag `v0.0.1-alpha`, marked as a
  pre-release, with a prebuilt Windows x64 binary
  (`orcisf_gui-v0.0.1-alpha-windows-x64.zip`, built via the same
  `windows-release` MSVC/Ninja build used to verify #16 above) attached
  as a release asset. Release notes summarize what's included (epics #1,
  #13, #20 complete) and link to the README's new GUI tutorial.

## 2026-08-11 — feat(src): 2D plane offset control unreachable from the viewport

- Issue #24 created on GitHub (label: `bug`).
- Scope: `src/gui/ViewportPanel.cpp` / `src/gui/Toolbar.cpp`
- User-reported gap: issue #22's plane-offset slider/input only lives in
  Toolbar's "View Plane" menu, which has to be reopened for every
  adjustment -- with a plane locked at the default 0.000 offset there is
  no discoverable way to actually move it while looking at the 3D view.
  Requested fix: an interactive slider + typeable field docked under the
  UCS icon (#23) in the viewport's bottom-left corner, not just the
  existing read-only readout there.

## 2026-08-11 — fix(src): implement issue #24 (plane offset control moved into viewport)

- Moved the plane-offset `InputFloat`/`SliderFloat` from `Toolbar`'s
  "View Plane" menu into `ViewportPanel::Draw()`, docked directly under
  the UCS icon (issue #23) in the viewport's bottom-left corner, as a
  real interactive ImGui overlay (`SetCursorScreenPos` within the same
  "Viewport" window, drawn after `Image()` so it renders on top and wins
  hover/click priority naturally). `Toolbar`'s menu now only selects the
  active plane (Free/X-Y/X-Z/Y-Z) -- the old menu offset widgets were
  deleted, not duplicated, so there's exactly one place to edit
  `plane_offset_xy`/`_xz`/`_yz`.
- Raised the UCS icon's vertical position (`kUcsBottomGap`) to leave room
  for the new control underneath it, and replaced the old read-only
  "Plane X-Y is at Z = ..." text readout with the interactive fields
  themselves.
- **Found and fixed a real bug while implementing this**: `hovered` (used
  to gate starting an orbit/pan drag) is captured once right after
  `Image()`, before the new overlay widgets are submitted later in the
  same frame -- without a fix, clicking the slider would also start an
  orbit-drag underneath it. Fixed with a new `offset_overlay_capturing`
  flag (`IsItemHovered()`/`IsItemActive()` on the two new widgets), ANDed
  into every orbit/pan/zoom-start gate, mirroring how `gizmo_capturing`
  already gates the same lines for ImGuizmo. Caught by reasoning through
  ImGui's submission-order hover model, not by interactive testing.
- Files: `src/gui/ViewportPanel.cpp`, `src/gui/Toolbar.cpp`.
- **Compiled successfully** (MSVC/Ninja, `windows-release` preset) and
  the built app was launched and confirmed running (process alive, no
  crash) after the change. **Not interactively click-tested** -- whether
  dragging the slider avoids triggering orbit, and whether a placed
  joint's coordinate exactly matches a nonzero offset, are the issue's
  own acceptance criteria and still need a real interactive pass. Issue
  #24 status set to `ready-for-review`, not `done`, until that's
  confirmed.

## 2026-08-11 — feat(src): Run panel dataset-path field + per-run output subfolder

- Issue #25 created on GitHub (label: `enhancement`).
- Scope: `src/gui/RunPanel.{h,cpp}`, `src/app/Application.{h,cpp}`,
  `src/engine/include/engine/Engine.h`/`Engine.cpp`
- User-reported via `/planner`: `RunPanel`'s own "Dataset path" text
  field is a confusing second source of truth once a dataset is already
  loaded via File > Open Data/New Data. Bundled with a second request:
  each optimization run (including re-optimize-from-last-best, #16)
  should write its `.opt/.str/.kdl/.inf/.his/.log.txt` output into a
  timestamped subfolder (`YYYY-MM-DD.HH.MM`, e.g. `2026-08-11.13.40`)
  instead of overwriting the same files in place every run. Flagged in
  the issue body as an engine-level change (`RunFullOptimization()`
  currently derives both input and output paths from one shared generic
  path), not just GUI wiring.

## 2026-08-11 — fix(src): implement issue #25 (Run panel path + per-run output subfolder)

- **Part 1**: `RunPanel` no longer has its own editable "Dataset path"
  field. `dataset_path_` is now a plain `std::string`, synced every frame
  from `Application`'s actually-loaded dataset via a new
  `RunPanel::SetDatasetPath()` (called at the top of
  `Application::OnFrame()`, before anything reads `CanRun()`). The panel
  shows the path read-only; `Run`/`CanRun()`/the icon toolbar's Run
  button all key off it automatically.
- **Part 2**: `engine::RunFullOptimization()` now returns the generic
  *output* path (was `void`) -- each run creates a
  `<dataset folder>/<YYYY-MM-DD.HH.MM>/` subfolder and writes
  `.opt/.str/.kdl/.inf/.his/.log.txt` there instead of overwriting the
  dataset's own files in place; input files are still read from the
  original path, unaffected. Threaded the new return value through
  `orcisf_cli`, `RunPanel` (`result_output_path_`, `SetOnResult`'s now
  3-arg callback), and `Application` (`last_run_output_path_`) --
  `report::WriteTextExport()`'s `source_generic_path` argument now uses
  `last_run_output_path_` instead of the input dataset path in all three
  call sites (`OnSaveRequested`/`OnSaveAsRequested`/
  `OnExportTextRequested`), or text export would silently stop finding
  `.his`/`.log.txt`.
- Files: `src/gui/RunPanel.{h,cpp}`, `src/gui/IconToolbar.cpp`,
  `src/app/Application.{h,cpp}`, `src/engine/include/engine/Engine.h`,
  `src/engine/src/Engine.cpp`, `src/engine/tools/orcisf_cli.cpp`.
- **Compiled successfully** (MSVC/Ninja, `windows-release` preset, all
  three targets) and **empirically verified** with `orcisf_cli optimize`
  against a scratch copy of `Example/Data01`: confirmed a real
  `<timestamp>/` subfolder was created with the run's six output files,
  and (via checksum) that the dataset's own top-level `.opt` was
  byte-identical before/after -- genuinely untouched. The built GUI
  launched and ran without crashing after the `RunPanel` changes, but the
  UI itself wasn't click-tested interactively. Issue #25 status set to
  `ready-for-review`, not `done`, until that's confirmed.

## 2026-08-11 — Published GitHub Release v0.0.2-alpha

- Tag `v0.0.2-alpha` (pre-release), with a prebuilt Windows x64 binary
  (`orcisf_gui-v0.0.2-alpha-windows-x64.zip`) built from the same
  `windows-release` MSVC/Ninja build used to verify issue #25. Bumped
  from `v0.0.1-alpha` per `AGENTS.md`'s alpha versioning scheme (patch
  number increments each alpha). Release notes summarize what changed
  since `v0.0.1-alpha`: issues #21-#25.

## 2026-08-11 — feat(src): wire up the app icon set (icons/) for all platform builds

- Issue #26 created on GitHub (label: `enhancement`).
- Scope: `src/CMakeLists.txt`, `src/app/main.cpp`, new Windows `.rc`
  resource file, new Linux `.desktop` file.
- User-reported via `/planner`: a full generated icon set already exists
  at repo-root `icons/` (Windows `.ico`, macOS `.icns`, Linux/general PNG
  sets, per `icons/README.txt`) but nothing in `src/` references it --
  confirmed by search, the built app currently has no icon on any
  platform (no .exe icon resource, no window icon, no macOS bundle icon,
  no Linux desktop-file icon). Scoped per-platform in the issue: Windows
  `.rc` + runtime `glfwSetWindowIcon()`, macOS `MACOSX_BUNDLE` +
  `.icns`, Linux `.desktop` file + runtime window icon (noting GLFW
  doesn't support `glfwSetWindowIcon` on macOS). Left the PNG-decoder
  choice for the runtime window icon (existing transitive `libpng` vs. a
  small vendored decoder) as an implementation decision, not dictated.

## 2026-08-11 — feat(src): implement issue #26 (app icon set wired into the build)

- **Windows**: new `src/app/orcisf.rc` (single numeric-ID `ICON`
  resource) embeds `icons/windows/orcisf.ico` as `orcisf_gui.exe`'s icon;
  added to the target's sources under `if(WIN32)`.
- **macOS**: `CMakeLists.txt` sets `MACOSX_BUNDLE TRUE` +
  `MACOSX_BUNDLE_ICON_FILE` (`icons/macos/orcisf.icns`, added with
  `MACOSX_PACKAGE_LOCATION "Resources"`) so a real `.app` bundle gets a
  Finder/Dock icon.
- **Windows/Linux runtime window icon**: new `src/app/AppIcon.{h,cpp}` --
  `ApplyWindowIcon()` decodes `icons/png/icon_{16,32,48,256}x*.png` via
  libpng's simplified API and calls `glfwSetWindowIcon()`; a deliberate
  no-op on macOS (GLFW doesn't support that call there). Icon PNGs are
  copied next to the built executable by a new `CMakeLists.txt`
  `POST_BUILD` step (same reasoning as the pre-existing MinGW-DLL copy);
  `AppIcon.cpp` resolves them relative to the executable's own directory
  (`GetModuleFileNameW`/`readlink("/proc/self/exe")`), not the process's
  cwd. Wired into `main.cpp` right after window creation.
- **Linux**: new `src/packaging/orcisf.desktop` + `install()` rules
  (`.desktop` file + a `hicolor` theme icon) for desktop-environment
  integration, effective via `cmake --install`.
- Added `libpng` as a *direct* `vcpkg.json` dependency (was already an
  installed transitive one via `libharu`'s PNG-in-PDF support --
  confirmed via `vcpkg_installed/vcpkg/info/libpng_*.list` before adding
  it explicitly).
- Files: `src/vcpkg.json`, `src/CMakeLists.txt`, `src/app/main.cpp`,
  `src/app/AppIcon.{h,cpp}` (new), `src/app/orcisf.rc` (new),
  `src/packaging/orcisf.desktop` (new). Also checked in the `icons/`
  asset directory itself (25 files, ~8.2MB) in a preceding commit, since
  it was sitting untracked in the working directory.
- **Compiled and empirically verified on Windows** (MSVC/Ninja,
  `windows-release` preset): the `.rc` resource compiles and links
  (`orcisf.rc.res` generated), the runtime icon PNGs land next to the
  built `.exe`, and the app launches/runs without crashing with
  `ApplyWindowIcon()` wired in. The icon's actual on-screen appearance
  was not confirmed via screenshot (a screenshot attempt captured an
  unrelated foreground window instead -- a known focus-stealing hazard
  in this environment, not worth repeatedly retrying for a cosmetic
  check). **macOS/Linux have no local toolchain to verify against** in
  this Windows-only dev environment -- written against each platform's
  documented CMake convention; CI (`build-src.yml`'s three-OS matrix) is
  the first real cross-platform check. Issue #26 status set to
  `ready-for-review`, not `done`, until CI (and ideally a real
  interactive screenshot on Windows) confirms it.

## 2026-08-11 — User retest of #21-#25: three follow-up issues filed

- User asked (`/planner`) to retest issues #21-#25 against a real running
  build and reported: (a) issue #25's "Enter a dataset path" message and
  Run-blocking still appearing, (b) a cosmetic gap between the title
  bar's Close button and the window's right edge, (c) no icon before
  panel titles, (d) no VS Code-style Alt-mnemonic menu access, (e) issue
  #22/#24's plane-offset control not visibly showing up.
- Did real interactive testing this session: launched the current build
  (MSVC/Ninja, `windows-release`, includes all of #21-#26), confirmed a
  DPI-awareness pitfall again applied to screen-coordinate automation
  (same issue #11's interactive-verification pass hit previously --
  fixed with `SetProcessDPIAware()`), opened the File menu and screenshotted
  it (now shows a **separate top-level "View Plane" menu**, confirming
  that part of #22 is present), and confirmed via a zoomed screenshot
  crop that the title bar buttons **do** have a visible gap before the
  window's right edge.
- Attempting to load a real dataset via the native "Open Data..."
  folder-picker to test the RunPanel/offset-visibility claims with
  blind coordinate-based UI automation did not cleanly succeed after a
  few attempts -- stopped rather than keep retrying, per this project's
  own "avoid automation rabbit holes" guidance (see AGENTS.md).
- Code review of current `main` found: (a) #25's fix (new wording, no
  editable field, `SetDatasetPath()` wired correctly) is genuinely
  present in current source -- if the user is still seeing the *old*
  message text, they're likely testing a build that predates commit
  `9fc3e17` (e.g. `v0.0.1-alpha`); (b) #22/#24's offset overlay code is
  also present and looks correctly gated, no code-level bug found by
  inspection.
- Filed three issues:
  - **#27** (bug) -- title bar buttons not flush to the window's right
    edge, confirmed via screenshot; hypothesized root cause
    (`ImGui::SameLine()`'s offset being relative to content-region start,
    not raw window width) documented for whoever fixes it, not asserted
    as certain.
  - **#28** (enhancement) -- Alt-mnemonic menu navigation (`&`-prefixed
    labels) + an icon before each panel's title, bundled since both are
    from the same UX-polish request.
  - **#29** (bug) -- tracks finishing the interactive re-verification
    this session couldn't complete (load a real dataset, confirm Run
    panel and plane-offset-overlay behavior end-to-end), and explicitly
    asks to confirm which build/commit the original report was made
    against.

## 2026-08-11 — Published GitHub Release v0.0.3-alpha

- Tag `v0.0.3-alpha` (pre-release), with a prebuilt Windows x64 binary
  (`orcisf_gui-v0.0.3-alpha-windows-x64.zip`, now including the `icons/`
  PNG subfolder issue #26 needs for the runtime taskbar/window icon)
  built from the same `windows-release` MSVC/Ninja build used to verify
  #26. Bumped from `v0.0.2-alpha` per `AGENTS.md`'s alpha versioning
  scheme. Release notes summarize #26 (fixed) and #27-#29 (filed, not
  yet fixed) from this session's retest pass.

## 2026-08-11 — Investigated #27: could not reproduce, was a screenshot-tooling artifact

- Picked up issue #27 (title bar buttons not flush to window right
  edge) via `/coder`. Before writing any fix, re-measured with a
  properly maximized window and a corrected, physical-pixel-accurate
  screenshot -- found the original "confirmed via screenshot" evidence
  was itself wrong: it mixed a DPI-aware `GetWindowRect()` call with
  .NET's `Screen.Bounds`/`CopyFromScreen()` (which returned a *logical*,
  scaled-down size in this environment despite `SetProcessDPIAware()`),
  so the screenshot only captured roughly the top-left quarter of the
  true physical screen -- the button cluster was actually rendering
  correctly at the true (uncaptured) physical right edge.
- Re-verified with a corrected capture (explicit physical-pixel bitmap
  size, not `Screen.Bounds`): the Minimize/Maximize/Close buttons are
  flush to the window's right edge with even, consistent spacing -- no
  gap, no bug. `CustomTitleBar.cpp`'s right-alignment math was reviewed
  and found correct as written (operates in ImGui's own logical
  coordinate system, unaffected by the OS-level DPI API inconsistency
  that caused the false positive).
- **No code change made.** Posted a comment on issue #27 explaining the
  finding and recommending it be closed as "cannot reproduce" -- not
  closed automatically, per this project's usual confirmation caution.
  Documented the underlying DPI-screenshot-measurement pitfall in
  `AGENTS.md` (relevant to issue #29's still-pending interactive
  verification too, which uses the same kind of tooling).

## 2026-08-11 — feat(src): implement issue #28 (Alt-mnemonics + panel icons)

- **Part 1**: added `&` mnemonics to `Toolbar::Draw()`'s six top-level
  menu labels (`&File`, `&Edit`, `View &Plane`, `&Loads`, `&Run`,
  `&View`) -- Dear ImGui's built-in mechanism, no new code needed beyond
  picking non-colliding letters (`View Plane` uses `P` since `V` was
  already claimed by the separate `View` layout-preset menu).
- **Part 2**: new `src/gui/PanelIcons.{h,cpp}` -- `DrawPanelIconHeader()`
  draws a small hand-drawn icon (same `ImDrawList` style as
  `IconToolbar.cpp`) + label + separator, called as the first thing
  after `ImGui::Begin()` in all seven panels (`ViewportPanel`,
  `PropertiesPanel`, `LoadsPanel`, `JointsMembersPanel`,
  `DetailingPanel`, `RunPanel`, `LogPanel`). Documented why this isn't
  literally inside the native dock-tab label (no public ImGui API for
  that without an icon font or touching `imgui_internal.h`) -- see
  AGENTS.md.
- Files: `src/gui/Toolbar.cpp`, `src/gui/PanelIcons.{h,cpp}` (new),
  `src/gui/{Viewport,Properties,Loads,JointsMembers,Detailing,Run,Log}Panel.cpp`,
  `src/CMakeLists.txt`.
- Compiled successfully (MSVC/Ninja, `windows-release`) after fixing one
  real build error (missing `<initializer_list>` include for the
  icon-drawing helpers' range-for loops). App launched and stayed
  running. **Visual confirmation (Alt-underlines, icons actually
  rendering) was not completed** -- hit this environment's recurring
  focus-stealing hazard again while trying to screenshot it (an
  unrelated app kept stealing foreground); stopped rather than keep
  retrying, per the project's automation-rabbit-hole guidance. Both
  mechanisms are standard/low-risk (ImGui's own mnemonic convention;
  the exact icon-drawing pattern already proven in `IconToolbar.cpp`).
  Issue #28 status set to `ready-for-review`, not `done`, until a real
  interactive/screenshot pass confirms it.

## 2026-08-11 — Added `rebuild` skill; picked up #29, found and fixed a real #28 regression

- Added `.claude/skills/rebuild/SKILL.md`: rebuilds `src/`'s
  `orcisf_gui`/`orcisf_cli` for whichever OS the agent is on, encoding
  how to locate the toolchain (CMake/MSVC/vcpkg) even when it isn't on
  the default `PATH` -- this session had to rediscover that location
  multiple times; now it's a reusable, documented skill. Registered in
  `AGENTS.md`'s Agent Skills section.
- Picked up issue #29 (interactively re-verify #21-#25's claims). Used
  the new `rebuild` skill to confirm the build was current, then
  attempted the interactive verification with a corrected, DPI-aware
  screenshot technique (explicit physical-pixel bitmap, per #27's
  finding) plus an `AttachThreadInput`-based foreground grab that
  reliably worked this time.
- **Found and fixed a real regression along the way**: opening the File
  menu revealed issue #28's "&"-prefixed menu labels ("&File", "&Edit",
  ...) were displaying the literal `&` character, not being parsed into
  an underlined mnemonic -- Dear ImGui's stock `BeginMenu()`/`MenuItem()`
  has no built-in equivalent to Win32's `&`-mnemonic convention, a wrong
  assumption in #28's original implementation. Reverted `Toolbar.cpp`'s
  labels to plain text, rebuilt, and re-screenshotted to confirm the fix.
  Filed a proper follow-up, **#30**, to implement real Alt-mnemonic
  support (hand-rolled: parse a marker, draw underline only while Alt is
  held, detect Alt+letter yourself) rather than re-attempt it hastily.
  Posted a correction comment on the already-closed #28.
- **Also confirmed via the same screenshot**: issue #28's Part 2 (panel
  icon headers) genuinely works -- the hand-drawn icon renders correctly
  before "Viewport"'s title text.
- **Issue #29's core objective (load a real dataset, confirm RunPanel
  and the plane-offset overlay) was NOT completed.** Repeated attempts
  to drive the native "Open Data..." folder-picker via coordinate-based
  clicks failed (wrong menu item clicked once; a stale coordinate reused
  from a differently-scaled screenshot missed a Cancel button), and an
  unrelated app (Proton Drive) spontaneously opened mid-sequence,
  stealing focus -- a new hazard, independent of the already-documented
  "another Claude/browser session steals focus" one. Stopped rather than
  keep retrying, per this project's automation-rabbit-hole guidance.
  Issue #29 remains open with its core objective unmet; `AGENTS.md` has
  the full account plus a lesson for the next attempt (always recompute
  click coordinates fresh from the most recent screenshot's own physical
  pixel space, consider keyboard-only navigation for native dialogs).

## 2026-08-11 — "Build and test issue #28": rebuild + panel-icon re-confirmation

- Used the `rebuild` skill to confirm `main`'s current state (mnemonic
  revert + panel icons) is already built (`ninja: no work to do`).
- Re-launched with the `AttachThreadInput` foreground-grab technique
  (worked reliably again) and a DPI-aware physical-pixel screenshot:
  re-confirmed plain menu labels (no stray `&`) and the Viewport panel's
  icon header, then additionally confirmed the Properties and Log
  panels' icon headers via targeted crops of the same screenshot --
  three different panels now individually verified, all going through
  the same `DrawPanelIconHeader()` code path, giving high confidence in
  the remaining four (`RunPanel`/`LoadsPanel`/`JointsMembersPanel`/
  `DetailingPanel`) without clicking through every tab individually.
- No code changes -- this was a verification-only pass. Issue #28 stays
  closed; no new issues found.

## 2026-08-11 — feat(src): application not DPI-aware on multi-monitor setups

- Issue #31 created on GitHub (label: `bug`).
- Scope: `src/app/main.cpp`, possibly `src/app/orcisf.rc`
- User-reported via `/planner`: on a two-monitor setup with mixed DPI,
  fonts/UI render correctly on a mid-resolution monitor but too small on
  a high-resolution one. Confirmed via code search: nothing in `src/`
  declares DPI awareness (no manifest, no
  `SetProcessDpiAwarenessContext()`), `glfwCreateWindow()` doesn't set
  `GLFW_SCALE_TO_MONITOR`, and no `io.FontGlobalScale`/
  `ImGui::GetStyle().ScaleAllSizes()` call exists anywhere -- a genuine,
  unambiguous gap, not a regression. Scoped in the issue: Per-Monitor-v2
  DPI awareness declaration, `GLFW_SCALE_TO_MONITOR`, startup
  font/style scaling from the actual launch monitor's content scale, and
  an explicit decision on whether live monitor-drag rescaling is in
  scope or a follow-up.

## 2026-08-11 — fix(src): implement issue #31 (DPI awareness)

- `src/app/main.cpp`: added `EnableWindowsDpiAwareness()` (Windows-only,
  resolves `SetProcessDpiAwarenessContext` dynamically via
  `GetProcAddress` using a locally-defined opaque handle type, so it
  doesn't depend on the SDK's `WINVER` target declaring
  `DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2`), called as the very
  first thing in `main()` before `glfwInit()`. Added
  `glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE)` before window
  creation. Read `glfwGetWindowContentScale()` once at startup and used
  it to `ImGui::GetStyle().ScaleAllSizes()` (after `ApplyModernTheme()`)
  and rebuild the default font at a scaled pixel size (crisper than
  `io.FontGlobalScale`).
- Live monitor-drag rescaling explicitly out of scope (documented why in
  `AGENTS.md`) -- content scale is read once at startup only.
- Compiled successfully (MSVC/Ninja, `windows-release`) and **visually
  confirmed via a real screenshot** on this environment's actual
  high-DPI (200%) monitor: menu/panel text is now noticeably larger and
  legible, a clear contrast to every prior screenshot taken earlier in
  this same session on the same monitor. Behavior on an actual
  mid-resolution/100% monitor was not verified (only one physical
  monitor available in this environment) -- reasoned to work correctly
  since the mechanism is monitor-agnostic and would return `1.0` there,
  but not empirically confirmed on a second display.
- Files: `src/app/main.cpp`.
- Issue #31 status set to `ready-for-review`, not `done`, until the
  mid-resolution-monitor case is confirmed on real hardware.

## 2026-08-11 — feat(src): implement issue #30 (real Alt-mnemonic menu navigation)

- `src/gui/Toolbar.cpp`: new `BeginMnemonicMenu(label, mnemonic_index,
  mnemonic_key, alt_held)` -- entirely via ImGui's public API, no
  `imgui_internal.h`. Draws an underline under the mnemonic character
  (positioned via `CalcTextSize()`) only while Alt is held, and opens
  the menu via `ImGui::OpenPopup(label)` (same label/ID `BeginMenu()`
  itself uses) on `Alt+<letter>`, a documented Dear ImGui idiom for
  programmatically opening a popup. All six top-level menus wired
  through it: File=F, Edit=E, "View Plane"=P (index 5, not the first
  letter -- V was already claimed by the separate "View" menu), Loads=L,
  Run=R, View=V -- the same letter assignment #28 originally picked.
- Compiled successfully (MSVC/Ninja, `windows-release`) via the
  `rebuild` skill. **Interactive confirmation was not completed** --
  repeated attempts to grab foreground for a screenshot landed on the
  user's own actively-in-use browser window (reading this project's
  GitHub issue #29 live), correctly identified as the user's active
  session rather than background interference, so automation was
  stopped and their window state restored rather than fighting for
  control of it.
- Files: `src/gui/Toolbar.cpp`.
- Issue #30 status set to `ready-for-review`, not `done`, until a real
  interactive pass (Alt held + screenshot, Alt+letter + screenshot)
  confirms it -- #28's own lesson (an assumption about this exact
  library turned out wrong) is reason enough not to claim this works
  without empirical confirmation, even though the techniques used here
  are standard/well-understood ones.

## 2026-08-11 — Second #29 attempt: root cause of the blocker precisely diagnosed

- Picked #29 back up via `/coder`. Checked `GetForegroundWindow()` first
  (not the user's own active session this time) before grabbing focus.
  Confirmed the app's own File menu is fully keyboard-navigable
  (Down/Down/Enter moved nav focus from "New Data" to "Open Data..." and
  activated it, screenshot-verified at each step) and that the native
  "Select Folder" dialog opens correctly.
- **Precisely diagnosed why loading a dataset keeps failing**: the
  dialog is Windows 11's newer WinUI3/XAML file picker (not the classic
  Win32 common-item dialog). Its "Folder:" text field does not accept
  synthetic `SetCursorPos`/`mouse_event` clicks or `SendKeys` typed text
  in this environment -- confirmed empty across five distinct attempts
  (direct click+type, `Ctrl+A`-then-type, `Ctrl+L` address-bar focus,
  double-click), each re-screenshotted immediately before/after to rule
  out stale coordinates. One attempt visibly navigated the sidebar tree
  instead, showing keyboard focus was landing elsewhere in the dialog
  entirely. This is a known category of issue (WinUI3/XAML controls
  needing real UI Automation, not raw `SendInput`, for reliable external
  automation) -- not a repeat of the previous coordinate-math mistakes.
- Cancelled cleanly via `{ESC}` each time (dialog-level accelerators do
  reach it, just not the text field specifically); no orphaned dialogs,
  no corrupted app state, process stopped cleanly at the end.
- **Still did not achieve #29's core objective** (load a dataset,
  confirm RunPanel/plane-offset behavior). Documented concrete next
  steps in `AGENTS.md` for whoever attempts this next: use a real UI
  Automation API instead of raw input synthesis for this dialog, or
  navigate the folder tree/file list by mouse instead of the text field,
  or hand off to the user for a manual load. Issue #29 left open.

## 2026-08-11 — Third #29 attempt: real root cause found (DPI-virtualized
automation script, not WinUI3), core objective verified, issue closed

- Picked #29 back up a third time via `/coder`. After `/rebuild`
  confirmed the current build, resumed the WinUI3 dialog investigation
  using a short scratch dataset path (`C:\Users\IT\Desktop\OrcisfTest\
  Data01`) and pure mouse navigation of the dialog's sidebar (a click on
  "RDP" had, in a still-earlier sub-attempt, shown that clicking a
  sidebar item *does* populate the "Folder:" field -- a clue the second
  attempt's "text field resists all input" diagnosis was incomplete).
- A stray click landed on Visual Studio Code instead of the dialog,
  bringing this session's own editor window to the foreground.
  Investigated instead of continuing to click blindly: `GetWindowRect()`
  on the (correctly) foregrounded ORCISF window reported logical
  `0,0,2560,1392` while screenshots were captured at physical
  `5120x2784` (this monitor's actual 200% DPI scaling) -- a 2x
  coordinate mismatch. **Root cause**: the PowerShell automation process
  was never marked DPI-aware, so Windows silently virtualized its
  `SetCursorPos`/`GetWindowRect` calls into logical pixels while the
  explicitly-sized screenshot `Bitmap`/`CopyFromScreen` calls stayed in
  physical pixels. This fully explains every prior "the WinUI3 dialog
  won't accept input" symptom -- clicks were landing at literally the
  wrong pixel the whole time, in this and the previous #29 attempt.
- Fix: call `SetProcessDPIAware()` once, before any cursor/window-rect
  call. Verified: `GetWindowRect()` afterward reported physical
  `0,0,5120,2784`, matching the screenshots exactly.
- With that fixed, the WinUI3 "Select Folder" dialog was navigated
  successfully via mouse clicks on its own sidebar/breadcrumb, and a
  real dataset (`Example/Apl1-1`, copied to
  `C:\Users\IT\Documents\orcisf\Apl1-1\aplikasi`) loaded into the app.
- **Both of #29's acceptance criteria confirmed via screenshot**:
  (1) RunPanel (#25) showed the loaded dataset path, populated cost/
  design parameters, and an enabled Run button -- not the "Enter a
  dataset path" placeholder the user had reported. (2) Selecting View
  Plane > "X-Y plane (locks Z)" switched the viewport to an orthographic
  top-down view and revealed the "Plane X-Y -- Z offset (m)" overlay
  (editable `0.000` field) plus an updated UCS icon in the bottom-left
  corner -- confirmed via a close-up crop. This was never actually
  broken; #29's own automation blocker had just prevented anyone from
  reaching the state that would exercise it.
- Typing a new offset value to confirm it moves geometry was attempted
  but interrupted by a spontaneous WhatsApp desktop notification toast
  stealing keyboard focus -- automation was stopped immediately rather
  than risk interacting with that toast (another instance of this
  environment's background-app focus-stealing hazard, same category as
  the second attempt's Proton Drive incident). Left unconfirmed; the
  overlay being a live editable field is otherwise strong evidence the
  binding works.
- Cleaned up all scratch screenshots and the `C:\Users\IT\Desktop\
  OrcisfTest` scratch folder. **Issue #29 closed** -- both criteria the
  user explicitly asked to have re-verified are now confirmed.

## 2026-08-11 — #31 reopened: DPI-awareness fix was incomplete

- User reported (via `/planner`, real dual-monitor use with two
  different-DPI monitors side by side) that the app is still not
  DPI-aware in practice: toolbar icons look too small, the title bar
  Close button is clipped on the right edge, sub-window panel content is
  clipped at the top, and moving the window to a different-DPI monitor
  live does nothing.
- Checked #31's status first (closed) and searched for other open
  DPI-related issues (none found) before deciding how to track this.
  Asked the user whether to reopen #31 or file a new issue, since #31's
  own acceptance criteria had explicitly scoped live monitor-drag
  rescaling as a "decide and document" follow-up rather than a hard
  requirement -- user chose to reopen #31 and fold this in, since it's
  the same root feature (DPI awareness) just incompletely covered by the
  original fix, not a different feature.
- Reopened #31 on GitHub, rewrote its body to document why the original
  fix (still correct as far as it went -- font/style scaling at startup)
  didn't cover: (1) `IconToolbar.cpp`'s hand-drawn `ImDrawList` icons
  (fixed pixel constants, never multiplied by `dpi_scale`), (2)
  `CustomTitleBar.cpp`'s Close-button right-alignment math (tuned before
  DPI scaling existed), (3) `Application.cpp`'s dockspace work-area
  constants (unscaled, causing top-clipping), (4) no
  `glfwSetWindowContentScaleCallback`-driven live rescale when the
  window moves between monitors. Posted an explanatory comment
  cross-referencing the change. Updated `AGENTS.md`'s Tracked Issues row
  and its #31 "read before touching" section with a pointer to this
  reopening and the expanded scope.

## 2026-08-11 — chore(src): one-shot build scripts (#32 created)

- Issue #32 created on GitHub (no duplicate found via search).
- Scope: `src/` build tooling -- `build.ps1` (Windows) and `build.sh`
  (macOS/Linux), wrapping the same OS-detection/toolchain-location/
  configure/build logic the `rebuild` agent skill already documents, but
  as standalone scripts usable without an AI agent in the loop.
- Labels: chore, enhancement

## 2026-08-11 — chore(src): implement issue #32 (build.ps1 / build.sh)

- Added `build.ps1` (Windows) and `build.sh` (macOS/Linux) at the repo
  root, mirroring the `rebuild` agent skill's toolchain-detection logic
  (CMake/MSVC/vcpkg located beyond `PATH`, matching preset picked by OS,
  configure-if-needed, then build `orcisf_gui`/`orcisf_cli`) as
  standalone scripts a human or CI can run without an AI agent.
- Updated `src/README.md`'s Building section to lead with the new
  scripts, keeping the manual `cmake --preset`/`cmake --build --preset`
  instructions as a documented alternative.
- **Interactively verified `build.ps1`** in this environment: found
  `C:\Qt\Tools\CMake_64\bin\cmake.exe` and the BuildTools `vcvars64.bat`
  exactly as the `rebuild` skill documents, located a real vcpkg
  checkout via `VCPKG_ROOT`, and successfully built both binaries. Found
  and fixed a real bug during this pass: the script's initial reported
  path for `orcisf_cli.exe` was wrong (assumed it landed directly in the
  build dir like `orcisf_gui.exe`; it actually lands in a nested
  `engine/` subdirectory, since it's defined in
  `src/engine/CMakeLists.txt` rather than the top-level `CMakeLists.txt`)
  -- caught by checking the reported path against the filesystem, not by
  code review alone. Fixed and re-verified: both `orcisf_gui.exe` and
  `engine/orcisf_cli.exe` confirmed present at the paths the script
  prints.
- **`build.sh` was not executed** (no macOS/Linux toolchain available in
  this environment) -- written against the identical logic and reviewed,
  but relies on CI (`.github/workflows/build-src.yml`) for its first
  real cross-platform confirmation.
- Files: `build.ps1` (new), `build.sh` (new), `src/README.md`,
  `AGENTS.md` (Validation section + Tracked Issues row)

## 2026-08-11 — chore: rename `rebuild` agent skill to `builder`; delegate to build.ps1/build.sh

- Renamed `.claude/skills/rebuild/` to `.claude/skills/builder/` at the
  user's request. Rewrote `SKILL.md` to call the repo's own
  `build.ps1`/`build.sh` (issue #32, added earlier the same day) rather
  than re-deriving toolchain-detection steps inline on every invocation
  -- the scripts are now the single source of truth for that logic, and
  the skill just invokes them and reports the result, using far fewer
  tokens per run.
- Updated every `AGENTS.md` reference from `rebuild` to `builder`
  (Agent Skills section, the Validation section's build.ps1/build.sh
  note, and one inline mention in the #30 Alt-mnemonics entry). Checked
  the other skills (`coder`, `planner`, `_shared`) for stray references
  -- none found.
- Files: `.claude/skills/rebuild/SKILL.md` -> `.claude/skills/builder/
  SKILL.md` (renamed + rewritten), `AGENTS.md`

## 2026-08-11 — chore: add `tester` agent skill

- New `.claude/skills/tester/SKILL.md` at the user's request: tests a
  freshly built binary against a specific issue's own Acceptance
  Criteria checklist (`/tester pick #X`), reporting PASS/FAIL/UNVERIFIED
  per criterion rather than a single aggregate verdict -- UNVERIFIED is
  a deliberate third state for criteria this environment genuinely can't
  mechanically check (e.g. a second physical monitor), so the skill
  isn't forced to guess a pass/fail either way.
- Delegates to `builder` for the binary (doesn't re-derive build steps)
  and to the same repo/token resolution `planner`/`coder` already use
  (doesn't re-derive that either). Bakes in this session's own hard-won
  automation lessons for the GUI-testable case (DPI-aware coordinates,
  foreground-window checks, stale-coordinate avoidance, focus-stealing
  hazards) by reference to `AGENTS.md`'s accumulated notes, rather than
  repeating them inline.
- Never auto-closes the remote issue or writes to `AGENTS.md`/
  `CHANGE_HISTORY.md` without explicit user confirmation, matching
  `planner`/`coder`'s existing caution around remote writes.
- Registered in `AGENTS.md`'s Agent Skills section.
- Files: `.claude/skills/tester/SKILL.md` (new), `AGENTS.md`

## 2026-08-11 — chore: add `reviewer` agent skill; document the full skill workflow

- New `.claude/skills/reviewer/SKILL.md` at the user's request: the
  final quality-assurance gate. Audits architecture/design fit, hunts
  for bugs and security gaps, confirms an issue's criteria pass
  end-to-end (distinct from `tester`'s per-criterion mechanical check),
  then acts on the verdict -- reopen with a comment if something's
  wrong (routing back to `coder`), or comment + propose closing if
  genuinely done, and propose a release if the change included an
  actual `src/` code change (never for docs/tooling-only changes).
  Every remote write (reopen/comment/close/release) requires explicit
  user confirmation, same caution as every other skill.
- Formalized the project's full five-stage workflow in `AGENTS.md`'s
  new "Agent Skill Workflow" subsection: `planner` (create issue) ->
  `coder` (implement) -> `builder` (compile) -> `tester` (per-criterion
  pass/fail) -> `reviewer` (end-to-end QA + close/reopen/release
  decision).
- Added an explicit rule at the top of the Agent Skills section: every
  skill reads `AGENTS.md` first, in full, before doing anything else --
  already true in practice for `planner`/`coder`/`tester`/`reviewer`
  (each already had this as their own Step 1), but `builder` didn't
  say so explicitly since it mostly delegates to `build.ps1`/`build.sh`
  -- added the same instruction there too, since a build failure can
  still need this file's context to interpret correctly.
- Updated the GitHub Workflow section's Releases bullet: releases still
  require explicit user confirmation, but now acknowledge `reviewer` as
  a legitimate source of a release *proposal* (never a release created
  without that confirmation either way).
- Files: `.claude/skills/reviewer/SKILL.md` (new),
  `.claude/skills/builder/SKILL.md`, `AGENTS.md`

## 2026-08-11 — fix: move build.ps1/build.sh from repo root into src/

- User feedback after actually running `.\build.ps1` from a fresh
  PowerShell session: the scripts only ever build `src/`, so they belong
  next to `vcpkg.json`/`CMakeLists.txt`/`CMakePresets.json` in `src/`,
  not the repo root. Moved both (`git mv`), updated each script's
  internal path derivation (script's own directory is now `src/`; repo
  root -- where the `vcpkg/` checkout is expected, per `src/README.md`'s
  bootstrap step -- is derived as its parent), and updated every
  reference in `src/README.md`, `AGENTS.md`, and the `builder` skill.
- The `VCPKG_ROOT` error the user hit was separately confirmed to be
  correct, expected behavior, not a bug: no `vcpkg/` checkout exists yet
  at their repo root and `VCPKG_ROOT` isn't set in their shell -- this
  session's own earlier successful runs only worked because a leftover
  vcpkg checkout happened to exist in this sandbox's temp directory from
  a prior session, which doesn't generalize to a fresh environment.
  Pointed the user at `src/README.md`'s existing one-time bootstrap
  steps rather than changing the script to paper over a missing
  dependency.
- Re-verified `src/build.ps1` end-to-end after the move (same toolchain
  paths located, both binaries built and confirmed present at their
  reported paths).
- Files: `src/build.ps1` (moved from `build.ps1`), `src/build.sh` (moved
  from `build.sh`), `src/README.md`, `AGENTS.md`,
  `.claude/skills/builder/SKILL.md`

## 2026-08-11 — feat(src): build.ps1/build.sh auto-bootstrap vcpkg

- User hit the "VCPKG_ROOT is not set" error on a real first run from a
  fresh clone and asked the script to just get vcpkg itself instead of
  erroring, matching `src/README.md`'s existing one-time bootstrap steps
  (`git clone` + `bootstrap-vcpkg.{bat,sh}` + set `VCPKG_ROOT`).
- Both scripts now clone vcpkg to `<repo root>/vcpkg` and run its
  bootstrap script automatically when neither `VCPKG_ROOT` nor an
  existing checkout is found, rather than stopping with instructions to
  do it by hand. Requires `git` on `PATH` -- fails with a clear message
  if missing, same as every other missing-tool case these scripts
  already handle.
- **Interactively verified end-to-end**: cleared `VCPKG_ROOT`, confirmed
  no `vcpkg/` checkout existed at the repo root, ran `build.ps1` fresh --
  it cloned vcpkg, bootstrapped it (`vcpkg.exe` produced), then
  configured (skipped, already configured from a prior checkout) and
  built successfully, all in one invocation with zero manual steps.
  Re-ran once more with `VCPKG_ROOT` still unset to confirm the
  now-existing checkout is auto-detected on a second run without
  re-cloning.
- Updated `src/README.md` to lead with "no manual setup required" and
  moved the manual vcpkg bootstrap steps into the "manual control"
  section as an alternative, not a prerequisite.
- Files: `src/build.ps1`, `src/build.sh`, `src/README.md`, `AGENTS.md`

## [2026-08-11] — fix(src): complete DPI awareness (#31 reopened scope)

- Issue #31 addressed on GitHub — the reopened, expanded scope: hand-drawn
  chrome scaling, the clipped title-bar buttons, panel clipping under the
  toolbar, and live monitor-to-monitor rescaling.
- Added `src/gui/UiScale.{h,cpp}`: one published DPI/content scale
  (`gui::UiScale()` / `gui::Scaled()`) that every hand-drawn `ImDrawList`
  glyph in the app now multiplies its pixel constants by. Converted
  `IconToolbar.cpp`, `CustomTitleBar.cpp`, `PanelIcons.cpp`, and
  `ViewportPanel.cpp`'s UCS icon + plane-offset overlay.
- `main.cpp`: consolidated all scale consumption into one `ApplyUiScale()`,
  added a per-frame `glfwGetWindowContentScale()` poll so dragging the
  window between different-DPI monitors rescales live, switched fonts to
  Dear ImGui 1.92's `style.FontScaleDpi` (dynamic re-rasterisation) instead
  of a startup-baked font atlas, and added an `ORCISF_UI_SCALE` override so
  high-DPI layout is testable on a 100%-only machine.
- **Two real bugs found by actually driving the app rather than by code
  review**, both documented in full in `AGENTS.md`'s #31 section:
  - The title-bar button cluster was positioned with
    `ImGui::SameLine(offset_x)`, whose offset is relative to the content
    origin and therefore silently added the menu bar group's
    `WindowPadding.x` — pushing the cluster past the window's right edge
    (14px at 100%, 28px at 200%). This is the true cause of the report
    closed as "not reproducible" under #27; that closure was wrong and
    `AGENTS.md`'s #27 section now says so.
  - Re-running `ApplyModernTheme()` before `ScaleAllSizes()` is not a
    style reset: the theme assigns ~15 fields while `ScaleAllSizes()`
    scales dozens, so `WindowMinSize`/`CellPadding`/`DockingSeparatorSize`/
    … compounded on every rescale. Symptom: docked panels permanently lost
    their tab bars after a 100% → 200% → 100% round trip. Fixed by
    snapshotting and restoring the whole `ImGuiStyle` struct.
- Also: the window title now elides to fit its drag zone (it previously
  grew until it overlapped the window buttons at high scale);
  `IconToolbar::kHeight` became `IconToolbar::Height()` so the dockspace
  work-area reservation tracks the real scaled row height; `BuildDockspace()`
  seeds the dock builder from `WorkPos`/`WorkSize` rather than the full
  viewport.
- Verified with DPI-aware screenshots at 100%/150%/200% and across four
  live 100%↔200% round trips (temporary key-driven toggle, since removed).
  Not verified: a real two-physical-monitor drag, and macOS/Linux.
- Files: `src/gui/UiScale.h`, `src/gui/UiScale.cpp`, `src/app/main.cpp`,
  `src/app/CustomTitleBar.cpp`, `src/app/Application.cpp`,
  `src/gui/IconToolbar.h`, `src/gui/IconToolbar.cpp`,
  `src/gui/PanelIcons.cpp`, `src/gui/ViewportPanel.cpp`,
  `src/CMakeLists.txt`, `src/README.md`, `AGENTS.md`

## [2026-08-11] — chore(src): tester pass on #31, issue closed

- `tester` skill ran against issue #31's 5 acceptance criteria using a
  fresh `windows-release` build of commit `96c343a` (the DPI-awareness
  fix above).
- **PASS (with measured evidence)**: hand-drawn chrome scaling (icon
  toolbar/UCS icon/panel icon headers), title-bar button visibility and
  alignment (Close-button glyph-vs-cell centering measured at -0.5px/
  0px/0px across 100%/150%/200%), no panel clipping under the toolbar/
  menu bar at any tested scale.
- **PASS (mechanism)**: live rescale with no compounding. Verified
  through the real `ApplyUiScale()`/per-frame-poll code path via a
  temporary F9 test hook in `main.cpp`, exercised across two full
  100%<->200% round trips, then reverted with `git checkout` (confirmed
  empty diff) and rebuilt before finishing -- the tree was clean at the
  same commit before and after.
- **UNVERIFIED**: the fifth criterion (verified on the actual
  dual-monitor Windows dev environment, live drag between two
  different-DPI monitors) -- this environment has one physical monitor
  (confirmed via `Screen.AllScreens`), so that specific hardware test
  could not be run here.
- Reported all 5 findings individually (not averaged into one verdict)
  and recommended leaving #31 open pending a real dual-monitor check;
  the user reviewed the breakdown and explicitly chose to close the
  issue anyway.
- Files: `AGENTS.md`, `CHANGE_HISTORY.md` (this entry); no source changes
  (the temporary test hook was reverted, not committed).

## [2026-08-11] — fix(src): move panel icons onto the dock tab button (#28 correction)

- Issue #35 created on GitHub, correcting a misimplementation from issue
  #28 Part 2: the panel icon was drawn as a new header row inside each
  panel's content (duplicating the title already shown on the tab
  button), not on the tab button itself as originally requested. The
  user looked for the original request under #13 and couldn't find it --
  it was never there; the actual implementation is under #28.
- Scope: `src/gui/PanelIcons.{h,cpp}` + the 7 panels that call
  `DrawPanelIconHeader()` (Viewport, Properties, Loads, Joints/Members,
  Detailing, Run Optimization, Log).
- Labels: bug, gui
- Note: two throwaway issues (#33, #34) were accidentally created while
  debugging a curl/JSON payload path issue during this creation -- both
  closed immediately as not-planned with an explanatory comment.

## [2026-08-11] — feat(src): split Joints/Members panel, order Loads after it

- Issue #36 created on GitHub -- prerequisite for #37: split the combined
  "Joints/Members" panel (issue #21) into two independent panels, and
  make Loads' tab default to immediately after Members in tab order.
- Scope: `src/gui/JointsMembersPanel.{h,cpp}`, `src/app/Application.{h,cpp}`
- Labels: enhancement, gui

## [2026-08-11] — feat(src): View menu restructure + close-button fix + rename

- Issue #37 created on GitHub, depends on #36: (1) fix a reported bug
  where a docked panel's tab close button doesn't actually close the
  panel, (2) restructure the View menu into Menubar (per-icon-toolbar-
  button visibility) / Subwindows (per-panel show/hide) / Layout
  (existing Default/Design/Optimization presets) sections, (3) rename
  the "Run Optimization" panel to "Optimization" everywhere the literal
  string appears.
- Scope: `src/gui/Toolbar.{h,cpp}`, `src/gui/RunPanel.cpp`,
  `src/app/Application.{h,cpp}`, `src/gui/IconToolbar.{h,cpp}`
- Labels: bug, enhancement, gui

## [2026-08-11] — fix(src): implement issue #35 -- panel icons on dock tab button

- Corrects #28 Part 2: each panel's hand-drawn icon now renders directly
  on its own dock tab button, immediately before the title text, instead
  of as a duplicate-title content-row header inside the panel body
  (`DrawPanelIconHeader()`, removed from all 7 panels).
- New `gui/PanelTitles.{h,cpp}`: every panel's `ImGui::Begin()` title is
  now `<N spaces><label>###<stable_id>` -- the leading spaces (recomputed
  every frame from live font metrics) reserve room for the icon overlay
  without ever changing the window's dock identity, since Dear ImGui's
  `"###"` marker excludes everything before it from the ID hash.
- New `app/DockTabIcons.{h,cpp}`: walks each panel's dock tab (via
  `imgui_internal.h`'s `ImGuiDockNode`/`ImGuiTabBar`, the same
  semi-public "Docking Builder" internals `Application.cpp` already used
  for issue #15) and draws its icon at the tab's actual label position.
- `PanelIcons.cpp`'s icon-drawing functions now take an explicit `size`
  parameter instead of a fixed 16px constant, so the same shapes can be
  reused at whatever size a dock tab needs.
- Verified via real screenshots at 100% and 200% `ORCISF_UI_SCALE`: all 7
  panels' icons render correctly positioned with no text overlap at both
  scales; the three view-layout presets' dock placement is unchanged
  from before this fix.
- Files: `src/gui/PanelIcons.{h,cpp}`, `src/gui/PanelTitles.{h,cpp}`
  (new), `src/app/DockTabIcons.{h,cpp}` (new), `src/app/Application.{h,cpp}`,
  `src/gui/ViewportPanel.cpp`, `src/gui/PropertiesPanel.cpp`,
  `src/gui/LoadsPanel.cpp`, `src/gui/JointsMembersPanel.cpp`,
  `src/gui/DetailingPanel.cpp`, `src/gui/RunPanel.cpp`, `src/gui/LogPanel.cpp`,
  `src/CMakeLists.txt`, `AGENTS.md`

## [2026-08-11] — feat(src): implement issue #36 -- split Joints/Members panel

- `JointsMembersPanel` (issue #21) split into two independent panels:
  `gui/JointsPanel.{h,cpp}` (joints table + cascade-delete confirmation
  modal, unchanged logic) and `gui/MembersPanel.{h,cpp}` (members table,
  unchanged logic) -- each its own dockable/closeable window.
- Reordered the shared dock group's tab order to Joints, Members, Loads
  (previously Loads, Joints/Members). Found via screenshot that Dear
  ImGui's displayed tab order actually follows each panel's `Draw()`/
  `Begin()` call order within the frame, not the
  `DockBuilderDockWindow()` call order used to set up the persistent
  dock/node association -- an assumption in the first attempt that
  turned out wrong, corrected after seeing the mismatch.
- `PanelIcon` enum's `JointsMembers` value split into `Joints`/`Members`,
  each with its own hand-drawn icon (three dots vs. a line with
  unfilled end-caps) so the two dock tabs read as visually distinct.
- Verified interactively with real data (not just the empty state): a
  scratch blank structure, two joints added, connected into a member --
  both new panels' tables correctly showed the live data, and the tab
  order matched Joints/Members/Loads/Log exactly.
- **Not verified this pass**: the cascade-delete confirmation modal
  (multiple precisely-aimed synthetic clicks on the table's per-row
  "Delete" button had no effect, root cause not found) -- this was
  already unverified before the split (see `AGENTS.md`'s #21 section),
  so it's not a new regression, but remains open follow-up work.
- Files: `src/gui/JointsPanel.{h,cpp}` (new), `src/gui/MembersPanel.{h,cpp}`
  (new), `src/gui/JointsMembersPanel.{h,cpp}` (deleted), `src/gui/PanelIcons.{h,cpp}`,
  `src/gui/PanelTitles.{h,cpp}`, `src/app/Application.{h,cpp}`,
  `src/app/DockTabIcons.cpp`, `src/CMakeLists.txt`, `AGENTS.md`

## [2026-08-11] — fix/feat(src): implement issue #37 -- close-button fix, View menu restructure, rename

- **Real bug fixed**: docked panels' tab close buttons flipped the panel's
  own open flag correctly but the tab never actually disappeared, because
  `Application::OnFrame()` called every panel's `Draw()` unconditionally
  every frame, trusting `ImGui::Begin(name, open)` to no-op once `*open`
  went false -- true for floating windows, not for Dear ImGui's docked
  tab-bar bookkeeping in this version. Root-caused via two temporary debug
  probes (io.MousePos/GetHoveredID() per click; a before/after snapshot of
  every panel's open flag around the Draw() calls), both reverted before
  commit. Fixed by gating each panel's `Draw()` call on its own open flag
  instead. Verified on two different panels (Viewport, Log) in two
  different dock groups; also confirmed the bug predates and is unrelated
  to #35's dynamic window-title mechanism (reproduced identically with a
  fully static title).
- **View menu restructured** into three sections (Toolbar.cpp's `View`
  menu): **Menubar** (new `gui/IconVisibility.h`, 8 checkable toggles for
  IconToolbar's buttons -- hidden buttons reflow cleanly, no gap), **
  Subwindows** (new `gui/PanelVisibility.h`, 8 checkable toggles bound
  directly to each panel's open-state bool -- doubles as how a closed
  panel gets reopened), **Layout** (the pre-existing #15 presets, moved
  from flat top-level items into their own submenu, unchanged otherwise).
- **Renamed** the "Run Optimization" panel's display text to
  "Optimization" (`RunPanel.cpp`'s window title + `ViewportPanel.cpp`'s
  hint text) -- `gui::kRunOptimizationId` (the dock/window identity) is
  unchanged, so no dock-layout code needed touching.
- Verified interactively via real screenshots: close-button fix on two
  panels; the full View > Subwindows close -> reopen cycle for Log (tab
  disappears, then reappears in its correct #36-ordered position with
  content intact); Menubar's New Data toggle hiding/reflowing the icon
  row; Layout's three presets still switching correctly; the
  "Optimization" rename showing correctly everywhere.
- Files: `src/app/Application.{h,cpp}`, `src/gui/Toolbar.{h,cpp}`,
  `src/gui/IconToolbar.{h,cpp}`, `src/gui/IconVisibility.h` (new),
  `src/gui/PanelVisibility.h` (new), `src/gui/RunPanel.cpp`,
  `src/gui/ViewportPanel.cpp`, `AGENTS.md`

## [2026-08-12] — chore(docs): compact AGENTS.md, move debugging narratives here

`AGENTS.md` had grown to ~2400 lines, with many per-issue "read before
touching" sections mixing durable rules/gotchas with multi-paragraph
narratives of how a bug was diagnosed, what automation attempts failed,
and session-by-session "what was verified interactively" writeups. Per
its own Change Log Policy, condensed each section to current-state rules
only and moved the process narrative here as dated entries below,
inferring dates from context. No factual/technical content changed --
this is a reorganization pass only.

### Issue #8 (reinforcement detailing) — verification narrative, 2026-08-10
`BuildDetailingDrawing()`'s geometry (bar counts, in-bounds positions,
tension/compression face per beam region, column bar count formula) was
unit-tested standalone before GUI wiring. The full rendering pipeline was
then verified interactively against a real optimization run of
`Example/Apl1-1` (screenshotted): a column's drawing (12D25 bars, correct
spacing/cover/stirrup labels) and a beam's drawing (both Tumpuan and
Lapangan sections, correct tension-face flip, all labels) exactly matched
the same run's `orcisf_cli`/`MEMBER_RESULTS` numeric output.

### Issue #9 (PDF + text export) — verification narrative, 2026-08-10
`WriteStructureFile()`/`WriteDiscreteTables()`'s round-trip and
`WritePdfReport()`/`WriteTextExport()` were exercised standalone against a
real completed `Example/Apl1-1` run: PDF generated/opened/screenshotted
page-by-page (cover/input summary, per-member table matching
`MEMBER_RESULTS` exactly, correct Tumpuan/Lapangan detailing pages); text
export produced all 12 files at the right sizes. The full GUI path was
then verified separately through the actual native NFD dialogs (not
simulated): "Export PDF..." opened a real Save-As dialog and produced a
byte-identical file to the standalone test; "Export Text..." opened a
real Select-Folder dialog and produced the same 12-file set. Round-trip
was checked by re-reading the exported dataset with
`orcisf_cli info`/`equilibrium` -- identical geometry/topology, perfect
(0.0) equilibrium residual.

### Issue #6 (interactive 3D editor) — verification narrative, 2026-08-10
Joint/member picking, numeric position entry live-updating the 3D view,
the Restrained checkbox, Delete Joint, Add Joint, Undo, and the
Validation panel were all exercised end-to-end (mouse/keyboard input
synthesized via Win32 API + `AttachThreadInput` to work around Windows'
foreground-focus protections, screenshotted to confirm). An actual
click-drag on the ImGuizmo gizmo's arrows was not attempted (sub-pixel
axis targeting judged too unreliable to script); the gizmo was confirmed
to render at the correct position and `Manipulate()`'s output feeds the
same verified `EditableStructure::MoveJoint()` call, so remaining risk is
isolated to ImGuizmo's own hit-testing.

### Issue #7 (load input GUI) — verification narrative, 2026-08-10
The full load-editor workflow was exercised end-to-end against a real
scratch copy of `Example/Apl1-1` (screenshotted at each step): existing
`.bbn` loads (35000 N/m on 4 beams) loaded and matched `LoadsPanel`'s
table exactly; placing a new joint load via Loads > Add Joint Load + a
viewport click updated Properties/`LoadsPanel` in sync; File > Save Loads
(.bbn) was actually clicked and the resulting file read back and diffed
-- byte-for-byte matching the GUI state. `ReadLoadsRaw()`/`WriteLoads()`
round-trip fidelity and `SetMemberLoad()`'s UDL fixed-end-force formula
were also unit-tested standalone.

### Issue #11 (New Data/Open Data, title/material form, .inf preview) — verification narrative, 2026-08-11
Build/launch confirmed via `Get-CimInstance`. `WriteInfPreview()`'s
output was verified standalone against `Example/Apl1-1`: wrote a fresh
`.inf` to a scratch path and diffed every section against the checked-in
`aplikasi.inf` -- numerically identical (only whitespace/column-alignment
differs). A follow-up interactive pass (synthesized Win32 input +
screenshots) confirmed `New Data` opening the real native Save-As dialog,
saving actually writing all 7 files, and the General section rendering
and being live-editable (typed a title, confirmed it round-tripped
through `File > Save` to the `.inp` file on disk). This pass hit a
DPI-awareness pitfall and an unrelated-window-focus hazard along the way,
and caught a real bug (documented in `AGENTS.md`'s #11 section): `Ctrl+S`
was display-only because `ImGui::MenuItem`'s shortcut-text parameter
doesn't bind the key chord by itself.

### Issue #14 (icon toolbar) — verification narrative, 2026-08-11
Verified interactively (synthesized Win32 input + screenshots): all 8
icons render with correct disabled/enabled dimming, hover tooltips work
(confirmed "Connect Joints" appearing on hover), and clicking "Add Joint"
correctly started a blank structure and added a joint (Properties panel
updated, Save/Undo icons became enabled immediately after, Redo/Run
correctly stayed disabled).

### Issue #15 (view layout presets) — verification narrative, 2026-08-11
Verified interactively: added a joint, switched through all three presets
and back to Default -- the joint/selection/undo history were all still
there throughout. All three presets render with the correct panel
arrangement, the View menu's checkmark correctly tracks the active
preset, and switching Design -> Optimization -> Default round-trips back
to the original layout pixel-for-pixel.

### Issue #25 (Run panel dataset-path / timestamped output) — verification narrative, 2026-08-11
Compiled cleanly (all three targets) and empirically exercised via
`orcisf_cli optimize` against a scratch copy of `Example/Data01`:
confirmed a real `<folder>/<timestamp>/` subfolder was created containing
the run's six output files, and via checksum that the dataset's own
top-level `.opt` file was byte-identical before and after -- genuinely
untouched. The built GUI was launched and confirmed running without
crashing after the `RunPanel` UI changes; the dataset-path-sync/Run-panel
display itself was not click-tested interactively.

### Issue #26 (app icon set) — verification narrative, 2026-08-11
Compiled successfully on Windows -- confirmed the `.rc` resource actually
compiles (`orcisf.rc.res` generated) and links, runtime icon PNGs land
next to the built `.exe`, and the app launches/runs without crashing with
`ApplyWindowIcon()` wired in. The icon's actual on-screen appearance
(Explorer/taskbar/window) was not confirmed via screenshot in this pass
-- a screenshot attempt captured an unrelated foreground window instead,
a known focus-stealing hazard in this environment, not worth repeatedly
retrying for a cosmetic check. macOS bundle packaging and Linux
`.desktop`/install rules were written against each platform's documented
CMake convention but have no local toolchain to verify against
(Windows-only dev environment) -- CI is the first real cross-platform
check for those two.

### Issue #27 (title bar buttons "not flush") — full investigation narrative, 2026-08-11
First pass investigated and could not reproduce: the original report
(filed during the #21-#25 retest pass) was based on a screenshot that
mixed a DPI-aware `GetWindowRect()` P/Invoke call with .NET's
`System.Windows.Forms.Screen.Bounds`/`Graphics.CopyFromScreen()` in the
same PowerShell session -- the latter returned a logical (scaled-down)
screen size despite `SetProcessDPIAware()` having been called, so the
screenshot only captured roughly the top-left quarter of the true
physical screen. The window (genuinely maximized to the full physical
resolution) rendered its title bar buttons correctly at the true physical
right edge -- just outside that undersized capture -- which looked
exactly like "a gap before the window's true edge." Fix for the tooling:
don't rely on `Screen.Bounds` for capture dimensions; size the capture
bitmap explicitly from a DPI-aware `GetWindowRect()`/
`GetSystemMetrics(SM_CXVIRTUALSCREEN)` call. Re-verified this way (a
4x-zoomed crop of the true physical top-right corner): the button cluster
appeared flush and evenly spaced. A comment was posted on issue #27
recommending it be closed as "cannot reproduce"; not closed automatically.

**Correction (2026-08-11, during issue #31's second pass): the bug WAS
real, and the "not reproducible" conclusion was wrong.** The
DPI-measurement tooling problem was genuine and did produce a misleading
screenshot -- but there was also a real right-alignment defect
underneath it. The original claim "there was never a plausible
code-level mechanism" missed one: `ImGui::SameLine(offset_x)` measures
from the content origin and silently adds the enclosing group's
`GroupOffset.x`, which for a menu bar equals `WindowPadding.x` (14px at
100%, 28px at 200%) -- so the cluster really was shifted past the
window's right edge. Fixed under #31 by positioning with absolute screen
coordinates. Lesson: "the math looks right" is not the same as having
checked what the API's offset is actually relative to.

### Issue #28 Part 1 (Alt-mnemonics, reverted) — narrative, 2026-08-11
The original #28 implementation added `&File`/`&Edit`/etc. to
`Toolbar::Draw()`'s menu labels, assuming Dear ImGui would strip the `&`
and underline the following letter (the Win32/Qt/wxWidgets convention)
once `ImGuiConfigFlags_NavEnableKeyboard` was set. It does not --
confirmed via a real screenshot during issue #29's retest pass: the
literal `&` character was displayed in the menu bar, completely unparsed.
Reverted to plain labels and rebuilt/re-screenshotted to confirm the
revert fixed it. This was found while working issue #29 (the "retest
#21-#25" follow-up), not #28 itself -- #28 was already closed by the time
this was discovered; a comment was posted there cross-referencing this
correction and the new issue #30, per this project's transparency
conventions.

### Issue #28 Part 2 (panel icon headers) — narrative, 2026-08-11
Confirmed genuinely working via real screenshots, first for Viewport
(issue #29's retest pass), then re-confirmed on a fresh build plus two
more panels (Properties, Log) in a dedicated "build and test #28" pass:
the small hand-drawn icon rendered correctly before each panel's title
text in its content area. The remaining four panels weren't individually
screenshotted, since they call the exact same function -- three-for-three
confirmations across genuinely different panels was treated as
sufficient confidence. Superseded by issue #35 (2026-08-11): the user
clarified the actual ask was an icon on the panel's own dock tab button,
not a duplicate-title content row.

### Issue #30 (real Alt-mnemonic navigation) — verification narrative, 2026-08-11
Compiled cleanly via the `builder` skill. Interactive confirmation (Alt
held -> underlines actually appear in the right place; Alt+letter -> the
right menu actually opens) was not completed -- attempting to grab
foreground for screenshotting repeatedly landed on the user's own
actively-in-use browser window (reading this project's GitHub issue #29
in real time), not an unrelated background process. Correctly identified
as the user's live session rather than background interference, so
automation was stopped and their window state was restored rather than
continuing to fight for control of their active session.

### Issue #35 (panel icons on dock tab) — verification narrative, 2026-08-11
Compiled cleanly and confirmed via real screenshots at 100% and 200%
`ORCISF_UI_SCALE`: all 7 panels' dock tabs show their icon correctly
positioned immediately before the title text with no overlap, at both
scales; the three view-layout presets' dock placement is pixel-identical
to before the change; no panel body shows a duplicate title/header row
anymore. Tab close buttons were still visually present in every
screenshot -- whether clicking them actually closed the panel turned out
to be a separate bug, investigated under issue #37.

### Issue #29 (interactive retest of #25/#22/#24) — full three-attempt narrative, 2026-08-11

**Attempt 1**: Fixed and confirmed the #28 mnemonic regression, and
confirmed #28's panel-icon-header Part 2 genuinely works via screenshot
-- both via a corrected, DPI-aware screenshot technique combined with an
`AttachThreadInput`-based foreground-grab that reliably worked for
getting the app's main window foregrounded and its menu bar clickable.
Driving the native "Open Data..." folder-picker dialog to actually load a
dataset did NOT work: coordinate-based clicking inside the dialog
repeatedly mis-hit targets (clicked "New Data" instead of "Open Data..."
once; a "Cancel" click coordinate computed from a previous screenshot's
displayed/scaled pixel position without re-multiplying by the DPI scale
factor missed entirely). A new hazard was also hit: an unrelated app
(Proton Drive, the user's own cloud-sync client) spontaneously opened its
own window mid-sequence, stealing focus -- left untouched. Lesson:
always recompute click coordinates from the most recent screenshot taken
after any UI state change, in the same physical-pixel coordinate space.
Core objective remained unverified; not closed.

**Attempt 2**: Confirmed the app's own File menu is fully
keyboard-navigable (Down/Down/Enter reliably moved nav focus from "New
Data" to "Open Data..." and activated it) and that the native "Select
Folder" dialog does open correctly in response. Precisely diagnosed
(incorrectly, as attempt 3 later found) blocker: the "Select Folder"
dialog on this Windows install is the newer WinUI3/XAML-based Windows 11
file picker; its "Folder:" text-entry field appeared not to accept
synthetic input -- neither `SetCursorPos`+`mouse_event` clicks nor
`SendKeys::SendWait()` typed text ever populated the field across five
distinct attempts. Hypothesized this was because WinUI3/XAML controls
require real UI Automation (e.g. the Windows UI Automation COM API, or a
wrapper like FlaUI) to reliably receive focus and input from external
automation. Cancelled cleanly via `{ESC}` each time, no orphaned dialogs.
Core objective still unverified; not closed.

**Attempt 3** (resolved): The real root cause of every prior
coordinate-mismatch failure was found: the PowerShell process driving
`SetCursorPos`/`GetWindowRect` was never marked DPI-aware, so Windows
silently virtualized its coordinate space to logical pixels
(`GetWindowRect` reported `0,0,2560,1392` on this 200%-scaled monitor)
while screenshots were still being captured in physical pixels
(`5120x2784`) -- a 2x mismatch between where clicks actually landed and
where the screenshots said they should land. This fully explains the
WinUI3 dialog symptoms blamed in attempt 2 -- it was never really about
WinUI3/XAML resisting synthetic input, it was ordinary clicks landing at
literally the wrong pixel. Fix: call `SetProcessDPIAware()` once at the
start of the automation script, before any `SetCursorPos`/`GetWindowRect`
call. Confirmed by re-checking `GetWindowRect()` after the fix:
`0,0,5120,2784`, matching the screenshot dimensions exactly. With that
fixed, mouse-driven navigation through the WinUI3 dialog worked on the
very next attempt -- no UI Automation library needed after all. Both of
#29's acceptance criteria were then directly confirmed with a real
dataset loaded (`Example/Apl1-1` copied to
`C:\Users\IT\Documents\orcisf\Apl1-1\aplikasi`, loaded via File > Open
Data...): RunPanel dataset gating (#25) showed the Dataset field
populated and Run enabled; the 2D plane-lock offset overlay (#22/#24)
rendered correctly (View Plane > X-Y showed the "Plane X-Y -- Z offset
(m)" field and updated UCS icon), contrary to the user's report -- it was
never broken, just unreached because earlier attempts couldn't get a
dataset loaded. Actually typing a new numeric Z-offset value and
confirming it moves a placed joint was attempted but interrupted by a
WhatsApp desktop notification toast stealing keyboard focus mid-sequence;
automation was stopped immediately rather than risk interacting with
that toast, so this specific round-trip remains unconfirmed. Issue #29
resolved and closed.

### Issue #18 (AutoCAD-style Add Joint guidance) — verification narrative, 2026-08-11
Verified interactively: toggling Add Joint mode (button highlights,
tooltip and menu checkmark update, status-bar hint text appears);
clicking empty viewport space placed a joint exactly at the click
position (Properties panel showed "Joint 1" with correct
auto-calculated position); the mode stayed active after placement -- a
second click placed "Joint 2" without re-opening any menu. Connect
Joints' new first-click-vs-second-click hint text distinction was
code-reviewed but not re-verified live in this pass (focus-stealing
interference from an unrelated process interrupted that specific check).

### Issue #21 (Joints/Members panel) — verification narrative, 2026-08-11
The code follows `LoadsPanel`'s and `PropertiesPanel`'s already-proven
patterns (row selection sync, inline edit commit, `math3d::Vec3`
construction) exactly, and `EditableStructure`'s public API/field names
were cross-checked against `EditableStructure.h`/`.cpp` directly.
Compiled successfully in a later session once a build toolchain became
available (see issue #16's entry). Interactive exercise of the
cascade-delete modal specifically (does it actually pop up on a real
multi-member joint delete) was not re-checked.

### Issue #36 (split Joints/Members panel) — verification narrative, 2026-08-11
Compiled cleanly; real interactive data flow confirmed via a scratch
blank structure (Add Joint x2, Connect Joints to link them into a
member) -- `JointsPanel`'s table correctly showed both joints' live
X/Y/Z, `MembersPanel`'s table correctly showed the resulting member with
the right Joint A/B, and the tab order matched Joints/Members/Loads/Log
exactly as required, all via real screenshots. The cascade-delete modal
was NOT re-verified this pass -- multiple precisely-aimed automated
clicks on the Joints table's per-row "Delete" button (confirmed via a
crosshair-annotated screenshot to be landing within the button's visible
rect, and via `GetCursorPos()`/`GetWindowRect()` to rule out a
coordinate-space mismatch) produced no hover highlight and no click
effect at all, for a reason not root-caused in this pass. Given the
modal's code is byte-for-byte the code #21 already shipped with the same
unverified caveat, this isn't a new regression risk introduced by the
split, but remains open follow-up work.

### Issue #37 (close-button bug) — root-cause instrumentation narrative, 2026-08-11
Root cause was found by direct instrumentation, not by reading Dear ImGui
source (not available -- this project links a vcpkg-installed,
headers-only copy of ImGui, no `.cpp` to read). Two temporary debug
probes (reverted before commit, not left in the codebase) proved the
`if (open) Begin()` trust was misplaced specifically for docked windows:
(1) logging `io.MousePos`/`ImGui::GetHoveredID()` on every click confirmed
clicks landed exactly where intended (no DPI/coordinate mismatch -- an
early, wrong hypothesis); (2) a before/after snapshot of every panel's
own open-flag around the `Draw()` calls proved the tab close button's
click does correctly flip `viewport_open_`/etc. to `false` (confirmed on
both `Viewport` and `Log`, in two different dock groups) -- yet the tab
kept rendering on every subsequent frame anyway. This led to the fix
documented in `AGENTS.md`'s #37 section (gate each `Draw()` call on its
own open flag). Verification: close-button fix confirmed on two
different panels in two different dock groups; the full View >
Subwindows close -> reopen cycle for `Log` (tab disappears, then
reappears in its correct #36-ordered position with prior content
intact); View > Menubar's "New Data" toggle hiding/reflowing the icon
row; View > Layout's three presets still switching correctly; the
"Optimization" rename showing correctly in both the tab title and the
Viewport's hint text.

### Issue #22 (2D plane-locked drawing) — verification narrative, 2026-08-10
The cross-product/up-vector math was hand-derived and re-checked
(catching the Y-Z sign bug documented in `AGENTS.md`); the code was
reviewed against `Camera.h`/`.cpp`'s existing patterns
(`Pan()`/`ScreenRay()`'s `Cross(right,forward)` up-derivation,
`EyePosition()`'s `target - Forward()*distance`). Compiled successfully
in a later session. The three plane views' actual on-screen rendering
and click-to-place-on-the-locked-plane behavior were not interactively
exercised in a running app.

### Issue #23 (UCS icon overlay) — verification narrative, 2026-08-11
The dot-product projection math was hand-checked for a few reference
orientations (default orbit camera: X should point right-ish, Y up, Z
out-of-screen-ish) and the code reuses already-verified `Camera` methods
rather than new vector math. Compiled successfully and the app was
launched and confirmed running without crashing. The icon's live
rotation-with-camera behavior was not interactively click-tested
pixel-by-pixel.

### Issue #24 (plane-offset control moved into viewport) — verification narrative, 2026-08-11
Compiled cleanly and the app was launched and confirmed running (process
alive, no crash) after the change. Not interactively click-tested --
specifically, whether dragging the slider/typing in the field actually
avoids triggering orbit, and whether a placed joint's locked coordinate
exactly matches a nonzero slider value, are the issue's own stated
acceptance criteria and still need a real interactive pass.

### Issue #19 (custom borderless window chrome) — verification narrative, 2026-08-11
Borderless window (no OS title bar/border) confirmed; title text renders
in the menu bar row; Minimize/Maximize/Close icons render correctly
including the Maximize<->Restore icon swap; clicking Maximize actually
maximized the window (screen-filling, confirmed via screenshot) and
clicking Maximize again restored it; the drag mechanism was confirmed
working emphatically -- an in-progress click sequence during testing
produced a real (if unintentionally large) window-position delta that
moved the live window off-screen, which was then recovered with a plain
Win32 `SetWindowPos` call (not a code change) to bring it back on
screen. macOS/Linux were not interactively verified in this environment
(no access to those platforms).

### Issue #16 (re-optimize from last best result) — verification narrative, 2026-08-11
Compiled cleanly (both `orcisf_cli` and `orcisf_gui`) and empirically
exercised via a standalone scratch program linked against
`orcisf_engine` (not checked in) against a scratch copy of
`Example/Data01`: a truncated baseline run (`harga=3.02672e+07
kendala=0`) followed by a seeded re-optimization (`harga=2.46519e+07
kendala=0`) -- lower cost, confirming the seed took effect and the
search genuinely continues rather than restarting. Re-running the
seeded case with `worker_threads=4` produced a bit-identical result to
`worker_threads=1`, confirming issue #4's determinism guarantee holds
with seeding active. The GUI checkbox/wiring itself was reviewed but not
interactively clicked through in a running app this session.

### Issue #31 (DPI awareness) — verification narrative, 2026-08-11
Compiled cleanly and visually confirmed via real DPI-aware screenshots
at 100%, 150% and 200%: toolbar/panel icons scale, the title bar buttons
are fully visible and correctly centred (measured -- the Close glyph
sits exactly half a button width from the true right edge at both 100%
and 200%), the title elides instead of colliding with the buttons, and
no docked panel is clipped under the menu bar/toolbar. Live rescaling
was exercised through the real per-frame path via a temporary
key-driven scale toggle (since removed) across four consecutive
100%<->200% round trips, screenshotted each time: the final 100% state
is identical to a fresh 100% launch, confirming no compounding. A
`tester` pass (2026-08-11) confirmed 4 of 5 acceptance criteria with
direct evidence and closed the issue on the user's explicit call:
criteria 1-3 were re-verified with fresh screenshots at 100%/150%/200%
via `ORCISF_UI_SCALE`, plus a pixel-level measurement of the Close
button's glyph-vs-cell centering (offset -0.5px/0px/0px across the three
scales -- sub-pixel); criterion 4 was re-exercised through the real
`ApplyUiScale()`/per-frame-poll path via a temporary F9 test hook
(reverted with `git checkout` and rebuilt before finishing -- confirmed
empty diff), two full 100%<->200% round trips, screenshotted: the final
100% frame is visually identical to a fresh 100% launch. Criterion 5
(verified on the actual dual-monitor Windows dev environment, live drag
between two different-DPI monitors) was explicitly reported UNVERIFIED
-- this environment has one physical monitor (confirmed via
`Screen.AllScreens`), so the specific hardware test could not run here.
The user closed the issue anyway, on the strength of criteria 1-4 plus
the shared mechanism.

## 2026-08-12 — Log tab regrouped with Properties/Optimization (Default/Design presets)

- Issue #38 created on GitHub
- Scope: `src/app/Application.cpp` (`BuildDefaultLayout()`/`BuildDesignLayout()`)
- Labels: enhancement, gui
- Requested: in the **Default** preset, tab `Log` into the right-side
  `Properties`/`Optimization` group (after Optimization) instead of the
  bottom `Joints`/`Members`/`Loads` strip; in the **Design** preset, tab
  `Properties`, `Optimization`, `Log` together in that order (Optimization
  right after Properties, Log right after Optimization), replacing the
  current split where Optimization+Log sit in a separate `dock_bottom_right`
  node from Properties.

## 2026-08-12 — Editable member type/joint endpoints + Add Member/Add Joint buttons + viewport labels

- Issue #39 created on GitHub
- Scope: `src/gui/MembersPanel.cpp`, `src/gui/JointsPanel.cpp`, `src/gui/editor/EditableStructure.cpp`, `src/gui/viewport/`
- Labels: enhancement, gui
- Requested: Members panel gets a per-row member-type dropdown and editable
  Joint A/Joint B fields, plus a `+` Add Member button (top-right, above the
  header) that inserts an empty editable row; Joints panel gets the same `+`
  Add Joint button pattern for typing new joint coordinates directly into a
  new row; both joint numbers and member numbers should render as labels in
  the 3D viewport for cross-referencing against the tables.

## 2026-08-12 — Per-DOF joint restraint editing (6 checkboxes + quick-support buttons)

- Issue #40 created on GitHub
- Scope: `src/gui/PropertiesPanel.cpp`, `src/gui/editor/EditableStructure.cpp`, `src/gui/viewport/`
- Labels: enhancement, gui
- Requested: replace the Properties panel's single fixed/free restraint
  checkbox with 6 independent per-DOF checkboxes (UX/UY/UZ/RX/RY/RZ) plus
  4 quick-support preset buttons (Fixed, Pinned, Roller, Free), and show a
  joint's restraint type in the viewport when clicked. This is the
  per-DOF refinement `AGENTS.md`'s issue #6 notes explicitly anticipated
  as a future follow-up (the single-toggle behavior was a deliberate
  scope cut, not an oversight).

## 2026-08-12 — Row-selection Selectable covers editable input cells in Joints/Members/Loads tables

- Issue #41 created on GitHub
- Scope: `src/gui/JointsPanel.cpp`, `src/gui/MembersPanel.cpp`, `src/gui/LoadsPanel.cpp`
- Labels: bug, gui
- Reported: the per-row `Selectable(SpanAllColumns)` row-selection highlight
  looks undersized versus the actual row height, and clicking near the top
  of a row only selects the row instead of activating the `InputFloat`/
  `InputText` cell underneath -- the user has to click further down before
  the input becomes editable. Affects all three panels sharing this pattern
  (#21/#36).

## [2026-08-12] — fix(src): implement issue #38 -- preset tab grouping fix

- Default preset: Log moved out of the bottom Joints/Members/Loads strip
  into the right-side node, now tabbed Properties -> Optimization -> Log.
- Design preset: removed the separate `dock_bottom_right` split; Properties
  (previously alone in its own column) is now tabbed together with
  Optimization and Log in one right-side node, same order.
- Tab order needed no `Draw()` call reordering in `OnFrame()` -- the
  existing call sequence already puts Properties/Run/Log in the right
  relative order (established convention from #36: `DockBuilderDockWindow()`
  call order doesn't control on-screen tab order, `Draw()` call order does).
- `BuildOptimizationLayout()` untouched, confirmed unaffected.
- Verified interactively via screenshots of all three presets switched
  through the View > Layout menu.
- Files: `src/app/Application.cpp`, `AGENTS.md`

## [2026-08-12] — fix(src): implement issue #41 -- Selectable click-blocking on editable table rows

- Root cause: `ImGuiSelectableFlags_SpanAllColumns` alone unconditionally
  claims every click across a row, at any Y, blocking later-column
  widgets (`InputFloat`) from ever receiving a click -- not a row-height
  mismatch as first assumed and shipped untested. Disproven by testing
  (multi-Y synthetic clicks + typed-value checks showed no effect from a
  height-only fix), then root-caused by adding direct
  `ImGui::GetHoveredID()`/`GetActiveID()` logging to the app's own
  `LogPanel` and, as a controlled diagnostic, temporarily removing
  `SpanAllColumns` entirely -- that alone restored click routing to the
  `InputFloat`.
- Real fix: `ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap`
  together (documented in `imgui.h` as letting subsequent overlapping
  widgets win hit-testing), plus an explicit `ImVec2(0, GetFrameHeight())`
  height kept for the cosmetic full-row highlight.
- Applied to all four `Selectable()` call sites: `JointsPanel.cpp` (one,
  canonical comment), `MembersPanel.cpp` (one, currently inert --
  pre-empts the same bug for issue #39's upcoming editable fields),
  `LoadsPanel.cpp` (two: member-loads and joint-loads tables).
- Verified via the same `ActiveID`/`HoveredID` instrumentation against
  `JointsPanel` (InputFloat activates and stays active after a direct
  click; the Joint-number column still selects the row transiently, and
  the viewport gizmo follows). `MembersPanel`/`LoadsPanel` got the
  byte-identical fix and a visual spot-check, not independently
  re-instrumented.
- All temporary debug logging (and its `#include <cstdio>`) removed
  from `Application::OnFrame()` before commit -- final binary has no
  diagnostic code in it.
- Files: `src/gui/JointsPanel.cpp`, `src/gui/MembersPanel.cpp`,
  `src/gui/LoadsPanel.cpp`, `src/app/Application.cpp`, `AGENTS.md`

## [2026-08-12] — feat(src): implement issue #39 -- editable member type/endpoints, Add Member/Add Joint, viewport labels

- `EditableStructure::SetMemberEndpoints()` (new): retargets a member's
  Joint A/Joint B, validating both are in range and distinct before
  touching `JJ`/`JK` (no-op, arrays untouched, on invalid input) --
  recomputes UDL fixed-end-forces for the new length.
- `EditableStructure::SetMemberTypeOverride()`/`GetMemberTypeOverride()`
  (new): a GUI-only per-member beam/column override (0/1/2 =
  Auto/Beam/Column), a new `std::array<int, kMak>` inside
  `EditableStructure`, shifted in lockstep with `JJ`/`JK`/`IA` on
  Add/DeleteMember. Deliberately NOT read by `engine::StructuralAnalysis`/
  `Optimizer` (both classify beam-vs-column purely from geometry
  throughout the ported algorithm) -- it only feeds `BuildSceneModel()`
  (new optional parameter) for display. `Validate()` now warns when an
  override disagrees with the member's actual geometry, since a Run
  always uses the geometry-derived type regardless of the override.
- `MembersPanel.cpp`: Type column is now an editable combo (Auto/Beam/
  Column); Joint A/Joint B are now editable `InputInt` cells wired to
  `SetMemberEndpoints()`; a new "+ Add Member" button (top-right, above
  the table) inserts a real member connecting the last two existing
  joints (disabled with a tooltip below 2 joints).
- `JointsPanel.cpp`: new "+ Add Joint" button (top-right, above the
  table) inserts a real joint at the current joints' centroid (origin if
  empty) -- an additional entry point alongside the existing toolbar/
  viewport-click Add Joint flow (#14/#18), not a replacement.
- `ViewportPanel.cpp`: new `DrawEntityLabels()` draws each joint's/
  member's number (`J1`, `M1`, ...) as hand-drawn `ImDrawList` text at
  its projected screen position (`proj * view * point`, perspective
  divide, clipped against the eye/NDC range) -- same overlay philosophy
  as the UCS icon (#23), but a real world-to-screen projection instead
  of direction-only dot products.
- Verified interactively end-to-end in this environment (screenshots at
  each step): placed 3 joints from a blank structure via viewport
  clicks (confirmed `J1`/`J2`/`J3` labels track each joint); Add Member
  created a member connecting joints 2/3 (confirmed `M1` label at its
  midpoint); setting its type override to Column updated the Properties
  panel title to "Batang 1 (Kolom)" and produced the expected geometry-
  mismatch validation warning; retargeting Joint A from 2 to 1 correctly
  reconnected the member and updated its computed length; typing an
  out-of-range joint number (99) was silently rejected with no change to
  the table, computed length, or viewport.
- Known scope limit (documented in AGENTS.md): the type-override array
  is not part of `UndoStack`'s `GeometrySnapshot`, so Undo/Redo can
  leave an override pointing at a shifted member index after a
  geometry-affecting undo.
- Files: `src/gui/editor/EditableStructure.{h,cpp}`,
  `src/gui/viewport/SceneModel.{h,cpp}`, `src/gui/MembersPanel.cpp`,
  `src/gui/JointsPanel.cpp`, `src/gui/ViewportPanel.cpp`,
  `src/app/Application.cpp`, `AGENTS.md`

## [2026-08-12] — feat(src): implement issue #40 -- per-DOF joint restraint editing

- `EditableStructure::SetJointDof()`/`GetJointDof()` (new): set/read a
  single `JRL` flag by DOF index (0..5 = UX,UY,UZ,RX,RY,RZ, matching
  `SetJointLoad()`'s `actions[6]` ordering). The old all-6-at-once
  `SetJointRestrained()` is unchanged and still backs `JointsPanel`'s
  existing single "Restrained" summary checkbox and `AddJoint()`'s
  free-by-default bootstrap.
- `PropertiesPanel.cpp`: the single "Restrained (fixed support)"
  checkbox is replaced with 6 independent UX/UY/UZ/RX/RY/RZ checkboxes
  plus four quick-support preset buttons (Fixed/Pinned/Roller/Free),
  both reading/writing the same `JRL` state so they can't drift out of
  sync.
- Deliberate correction from the issue's literal text: Roller restrains
  **UY** (vertical translation), not UZ -- this codebase's vertical axis
  is Y everywhere else (`Camera::WorldUp()`, `orcisf_cli`'s "arah 2 =
  Y"), and the issue's "Z assumed vertical" wording would only be
  correct under a Z-up convention this project doesn't use. Documented
  in AGENTS.md so it isn't "corrected" back by a future agent trusting
  the original issue text over the actual codebase convention.
- `ViewportPanel.cpp`: new `ClassifyRestraintPreset()` labels the
  selected joint's restraint state (`Fixed`/`Pinned`/`Roller`/`Free`/
  `Custom`) appended to its existing `J#` label from issue #39, reusing
  the same world-to-screen projection.
- Verified interactively end-to-end (screenshots): a fresh joint showed
  `[Free]`; clicking Pinned checked UX/UY/UZ and updated the label to
  `[Pinned]`, with `JointsPanel`'s old summary checkbox correctly
  becoming checked too; clicking Roller checked only UY (not UZ) with
  the label showing `[Roller]`; manually adding RX on top produced
  `[Custom]` as expected.
- Files: `src/gui/editor/EditableStructure.{h,cpp}`,
  `src/gui/PropertiesPanel.cpp`, `src/gui/ViewportPanel.cpp`, `AGENTS.md`

## [2026-08-12] — chore(src): tester pass on issues #35-#41

- Ran the `tester` skill against a freshly built `windows-release` binary
  (confirmed identical to the implementation-session build, `ninja: no
  work to do`) and checked every Acceptance Criterion for #35, #36, #37,
  #38, #39, #40, #41 against it -- see each issue's GitHub body for the
  exact checklist. Result: 35 of ~37 criteria PASS, 2 UNVERIFIED, 0 FAIL.
- **Real finding, not just confirmation**: #36's cascade-delete
  confirmation modal, which `AGENTS.md`'s #36 section had flagged as
  unverified/possibly broken ("precisely-aimed synthetic clicks on the
  Joints table's per-row 'Delete' button produced no effect, for a
  reason not root-caused"), now works correctly -- built two joints and
  a connecting member, clicked Delete on a joint with an attached
  member, got the "Delete Joint + Members" confirmation modal, confirmed
  it cascades and compacts indices correctly. Almost certainly a side
  effect of #41's `ImGuiSelectableFlags_AllowOverlap` fix (the Delete
  `SmallButton` sits in a column `SpanAllColumns` would have blocked
  the same way it blocked `InputFloat` cells), not a separate fix.
- #37's tab-close-button fix verified precisely: clicking a tab's own
  "x" (distinct from the dock-node-level "x" at the tab bar's far right,
  which closes the whole group) closes only that one panel; View >
  Subwindows correctly reopens it.
- #38's Design-preset grouping initially looked broken (Log missing from
  the Properties/Optimization group) -- traced to Log having been
  closed by an earlier close-button test in the same session, not a
  real layout bug; rechecking it confirmed correct grouping.
- #41's top-edge click criterion (the original reported symptom) was
  directly re-tested: double-clicking the very top pixels of a table
  row's `InputFloat` cell, immediately below the row-above's separator,
  correctly activated the field rather than just selecting the row.
- The one criterion this tester pass left UNVERIFIED (#35's DPI-scaled
  icon sizing at non-100% `ORCISF_UI_SCALE`, not checked automatically
  this session) was closed out the same day by the user, who manually
  tested it on a real high-DPI monitor at their office and confirmed
  icons resize correctly -- this matches what #35's own implementation
  session already found at 100%/200% via `ORCISF_UI_SCALE` (see
  `AGENTS.md`'s issue #35 section), now independently reconfirmed on
  real hardware. With this, every Acceptance Criterion across #35-#41
  is PASS (automated or user-confirmed) -- no FAIL, no remaining
  UNVERIFIED.
- Per this project's tester-skill convention, no remote issue was
  commented on, closed, or otherwise modified -- final QA/close-out is
  the `reviewer` skill's job, not `tester`'s.

## [2026-08-12] — chore(src): reviewer pass on #35-#41, all closed, #42 filed, v0.0.4-alpha released

- Ran the `reviewer` skill against #35-#41: architecture/design audit
  (patterns, scope discipline, regressions against documented
  "deliberate deviation" notes), bug/security audit, and a fresh
  end-to-end confirmation on a freshly built binary. Verdict: PASS WITH
  NOTES on all seven -- every Acceptance Criterion genuinely met, no
  regressions, no scope creep in any of the seven commits (`git show
  --stat` confirmed each touches only files relevant to its own issue).
- **Real finding, filed as a new issue (#42) rather than blocking this
  batch**: `engine::StructureData::JRL`/`AJ` are sized `kMak` (825) but
  every per-DOF accessor (`EditableStructure::AddJoint`/
  `SetJointRestrained`/`SetJointLoad`/#40's `SetJointDof`/`GetJointDof`)
  indexes them via `6 * joint_id - 5 + dof`, needing up to `6 * NJ`
  slots -- a latent out-of-bounds write past ~137 joints, via
  `LegacyArray::operator[]`'s unchecked `std::vector::operator[]`. This
  predates #35-#41 (the same indexing formula exists in the original
  1999 Borland source and in issue #6/#7's original per-DOF code) but
  is more easily reachable now that #39 added a one-click "+ Add Joint"
  button. No dataset in this repo comes close to 137 joints.
- Commented on and closed #35, #36, #37, #38, #39, #40, #41 on GitHub
  (each comment summarizes what was independently re-confirmed for that
  issue plus a pointer to #42).
- Filed #42 (`fix(src): JRL/AJ arrays sized kMak but indexed
  6x-per-joint -- latent OOB past ~137 joints`) with the finding above
  as its Objective/Acceptance Criteria.
- Released **v0.0.4-alpha** (GitHub Release, prebuilt Windows binary),
  the first release since v0.0.3-alpha (2026-08-11) to include all of
  #35-#41's changes.
- Files: `AGENTS.md`, `CHANGE_HISTORY.md` (no functional code changed
  by this pass itself).

## [2026-08-12] — fix(src): implement issue #42 -- JRL/AJ array-sizing fix

- Investigation before implementing found the real scope was bigger than
  #42's own text: not just `JRL`/`AJ` but 6 more fields
  (`ID`/`DF`/`AE`/`AC`/`AR`/`DJ`) share the identical bug -- all declared
  `LegacyArray<T>` (sized `kMak`=825) but indexed by global DOF number
  (up to `6*NJ`) inside `StructuralAnalysis.cpp`'s actual solver
  (`Hasil()`, stiffness assembly), confirmed by tracing every one against
  `StructuralAnalysis.cpp`/`LegacyIO.cpp`. `IM`/`AMD`/`T_K`/`LML` were
  checked too and don't need it (indexed by small fixed ranges, not
  DOF-range). This settled the issue's own open "resize vs. cap" question:
  capping GUI-added joints wouldn't fully fix it, since a hand-edited
  `.inp` file loaded directly bypasses any GUI cap and reaches the same
  solver code.
- Fix: new `engine::kMaxDof = 6 * kMak` constant; `LegacyArray`'s
  constructor now takes an optional `size` parameter (default `kMak`,
  unchanged for every other field); the 8 affected fields construct with
  `{kMaxDof}` via in-class initializers. Pure data-layout change -- every
  call site already used the correct `6*joint-5+dof` indexing formula,
  they were just writing into an undersized backing array.
- Real build issue hit and fixed: marking the new constructor `explicit`
  broke `engine::StructureData{}` aggregate-initialization under MSVC
  (`-std:c++20`) with "no appropriate default constructor available" --
  MSVC doesn't consider an `explicit` constructor eligible there even
  though it's usable with zero arguments via its default argument.
  Removed `explicit` (not needed here; nothing in this codebase relies on
  blocking implicit `int -> LegacyArray` conversions) and the build
  succeeded.
- Verified: a temporary standalone test (not checked in) constructed a
  `StructureData` with 200 joints (index up to 1200, past the old
  825-sized bound) and round-tripped values on all 8 fields correctly;
  `orcisf_cli info`/`equilibrium` against a real `Example/Apl1-1` dataset
  showed no regression (identical 0.015625 equilibrium residual); GUI
  smoke-tested (Add Joint, restraint label) with no crash.
- Files: `src/engine/include/engine/StructureData.h`, `AGENTS.md`

## [2026-08-12] — chore(src): tester pass on issue #42

- Ran the `tester` skill against a freshly built binary (`ninja: no work
  to do`, confirming it matches commit `fc68ffe`) and checked all 4 of
  #42's Acceptance Criteria independently, not by trusting the `coder`
  session's own claims:
  - Fix implemented (resize approach, `kMaxDof = 6 * kMak` applied to all
    8 fields) -- confirmed by reading the current source directly.
  - No other code assumes the 8 fields are `kMak`-sized -- independently
    grepped `LegacyIO.cpp`/`StructuralAnalysis.cpp`/`Optimizer.cpp`/
    `EditableStructure.cpp`; only `AddJoint()`/`AddMember()`'s guards
    reference `kMak`, and those correctly gate the joint/member-count
    arrays, not the resized DOF arrays.
  - Regression check -- wrote and ran an independent standalone test
    (not checked in, not reusing the coder session's already-deleted
    one) through the real `EditableStructure::AddJoint()`/
    `SetJointDof()`/`GetJointDof()` API at 300 joints (index up to 1800,
    vs. the coder session's 200/1200) -- all values round-tripped
    correctly. Independently re-ran `orcisf_cli equilibrium` against
    `Example/Apl1-1` -- identical 0.015625 residual, no regression.
  - Documentation -- confirmed present in `AGENTS.md`'s `engine/`
    section and this file.
  - Result: 4/4 PASS, 0 FAIL, 0 UNVERIFIED. One nuance flagged (not a
    fail): the "confirm with the user which approach" criterion was
    satisfied by explaining the (technically one-sided, not a genuine
    toss-up) resize-vs-cap reasoning to the user before implementing,
    not by pausing on an explicit yes/no first.
- Per user request, this write-back (this entry + the `AGENTS.md`
  Tracked Issues status update) is now applied automatically by the
  `tester` skill rather than proposed-then-confirmed each time --
  updated `.claude/skills/tester/SKILL.md` accordingly. Remote
  issue comments/closes are unaffected by this change and still require
  explicit confirmation.
- Files: `AGENTS.md`, `CHANGE_HISTORY.md`,
  `.claude/skills/tester/SKILL.md` (no functional src/ code changed by
  this pass itself).

## [2026-08-12] — chore: reviewer pass on #42, closed, v0.0.5-alpha released; reviewer skill now always releases

- Ran the `reviewer` skill against issue #42's real code change
  (`fc68ffe`). Architecture/design audit: clean, minimal, surgical diff,
  no scope creep. Bug/security audit: specifically re-checked the one
  workaround the fix's own commit flagged (dropping `explicit` from
  `LegacyArray`'s constructor to work around an MSVC aggregate-init
  quirk) -- grepped the whole codebase for any place that could exploit
  the now-implicit conversion (none found), and traced
  `Optimizer.cpp`'s worker-thread clone path (`workers.assign(...,
  sd)`, `SyncSearchState()`) to confirm issue #4's determinism guarantee
  is unaffected (the change only added a constructor parameter, never
  touched copy/move semantics). No findings. Verdict: **PASS**.
- Issue #42 was already closed (per direct user instruction, before
  this reviewer pass ran) -- no reopen/close action needed here.
- Released **v0.0.5-alpha** (GitHub Release + prebuilt Windows binary),
  the first release to include #42's fix.
- Per user request ("yes always", after confirming the release once),
  `reviewer`'s Step 8 no longer proposes-then-waits for release
  creation on a PASS/PASS WITH NOTES verdict -- it cuts the release
  automatically now. Issue close/reopen/comment actions are unaffected
  and still require explicit confirmation; updated
  `.claude/skills/reviewer/SKILL.md` accordingly.
- Files: `AGENTS.md`, `CHANGE_HISTORY.md`,
  `.claude/skills/reviewer/SKILL.md` (no other src/ code changed by
  this pass itself).

## [2026-08-12] — epic(docs): file algorithm-documentation epic + 5 sub-issues

- Filed epic #43 ("document all algorithms/formulas used in the
  application") plus five sub-issues, following this project's existing
  epic pattern (#1, #13, #20): #44 (Loads), #45 (Structural Analysis),
  #46 (Beam), #47 (Column), #48 (Optimization). Each requires citing
  both the legacy `Optimasi Beton/Source/*.hpp` implementation and the
  modern `src/engine/` port, per the epic's own acceptance criteria and
  the user's explicit confirmation that content should be code-cited
  rather than theory-only.
- Also synced 6 stale `Tracked Issues` rows found during the routine
  Step 1B check: #24, #25, #26, #28, #30, #32 were all already `closed`
  on GitHub but still showed `ready-for-review` in `AGENTS.md`.
- Files: `AGENTS.md`, `CHANGE_HISTORY.md`


## [2026-08-12] — feat(docs): implement algorithm-documentation epic #43-#48

- New `documentation/` folder (root), one page per lettered section, each
  opening/closing with a "🏠 Home" link back to `README.md#algorithms`
  (the new "Algorithms" section added there, indexing all five pages):
  - `loads.md` (#44) -- `.bbn` format, member-load fixed-end-force
    formulas, self-weight (beam-as-distributed-load vs.
    column-as-joint-point-load), and the `beban()`/`Beban()` global
    load-vector assembly.
  - `structural-analysis.md` (#45) -- cross-section inertia/torsion
    (including the Roark's-formula branch), member inclination
    ("kemiringan"/`periksa_batang()`), local-to-global stiffness matrix
    assembly + banded superposition, the modified-Choleski
    `banfac()`/`bansol()` solve, and full result recovery
    (`hasil()`: displacements, member forces, support reactions).
  - `beam.md` (#46) -- the lapangan/tumpuan dual-region design rationale,
    Whitney-stress-block flexural design, stirrup/shear design, and the
    cracked-transformed-section deflection check.
  - `column.md` (#47) -- reinforcement ratio/spacing, slenderness,
    Hulse & Mosley's equivalent-uniaxial biaxial P-M interaction method
    (including the false-position root-finding search for the neutral
    axis), and stirrup design.
  - `optimization.md` (#48) -- the discrete design-variable model
    (`JVD`), the fitness function, and the full Flexible Polyhedron
    driver loop (centroid search direction, discrete-step new-point
    search, replace-or-shrink, convergence), explicitly mapped against
    classic Nelder-Mead's reflect/expand/contract/shrink steps to show
    what's the same and what's adapted; a summary-level pointer to
    `AGENTS.md`'s already-authoritative issue #4 threading notes rather
    than duplicating them.
- Every formula cites both the original 1999 Borland source
  (`Optimasi Beton/Source/*.hpp`, exact line ranges) and the modern
  `src/engine/` port (exact file/function), per the epic's own
  requirement and the user's explicit "code-cited, not theory-only"
  confirmation when this epic was planned. Verified every cited legacy
  file/line range and every cited modern file exists and matches (spot-
  checked function-boundary lines directly against the source).
- This is documentation only -- no `src/` code changed, nothing to
  build or run; verification was reading the actual source line-by-line
  against what each page claims, not a compile/GUI check.
- Per this project's standing overnight-autonomous-work convention (see
  the 2026-08-11/2026-08-12 entries for issues #38-#42), issues #43-#48
  are left **open** for the user's own review rather than closed
  automatically.
- Files: `README.md`, `documentation/loads.md`,
  `documentation/structural-analysis.md`, `documentation/beam.md`,
  `documentation/column.md`, `documentation/optimization.md`,
  `AGENTS.md`

## [2026-08-12] — chore: close epic #43 and sub-issues #44-#48

- User reviewed the algorithm-documentation epic (`documentation/`
  folder, all five pages) and confirmed it's done -- commented on and
  closed #44, #45, #46, #47, #48, then the epic #43 itself.
- Files: `AGENTS.md`
