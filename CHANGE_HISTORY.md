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
