# AGENTS.md

> **READ THIS FIRST.** Every AI model that plans, reviews, or edits this repository
> — Claude, Gemini, Antigravity, GitHub Copilot, Cursor, and any other agent —
> **must read this file before doing anything else.** It is the single source of
> truth for the big picture, architecture, and working rules of the **ORCISF**
> project. After making any architectural or structural change, **you must update
> this file** so it stays accurate for the next agent.
>
> **Compaction rule:** Keep this file describing *current* architecture and rules.
> When it grows large or you record dated/implementation history, move the
> chronological detail into [`CHANGE_HISTORY.md`](CHANGE_HISTORY.md) and leave only
> the current state here. See **Change Log Policy** at the bottom.

---

## Big Picture

**ORCISF** (**O**ptimizing **R**einforced **C**oncrete **I**n **S**pace **F**rame
structure) is a **1998–1999 undergraduate thesis (Tugas Akhir S1) engineering
program** written by **Yohan Naftali** (student no. 7712/TS, Program Studi Teknik
Sipil, peminatan Struktur, **Universitas Atma Jaya Yogyakarta**; advisors
**Dr. Ir. FX. Nurwadji Wibowo, M.Sc.** and **Ir. Ch. Arief Sudibyo**). It is a
**DOS-style Win32 console application in Borland C++ 5.02** that:

1. **Analyzes** a 3D reinforced-concrete building frame (space frame) using the
   direct-stiffness method (adapted from Weaver & Gere), and
2. **Optimizes** the cross-section dimensions and reinforcement of every beam and
   column using the **"Flexible Polyhedron"** method — a generalized-simplex,
   direct-search (derivative-free) optimizer in the Nelder–Mead family — to
   **minimize total material cost** (concrete + reinforcing steel) subject to
   strength and serviceability constraints.

This repository is the **archive of that original thesis project**: source code,
compiled binary, sample datasets, and the full written manuscript (in Indonesian).
There is currently **no modern/ported implementation** — see [`src/` (reserved, empty)](#src-reserved-empty)
below.

```
User (console menu, Tampilan.hpp)
      │
      ▼
Input files (*.inp/*.isd/*.idl/*.ijl/*.ids/*.ijs/*.bbn)  ──┐
      │                                                    │
      ▼                                                    │
Struktur.hpp + Solver.hpp   (3D stiffness-method analysis) │
      │                                                     │
      ▼                                                     │
Kolom.hpp / Balok.hpp + Kendala.hpp  (RC design + cost/constraint "fitness")
      │                                                     │
      ▼                                                     │
Polyhedron.hpp + friends   (Flexible Polyhedron direct-search optimizer)
      │
      ▼
Output files (*.opt/*.str/*.kdl/*.inf/*.his)  ◄── Cetak.hpp
```

---

## Repository Layout

```
orcisf/
├── AGENTS.md                        # this file — architecture & working rules
├── CLAUDE.md                        # points every agent at AGENTS.md first
├── CHANGE_HISTORY.md                # dated project history
├── README.md                        # human-facing project overview
├── .claude/skills/                  # agent skills (see Agent Skills below)
├── .github/workflows/build-src.yml  # CI: builds src/ on Windows/macOS/Linux
├── src/                             # modern cross-platform GUI port (in progress) — see below
├── icons/                           # app icon set (issue #26 wires this into src/'s build --
│                                    # windows/orcisf.ico, macos/orcisf.icns, linux/+png/ PNG
│                                    # sizes, variants/ concepts; see icons/README.txt)
└── Optimasi Beton/                  # the original 1998-1999 thesis deliverable
    ├── BacaSaya.txt                 # original Indonesian README / user manual
    ├── orcisf.exe / orcisf.ico      # compiled Win32 console binary + icon
    ├── cw3230.dll                   # Borland C++ runtime DLL required by the .exe
    ├── Optimasi Struktur Beton.doc  # (2006) — EMPTY, see note below
    ├── Source/                      # Borland C++ 5.02 source (see below)
    ├── Example/                     # sample input/output datasets (see below)
    └── Teori/                       # full thesis manuscript, Indonesian, *.doc/pdf
        ├── Cover/                   # title page, preface, motto, validation sheet
        ├── Daftar/                  # table of contents, list of figures/tables/refs
        ├── Isi/                     # BAB I–V chapters + Intisari (abstract) + Proposal
        ├── Lampiran/                # 38 appendices (mostly source-code listings)
        ├── *.pdf                    # "Optimasi Polyhedron[.Naftali].pdf" summaries
        └── latex/                   # LaTeX transcription of the thesis — see below
```

> **`Optimasi Struktur Beton.doc` has no recoverable content.** Verified by
> both `antiword` and a manual parse of the Word binary FIB/piece-table
> structure (`ccpText = 1`, i.e. only the trailing paragraph mark; no
> embedded objects or images either) — this is not a guess. Despite the
> filename, don't treat it as a "condensed paper" source; the real thesis
> text lives in `Teori/Isi/*.doc`.

### `Teori/latex/` — LaTeX transcription of the thesis

A LaTeX build of the actual thesis manuscript (`Teori/Isi/BAB I.doc`
through `BAB V.doc`, `Intisari.doc`, `Cover/*.doc`, `Daftar/*.doc`),
produced by extracting text with `antiword` and reconstructing the
formulas that were embedded as MathType/Equation Editor OLE objects (not
plain text) by cross-referencing the surrounding prose against the actual
C++ implementation in `Source/*.hpp`. **Read
[`Teori/latex/LEGACY_TRANSCRIPTION_NOTES.md`](Optimasi%20Beton/Teori/latex/LEGACY_TRANSCRIPTION_NOTES.md)
before trusting or extending any specific number/formula in it** — it
documents exactly what was verbatim, what was reconstructed (and how), and
what could not be recovered at all (figures, the Tabel 2-1 stiffness
matrix image — reconstructed from the cited Weaver & Gere reference
instead). `main.tex` compiles cleanly with Tectonic 0.17.0 (`main.pdf` is
checked in); no `babel`/Indonesian language-pack dependency.

### `src/` — modern cross-platform GUI port (in progress, tracked by #1–#9)

A ground-up rewrite of the engine as a cross-platform (Windows 11/macOS/
Linux) desktop GUI, tracked as epic **#1** with sub-issues **#2–#9** (see
**Tracked Issues** below). It does not touch `Optimasi Beton/Source/` (the
legacy code stays a read-only historical reference). Full build/dependency
docs: [`src/README.md`](src/README.md).

**Stack** (ratified in #2): **Dear ImGui** (docking) + **GLFW** + OpenGL3
for the app shell/panels, **ImGuizmo** for 3D manipulation gizmos,
**ImPlot** for charts, **nativefiledialog-extended** for native file/folder
dialogs, **libharu (HPDF)** for PDF export — dependency-managed via
**vcpkg manifest mode** (`src/vcpkg.json`), built with **CMake presets**
(`src/CMakePresets.json`: `{windows,macos,linux}-{debug,release}`, driven
by `$env{VCPKG_ROOT}`). CI: `.github/workflows/build-src.yml` matrix-builds
all three OS targets on every push/PR touching `src/`.

**Current state (all of epic #1's sub-issues, #2 through #9, have landed --
see "Epic #1 complete" below for what that does and doesn't mean):**
```
src/
├── vcpkg.json / CMakeLists.txt / CMakePresets.json
├── app/
│   ├── main.cpp             # GLFW/OpenGL3/ImGui bootstrap + render loop;
│   │                        # also: gl3w init, NFD_Init/Quit; #19: sets
│   │                        # GLFW_DECORATED=false, calls ApplyModernTheme();
│   │                        # #26: calls AppIcon::ApplyWindowIcon(window)
│   ├── AppIcon.{h,cpp}      # #26: runtime taskbar/window icon via
│   │                        # glfwSetWindowIcon() + libpng (Windows/Linux
│   │                        # only -- no-op on macOS, see its own header)
│   ├── orcisf.rc            # #26: Windows .exe icon resource (icons/windows/orcisf.ico)
│   ├── CustomTitleBar.{h,cpp} # #19 Phase 0: title text/drag-zone/Minimize-
│   │                        # Maximize-Close, drawn *inside* Toolbar's menu
│   │                        # bar row (not its own window -- see the #19
│   │                        # note below for why), platform-agnostic GLFW
│   │                        # calls only, no native handles
│   ├── Theme.{h,cpp}        # #19 Phase 0: ApplyModernTheme(), dark-slate
│   │                        # ImGui style override (rounding/padding/colors)
│   └── Application.{h,cpp}  # docking layout (Viewport | Properties/RunPanel/Loads/Log);
│                            # owns loaded_sd_ (the one editable StructureData,
│                            # loaded via ReadDataset+ReadLoadsRaw -- not
│                            # LoadDatasetForViewing, see gui/editor/ note
│                            # below), the derived SceneModel, Selection,
│                            # UndoStack, EditorOptions; File > Open Folder...
│                            # / Save Loads (.bbn) (NFD), Add Joint / Undo /
│                            # Redo, and Run-completion wiring; #19: owns a
│                            # raw GLFWwindow* (SetWindow()) forwarded only
│                            # to CustomTitleBar::Draw()
├── gui/
│   ├── UiScale.{h,cpp}          # #31: the current DPI/content scale, published
│   │                            # by main.cpp and read via Scaled() by every
│   │                            # hand-drawn ImDrawList glyph in the app --
│   │                            # see the #31 note below before adding chrome
│   ├── Toolbar.{h,cpp}          # menu bar; File > Open Folder... (#5), the
│   │                            # Edit menu (#6: Undo/Redo, Add Joint, Connect
│   │                            # Joints, Snap to Grid), the Loads menu +
│   │                            # File > Save Loads (.bbn) (#7), and File >
│   │                            # Export PDF.../Export Text... (#9) are wired
│   ├── IconToolbar.{h,cpp}      # #14: icon-button row docked below the menu
│   │                            # bar (New/Open/Save/Undo/Redo/Add Joint/
│   │                            # Connect/Run) -- fixed curated set, hand-drawn
│   │                            # ImDrawList icons, no icon-font dependency
│   ├── ViewportPanel.{h,cpp}    # #5: offscreen OpenGL render of a SceneModel,
│   │                            # orbit/pan/zoom camera, click-to-pick a joint
│   │                            # or member; #6: ImGuizmo translate handle for
│   │                            # the selected joint, connect-mode clicking to
│   │                            # add a member; #7: load-placement-mode clicking
│   │                            # to place a default member/joint load
│   ├── PropertiesPanel.{h,cpp}  # #5: shows the selected member's dimensions +
│   │                            # (if available) demand/capacity/kendala;
│   │                            # #6: numeric X/Y/Z + restrained-toggle for a
│   │                            # selected joint, Delete Joint/Member buttons,
│   │                            # always-visible EditableStructure::Validate()
│   │                            # issue list (blocks a future Run when non-empty);
│   │                            # #7: numeric load fields (member W, joint
│   │                            # Fx..Mz) + Clear Load button
│   ├── LoadsPanel.{h,cpp}       # #7: load-schedule table -- one row per
│   │                            # member/joint with a nonzero raw load,
│   │                            # inline-editable, row click syncs Selection
│   ├── DetailingPanel.{h,cpp}   # #8: 2D reinforcement drawing (concrete
│   │                            # outline, bars, stirrups, dimension labels)
│   │                            # for whichever member is selected -- view-only,
│   │                            # docked as a "Detailing" tab alongside Viewport
│   ├── RunPanel.{h,cpp}         # #4: dataset/options form, worker-thread-count
│   │                            # slider, Run/Cancel, live progress bar -- runs
│   │                            # engine::RunFullOptimization on a background
│   │                            # std::thread so the UI never blocks; on success
│   │                            # (#5) hands the finished StructureData to
│   │                            # Application to refresh the viewport
│   ├── LogPanel.{h,cpp}         # run/status log -- functional; detailed
│   │                            # calc log to disk is done, see engine/
│   ├── viewport/                # #5: rendering/camera/scene-data, no ImGui
│   │   │                        # dependency except ViewportPanel.cpp itself
│   │   ├── Math3D.h             # minimal Vec3/Mat4 (no GLM dependency)
│   │   ├── Camera.{h,cpp}       # orbit camera (target/distance/yaw/pitch)
│   │   ├── SceneModel.{h,cpp}   # engine::StructureData (+ optional
│   │   │                        # MemberResult list) -> render-ready
│   │   │                        # joints/members/loads snapshot; PickMember()/
│   │   │                        # PickJoint() (ray-vs-segment/sphere, no GL
│   │   │                        # involved); #7 added MemberLoadVisual/
│   │   │                        # JointLoadVisual, read from sd's raw W/AJ
│   │   └── SceneRenderer.{h,cpp} # owns the FBO/shader/cube-mesh GL objects,
│   │                              # draws a SceneModel from a Camera into a
│   │                              # texture for ViewportPanel's ImGui::Image;
│   │                              # #7 added DrawArrow()/DrawLoads() (thin-box
│   │                              # shaft + small-cube head, fixed visual length)
│   ├── editor/                  # #6/#7: geometry+load editing, no ImGui/GL dependency
│   │   ├── Selection.h          # SelectionKind{None,Joint,Member} + EditorOptions
│   │   │                        # (connect-mode, snap-to-grid, #7's load_mode)
│   │   │                        # shared by ViewportPanel/PropertiesPanel/Toolbar
│   │   ├── EditableStructure.{h,cpp} # Add/Move/Delete joint, Add/Delete
│   │   │                        # member, restraint toggle, Validate() (#6);
│   │   │                        # SetMemberLoad/ClearMemberLoad/SetJointLoad/
│   │   │                        # ClearJointLoad (#7, raw self-weight-free
│   │   │                        # values) -- mutates an engine::StructureData&
│   │   │                        # in place
│   │   └── UndoStack.{h,cpp}    # GeometrySnapshot (X/Y/Z/JRL/AJ per joint,
│   │                              # JJ/JK/IA/W/AML per member -- NOT a full
│   │                              # StructureData copy) + push/undo/redo
│   └── detailing/                # #8: pure geometry, no ImGui/GL dependency --
│       └── DetailingLayout.{h,cpp}  # engine::MemberResult -> DetailingDrawing
│                                # (concrete outline, RebarCircle positions,
│                                # DimensionLabel text, local mm coordinates)
│                                # -- #9's PDF export reuses this exact layout
│                                # with a different renderer (libharu instead
│                                # of ImDrawList), which is the whole reason
│                                # the layout/render split exists
├── report/                      # #9: PDF + legacy-text export (named `report/`,
│   │                            # not the issue's suggested `/src/export` --
│   │                            # `export` is a reserved C++ word, can't be a
│   │                            # namespace identifier)
│   ├── TextExport.{h,cpp}       # writes the full legacy file set (.inp/.isd/
│   │                            # .idl/.ijl/.ids/.ijs/.bbn always; .opt/.str/
│   │                            # .kdl/.inf + copies of .his/.log.txt too if
│   │                            # a completed, not-since-edited run exists)
│   └── PdfExport.{h,cpp}        # HPDF-based report: cover/input summary, a
│                                # per-member summary table, and one detailing
│                                # page per member (reusing
│                                # gui::BuildDetailingDrawing() -- see above)
└── engine/                      # #3/#4/#5/#7/#8/#9: headless analysis/design/optimizer
    ├── README.md                # full architecture + validation writeup — read before touching this dir
    ├── include/engine/*.h, src/*.cpp   # one pair per ported legacy file, see README's table
    │   (MemberResults.h/.cpp is #5's addition, see below, extended by #8
    │   with reinforcement fields; LegacyIO's ReadLoadsRaw()/WriteLoads()
    │   are #7's; WriteStructureFile()/WriteDiscreteTables() are #9's --
    │   see their own notes below)
    └── tools/orcisf_cli.cpp     # headless CLI: `info`/`equilibrium`/`optimize [worker_threads] [rng_seed] [quiet]`
                                  # (optimize's MEMBER_RESULTS section also prints
                                  # reinforcement: bar counts/diameters/spacing)
```
`Toolbar`/`ViewportPanel`/`PropertiesPanel` had inert placeholders under
issue #2; #5 gave them their first real implementation (view-only: load,
render, inspect), #6 added interactive geometry editing, #7 added load
editing (`LoadsPanel` is new), #8 added the `DetailingPanel` + its
`gui/detailing/` layout subsystem, and #9 added `src/report/` (PDF + full
legacy-text export) plus the corresponding `Toolbar`/`Application` wiring.
Read the relevant issue before starting work here — each has detailed
acceptance criteria. If a *new* sub-issue is ever added to this epic (or a
new epic reuses this codebase), extend this section (new subsystems, data
model, how the engine/GUI/export layers connect) rather than replacing it
wholesale, per the Change Log Policy.

**`engine::MemberResult`'s reinforcement fields + `gui/detailing/` (issue
#8) — read before touching detailing drawings:**
- **All the reinforcement data was already being computed by
  `ComputeMemberResults()` (issue #5)** -- `IsiElemenBalokFields()`/
  `IsiElemenKolomFields()` already set the legacy `DIA1lap`/`NL1lap`/...
  (beam) and `DIA`/`N_DIA` (column) fields as a side effect of computing
  dimensions/results; #8 just added capturing those into `MemberResult`
  too (no new engine computation). If a field you need for detailing
  isn't on `MemberResult` yet, check whether `IsiElemenBalokFields`/
  `IsiElemenKolomFields`/`DesignColumn` already sets it on `sd` before
  writing new engine logic.
- **Bar counts (`lap_n_tarik` etc., `col_n_dia`) are `float`, matching the
  legacy `NL1`/`N_DIA` fields exactly** (discrete-table lookups resolve to
  float even for "count" tables) -- always integral in practice; format
  with `%.0f`/`std::lround()`, don't change the type to `int` without
  checking every call site.
- **Tension bars flip faces between lapangan and tumpuan on purpose --
  this is physically correct, not a placeholder.** `BuildDetailingDrawing()`
  places lapangan (midspan) tension bars at the bottom and tumpuan
  (support) tension bars at the top, matching the standard sagging
  (positive moment, `MLAP`) vs. hogging (negative moment, `MTUM`) moment
  convention -- it's exactly why the legacy format tracks two independent
  reinforcement sets per beam in the first place. Verified both by a
  standalone geometry test (asserts tension-bar Y sign per region) and by
  eyeballing a real rendered drawing (`Example/Apl1-1`'s batang 8: tumpuan
  tension top / lapangan tension bottom, both matching the CLI's
  `MEMBER_RESULTS` numbers exactly).
- **Column bars are placed evenly around all four sides given only
  `N_DIA`** (bars per side, legacy convention) -- total unique bars is
  `4*N_DIA-4` (corners shared between adjacent sides), matching
  `WriteFinalResults()`'s `(4*N_DIA-4) D DIA` display exactly. Verified
  with a standalone test (`N_DIA=3` -> 8 bars, all within the outline).
- **`DetailingLayout.{h,cpp}` has zero ImGui/OpenGL dependency on
  purpose** -- it only depends on `engine::MemberResult`/`gui::MemberVisual`
  and produces plain-data `DetailingDrawing`/`DetailingSection`/
  `RebarCircle`/`DimensionLabel` structs in local mm coordinates (origin
  at the section center, +Y up). `DetailingPanel.cpp` is the *only* GUI
  file that touches `ImDrawList` to render it -- `report/PdfExport.cpp`
  (issue #9) is the second renderer this split was built for: it calls
  `BuildDetailingDrawing()` directly and draws the identical layout with
  HPDF calls instead, no bar-position recomputation. If you ever need a
  *third* rendering target, follow the same pattern rather than
  special-casing detailing logic into that renderer.
- **Dimensioning is simple text labels anchored at a point, not a full
  parametric CAD dimension-line-with-arrowheads system** -- judged
  sufficient for "usable as a reference drawing" without the added
  complexity; revisit only if a future issue specifically asks for
  arrowed extension lines.
- **What was verified:** `BuildDetailingDrawing()`'s geometry (bar counts,
  in-bounds positions, correct tension/compression face per beam region,
  column bar count formula) was unit-tested standalone before GUI wiring.
  The full rendering pipeline was then verified interactively in this
  environment against a real optimization run of `Example/Apl1-1`
  (screenshotted): a column's drawing (12D25 bars, correct spacing/cover/
  stirrup labels) and a beam's drawing (both Tumpuan and Lapangan
  sections, correct tension-face flip, all labels) were confirmed to
  exactly match the same run's `orcisf_cli`/`MEMBER_RESULTS` numeric output.

**`src/report/` + `engine::WriteStructureFile()`/`WriteDiscreteTables()`
(issue #9) — read before touching export:**
- **`WriteStructureFile()` recomputes `NRJ`/`NR`/`ND`/`N` from the current
  `sd.JRL` rather than trusting `sd`'s own fields** (a joint counts as
  restrained if any of its 6 DOF flags is 1; `ND = 6*NJ`, `N = ND - NR`) --
  same reasoning as `WriteLoads()` needing raw values: an edited-then-
  exported dataset has no reliable `NRJ`/`NR` sitting around (the editor
  doesn't maintain them), so re-deriving from the ground truth (`JRL`) at
  write time is the only way exported `.inp` files stay internally
  consistent. A partially-restrained joint's *actual* `JRL` pattern is
  preserved as-is (e.g. a pin support with translations fixed but
  rotations free), not forced to all-6.
- **`WriteDiscreteTables()` is a pure echo** -- the five discrete design
  tables (`.isd`/`.idl`/`.ijl`/`.ids`/`.ijs`) aren't touched by the editor
  (#6/#7 don't create/resize them), so this just writes back whatever
  `ReadDiscreteTables()` populated. Round-trip verified with a standalone
  test: read a real dataset, write it to a scratch path, read it back,
  compare every field (`NJ`/`M`/`NRJ`/`NR`/`ND`/`N`, every joint/member/
  discrete-table value) -- bit-identical, including the recomputed
  `NRJ`/`NR`/`ND`/`N` matching the originals exactly.
- **"Download as text" (`report::WriteTextExport()`) always writes
  `.inp`/`.isd`/`.idl`/`.ijl`/`.ids`/`.ijs`/`.bbn` (mirroring whatever's
  currently in `sd`), and *additionally* writes `.opt`/`.str`/`.kdl`/`.inf`
  + copies `.his`/`.log.txt` from the run's original location only if
  `has_run_results` is true** (a completed run with no edit since --
  `Application` tracks this the same way it already tracked "does the
  scene have `MemberResult` data", see its header comment). `.his`/
  `.log.txt` are copied rather than regenerated because they're
  per-generation history that only ever existed as a byproduct of the
  original run -- there's no way to reconstruct them from the final
  `StructureData` alone. `Application::OnRunResult()` already zeroes
  post-run `W`/`AJ` for the *load editor*'s sake (see the note below); the
  *same* `has_run_results_` flag independently gates PDF export and the
  `.opt`/`.str`/`.kdl`/`.inf`/`.his`/`.log.txt` part of text export, and is
  cleared on any subsequent edit (`RebuildSceneAfterEdit()`) since edited
  geometry invalidates `var_b`/`var_k`/`fitstr` (member ids/counts can
  shift on delete).
- **Namespace is `orcisf::report`, not `orcisf::export`** -- `export` has
  been a reserved C++ word since C++98 (unused "export templates" feature,
  repurposed for C++20 modules), so it can't be a namespace identifier
  even outside a module build. The directory is `src/report/`, not the
  issue's literally-suggested `/src/export`.
- **HPDF error handling uses `setjmp`/`longjmp`** (`PdfExport.cpp`'s
  `ErrorHandler`), the standard pattern in every libharu example, wrapping
  the whole `WritePdfReport()` body. This is technically UB if a
  non-trivial-destructor C++ object is live on the stack when a `longjmp`
  fires (destructors don't run) -- accepted here as a known, standard
  tradeoff of using a C library's callback-based error model from C++,
  not something to "fix" by rewriting HPDF's API surface.
- **What was verified:** `WriteStructureFile()`/`WriteDiscreteTables()`'s
  round-trip (above) and `WritePdfReport()`/`WriteTextExport()` were both
  exercised standalone (a real completed run of `Example/Apl1-1`: PDF
  generated, opened, and screenshotted page-by-page -- cover/input
  summary, the per-member summary table matching `MEMBER_RESULTS` exactly,
  and a detailing page with correct Tumpuan/Lapangan sections; text export
  produced all 12 files, `.his`/`.log.txt` included, at the right sizes).
  The full GUI path was then verified separately, through the *actual*
  native NFD dialogs (not simulated): "Export PDF..." opened a real "Save
  As" dialog (pre-filled `report.pdf`, `*.pdf` filter) and produced a
  byte-identical file to the standalone test; "Export Text..." opened a
  real "Select Folder" dialog and produced the same 12-file set. The
  round-trip criterion was checked by re-reading the exported dataset with
  `orcisf_cli info`/`equilibrium` -- identical geometry/topology, and a
  perfect (0.0) equilibrium residual.

**`engine/`'s `ReadLoadsRaw()`/`WriteLoads()` (issue #7) — the load-editor's
critical invariant:** the legacy `.bbn` file *never* includes self-weight
(`Pembebanan.hpp`'s `load_data()` writes it before self-weight is ever
computed); `ReadLoads()` (used by real analysis/optimization runs, #3/#4)
re-derives self-weight fresh into `sd.W`/`sd.AJ` on every call via
`BeratSendiri()`. The GUI editor uses `ReadLoadsRaw()` (parses the file
without that side effect) and `WriteLoads()` (writes exactly what's in
`sd.W`/`AML`/`AJ`, unconditionally) instead — **never point `WriteLoads()`
at a StructureData that went through `ReadLoads()`/`BeratSendiri()`**, or
self-weight gets baked into the file and double-counted on the next real
`ReadLoads()`. This is also why `Application::OnRunResult()` explicitly
zeroes `sd.W`/`AML`/`AJ` before treating a just-completed run's
`StructureData` as an editable one — there's no cheap way to separate a
run's self-weight-inflated joint actions back into "raw user load" per
joint (a joint's self-weight contribution can come from multiple columns
sharing it), so post-run load editing starts from a clean slate rather
than risk showing numbers the user never entered.

**`gui/editor/` (issues #6/#7) — read before touching geometry/load editing:**
- **Only geometry fields are edited/compacted, deliberately not every
  legacy per-member array.** `EditableStructure` (add/move/delete
  joint/member) and `UndoStack`'s `GeometrySnapshot` both touch only
  `NJ`/`M`/`X`/`Y`/`Z`/`JRL` (per joint) and `JJ`/`JK`/`IA` (per member) --
  every other per-member field (`SFF`, `SM`, `AM`, `W`, `MTUM_*`,
  `var_b`/`var_k`, `R11..R33`, ...) is analysis/design *scratch state* that
  `Inersia()`/`Struktur()`/the optimizer recompute from scratch on the next
  run regardless of what edits happened in between, so there's nothing to
  keep consistent there. Don't "complete" this list to cover those fields
  without a concrete reason (it would only add dead-weight copying, per
  the same lesson issue #4 learned about full `StructureData` copies).
  `T_K`/`ND`/`N` are legacy input-parsing scratch, also untouched.
- **Deleting a joint/member compacts indices, not tombstones them** --
  `DeleteJoint()`/`DeleteMember()` shift every higher-indexed
  joint/member down by one and remap `JJ`/`JK` accordingly, keeping the
  1-based arrays contiguous (the invariant every other legacy array in
  this port relies on: `NJ`/`M` as "how many are in use", no gaps). Verified
  with a standalone test (not checked in): adding a 4-joint/4-member square,
  deleting one joint, and checking the cascading member deletion +
  reindexing + undo/redo round-trip all landed correctly.
- **Restraint editing is a single fixed/free toggle, not per-DOF.**
  `SetJointRestrained()` sets/clears all 6 `JRL` DOF flags for a joint at
  once. Fine-grained per-DOF restraint editing wasn't in #6's acceptance
  criteria; add it as a refinement if a future issue needs it, don't
  silently change this toggle's meaning.
- **Wiring edits into a `RunPanel` run was explicitly out of scope for
  #6** — `RunPanel`/`RunFullOptimization` still always re-read a dataset
  from disk by path; there's no path yet from "edited in-memory geometry"
  to "run an optimization on it" (no save/export-to-.inp exists either).
  That's natural territory for #7 or a dedicated follow-up, not something
  #6 was required to solve.
- **What was interactively verified vs. reasoned through (#6):** joint/member
  picking, numeric position entry live-updating the 3D view, the
  Restrained checkbox, Delete Joint, Add Joint, Undo, and the Validation
  panel were all exercised end-to-end in this environment (mouse/keyboard
  input synthesized via Win32 API + `AttachThreadInput` to work around
  Windows' foreground-focus protections, screenshotted to confirm). An
  actual click-drag *on* the ImGuizmo gizmo's arrows was not (sub-pixel
  axis targeting was judged too unreliable to script); the gizmo was
  confirmed to render at the correct position and `Manipulate()`'s output
  feeds the exact same `EditableStructure::MoveJoint()` call the verified
  numeric-entry path uses, so the remaining risk is specifically in
  ImGuizmo's own hit-testing, not in this project's code.
- **Issue #7's load types match the legacy format exactly, nothing more.**
  `Pembebanan.hpp`'s `.bbn` format only ever supported two load
  categories: a uniform distributed load on a member (`W`, transverse,
  gravity-sense-positive), and a 6-DOF generalized action at a joint
  (`AJ`, "arah 1..6" = Fx,Fy,Fz,Mx,My,Mz). There is no separate "point
  load" or "wind load" type in the format -- both are just a joint action
  with the appropriate components set. The Loads toolbar menu's "Add
  Member Load"/"Add Joint Load" placement modes and `LoadsPanel`'s two
  tables deliberately mirror this 2-category structure; don't invent a
  third category (e.g. a point load *on a member*, or a partial-length
  UDL) without confirming the legacy format can actually represent it --
  otherwise `WriteLoads()` couldn't round-trip it and #7's "round-trips
  correctly to/from the legacy .bbn format" criterion would silently break.
- **Click-to-place uses a default value, not zero, so the placed load is
  immediately visible.** `ViewportPanel::HandlePicking()` sets 5000 N/m
  (member) or (0, -10000, 0, 0, 0, 0) N (joint, downward) on the clicked
  entity, which the user then fine-tunes in Properties/`LoadsPanel`. Same
  reasoning as Add Joint defaulting to the scene's bounding-sphere center
  in #6 -- placement should produce something visible/editable, not a
  silent no-op.
- **Load arrow glyphs are a fixed visual length, not scaled by
  magnitude**, and moments (Mx/My/Mz) render as a plain marker, not a
  directional glyph -- see the comment above `SceneRenderer::DrawLoads()`
  for the reasoning (N/m and N values span orders of magnitude across
  real datasets; exact numbers always live in `LoadsPanel`/Properties).
- **What was interactively verified for #7:** the full load-editor
  workflow was exercised end-to-end against a real scratch copy of
  `Example/Apl1-1` in this environment (screenshotted at each step): the
  existing `.bbn` loads (35000 N/m on 4 beams) loaded and rendered
  correctly and matched `LoadsPanel`'s table exactly; placing a new joint
  load via Loads > Add Joint Load + a viewport click updated
  Properties/`LoadsPanel` in sync; **File > Save Loads (.bbn) was
  actually clicked and the resulting file on disk was read back and
  diffed** -- it contained both the original 4 member loads and the new
  joint load, byte-for-byte matching what the GUI showed. `ReadLoadsRaw()`
  ↔ `WriteLoads()` numeric round-trip fidelity (every `W`/`AML`/`AJ` value
  bit-identical after a write+re-read) and `SetMemberLoad()`'s UDL
  fixed-end-force formula were also unit-tested standalone beforehand.

**Issue #11 additions (File > New Data/Open Data..., title/material form,
`.inf` preview) — read before touching this area:**
- **`File > New Data`** (`Application::OnNewDataRequested()`) prompts for
  a save location up front via `PromptForGenericPath()` (a single NFD
  save dialog reused by `Save As...` too -- folder navigation + typed
  filename, extension stripped to get the legacy generic path), then
  calls the same `LoadStructure(engine::StructureData{}, nullptr, ...)`
  path `OnAddJointRequested()` already used implicitly when nothing was
  loaded, and immediately writes the initial file set via
  `report::WriteTextExport()` so the chosen location has real files on
  disk right away (not just a remembered path). Cancelling the dialog
  leaves whatever was previously loaded untouched. `Open Folder...` is
  relabeled `Open Data...` (label only, same NFD folder-picker/callback).
- **`File > Save`/`Save As...`** are thin wrappers around the existing
  `report::WriteTextExport()` (issue #9) -- `Save` writes to
  `loaded_dataset_path_` (falling back to `Save As...` if empty),
  `Save As...` always re-prompts via `PromptForGenericPath()` and updates
  the tracked path on success. No new export logic; this was purely
  missing menu entry points into machinery #9 already built.
- **`PropertiesPanel`'s new "General" section** (shown when nothing is
  selected) is a direct-mutation form over `EditableStructure::Sd()` (a
  new mutable accessor, alongside the existing const `SdForUndo()`) for
  `ISN`/`E`/`G`/`FC`/`FY`/`FYS` -- all pre-existing `StructureData`
  fields, this was purely a missing form. Deliberately **not** routed
  through `UndoStack`/`GeometrySnapshot` like geometry edits are: these
  are scalar fields that don't affect array sizes/indices, so there's no
  compaction invariant to protect (same reasoning `RunPanel`'s
  `harga_beton_`/`harga_besi_` fields already use). The read-only
  "Structural Parameters (auto-calculated)" fields (members/DOF/joints/
  restraints) call `engine::ComputeRestraintSummary()`, not `sd`'s own
  `NR`/`NRJ`/`N` -- see next bullet.
- **`engine::ComputeRestraintSummary()`** (new in `LegacyIO.h`/`.cpp`) is
  `WriteStructureFile()`'s existing NRJ/NR/ND/N-from-JRL recomputation
  logic, extracted into a shared public function (also returns the
  restrained-joint index list, replacing `sd.T_K`) so
  `WriteStructureFile()`, the new `WriteInfPreview()`, and the Properties
  panel's read-only fields all agree on one derivation instead of three.
- **`engine::WriteInfPreview()`** writes only the .inf "input echo"
  portion (title/params/material/coordinates/members/restraints --
  `CETAK.HPP` lines 26-83) from the *current* in-memory `sd`, independent
  of a completed run -- unlike `WriteFinalResults()`'s private
  `WriteInformasi()`, which trusts `sd.NR`/`NRJ`/`N`/`T_K` and is only
  ever called post-run. Deliberately excludes the two load-echo sections
  that follow in the real legacy `.inf` (`Gaya di Ujung Batang Terkekang
  Akibat Beban`, `Beban Titik`) -- issue #11 didn't ask for them; add
  them the same way if a future issue wants full `.inf` parity. Wired to
  `File > Export INF Preview...` via an NFD save dialog.
- **What was verified:** the full GUI build succeeds and the app launches/
  runs without crashing with all of the above wired in (confirmed via
  `Get-CimInstance`, same technique as issue #12's verification).
  `WriteInfPreview()`'s output was verified standalone (not through the
  GUI) against a real dataset: read `Example/Apl1-1`, wrote a fresh
  `.inf` to a scratch path, and diffed every one of the six sections
  against the checked-in `aplikasi.inf` -- numerically identical (only
  whitespace/column-alignment differs, since the new writer uses
  tab-separated output rather than `CETAK.HPP`'s `setw()` column
  formatting).
- **Interactively verified in a follow-up pass** (synthesized Win32
  mouse/keyboard input + screenshots, same technique issue #6 used --
  see `CHANGE_HISTORY.md`'s 2026-08-11 "Interactive verification" entry
  for the full account, including a DPI-awareness pitfall and an
  unrelated-window-focus hazard hit along the way): `New Data` opening
  the real native Save-As dialog, saving actually writing all 7 files,
  the General section rendering and being live-editable (typed a title,
  confirmed it round-tripped through `File > Save` to the `.inp` file on
  disk). This also caught a real bug: **`Ctrl+S` was display-only** --
  `ImGui::MenuItem`'s shortcut-text parameter doesn't bind the key
  chord by itself. Fixed with an explicit
  `ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)` check in
  `Application::OnFrame()`, gated on the same `can_export_text`
  condition the menu item uses, and re-verified with the actual key
  chord (not a menu click).

**`gui/IconToolbar.{h,cpp}` (issue #14, part of epic #13) — read before
touching:**
- **Fixed curated button set, not a fully user-customizable Quick Access
  Toolbar.** #14's acceptance criteria allowed scoping down from full
  drag-to-reorder/add-remove customization if that proved too large for
  one issue -- it did, so this ships New Data/Open Data/Save/Undo/Redo/
  Add Joint/Connect Joints/Run as a fixed row. A future issue can add
  customization on top without changing how `IconButton()` draws/wires a
  button.
- **Icons are hand-drawn `ImDrawList` primitives, not an icon font.**
  Matches this project's preference for small, dependency-free GUI code
  (`SceneRenderer`'s hand-rolled GL primitives is the same philosophy) --
  no `vcpkg.json` change needed. `IM_PI` isn't part of ImGui's public
  API surface (only `imgui_internal.h` has it) -- `IconToolbar.cpp`
  defines its own `kPi` constant rather than pulling in the internal
  header for one constant.
- **The toolbar is a plain ImGui window (`ImGuiWindowFlags_NoDecoration`
  positioned/sized to span the viewport width at a fixed height), not
  drawn via `BeginMainMenuBar()`-adjacent API**, so it doesn't
  automatically shrink the dockspace's work area the way the main menu
  bar does. `Application::OnFrame()` manually adjusts
  `ImGui::GetMainViewport()->WorkPos.y`/`WorkSize.y` by
  `IconToolbar::kHeight` right after drawing it and before
  `BuildDockspace()` -- if you resize the toolbar, change `kHeight` (a
  public static constexpr on `IconToolbar`), don't hardcode a second
  number in `Application.cpp`.
- **`RunPanel` gained a public `CanRun()`/`TriggerRun()`** so the
  toolbar's Run button can start a run without duplicating `StartRun()`'s
  logic or exposing `dataset_path_` -- both just call the exact same
  private `StartRun()` the panel's own button already used, gated the
  same way (`!IsRunning() && dataset_path_[0] != '\0'`).
- **What was verified:** interactively, in this environment (synthesized
  Win32 input + screenshots, same technique as #11's follow-up pass) --
  all 8 icons render with correct disabled/enabled dimming, hover
  tooltips work (confirmed "Connect Joints" appearing on hover), and
  clicking "Add Joint" correctly started a blank structure and added a
  joint (Properties panel updated, Save/Undo icons became enabled
  immediately after, Redo/Run correctly stayed disabled).

**View layout presets (issue #15, part of epic #13) — read before
touching `Application::BuildDockspace()`/`Toolbar`'s View menu:**
- **`ViewLayoutPreset` (`Default`/`Design`/`Optimization`) lives in
  `Toolbar.h`**, not a new file -- it's a View-menu-owned concept the
  menu itself needs to check-mark the active preset, so it's colocated
  there rather than in `gui/editor/Selection.h`'s other shared enums
  (`SelectionKind`, `LoadPlacementMode`), which are viewport/editor-owned.
- **`BuildDockspace()` rebuilds the dock layout whenever
  `current_layout_ != built_layout_preset_`**, not just once at startup
  the way it did pre-#15 (`dockspace_initialized_` alone used to gate
  it). Each preset has its own `BuildXLayout(ImGuiID dock_main)` free
  function in `Application.cpp`'s anonymous namespace -- add a new
  preset by adding one more such function and a `switch` arm, not by
  branching inline in `BuildDockspace()` itself.
- **No panel is ever hidden by a preset, only resized/re-tabbed.** All
  six panels (`Viewport`/`Detailing`/`Properties`/`Loads`/`Run
  Optimization`/`Log`) are docked somewhere in every preset -- Design
  tabs `Run Optimization`+`Log` together in a smaller corner (still one
  click away), Optimization does the same to
  `Viewport`+`Detailing`/`Properties`+`Loads`. Don't "improve" a preset
  by dropping a `DockBuilderDockWindow` call for a panel you think is
  irrelevant to that workflow stage -- that would make it fully
  unreachable except via re-switching presets, which the acceptance
  criteria didn't ask for and would be a regression for anyone who
  docked something custom there.
- **Switching presets is purely a layout change** -- `current_layout_`
  is the only new state, and `BuildDockspace()`'s rebuild only calls
  `ImGuiDockBuilder*` functions, never touches `loaded_sd_`/`editable_`/
  selection/undo state. Verified interactively: added a joint, switched
  through all three presets and back to Default, the joint/selection/
  undo history were all still there throughout.
- **What was verified:** interactively, in this environment (same
  synthesized-input technique as #11/#14's passes) -- all three presets
  render with the exact panel arrangement described above, the View
  menu's checkmark correctly tracks the active preset, and switching
  Design -> Optimization -> Default round-trips back to the original
  layout exactly (pixel-for-pixel arrangement, not just "close enough").

**`RunPanel`'s "Regenerate Seed" button (issue #17) — a real bug was
found and fixed here, worth knowing if you touch `rng_seed_` again:**
`rng_seed_` is `unsigned int`, but the field round-trips through
`ImGui::InputInt` (a signed `int`) via `rng_seed_i`. A raw
`std::random_device` value can exceed `INT_MAX`, which displayed as a
*negative* number in the field -- and worse, the field's own edit path
(`rng_seed_ = static_cast<unsigned int>(std::max(0, rng_seed_i))`) would
have silently clamped that negative display back to `0` (the "always
random" sentinel) on the next edit. Fixed by generating with
`std::uniform_int_distribution<int>(1, INT_MAX)` instead of a raw
`std::random_device` draw, so the value is always positive and
int-representable. Caught by actually clicking the button in the running
app and reading the field's displayed value, not just by code review --
verified interactively in this environment (two separate clicks produced
two different valid positive seeds).

**Run panel dataset-path field removed; per-run timestamped output
subfolder (issue #25) — read before touching `RunPanel`'s dataset-path
handling or `engine::RunFullOptimization()`'s output paths:**
- **`RunPanel` no longer owns an independently-typed dataset path.** The
  old `char dataset_path_[512]` field, edited via its own `InputText`,
  was a second source of truth that could (and did, per the user report
  this fixes) drift out of sync with whatever `Application` actually had
  loaded via File > Open Data/New Data/Save As. `dataset_path_` is now a
  plain `std::string`, kept in sync via a new `RunPanel::SetDatasetPath()`
  that `Application::OnFrame()` calls once per frame, **before** anything
  that reads `RunPanel::CanRun()` (the icon toolbar's Run button) or
  calls `Draw()` -- ordering matters here, since `CanRun()` reflects
  whatever `dataset_path_` held as of the *previous* `SetDatasetPath()`
  call otherwise. The panel now just displays the loaded path read-only.
- **`engine::RunFullOptimization()`'s signature changed from `void` to
  returning `std::string`** -- the generic *output* path a run's
  `.opt/.str/.kdl/.inf/.his/.log.txt` actually landed in. Every run now
  creates a fresh subfolder next to the dataset (`YYYY-MM-DD.HH.MM`, e.g.
  `2026-08-11.13.40`) and writes there instead of overwriting the
  dataset's own files in place -- **input** (`.inp/.isd/.idl/.ijl/.ids/
  .ijs/.bbn`) is still read from `generic_dataset_path` itself,
  unaffected; only output moved. This needed two independent
  `LegacyIO::DatasetPaths` inside `RunFullOptimization()` (`input_paths`
  for reading, a second `paths` built from the timestamped generic path
  for everything written) -- don't collapse them back into one, that was
  exactly the bug this issue reports for the old in-place-overwrite
  behavior (specifically: no isolation between separate runs/re-runs,
  including issue #16's re-optimize-from-last-best).
- **Every caller of `RunFullOptimization()` needed updating for the new
  return value**, not just `RunPanel`: `orcisf_cli.cpp`'s `optimize`
  command now prints `Output written to: <path>`; `RunPanel::StartRun()`
  captures it into a new `result_output_path_` member, threaded through
  `SetOnResult`'s callback (now 3 args: `StructureData`, input dataset
  path, output path) to `Application::OnRunResult()`, which stores it in
  a new `last_run_output_path_` member.
- **`report::WriteTextExport()`'s `source_generic_path` argument (the
  path it copies `.his`/`.log_detail` from when `has_run_results_` is
  true) must be `last_run_output_path_`, not `loaded_dataset_path_`,
  everywhere it's called** (`OnSaveRequested`/`OnSaveAsRequested`/
  `OnExportTextRequested`) -- `loaded_dataset_path_` is the *input*
  dataset's path (used for Save/Save As, unaffected by this issue);
  after this change it no longer points at where a run's `.his`/
  `.log.txt` actually live. Get this wrong and text export silently
  stops copying those two files (the `!source.empty()` guard in
  `TextExport.cpp` fails open, not loud, on a stale/wrong path).
- **Two runs started within the same minute against the same dataset
  reuse the same output subfolder** (last one wins) -- an accepted minor
  edge case per the issue's own acceptance criteria, not otherwise
  handled.
- **What was verified:** compiled cleanly (MSVC/Ninja, `windows-release`
  preset, all three targets: `orcisf_engine`, `orcisf_cli`, `orcisf_gui`)
  and empirically exercised via `orcisf_cli optimize` against a scratch
  copy of `Example/Data01`: confirmed a real `<folder>/<timestamp>/`
  subfolder was created containing the run's six output files, and (via
  checksum, not just mtime) that the dataset's own top-level `.opt` file
  was byte-identical before and after -- genuinely untouched. The built
  GUI was launched and confirmed running without crashing after the
  `RunPanel` UI changes, but the dataset-path-sync/Run-panel-display
  itself was not click-tested interactively; that's still a good next
  step for full confidence.

**App icon set wired into the build (issue #26) — read before touching
`src/app/AppIcon.{h,cpp}`, `src/app/orcisf.rc`, `src/packaging/
orcisf.desktop`, or the icon-related blocks in `src/CMakeLists.txt`:**
- **`icons/` lives at the repo root, not under `src/`** (shared
  reference art, not build-generated) -- `CMakeLists.txt` sets
  `ORCISF_ICONS_DIR` once (`${CMAKE_CURRENT_SOURCE_DIR}/../icons`) and
  every platform block below reuses it. See `icons/README.txt` for what
  each subfolder/file is for.
- **Three genuinely different mechanisms, one per platform** -- don't
  assume one implementation covers all three:
  - **Windows**: `app/orcisf.rc` (a single numeric-ID `ICON` resource,
    no companion `resource.h` needed) embeds `icons/windows/orcisf.ico`
    as the `.exe`'s own icon. Added to `orcisf_gui`'s sources only under
    `if(WIN32)` -- CMake auto-detects the `.rc` extension and invokes
    `rc.exe`/`windres` for it, no `enable_language()` call needed.
  - **macOS**: `MACOSX_BUNDLE TRUE` + `MACOSX_BUNDLE_ICON_FILE` (naming
    `icons/macos/orcisf.icns`, added to the target's sources with
    `MACOSX_PACKAGE_LOCATION "Resources"`) packages `orcisf_gui` as a
    real `.app` bundle. This is the *only* place macOS gets an icon from
    -- `AppIcon.cpp`'s `ApplyWindowIcon()` is a deliberate no-op there
    (see next bullet).
  - **Windows/Linux runtime window icon** (taskbar/Alt+Tab/window-manager
    decorations, independent of the `.exe`/bundle icon above):
    `AppIcon.cpp`'s `ApplyWindowIcon()` decodes PNGs via libpng's
    simplified API (`png_image_begin_read_from_file`/`_finish_read` --
    far less boilerplate than the full streaming API for one-shot small
    icons) and calls `glfwSetWindowIcon()`. **GLFW does not support
    `glfwSetWindowIcon()` on macOS** (documented GLFW behavior, not a
    bug) -- `ApplyWindowIcon()` returns immediately there.
- **`libpng` is now a *direct* `vcpkg.json` dependency**, even though it
  was already an installed transitive one (via `libharu`'s PNG-in-PDF
  embedding) -- confirmed via `vcpkg_installed/vcpkg/info/libpng_*.list`
  before adding it. Declaring it directly means this doesn't silently
  break if `libharu` ever stops needing PNG.
- **Runtime icon PNGs are resolved relative to the executable's own
  directory, not the process's current working directory** -- a real
  gotcha `AppIcon.cpp`'s `ExecutableDir()` exists specifically to avoid
  (platform-specific: `GetModuleFileNameW` on Windows, `readlink
  ("/proc/self/exe")` on Linux; unused/never called on macOS). CMake's
  `POST_BUILD` step copies `icons/png/icon_{16,32,48,256}x*.png` into a
  `icons/` subfolder next to the built executable -- same reasoning as
  the pre-existing MinGW-runtime-DLL `POST_BUILD` copy in this file.
- **Linux packaging (`.desktop` file + hicolor icon theme install) only
  takes effect via `cmake --install`**, a packaging step this project
  doesn't otherwise script or run in CI -- the `install()` rules
  (`if(UNIX AND NOT APPLE)`) were reviewed against XDG conventions but
  **not exercised on a real Linux machine** (none available in this
  environment).
- **What was verified vs. not:** compiled successfully on Windows
  (MSVC/Ninja, `windows-release` preset) -- confirmed the `.rc` resource
  actually compiles (`orcisf.rc.res` generated) and links, the runtime
  icon PNGs land next to the built `.exe`, and the app launches/runs
  without crashing with `ApplyWindowIcon()` wired in. **The icon's
  actual on-screen appearance (Explorer/taskbar/window) was not
  confirmed via screenshot** in this pass (a screenshot attempt captured
  an unrelated foreground window instead, a known hazard in this
  environment -- see the focus-stealing note elsewhere in this file; not
  worth repeatedly retrying for a cosmetic check). macOS bundle
  packaging and Linux `.desktop`/install rules were written against each
  platform's documented CMake convention but have **no local toolchain
  to verify against** (Windows-only dev environment) -- CI
  (`build-src.yml`, matrix over all three OSes) is the first real
  cross-platform check for those two.

**Issue #27 (title bar buttons "not flush to window edge") — investigated,
could not reproduce; read this before trusting a screenshot-based UI bug
report in this environment again:**
- **This was a false positive caused by a DPI-measurement-tooling bug,
  not a real product defect.** The original report (filed during the
  #21-#25 retest pass) was based on a screenshot that mixed a
  DPI-aware `GetWindowRect()` P/Invoke call with .NET's
  `System.Windows.Forms.Screen.Bounds`/`Graphics.CopyFromScreen()` in
  the same PowerShell session -- the latter returned a *logical*
  (scaled-down) screen size despite `SetProcessDPIAware()` having been
  called, so the screenshot only captured roughly the top-left quarter
  of the true physical screen. The window (genuinely maximized to the
  full physical resolution) rendered its title bar buttons correctly at
  the true physical right edge -- just outside that undersized capture
  -- which looked exactly like "a gap before the window's true edge."
- **The fix for the tooling, if you hit this again**: don't rely on
  `System.Windows.Forms.Screen.Bounds` for capture dimensions in this
  environment. Instead, size the capture bitmap explicitly from a
  DPI-aware `GetWindowRect()`/`GetSystemMetrics(SM_CXVIRTUALSCREEN)`
  P/Invoke call, and pass that same explicit size to
  `Graphics.CopyFromScreen()` rather than trusting `Screen.Bounds` to
  already be in the right coordinate space. Re-verified this way (a
  4x-zoomed crop of the true physical top-right corner): the button
  cluster **is** flush and evenly spaced, no gap, no code change needed.
- **`CustomTitleBar.cpp`'s existing right-alignment math is correct as
  written** -- `ImGui::SameLine(drag_zone_end_x, 0.0f)` using
  `ImGui::GetWindowWidth()` operates entirely within ImGui's own
  internal logical coordinate system (not affected by the OS-level DPI
  API inconsistency above), so there was never a plausible code-level
  mechanism for the reported gap once the measurement was corrected.
  Don't "fix" this code without a *real* (physical-pixel-accurate)
  screenshot showing an actual gap first.
- A comment explaining this was posted on issue #27 recommending it be
  closed as "cannot reproduce"; not closed automatically (per this
  project's usual caution around closing issues without confirmation).
- **Correction (2026-08-11, during issue #31's second pass): the bug WAS
  real, and this "not reproducible" conclusion was wrong.** The
  DPI-measurement tooling problem described above was genuine and did
  produce a misleading screenshot -- but there was also a real
  right-alignment defect underneath it, of exactly the reported kind. The
  claim above that "there was never a plausible code-level mechanism"
  missed one: `ImGui::SameLine(offset_x)` measures from the *content*
  origin and silently adds the enclosing group's `GroupOffset.x`, which
  for a menu bar equals `WindowPadding.x` (14px at 100%, 28px at 200%) --
  so the cluster really was shifted past the window's right edge. Fixed
  under #31 by positioning with absolute screen coordinates; see that
  section for the full explanation. Lesson worth keeping: "the math looks
  right" is not the same as having checked what the API's offset is
  actually relative to.

**Panel icon headers (issue #28 Part 2) — read before touching
`gui/PanelIcons.{h,cpp}`; Alt-mnemonics (Part 1) were reverted, see below
and issue #30:**
- **Panel icons could NOT be "an icon inside the native dock-tab label
  text"** -- Dear ImGui has no public API to embed an arbitrary glyph
  inside a window's title/tab-bar string without either an icon font (a
  new asset/dependency this project doesn't otherwise need) or reaching
  into `imgui_internal.h` to hook the *shared* dock-node tab bar
  (fragile, version-coupled, and not a pattern used elsewhere in this
  codebase). **Decided instead**: each panel calls
  `gui::DrawPanelIconHeader(PanelIcon::X, "Label")`
  (`gui/PanelIcons.{h,cpp}`, new) as the very first thing after
  `ImGui::Begin()` -- draws a small hand-drawn icon (same
  `ImDrawList`-primitives-in-a-`P()`-helper style as `IconToolbar.cpp`
  and `ViewportPanel.cpp`'s UCS icon, no icon font) plus the label text
  and a separator, inside the panel's own content area -- a header *row
  inside the content*, not literally inside the OS-drawn tab strip, a
  deliberate documented tradeoff.
- **`ViewportPanel::Draw()`'s icon header consumes a little of
  `ImGui::GetContentRegionAvail()`** before the 3D render texture size
  is computed -- intentional and harmless, not a bug.
- **Confirmed genuinely working via real screenshots**, first for
  Viewport (issue #29's retest pass), then re-confirmed on a fresh build
  plus two more panels (Properties, Log) in a dedicated "build and test
  #28" pass: the small hand-drawn icon renders correctly before each
  panel's title text in its content area, exactly as designed. The
  remaining four panels (`RunPanel`, `LoadsPanel`, `JointsMembersPanel`,
  `DetailingPanel`) weren't individually screenshotted -- they call the
  exact same `DrawPanelIconHeader()` function, so three-for-three
  confirmations across genuinely different panels is treated as
  sufficient confidence for the rest rather than clicking through every
  remaining tab for marginal additional certainty.

**Part 1 (Alt-mnemonics) was reverted -- Dear ImGui does NOT parse
`&`-prefixed labels; this was a wrong assumption, caught by actually
running a build, not by code review:**
- The original #28 implementation added `&File`/`&Edit`/etc. to
  `Toolbar::Draw()`'s menu labels, assuming Dear ImGui would strip the
  `&` and underline the following letter (the Win32/Qt/wxWidgets
  convention) once `ImGuiConfigFlags_NavEnableKeyboard` was set. **It
  does not** -- confirmed via a real screenshot during issue #29's
  retest pass: the literal `&` character was displayed in the menu bar
  ("&File", "&Edit", ...), completely unparsed. Dear ImGui has no
  built-in equivalent to this mnemonic system for regular menu labels.
- **Reverted to plain labels** (`File`, `Edit`, `View Plane`, `Loads`,
  `Run`, `View`) rather than ship the visibly-broken `&`-prefixed text.
  Rebuilt and re-screenshotted to confirm the revert actually fixed it.
- **Real Alt+letter mnemonic support needs to be hand-rolled** if wanted
  (parse a mnemonic marker yourself, draw an underline only while Alt is
  held, detect Alt+letter yourself and open the menu) -- scoped as a
  fresh issue, **#30**, rather than attempted again under time pressure
  within the same pass that just found the bug. Don't re-add bare `&`
  prefixes to these labels without implementing that machinery first.
- **This was found while working issue #29** (the "retest #21-#25"
  follow-up), not #28 itself -- #28 was already closed by the time this
  was discovered; a comment was posted there cross-referencing this
  correction and #30, per this project's transparency conventions
  (see the #13/epic-13 closure-correction precedent earlier in this file
  for the established pattern of correcting past claims openly rather
  than silently).

**Issue #30 (real Alt-mnemonic menu navigation) — implemented; read before
touching `Toolbar.cpp`'s `BeginMnemonicMenu()`:**
- **Entirely public-API, no `imgui_internal.h`** -- two independent
  pieces, both necessary:
  1. **Underline rendering**: after `ImGui::BeginMenu(label)` draws the
     plain-text menu bar item, `BeginMnemonicMenu()` computes the
     mnemonic character's on-screen x-range via `ImGui::CalcTextSize()`
     on the label's prefix substring and the single mnemonic character,
     then draws a short line under it with
     `ImGui::GetWindowDrawList()->AddLine()` -- but only while
     `ImGui::IsKeyDown(ImGuiMod_Alt)` is true that frame, matching the
     Windows convention of hiding accelerators until Alt is held. The
     left-edge x-offset assumes `style.FramePadding.x` padding before
     the label text (Dear ImGui's own main-menu-bar item layout) --
     not a publicly exposed constant, so a 1-2px misalignment is
     possible; a cosmetic risk, not a functional one.
  2. **Opening via Alt+letter**: `ImGui::OpenPopup(label)` is called
     (only on the exact frame `ImGui::IsKeyChordPressed(ImGuiMod_Alt |
     mnemonic_key)` fires) with the *identical* label string
     `BeginMenu(label)` uses for its own ID, in the same ID-stack scope
     (both direct children of the main menu bar) -- this is a documented
     Dear ImGui idiom for programmatically opening a specific popup/menu
     without a click, and `BeginMenu()` recognizes it as open the same
     frame since `OpenPopup()` runs first.
- **Mnemonic index is the character's index within the label, not
  necessarily 0** -- `"View Plane"`'s mnemonic is `P` at index 5 (`V`0
  `i`1 `e`2 `w`3 ` `4 `P`5), not the first letter, since `V` was already
  claimed by the separate `"View"` layout-preset menu. Letters: File=F,
  Edit=E, View Plane=P, Loads=L, Run=R, View=V (same assignment #28
  originally picked, before the "&" attempt was found broken).
- **What was verified vs. not:** compiled cleanly (MSVC/Ninja,
  `windows-release`) via the `builder` skill. **Interactive
  confirmation (Alt held -> underlines actually appear in the right
  place; Alt+letter -> the right menu actually opens) was not
  completed** -- attempting to grab foreground for screenshotting
  repeatedly landed on the user's own actively-in-use browser window
  (reading this project's GitHub issue #29 in real time), not an
  unrelated background process. Correctly identified as the user's live
  session rather than background interference, so automation was
  stopped and their window state was restored rather than continuing to
  fight for control of their active session. The underline-position math
  and the `OpenPopup`-before-`BeginMenu` idiom are both standard,
  well-understood Dear ImGui techniques (not the kind of "assumed but
  wrong" mistake #28's `&`-prefix attempt was, which assumed a
  *library* feature that plain research/testing would have caught
  immediately) -- but given #28's own lesson that assumptions about this
  exact library should be verified empirically before trusting them,
  don't treat this as fully confirmed until a real interactive pass
  (Alt held + screenshot, Alt+letter + screenshot) succeeds.

**Issue #29 (interactive retest of RunPanel dataset gating + plane-offset
overlay) — still blocked, not resolved this pass; read before attempting
this again:**
- **What this pass DID accomplish**: fixed and confirmed the #28
  mnemonic regression above (a real, concrete finding), and confirmed
  #28's panel-icon-header Part 2 genuinely works via screenshot -- both
  via a corrected, DPI-aware screenshot technique (explicit physical-pixel
  bitmap size, not `.NET`'s `Screen.Bounds`, per issue #27's finding)
  combined with an `AttachThreadInput`-based foreground-grab (see #11's
  original interactive-verification notes) that *did* reliably work this
  time for getting the app's main window foregrounded and its menu bar
  clickable.
- **What still did NOT work: driving the native "Open Data..."
  folder-picker dialog to actually load a dataset.** Coordinate-based
  clicking inside the dialog repeatedly mis-hit targets (clicked "New
  Data" instead of "Open Data..." once; a "Cancel" click coordinate
  computed from a *previous* screenshot's displayed/scaled pixel
  position without re-multiplying by the DPI scale factor missed
  entirely). **A new hazard was also hit**: an unrelated app (Proton
  Drive, this user's own cloud-sync client) spontaneously opened its own
  window mid-sequence, stealing focus -- not triggered by anything this
  session did, just an environment where background apps can grab
  foreground focus unpredictably at any time, independent of the
  already-documented "another Claude/browser session steals focus"
  hazard. Left untouched (did not force-close another app's window).
- **Lesson for next attempt**: always recompute click coordinates from
  the *most recent* screenshot taken *after* any UI state change, in the
  same physical-pixel coordinate space that screenshot was captured in
  -- never reuse a coordinate read from an earlier screenshot's displayed
  (scaled-for-viewing) dimensions without re-deriving it fresh. Consider
  keyboard-only navigation (arrow keys + Enter within the folder picker,
  or typing a path directly into the address bar and confirming with a
  single unambiguous key) over mouse coordinates for native dialogs,
  since a single missed click can silently land on the wrong control
  with no error.
- **Issue #29's core objective (confirm RunPanel enables Run and the
  plane-offset overlay appears once a real dataset is loaded) remains
  unverified.** Code review (from the previous pass, still valid) found
  no defect in either code path. Don't close #29 until a real load
  succeeds and both behaviors are actually observed.

**Second attempt at #29 -- root cause of the automation failure
precisely diagnosed this time (a specific dialog control resists
synthetic input, not a coordinate-math mistake); still not resolved:**
- **Progress made**: this pass confirmed the app's own File menu is
  fully keyboard-navigable (Down/Down/Enter reliably moved nav focus
  from "New Data" to "Open Data..." and activated it, confirmed via
  screenshots showing the nav-focus highlight box moving correctly) and
  that the native "Select Folder" dialog does open correctly in
  response. Foreground-grabbing via `AttachThreadInput` also worked
  reliably again once confirmed the *actual* current foreground window
  wasn't the user's own active session (checked `GetForegroundWindow()`
  first, out of caution, before grabbing anything).
- **Precisely diagnosed blocker**: the "Select Folder" dialog on this
  Windows install is the **newer WinUI3/XAML-based Windows 11 file
  picker** (visible from its "Home"/"Gallery" sidebar entries and rounded
  scrollbar styling, not the classic Win32 common-item-dialog look).
  Its "Folder:" text-entry field **does not accept synthetic input** in
  this environment -- neither `SetCursorPos`+`mouse_event` clicks
  (single or double-click, at coordinates re-verified correct via a
  screenshot taken immediately before each attempt) nor
  `SendKeys::SendWait()` typed text ever populated the field; it stayed
  visibly empty across five distinct attempts (direct click+type,
  `Ctrl+A`-then-type, `Ctrl+L`-then-`Ctrl+A`-then-type, double-click,
  each re-screenshotted to confirm). One `Ctrl+A` attempt visibly
  expanded/navigated the left sidebar's tree view instead, indicating
  keyboard focus was landing somewhere else in the dialog entirely, not
  the text field, regardless of where the mouse had just clicked.
- **Why this is a plausible, not just a "kept failing" explanation**:
  WinUI3/XAML controls are known to require real UI Automation (e.g. the
  Windows UI Automation COM API, or a wrapper like FlaUI) to reliably
  receive focus and input from external automation -- raw `SendInput`
  mouse/keyboard events that work fine against classic Win32 controls
  (confirmed working against this *app's own* ImGui-rendered menu, and
  against the *older-style* "Save As" dialog seen in an earlier #29
  attempt, which behaved differently) are known to be unreliable against
  this specific newer dialog family. This is a testable, falsifiable
  claim -- not a shrug.
- **What this means for the next attempt**: don't repeat plain
  `SetCursorPos`/`SendKeys` against this dialog's text field expecting a
  different result. Either (a) use a real UI Automation library/API
  instead of raw input synthesis for this specific dialog, (b) drive
  the dialog via mouse-only navigation through the folder tree/file list
  (which *did* visually respond to clicks, e.g. the sidebar tree
  expanded) rather than the text field, clicking down into the target
  folder and then clicking "Select Folder", or (c) ask the user to load
  the dataset manually and hand off from there. Cancelled cleanly via
  `{ESC}` (which *did* reach the dialog -- window-level accelerators
  seem to work even when the text field doesn't) each time, leaving no
  orphaned dialogs or corrupted app state.
- **Issue #29's core objective is still unverified.** Not closed.

**Third attempt at #29 -- root cause of automation failures found (not the
dialog, a DPI-virtualization bug in the automation script itself), core
objective finally verified, issue resolved:**
- **Real root cause of every prior coordinate-mismatch failure**: the
  PowerShell process driving `SetCursorPos`/`GetWindowRect` was never
  marked DPI-aware, so Windows silently virtualized its coordinate space
  to *logical* pixels (`GetWindowRect` reported `0,0,2560,1392` on this
  200%-scaled monitor) while screenshots were still being captured in
  *physical* pixels (`5120x2784`, via an explicitly-sized
  `Bitmap`/`CopyFromScreen`) -- a 2x mismatch between where clicks
  actually landed and where the screenshots said they should land. This
  fully explains the WinUI3 dialog symptoms blamed in the second attempt
  (sidebar clicks consistently landing one row group off, "Ctrl+A"
  appearing to hit the tree view instead of the text field) -- it was
  never really about WinUI3/XAML resisting synthetic input, it was
  ordinary clicks landing at literally the wrong pixel. **Fix**: call
  `SetProcessDPIAware()` once at the start of the automation script,
  before any `SetCursorPos`/`GetWindowRect` call, so both operate in the
  same physical-pixel space the screenshots already used. Confirmed by
  re-checking `GetWindowRect()` after the fix: `0,0,5120,2784`, matching
  the screenshot dimensions exactly.
- **With that fixed, mouse-driven navigation through the WinUI3 "Select
  Folder" dialog worked on the very next attempt** -- no UI Automation
  library needed after all; the second attempt's diagnosis of "this
  dialog's text field resists synthetic input" was itself a
  DPI-coordinate-mismatch artifact, not a real WinUI3 limitation. Revise
  future automation in this environment accordingly: always call
  `SetProcessDPIAware()` first, don't reach for UI Automation/FlaUI as a
  first resort for a misbehaving native dialog on a high-DPI machine --
  check the coordinate space first.
- **Both of #29's acceptance criteria were then directly confirmed**
  with a real dataset loaded (`Example/Apl1-1` copied to
  `C:\Users\IT\Documents\orcisf\Apl1-1\aplikasi`, loaded via File > Open
  Data...): (1) **RunPanel dataset gating (#25)** -- the Dataset field
  showed the loaded path, all cost/design parameter fields were
  populated, and the **Run** button rendered enabled (not the "Enter a
  dataset path to enable Run" placeholder the user had reported still
  seeing). (2) **2D plane-lock offset overlay (#22/#24)** -- selecting
  View Plane > "X-Y plane (locks Z)" correctly switched the viewport to
  an orthographic top-down projection and revealed a
  "Plane X-Y -- Z offset (m)" label with an editable `0.000` numeric
  field plus an updated UCS icon in the viewport's bottom-left corner,
  confirmed via a close-up crop -- this was **not** broken, contrary to
  the user's report; it was rendering correctly, just never reached
  because #29's earlier attempts couldn't get a dataset loaded to fully
  exercise the interactive view-locking path.
- **Not independently re-verified this pass**: actually typing a new
  numeric Z-offset value and confirming it moves a placed joint (the
  interaction was attempted, but a WhatsApp desktop notification toast
  spontaneously popped up and stole keyboard focus mid-sequence --
  another instance of this environment's background-app focus-stealing
  hazard, same category as the second attempt's Proton Drive incident.
  Automation was stopped immediately rather than risk interacting with
  that toast). The overlay's presence and its being a live `ImGui`
  numeric input (not a static label) is strong evidence the underlying
  binding works, consistent with #22/#24's original code review, but the
  specific "typing a value moves geometry" round-trip is unconfirmed.
- **Issue #29 resolved and closed** -- both criteria the user explicitly
  asked to have re-verified are now interactively confirmed.

**AutoCAD-style Add Joint + editing guidance (issue #18, part of epic
#13) — read before touching `EditorOptions::add_joint_mode`/
`ViewportPanel::HandlePicking()`/`Toolbar.cpp`'s status-hint block:**
- **"Add Joint" changed from an instant single-shot action to a
  click-to-place mode**, per explicit user decision (the issue's own
  acceptance criteria flagged this as something to confirm before
  implementing, since it changes pre-existing behavior) -- matching
  Connect Joints/the load-placement modes' existing click-driven pattern
  instead of being the odd one out. `Application::OnAddJointRequested()`
  now only does the "bootstrap a blank structure if none loaded" half of
  its old job; the actual placement moved to a new
  `options.add_joint_mode` branch in `ViewportPanel::HandlePicking()`,
  which ray-casts the click against the horizontal plane through
  `camera_.target.y` (not a fixed world height) and calls
  `editable->AddJoint()` directly there, snapping to
  `options.grid_size_m` if `snap_to_grid` is on. Like Connect Joints, it
  stays active after each placement (toggle again, or pick a different
  mode, to stop) -- both `Toolbar`'s Edit-menu item and `IconToolbar`'s
  button are checkable toggles now, not fire-once buttons.
- **Real bug found and fixed during interactive verification:**
  `ViewportPanel::Draw()` used to show a static "No dataset loaded" text
  placeholder -- instead of the interactive 3D image widget -- whenever
  `scene.Empty()` (0 joints and 0 members), with no distinction from
  "nothing loaded at all". That's exactly the state right after
  bootstrapping a blank structure for Add Joint mode, so the very first
  click had **nothing to click on** -- a hard dead end, caught by
  actually driving the app (Properties panel stayed at "Number of
  joints: 0" after a click that should have placed one). Fixed by
  gating the placeholder on `!editable && scene.Empty()` instead of
  `scene.Empty()` alone -- `editable` is only non-null once *something*
  (even a blank in-memory structure) is loaded, see
  `Application::LoadStructure()` -- so a blank-but-editable dataset now
  renders its (empty) interactive viewport correctly.
- **What was verified interactively:** toggling Add Joint mode
  (button highlights, tooltip and menu checkmark update, status-bar
  hint text appears); clicking empty viewport space placed a joint
  exactly at the click position (Properties panel showed "Joint 1" with
  correct auto-calculated position); the mode stayed active after
  placement -- a second click placed "Joint 2" without re-opening any
  menu. Connect Joints' new first-click-vs-second-click hint text
  distinction was code-reviewed but not re-verified live in this pass
  (focus-stealing interference from an unrelated process in this
  environment interrupted that specific check; the code mirrors the
  exact `connect_first_joint` state check already proven correct
  elsewhere in this file).

**`gui/JointsMembersPanel.{h,cpp}` (issue #21, part of epic #20) — read
before touching this panel or `EditableStructure::DeleteJoint()`'s
callers:**
- **Docked as "Joints/Members", tabbed alongside "Loads" in all three
  view-layout presets** (`Application.cpp`'s `BuildDefaultLayout()`/
  `BuildDesignLayout()`/`BuildOptimizationLayout()`) -- mirrors where
  `LoadsPanel` lives, since both are tabular schedule/list panels over the
  same `SceneModel`.
- **Reuses `LoadsPanel`'s exact pattern deliberately, not a new one**:
  `ImGui::Selectable(..., ImGuiSelectableFlags_SpanAllColumns)` per row
  syncing `Selection` (so the 3D viewport highlights/frames the clicked
  row), inline `ImGui::InputFloat` + `IsItemDeactivatedAfterEdit()` for
  numeric edits, `SmallButton("Delete")` per row. Joint position edits go
  through `EditableStructure::MoveJoint()` -- the **same** call path the
  ImGuizmo gizmo and Properties panel's numeric fields already use, so
  this is a third *entry point* to move a joint, not a second
  implementation of movement.
- **Deleting a joint that has connected members now shows a confirmation
  modal listing which members will also be deleted, instead of deleting
  silently.** `EditableStructure::DeleteJoint()` itself is unchanged --
  it already cascades (see the `gui/editor/` note above) -- this panel
  just computes the touching-member list itself (`TouchingMembers()` in
  `JointsMembersPanel.cpp`, a read-only walk of `sd.JJ`/`sd.JK`) *before*
  calling `DeleteJoint()`, and only calls it immediately (no modal) when
  that list is empty. A joint delete from the 3D viewport or Properties
  panel (if either grows one later) does **not** get this warning --
  scoped to this panel only, per issue #21's acceptance criteria; extend
  the same `TouchingMembers()` + modal pattern there if a future issue
  asks for it.
- **The confirmation popup's `OpenPopup`/`BeginPopupModal` calls
  deliberately live outside each row's `ImGui::PushID(joint_id)` scope.**
  `ImGui::OpenPopup(str_id)` hashes the popup's ID together with
  whatever's on the current ID stack -- calling it *inside* a row's
  `PushID` would produce a per-row-unique popup ID that
  `BeginPopupModal` (called once, outside any row loop) could never
  match, so the modal would silently never open. The fix: the Delete
  button handler only sets a `bool& open_delete_popup` out-parameter and
  pending-state fields; `JointsMembersPanel::Draw()` calls
  `ImGui::OpenPopup()` itself after the table/row loop has fully
  unwound. Caught by reasoning through ImGui's ID-stack model while
  writing this, not by interactive testing -- **this panel has not yet
  been interactively verified in a running build** (no local build
  toolchain available in this environment this session, see Validation
  below); re-verify the modal actually opens on a real cascade-delete
  before treating #21 as fully done.
- **What was verified:** the code follows `LoadsPanel`'s and
  `PropertiesPanel`'s already-proven patterns (row selection sync,
  inline edit commit, `math3d::Vec3` construction) exactly, and
  `EditableStructure`'s public API/field names (`JJ`/`JK`/`M`/`SdForUndo()`)
  were cross-checked against `EditableStructure.h`/`.cpp` directly.
  **Compiled successfully** in a later session (MSVC/Ninja,
  `windows-release` preset, both `orcisf_cli` and `orcisf_gui` targets) --
  see the #16 entry below for how a build toolchain became available.
  Interactive exercise of the cascade-delete modal specifically (does it
  actually pop up on a real multi-member joint delete) was not re-checked
  -- CI plus a real interactive pass are still the next steps for full
  confidence.

**2D plane-locked drawing (issue #22, part of epic #20) — read before
touching `gui/viewport/Camera.{h,cpp}`, `gui/viewport/Math3D.h`'s
`Orthographic()`, or `ViewportPanel`'s `add_joint_mode` branch:**
- **`ViewPlane` (`Free`/`XY`/`XZ`/`YZ`) lives in `gui/editor/Selection.h`,
  not `Camera.h`**, alongside `LoadPlacementMode` -- `EditorOptions` needs
  it (new `view_plane` + independently-remembered `plane_offset_xy`/
  `plane_offset_xz`/`plane_offset_yz` fields), and `Camera.h` includes
  `Selection.h` to reuse the same enum rather than duplicating it. This
  makes the viewport layer depend on the editor layer's header (the
  opposite direction from this project's usual layering) but doesn't
  create an include cycle -- `Selection.h` has no dependencies of its own.
- **`Camera` bypasses yaw/pitch entirely in a locked plane**, via two new
  private members (`ortho_forward_`/`ortho_right_`) set by
  `SetViewPlane()`. This isn't just style: `Forward()`'s usual yaw/pitch
  formula can point straight along world Y (the XZ/top-down plane), and
  `Right()`'s usual `Cross(Forward(), WorldUp())` degenerates to zero
  when `Forward() == WorldUp()` -- a real gimbal-lock case, not a
  hypothetical one, since a top-down structural view is exactly what
  issue #22 asks for. `ViewMatrix()`'s up-vector hint has the matching
  fix (`Cross(Right(), Forward())` instead of `WorldUp()` once locked).
- **A real sign bug was caught by hand-deriving the cross product, not by
  running it** (no build toolchain was available this session -- see
  Validation below): the Y-Z (elevation) plane's `ortho_right_` was
  initially `{0,0,1}`, which makes `Cross(right,forward)` (the up vector
  every Camera method derives, matching `Pan()`/`ScreenRay()`'s existing
  pattern) work out to world **-Y** -- since Y is this project's vertical
  axis, that would have rendered the elevation view upside-down. Fixed to
  `{0,0,-1}`. The X-Y and X-Z planes' basis vectors were hand-verified
  the same way and are correct as written.
- **Orthographic projection reuses `Camera::distance`** (the same field
  `Zoom()` already scales via scroll) as the ortho frustum's half-height,
  so zooming feels consistent switching between perspective and a locked
  plane -- no separate "ortho zoom level" field.
- **`ScreenRay()` returns parallel rays in locked-plane mode** (all
  sharing `Forward()` as direction, fanned out by `right`/`up` instead of
  converging at the eye) to match the orthographic projection.
  `ViewportPanel::HandlePicking()`'s `add_joint_mode` branch exploits
  this: since the ray direction *is* the locked axis, intersecting at any
  `t` (it uses `camera_.distance`) lands exactly on the plane, and the
  locked coordinate is then force-set to the exact configured offset
  (not left to whatever the ray math produced) so floating-point drift
  never nudges a placed point off a plane the user explicitly typed/slid
  to. Grid-snap (if enabled) only snaps the two *free* axes for the same
  reason -- snapping the locked axis too could silently override the
  offset.
- **Orbit (mouse-drag and arrow-key) is a no-op while a plane is
  locked** -- orbiting would rotate away from the locked plane, which
  would contradict "locked". Pan and zoom still work normally so the
  user can navigate within the plane. `ImGuizmo::SetOrthographic()` is
  now set from `camera_.IsOrthographic()` each frame (was hardcoded
  `false`) so the gizmo's own hit-testing matches the actual projection.
- **The plane selector lives in `Toolbar`'s "View Plane" menu; the offset
  itself is edited in the viewport** (moved there by issue #24 after the
  original menu-only placement proved unreachable/undiscoverable while
  looking at a locked plane -- see #24's own section below for the full
  story). Originally (as first implemented here) the viewport only drew a
  read-only text readout; that's now a real interactive control docked
  under the UCS icon (issue #23).
- **What was verified:** the cross-product/up-vector math was
  hand-derived and re-checked (catching the Y-Z sign bug above); the
  code was reviewed against `Camera.h`/`.cpp`'s existing patterns
  (`Pan()`/`ScreenRay()`'s `Cross(right,forward)` up-derivation,
  `EyePosition()`'s `target - Forward()*distance`). **Compiled
  successfully** in a later session (MSVC/Ninja, `windows-release`
  preset) -- see the #16 entry below. The three plane views' actual
  on-screen rendering and click-to-place-on-the-locked-plane behavior
  were not interactively exercised in a running app; that's still the
  next step for full confidence (CI plus a real interactive pass).

**"Re-optimize from last best result" (issue #16, part of epic #13) — read
before touching `engine::OptimizationOptions`'s `seed_*` fields,
`Optimizer.cpp`'s `AcakVariabel()`, or `RunPanel`'s `has_result_`/
`reoptimize_from_last_`:**
- **This is a real engine-level change, not just a GUI option** -- per
  the issue's own explicit warning. `AcakVariabel()` (port of
  `Pengacakan.hpp`'s `acak_variabel()`) normally randomizes population
  slot 0; when `OptimizationOptions::seed_from_previous_best` is true and
  `seed_var_b`/`seed_var_k` exactly match this run's `12*jum_balok`/
  `5*jum_kolom` sizes, a new `SeedStrukturAwal()` copies that design into
  slot 0 instead. Slot 1 (`CariStrukturAwal()`'s all-max-index reference
  design) and slots 2..JSTD-1 (random) are untouched either way.
  `engine/README.md` has the full writeup, including why the seeded
  design's cost can never come out worse than the seed's own cost
  (the evaluation-then-sort-then-search-only-improves-or-shrinks
  argument) -- read that before changing any of this logic.
- **The size-match validity check runs in the engine itself, not only in
  the GUI** -- a mismatch (edited geometry since the seed was captured,
  changing beam/column classification) silently falls back to a normal
  random slot 0, never a crash or OOB write. `RunPanel` additionally only
  *offers* the checkbox when its own `has_result_`/`result_dataset_path_`
  say a completed run exists for the exact dataset path currently
  entered -- defense in depth, not the only check.
- **`RunPanel::has_result_` is stricter than "a run finished which
  didn't error"** -- it's specifically "the run that finished was neither
  cancelled nor errored, and hasn't been superseded by a newer run
  starting" (cleared unconditionally at the top of every `StartRun()`,
  set true only in `Draw()`'s success branch). This matters because
  `result_sd_` is reused as the *next* run's output buffer too -- reading
  its best-slot design for seeding must happen in `StartRun()` (UI
  thread, before the worker thread starts overwriting `result_sd_`), not
  lazily later.
- **Doesn't affect issue #4's threading determinism**: seeding replaces
  one RNG draw (`Randomisasi()` for slot 0) with a plain array copy,
  entirely inside the single-threaded population-generation step that
  runs *before* any worker threads exist -- `worker_threads` still only
  changes wall-clock time, never the numeric result, with or without
  seeding.
- **What was verified:** compiled cleanly (both `orcisf_cli` and
  `orcisf_gui`, MSVC/Ninja, `windows-release` preset) and empirically
  exercised via a standalone scratch program linked against
  `orcisf_engine` (not checked in) against a scratch copy of
  `Example/Data01`: a truncated baseline run
  (`harga=3.02672e+07 kendala=0`) followed by a seeded re-optimization
  (`harga=2.46519e+07 kendala=0`) -- lower cost, confirming the seed took
  effect and the search genuinely continues rather than restarting.
  Re-running the seeded case with `worker_threads=4` produced a
  bit-identical result to `worker_threads=1`, confirming issue #4's
  determinism guarantee holds with seeding active. Full details and exact
  numbers in `engine/README.md`'s "Re-optimize from last best" section.
  The GUI checkbox/wiring itself (`RunPanel`'s `has_result_`/
  `reoptimize_from_last_`) was reviewed but not interactively clicked
  through in a running app this session -- the underlying engine
  mechanism it drives is what was directly verified.

**UCS icon overlay (issue #23, part of epic #20) — read before touching
`ViewportPanel.cpp`'s `DrawUcsIcon()`:**
- **Hand-drawn `ImDrawList` overlay, not a 3D gizmo mesh added to
  `SceneRenderer`** -- per the issue's own design note and this project's
  established preference for small, dependency-free GUI code
  (`IconToolbar.cpp`'s icons, same reasoning). Lives as a free function in
  `ViewportPanel.cpp`'s anonymous namespace, called from `Draw()`.
- **Projected via dot products against `Camera::Right()`/`Forward()`, not
  a full view-matrix multiply.** `sx = Dot(axis, right)`,
  `sy = Dot(axis, Cross(right, forward))` gives each world axis's 2D
  screen-space direction directly -- correct in both perspective and the
  orthographic/plane-locked views (#22) since it depends only on camera
  orientation, not projection type, and reuses the exact up-vector
  derivation `Camera::Pan()`/`ScreenRay()` already use internally (no new
  math to get wrong here, unlike #22's cross-product sign bug).
  `Camera::Right()`/`Forward()` are already public.
- **Never creates an ImGui item** (draws straight onto
  `ImGui::GetForegroundDrawList()`, no `InvisibleButton`/etc.), so it
  can't intercept orbit/pan/click-to-place input -- satisfies that
  acceptance criterion by construction, not by an explicit input-passthrough
  check.
- **Shares the viewport's bottom-left corner with #24's plane-offset
  control** -- the UCS icon is drawn first, unconditionally (every frame,
  every view mode), positioned with enough clearance below it
  (`kUcsBottomGap` in `ViewportPanel.cpp`) for that control to render
  underneath without running off the bottom of the viewport. Originally
  (issue #23) it shared the corner with a read-only text readout to its
  *right* instead -- #24 replaced that readout with an interactive
  control docked *below* the icon; see #24's own section for why.
- **What was verified:** the dot-product projection math was
  hand-checked for a few reference orientations (default orbit camera:
  X should point right-ish, Y up, Z out-of-screen-ish) and the code
  reuses already-verified `Camera` methods rather than new vector math.
  **Compiled successfully** (MSVC/Ninja, `windows-release` preset) and
  the app was launched and confirmed running without crashing (see #24's
  entry below for the same check after that issue's changes). The icon's
  live rotation-with-camera behavior was not interactively click-tested
  pixel-by-pixel; still a good next step for full confidence.

**Plane-offset control moved into the viewport (issue #24, follow-up to
#22/#23) — read before touching `ViewportPanel.cpp`'s plane-offset
overlay or `Toolbar.cpp`'s "View Plane" menu:**
- **User-reported gap, not a speculative improvement**: #22 put the
  offset `InputFloat`/`SliderFloat` inside Toolbar's "View Plane" menu --
  reopening a dropdown for every adjustment made the offset practically
  unreachable while a plane was locked at the default 0.000 and the user
  was looking at the 3D view. Fixed by moving the actual editing widgets
  into `ViewportPanel::Draw()`, docked directly under the UCS icon (#23)
  in the viewport's bottom-left corner; `Toolbar`'s menu now only selects
  *which* plane is active (Free/X-Y/X-Z/Y-Z), not its offset -- **one
  place edits `plane_offset_xy`/`_xz`/`_yz`, not two** (the old menu
  slider/input was deleted, not just supplemented, per the issue's own
  "don't leave two inconsistent widgets" acceptance criterion).
- **A real ImGui overlay, not a foreground-drawlist-only readout like
  #23's UCS icon.** `ImGui::SetCursorScreenPos()` places a label +
  `InputFloat` + `SliderFloat` at an arbitrary screen position *within
  the already-open "Viewport" window* (no separate `Begin`/`End`), drawn
  after `ImGui::Image()` so they render on top of it and naturally win
  hover/click priority over the image at that screen position -- the
  same "later-submitted widget wins" rule any overlapping ImGui widgets
  follow, no special-casing needed for the rendering itself.
- **`hovered` (captured once, right after `Image()`) can't tell the
  orbit-click-start logic about these widgets, because they're submitted
  *after* that capture** -- a real bug that would have let a click on the
  slider also start an orbit-drag underneath it. Fixed with a separate
  `offset_overlay_capturing` flag (`ImGui::IsItemHovered() ||
  IsItemActive()` on each of the two widgets), ANDed into every
  orbit/pan/zoom-start gate below (mirroring how `gizmo_capturing`
  already gates those same lines for ImGuizmo) -- caught by reasoning
  through ImGui's submission-order hover model while writing this, not
  by interactive testing.
- **What was verified:** compiled cleanly (MSVC/Ninja, `windows-release`
  preset) and the app was launched and confirmed running (process alive,
  no crash) after the change. **Not interactively click-tested** --
  specifically, whether dragging the slider/typing in the field actually
  avoids triggering orbit, and whether a placed joint's locked coordinate
  exactly matches a nonzero slider value, are the issue's own stated
  acceptance criteria and still need a real interactive pass (or a
  screenshot-based check) before treating #24 as fully done.

**Custom borderless window chrome (issue #19, Phase 0) — read before
touching `app/CustomTitleBar.{h,cpp}`/`app/Theme.{h,cpp}`/`main.cpp`'s
window setup:**
- **The title bar is drawn *inside* `Toolbar`'s main menu bar row, not as
  its own ImGui window.** This was not a style choice -- a standalone
  top-of-viewport window (positioned via `SetNextWindowPos`/manually
  shrinking `viewport->WorkPos`, the same trick `IconToolbar` uses to
  stack *below* the menu bar) rendered **zero visible pixels** when
  placed *above* the menu bar: `ImGui::BeginMainMenuBar()` always claims
  the true top of the viewport for itself regardless of what another
  window already drew there, so the later-submitted menu bar painted
  completely over it every frame. Confirmed with a bright debug fill
  color that never appeared on screen. The fix: `Toolbar::SetTitleBarDrawer()`
  is a generic callback `Toolbar::Draw()` invokes right before
  `EndMainMenuBar()`, and `Application` wires it to
  `CustomTitleBar::Draw()`, which draws title text + a drag zone +
  Minimize/Maximize/Close *as ordinary widgets inside the already-open
  menu bar window* (matching how VS Code's own custom title bar embeds
  its menu). If you ever need a second "always on top" ImGui bar, don't
  repeat the standalone-window approach without accounting for this.
- **`style.ItemSpacing` (nonzero in `ApplyModernTheme()`) breaks
  right-aligned button math if you forget it.** The three title-bar
  buttons are laid out with `ImGui::SameLine(x, 0.0f)` (explicit
  zero spacing) between them -- using default `SameLine()` silently
  widens the cluster by `2 * ItemSpacing.x` and pushes the Close button
  past the window's right edge, making it unclickable/invisible. Caught
  interactively (Close was missing from a screenshot), not by code
  review.
- **`Application` now owns a raw `GLFWwindow*` (`window_`, set once via
  `Application::SetWindow()` from `main.cpp` after construction)** --
  the one deliberate exception to "only `main.cpp` touches GLFW types"
  the rest of this port has followed since #2. It's stored opaquely
  (forward-declared `struct GLFWwindow;`) and only ever handed to
  `CustomTitleBar::Draw()`; `Application`/`Toolbar` never call a GLFW
  function themselves.
- **Dragging is anchored to an absolute screen-cursor position captured
  once at drag start (`ImGui::IsItemActivated()`), not accumulated
  per-frame `ImGui::GetIO().MouseDelta`.** This is not a native OS drag
  either way (that's Phase 1+, see below) -- but the delta-based version
  was tried first and **visibly jittered/shook while dragging**, caught
  interactively. Root cause: `glfwSetWindowPos()` moves the window but
  not the physical cursor, so the cursor's *window-relative* position
  (what GLFW's mouse callback, and therefore ImGui's `MouseDelta`,
  reports) silently shifts by the exact opposite of the move on the very
  next frame -- a self-inflicted feedback oscillation. The fix
  (`dragging_`/`drag_start_cursor_screen_*_`/`drag_start_window_*_`
  members) recomputes `glfwGetWindowPos() + glfwGetCursorPos()` fresh
  every frame (both always-current, not event-cached) and references
  everything back to the drag-start anchor, which has no feedback path.
  Verified with a scripted small-step drag logging window position after
  each step: perfectly linear/monotonic movement, zero oscillation, in
  both directions and through negative (partially off-screen) X.
  Don't revert to the delta-based version without re-testing this.
- This whole mechanism (whichever version) is deliberate Phase 0 scope
  (see the issue's platform-agnostic-baseline rationale) -- Phase 1+
  replaces it with real native dragging per platform. It's a no-op on
  Wayland (`glfwGetPlatform() == GLFW_PLATFORM_WAYLAND`), surfaced via a
  disabled-looking drag zone + tooltip rather than failing silently.
- **What was verified interactively:** borderless window (no OS title
  bar/border) confirmed; title text renders in the menu bar row;
  Minimize/Maximize/Close icons render correctly including the
  Maximize<->Restore icon swap; clicking Maximize actually maximized the
  window (screen-filling, confirmed via screenshot) and clicking
  Maximize again restored it; the drag mechanism was confirmed working
  emphatically -- an in-progress click sequence during testing produced
  a real (if unintentionally large) window-position delta that moved the
  live window off-screen, which was then recovered with a plain Win32
  `SetWindowPos` call (not a code change) to bring it back on screen.
  macOS/Linux were not interactively verified in this environment (no
  access to those platforms); verify via a real build on each before
  treating Phase 0 as fully done there, per the issue's acceptance
  criteria.

**DPI awareness (issue #31) — read before touching `main.cpp`'s startup
sequence or its `EnableWindowsDpiAwareness()`/font/style-scaling code:**
- **Root cause was a genuine, total absence of DPI handling** -- nothing
  in `src/` declared DPI awareness or reacted to monitor content scale
  before this fix, confirmed by code search. On a multi-monitor setup
  with mixed DPI, Windows either bitmap-stretched the whole window
  (blurry) or rendered it at a fixed logical size that didn't match a
  high-DPI monitor's actual pixel density -- showing up as "fonts too
  small on the high-resolution monitor, fine on the mid-resolution one."
- **Runtime call, not an embedded manifest.** `EnableWindowsDpiAwareness()`
  (Windows-only, called as the very first thing in `main()`, before even
  `glfwInit()` -- Windows locks in the process's DPI-awareness mode the
  first time it's queried) resolves `SetProcessDpiAwarenessContext` via
  `GetProcAddress` rather than linking it directly or depending on
  `<windows.h>`'s `DPI_AWARENESS_CONTEXT`/
  `DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2` (only declared when the
  SDK's `WINVER` target is Windows 10 1703+, which this project doesn't
  otherwise require) -- a locally-defined opaque `void*` handle type and
  the constant's documented fixed value (`-4`) are used instead, so this
  compiles regardless of SDK version and is a graceful no-op on older
  Windows 10 builds that don't export the function.
- **`gui/UiScale.{h,cpp}` is the one source of truth for the scale.**
  `main.cpp` publishes it via `gui::SetUiScale()`; every hand-drawn piece
  of chrome reads it via `gui::Scaled(base_value)`. It lives in `gui/`,
  not `app/`, purely so `app/` (which already depends on `gui/`) can read
  it without inverting this port's layering. **Any new hand-drawn
  `ImDrawList` UI must write its pixel constants as `Scaled(x)`** --
  ImGui scales its own widgets and font, but knows nothing about
  `AddLine`/`AddRect`/`AddCircle` coordinates, which is exactly how the
  toolbar icons ended up physically tiny on a 200% monitor while the text
  beside them was correct. Already converted: `IconToolbar.cpp`,
  `CustomTitleBar.cpp`, `PanelIcons.cpp`, `ViewportPanel.cpp`'s UCS icon
  and plane-offset overlay.
- **Four cooperating pieces, all needed together**: (1)
  `EnableWindowsDpiAwareness()` (Windows-only), (2)
  `glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE)` before
  `glfwCreateWindow()`, (3) `main.cpp`'s `ApplyUiScale(scale)`, which is
  the *only* place the scale is consumed -- style metrics, font size and
  `gui::SetUiScale()` all in one function so the startup path and the
  live-monitor-change path can never drift, and (4) a per-frame poll of
  `glfwGetWindowContentScale()` between `glfwPollEvents()` and
  `ImGui::NewFrame()` that calls `ApplyUiScale()` again on any change.
- **`ApplyUiScale()` restores a snapshot of the *entire* `ImGuiStyle`
  struct before scaling -- calling `ApplyModernTheme()` again is not a
  substitute, and this is not a stylistic preference.**
  `ImGuiStyle::ScaleAllSizes()` multiplies whatever is currently set (it
  is not idempotent), and it scales dozens of fields the theme never
  assigns: `WindowMinSize`, `CellPadding`, `DockingSeparatorSize`,
  `TabMinWidth*`, `DisplaySafeAreaPadding`, `TouchExtraPadding`, ... Only
  re-running the theme leaves all of those permanently compounded. The
  observed symptom was **docked panels losing their tab bars for good
  after a 100% -> 200% -> 100% round trip** (a doubled `WindowMinSize`/
  `DockingSeparatorSize` surviving at 100%) -- found by actually driving
  the round trip and screenshotting it, not by reading the code. The
  snapshot is taken on the first call, before any scaling, and
  `FontSizeBase` (maintained by ImGui per frame, not by the theme) is
  carried across the restore rather than stamped from the snapshot.
- **Fonts scale via `style.FontScaleDpi`, not a rebuilt atlas.** This
  project is on Dear ImGui **1.92.8**, whose dynamic font system
  re-rasterizes at the new pixel size automatically and stays crisp --
  the previous `io.Fonts->AddFontDefault()` at `13.0f * dpi_scale` baked
  the scale into the atlas at startup with no way to change it later, so
  it could not have supported live rescaling at all. Don't reintroduce a
  manual atlas rebuild here.
- **`IconToolbar::kHeight` is now `IconToolbar::Height()`** (a function of
  the scale; `kBaseHeight` is the 100% value). `Application::OnFrame()`'s
  dockspace work-area reservation calls the same function -- never
  reintroduce a second literal, or panels overlap the toolbar at any
  scale != 1.0. `IconToolbar::Draw()` also positions itself from
  `viewport->WorkPos` rather than re-deriving the menu bar height from
  `GetFrameHeight()`, and `BuildDockspace()` seeds the builder from
  `WorkPos`/`WorkSize` rather than the full viewport.
- **`CustomTitleBar`'s button cluster is positioned with
  `SetCursorScreenPos()`, never `SameLine(offset_x)`** -- this was the
  actual root cause of the long-standing "Close button not flush /
  clipped at the right edge" report (issue #27 previously closed it as
  not-reproducible; it was real, just subtle). `SameLine()`'s offset is
  measured from the content origin, and `BeginMenuBar()` opens a group
  whose `GroupOffset.x == WindowPadding.x`, so the whole cluster was
  silently pushed `WindowPadding.x` past the window's right edge. At 100%
  that is `ApplyModernTheme()`'s 14px, which reads as "the X glyph sits
  right of centre"; at 200% it is 28px of a 92px button, i.e. visibly cut
  off. Absolute screen coordinates have no such hidden term.
- **The window title is elided to fit the drag zone.** It is drawn
  straight onto the draw list, which does no layout, so at a large scale
  the (unchanged) title string simply grew until it ran underneath the
  Minimize/Maximize/Close glyphs -- both unreadable. Now trimmed with an
  ellipsis against the same `buttons_start_screen_x` anchor.
- **`ORCISF_UI_SCALE` overrides the monitor's reported scale** (e.g.
  `set ORCISF_UI_SCALE=2`). It exists mainly so high-DPI layout is
  testable on a machine with only a 100% monitor attached -- which is how
  these symptoms went unreproduced for several passes. Use it before
  claiming a DPI-related layout bug can't be reproduced locally.
- **What was verified:** compiled cleanly (MSVC/Ninja, `windows-release`)
  and **visually confirmed via real DPI-aware screenshots** at 100%,
  150% and 200%: toolbar/panel icons scale, the title bar buttons are
  fully visible and correctly centred (measured -- the Close glyph sits
  exactly half a button width from the true right edge at both 100% and
  200%), the title elides instead of colliding with the buttons, and no
  docked panel is clipped under the menu bar/toolbar. Live rescaling was
  exercised through the real per-frame path via a temporary key-driven
  scale toggle (since removed) across four consecutive
  100%<->200% round trips, screenshotted each time: the final 100% state
  is identical to a fresh 100% launch, confirming no compounding.
  **Not verified**: an actual two-physical-monitor drag (only one
  monitor is available in this environment) and macOS/Linux -- the
  mechanism is the same `glfwGetWindowContentScale()` poll either way,
  but the specific drag gesture is still the user's own office-machine
  test.

**`gui/viewport/` (issue #5) — three things worth knowing before touching it:**
- **Member cross-section thickness/orientation is a schematic
  approximation, not the legacy local-axis convention.** `SceneRenderer`
  orients each member's box via `Cross()`-derived basis vectors from the
  member's own direction (picking world-Y, or world-Z if near-vertical, as
  a reference), *not* the legacy `R11..R33` rotation matrices
  `StructuralAnalysis.cpp` computes during `Struktur()`. This was a
  deliberate simplification to keep rendering independent of whether an
  analysis has actually run (geometry-only views load fine with no
  `Struktur()` call) — don't "fix" it to match `R11..R33` without checking
  whether that still holds.
- **Normals skip a proper normal matrix on purpose.** See the comment
  above `kVertexShaderBody` in `SceneRenderer.cpp` for the (specific to
  this exact case: unit-cube axis-aligned normals + positive-diagonal-scale
  model matrices) derivation of why `mat3(uModel) * aNormal` is correct
  here without `transpose(inverse(model))` — don't copy that shortcut
  into a renderer with non-axis-aligned meshes or negative scale.
- **`PickMember()` (`SceneModel.cpp`) is pure geometry (ray-vs-segment,
  Ericson's closest-point algorithm with one side unbounded), independent
  of OpenGL** — verified with a standalone test program (not checked in)
  against synthetic scenes before wiring it into `ViewportPanel`'s click
  handling, since a live click-through-the-3D-view test isn't automatable
  in this environment.
- **MinGW/local-dev-only:** `CMakeLists.txt` copies the compiler's own
  `libstdc++-6.dll`/`libgcc_s_seh-1.dll`/`libwinpthread-1.dll` next to
  `orcisf_gui.exe` post-build when `MINGW` is true (no-op on the MSVC/
  x64-windows triplet CI actually uses). This was necessary because
  another installed program's bin directory (PostgreSQL, in this case)
  shipped an ABI-incompatible `libwinpthread-1.dll` earlier in PATH,
  causing an "Entry Point Not Found: clock_gettime64" launch failure that
  had nothing to do with this project's own code — a real hazard for any
  MinGW-based Windows dev environment with other MSYS2/MinGW-adjacent
  tools installed, not just this one.

**`engine/` (issue #3) — read
[`src/engine/README.md`](src/engine/README.md) before touching it.** It's a
line-by-line port of `Optimasi Beton/Source/*.hpp` (direct-stiffness
analysis, RC beam/column design, the Flexible Polyhedron optimizer) into a
headless library with no GUI/console dependency, keeping the legacy
Indonesian identifiers and 1-based array indexing on purpose (see
`StructureData.h`) for line-by-line checkability against the original. Two
things every future agent touching it must know:
- **A handful of legacy behaviors are deliberately preserved even though
  they look like bugs or asymmetries** (e.g. `KendalaHarga()` adjusts a
  column's stirrup spacing/side length but not a beam's, unlike the
  optimizer's own per-generation evaluation path; `WriteFinalResults()`'s
  `.str` section reflects a different population slot's geometry than its
  `.opt`/`.kdl` sections). **Do not "fix" these without checking
  `engine/README.md`'s "Deliberate deviations" section first** — they're
  characteristics of the 1999 algorithm, not oversights in the port, and
  "fixing" them would silently change numerical output.
- **Multi-threading (#4) is a fork-join per hot loop, not a persistent pool.**
  `RunOptimization()` builds `options.worker_threads - 1` extra
  `StructureData` clones once (each worker needs its own scratch state —
  `Inersia`/`Struktur`/design/constraint functions all mutate fields like
  `SFF`/`SM`/`AM` on whichever `StructureData` they're given), then spawns
  `std::thread`s per generation via `RunOnLanes` for two loops:
  `EvaluatePopulationParallel` (the initial population, `JSTD` candidates,
  once) and `CariBaruParallel` (`cari_baru()`'s trial search, every
  generation). **Do not resurrect a full `StructureData` copy inside the
  per-generation sync path** — that was tried and measured ~3x *slower*
  than single-threaded (a `StructureData` is ~14MB; copying it every
  generation dwarfs the actual trial-search work for modest structures).
  `SyncSearchState()` copies only the JVD-sized fields a trial actually
  reads instead. Below `kMinTrialsPerWorker` trials/lane,
  `CariBaruParallel` falls back to the identical sequential `CariBaru()` —
  for the bundled `Example/` datasets (M≈21 members) `TrialCount()` is
  almost always in the low single digits, so most generations take that
  fallback; this is expected, not a bug, and real wall-clock speedup should
  be expected mainly on larger structures than what's bundled here.
  Determinism is verified (bit-identical `POPULATION_DUMP` output between
  `worker_threads=1` and `worker_threads=8` for the same `rng_seed`, with
  the fallback threshold temporarily forced low to actually exercise the
  parallel merge path rather than only the sequential fallback) — see
  `engine/README.md`'s Validation section for the exact commands.
- **Never point the CLI/GUI's output at `Optimasi Beton/Example/` directly**
  — output filenames are case-insensitive-colliding with the checked-in
  reference files on Windows (`GEDUNG.opt` == `gedung.opt`). Always use a
  scratch copy. (This is exactly how the checked-in Example outputs almost
  got overwritten during #3's own validation — restored via `git restore`,
  no harm done, but a real hazard for the next agent too.)

---

## Program Architecture (`Optimasi Beton/Source/`)

Written entirely in **Indonesian** (identifiers, comments, console prompts) —
this is intentional and part of the historical record. Keep new comments/strings
in the same language if extending this legacy code; do not silently translate
existing files. The program is **header-only** (`.hpp`) plus a single tiny
`.cpp` entry point; there is no build system beyond the Borland IDE project file
`ORCISF.ide` (binary, IDE-generated — don't hand-edit it).

- **Entry point:** [`ORCISF.cpp`](Optimasi%20Beton/Source/ORCISF.cpp) — defines
  `const int mak=825;` (the global max-array-size bound used throughout every
  fixed-size array/matrix in the program) and `void main()`, which just calls
  `menu_utama()`.
- **Aggregating header:** [`Header.hpp`](Optimasi%20Beton/Source/Header.hpp)
  `#include`s every module in dependency order; `Proto.hpp` holds every function
  prototype up front (old-style C++, no separate `.cpp`/translation units).
- **Kernel:** `Proto.hpp` (prototypes), `Variabel.hpp` (**all** global
  state/arrays — sized `[mak]`), `InOut.hpp` (interactive console I/O + generic
  filename → extension wiring).
- **Structural mechanics** (direct-stiffness method for 3D space frames,
  developed from Weaver & Gere): `Struktur.hpp` (`struktur()` assembles &
  solves), `Pembebanan.hpp` (loads), `Solver.hpp` (`banfac`/`bansol` — modified
  Choleski banded-matrix factorization/solve).
- **Reinforced-concrete design:**
  - `Kolom.hpp` — class `kolom`: biaxial column capacity/design check
    (slenderness, PN/MNX/MNY interaction, stirrup requirements).
  - `Balok.hpp` — class `balok`: beam flexural/shear design (lapangan = midspan
    vs. tumpuan = support regions, deflection check).
  - `ELEMEN.HPP` — fills beam/column element properties from the current
    discretized design-variable set (`isi_elemen_balok`, `isi_elemen_kolom`).
  - Instantiating a `balok`/`kolom` object as a bare local (e.g. `balok
    lapangan;`) is the pattern used throughout to trigger its constructor's
    side-effecting calculation, which sets the shared globals `kendala`
    (constraint violation) and `harga` (cost) — **not idiomatic modern C++**,
    but consistent throughout; preserve the pattern if touching this code.
- **Optimization — "Flexible Polyhedron" direct search**
  (generalized-simplex / Nelder-Mead family):
  - `Polyhedron.hpp` — `optimasi()`, the main driver (see below).
  - `Penormalan.hpp` — `normalisasi_*`/`unnormalisasi`: pack/unpack the combined
    beam+column discrete design vector.
  - `Pengacakan.hpp` — `acak_variabel()`: randomizes the initial population of
    candidate designs ("struktur" 0..`JSTD-1`).
  - `Diskritisasi.hpp` — `konversi()`: rounds a float to the nearest discrete
    index (design variables are indices into discretized value tables, not
    continuous values — see **Discrete design variables** below).
  - `Kendala.hpp` — `Kendala_Harga()`: for a given design-variable set, runs the
    RC design checks for every beam/column and returns the **fitness**
    `finalti / (harga + finalti*kendala)` (cost combined with a constraint
    penalty; `finalti` is the user-supplied penalty factor).
  - `Telusur.hpp` — `penelusuran()`: locates the search direction from the
    midpoint of the "good" points toward/away from the worst point.
  - `Baru.hpp` — `cari_baru()`: reflects/expands along that direction to find a
    new candidate point.
  - `Pengurutan.hpp` — `sort()`: bubble-sorts the population by fitness.
  - **Driver flow** (`optimasi()` in `Polyhedron.hpp`): classify each member as
    beam or column by inclination (`periksa_batang`, `CXZ`) → design-variable
    count `JVD = 12*jum_balok + 5*jum_kolom` → population size
    `JSTD = JVD*fak_kali + fak_plus` → generate & evaluate `JSTD` random initial
    designs → sort by fitness → loop: search direction → new point → replace
    worst if better, else shrink ("penyusutan") → check convergence (best
    fitness unchanged, or too many consecutive shrinks) or `j_iterasi_mak`
    generations → write `.his` log throughout, then `cetak_akhir()` writes the
    final `.opt`/`.str`/`.kdl`/`.inf` files.
- **Shell / presentation:** `Tampilan.hpp` (`menu_utama()` — the console menu,
  see **Running the program** below; `about()` — banner), `CETAK.HPP`
  (`cetak_akhir()` — writes the four final output files).
- `BorlandC.hpp` — the legacy Borland C++ 5.02 include set (`iostream.h`,
  `conio.h`, `dos.h`, `_defs.h`, …, all pre-standard headers with no
  namespaces). **This will not compile as-is on a modern compiler** (MSVC/GCC/
  Clang) — see **Build & Run** below.

### Discrete design variables (why the numbers look like indices, not sizes)

Cross-sections and reinforcement are **not** continuous — the user supplies
discrete candidate tables per structure (widths/heights/column sides in `.isd`,
bar diameters in `.idl`/`.ids`, bar counts in `.ijl`, stirrup spacings in
`.ijs`). A design variable (e.g. `var_b[struct][0]`) stores an **index into**
one of these tables (`sisi_d_B`, `DIA_d`, `NL_d`, `DIAS_d`, `JS_d`, …), and
`isi(index, table)` resolves it to the actual value. Per-beam there are **12**
such indices (B, H, top/bottom bar diameter+count at midspan, same at support,
stirrup diameter, stirrup spacing); per-column there are **5** (side, bar
diameter, bar count, stirrup diameter, stirrup spacing). This is what
`JVD = 12*jum_balok + 5*jum_kolom` counts.

---

## Data / File Format Convention (`InOut.hpp`, `Pembebanan.hpp`, `Tampilan.hpp`)

The user supplies one **generic base filename** (no extension, e.g. `contoh`);
the program derives every input/output filename by appending a fixed extension
(8.3-style, DOS-era). All input files are **plain ASCII** and editable in a text
editor. **Keep this table in sync with `InOut.hpp`/`Variabel.hpp` if the file
format is ever changed:**

| Ext | Direction | Content |
|---|---|---|
| `.inp` | in | General structure data (geometry, joints, restraints, materials) |
| `.isd` | in | Discrete beam width/height & column side dimensions |
| `.idl` | in | Discrete main (longitudinal) reinforcement diameters |
| `.ijl` | in | Discrete main reinforcement bar counts |
| `.ids` | in | Discrete stirrup diameters |
| `.ijs` | in | Discrete stirrup spacings |
| `.bbn` | in | Load data |
| `.his` | out | Optimization run history/log (one line per shrink/convergence event) |
| `.opt` | out | Final optimized beam/column dimensions & reinforcement |
| `.str` | out | Structural analysis results (forces/displacements) |
| `.kdl` | out | Constraint-check results |
| `.inf` | out | Echo of the input/geometry data actually used |

`Optimasi Beton/BacaSaya.txt` is the original author-written user manual for
this format (Indonesian) — read it before changing any I/O routine.

---

## Sample Datasets (`Optimasi Beton/Example/`)

Each subdirectory is a self-contained example with a matching set of the
extensions above:
- `Data01`, `Data02+3`, `Data02x2`, `Data03x2`, `Data04+3` — original thesis
  validation datasets (referenced from the `Lampiran`/`Isi` chapters).
- `Apl1-1`..`Apl1-5`, `Apl2-1`..`Apl2-5`, `Apl3-1`..`Apl3-5` — application/case
  study datasets (3 building configurations × 5 variants each).
- `2006/` — a later recompiled build (`orcisf.exe` + `cw3230.dll` dated 2006)
  bundled with its own `aplikasi.*` dataset; kept alongside the 1999 binary in
  the root of `Optimasi Beton/` as a historical snapshot.

---

## Running the Program (as originally shipped)

`Optimasi Beton/orcisf.exe` is a **Win32 console application**; it requires
`cw3230.dll` (Borland runtime) in the same directory. Run it from a Windows
console (`cmd.exe`) in that folder. The interactive menu (`menu_utama()` in
`Tampilan.hpp`):

1. **Input data awal ke file** — enter general + discrete design data, writes
   `.inp`/`.isd`/`.idl`/`.ijl`/`.ids`/`.ijs`.
2. **Input data beban ke file** — enter load data, writes `.bbn`.
3. **Melihat isi file input** — browse an existing input set (general data,
   joint coordinates, member info, restraints, fixed-end forces, joint loads).
4. **Mengoptimasi struktur** — prompts for the generic filename, concrete/steel
   unit prices, column/beam cover thickness, penalty factor, max iterations,
   and the population-size add/multiply factors, then runs `optimasi()`
   (per `BacaSaya.txt`: use ≥10000 iterations for good results).
5. **Keluar** — exit.

To run a bundled `Example/` case, enter its path relative to the working
directory as the generic filename (e.g. `example/contoh`), per `BacaSaya.txt`.

---

## Build & Run From Source (legacy toolchain)

The source targets **Borland C++ 5.02** for **Win32 console**, using
pre-standard headers (`iostream.h`, `conio.h`, `dos.h`, DOS-console calls like
`gotoxy`/`clrscr`/`textcolor`/`gettime`). It **cannot be compiled unmodified**
with a modern compiler (MSVC/GCC/Clang/MinGW) — those headers and console APIs
don't exist there. To rebuild as originally written you need Borland C++ 5.02
(or a compatible Borland/Turbo C++ toolchain) and open
[`ORCISF.ide`](Optimasi%20Beton/Source/ORCISF.ide). If asked to **port** this to
a modern toolchain, that is a nontrivial rewrite (replace `conio.h` console I/O,
`void main()` → `int main()`, `<iostream.h>` → `<iostream>` + `std::`, DOS timing
calls, etc.) — treat it as new work under `src/`, not an in-place edit of
`Optimasi Beton/Source/`, so the historical source stays intact.

---

## Conventions & Guardrails

- **`Optimasi Beton/` is a historical archive.** Do not "clean up," reformat,
  translate, or modernize the original 1999 source/binary/manuscript in place
  unless the user explicitly asks for a fix to that exact legacy code. New
  work (ports, tooling, docs) belongs elsewhere (`src/`, root-level docs).
- **Keep Indonesian identifiers/comments/strings** when editing existing legacy
  files — the codebase and thesis are historically and linguistically
  consistent; don't mix languages within a file.
- **`mak = 825`** (in `ORCISF.cpp`) is the compile-time bound for every
  `[mak]`-sized global array/matrix — changing it changes memory layout
  throughout and requires a full rebuild; it is not a runtime-configurable
  limit.
- **File-naming convention is load-bearing**: the generic-name + fixed-extension
  scheme is duplicated across `InOut.hpp`, `Pembebanan.hpp`, and `Tampilan.hpp`.
  If you ever change an extension or add a new I/O file, update **all three**
  plus the table in this file and in `BacaSaya.txt`.
- **`ORCISF.ide` is a binary, IDE-generated project file** — don't hand-edit it;
  regenerate via the Borland IDE if the file list changes.
- **`.doc` files under `Teori/`** are legacy MS Word 97-2003 binary format —
  don't attempt to "fix" or re-save them as part of unrelated changes.
- **No test harness exists.** The only historical validation is: run
  `orcisf.exe` (or a rebuild) against the `Example/` datasets and compare
  `.opt`/`.str`/`.kdl` output to what's already checked in for that example, or
  to the results discussed in `Teori/Isi/BAB IV.doc`.
- **License:** no license file is currently included in this repository. Don't
  add one speculatively — confirm with the user (the original author) first.

---

## Agent Skills (`.claude/skills/`)

AI agents working in this repo should check [`.claude/skills/`](.claude/skills/)
for skills that apply to the current task and follow them.

**Every skill in this project reads this file (`AGENTS.md`) first**, in
full, before doing anything else — not just the section that looks
relevant to its own job. This is a small, densely cross-referenced
project where a change in one area (the engine's 1-based arrays, a
DPI-scaling gotcha, a documented "deliberate deviation") routinely
matters to work that looks unrelated at first glance. Each skill's own
`SKILL.md` restates this, but it's the rule, not a suggestion any one
skill can skip because its task seems self-contained.

- **`planner`** ([`.claude/skills/planner/SKILL.md`](.claude/skills/planner/SKILL.md)) —
  context-aware issue tracker for GitHub. Use this instead of raw API
  calls/`curl` whenever creating, updating, or checking issues/tickets, even if
  the user doesn't say "GitHub" explicitly (e.g. "create a task", "log this as
  an issue", "plan this feature").
- **`coder`** ([`.claude/skills/coder/SKILL.md`](.claude/skills/coder/SKILL.md)) —
  picks up an open issue from the remote tracker, plans/splits the work, and
  implements the fix following this file's conventions. Use for "fix an
  issue", "work on a ticket", "implement issue #X". Shares remote-repo/token
  resolution with `planner` via [`.claude/skills/_shared/references/`](.claude/skills/_shared/references/).
- **`builder`** ([`.claude/skills/builder/SKILL.md`](.claude/skills/builder/SKILL.md)) —
  rebuilds `src/`'s `orcisf_gui`/`orcisf_cli` for whichever OS the agent is
  currently on, after a fix/issue/review lands. Use whenever about to verify a
  `src/` change and a fresh binary is needed. Runs the repo's own
  `src/build.ps1`/`src/build.sh` (issue #32) rather than re-deriving
  toolchain paths inline -- see this file's Validation section for the
  toolchain locations those scripts already know about in this environment.
- **`tester`** ([`.claude/skills/tester/SKILL.md`](.claude/skills/tester/SKILL.md)) —
  tests a freshly built binary against a specific GitHub issue's own
  Acceptance Criteria checklist, criterion by criterion (PASS/FAIL/
  UNVERIFIED, not a single aggregate verdict). Use for "test issue #X",
  "/tester pick #X". Calls `builder` for the binary rather than building
  inline, and reuses `planner`/`coder`'s repo/token resolution -- doesn't
  duplicate either. Never auto-closes the remote issue or writes to
  `AGENTS.md`/`CHANGE_HISTORY.md` without explicit confirmation, same as
  `planner`/`coder`.
- **`reviewer`** ([`.claude/skills/reviewer/SKILL.md`](.claude/skills/reviewer/SKILL.md)) —
  the final quality-assurance gate: audits code architecture/design,
  hunts for bugs and security gaps, and confirms an issue's Acceptance
  Criteria genuinely pass end-to-end (not per-criterion like `tester` --
  this is the human-facing sign-off). Reopens the issue with a comment if
  something's wrong, or comments + proposes closing it if it's genuinely
  done, and proposes a release if the change warrants one. Use for
  "review issue #X", "/reviewer pick #X", "is #X really done". Never
  takes a remote write action (reopen/comment/close/release) without
  explicit confirmation of that specific action.

### Agent Skill Workflow

This project's steady-state workflow for any change is a five-stage
pipeline, each stage owned by one skill:

```
planner  -->  coder  -->  builder  -->  tester  -->  reviewer
(create      (implement   (compile     (per-        (end-to-end QA,
 issue)       the fix)     a binary)    criterion     architecture/
                                        pass/fail)    security audit,
                                                       close/reopen,
                                                       propose release)
```

- **`planner`** turns a report/request into a well-formed GitHub issue
  with testable Acceptance Criteria (the contract every later stage
  checks against).
- **`coder`** implements against that issue's criteria, following this
  file's conventions.
- **`builder`** produces a fresh, verifiably current binary — no stage
  after this one should test or review a stale build.
- **`tester`** mechanically checks each Acceptance Criterion against
  that binary and reports PASS/FAIL/UNVERIFIED per item — narrow and
  literal, not a judgment call.
- **`reviewer`** is the judgment call: architecture/design fit, bug and
  security audit, confirming the feature genuinely works end-to-end (not
  just criterion-by-criterion), and then acting on the verdict — reopen
  the issue with a comment if something's wrong (routing back to
  `coder`), or comment and propose closing it if it's genuinely done. If
  the change included an actual `src/` code change (not just docs/
  tooling), `reviewer` also proposes whether a new release is warranted.

Not every change needs all five stages run as separate invocations in
one sitting — a trivial fix might reasonably combine `coder`+`builder`
verification inline — but for anything closing out a tracked issue,
running `tester` then `reviewer` before considering it truly done is
this project's expected practice, and skipping straight from `coder` to
closing an issue without either is a process gap worth flagging to the
user, not silently doing anyway.

Every stage that touches the remote issue tracker or creates a release
follows the same rule the rest of this file establishes: propose the
action, get explicit confirmation, then act — a prior confirmation for
one action (e.g. "commit and push") never implies permission for a
later, different one (e.g. closing an issue or cutting a release).

---

## GitHub Workflow

- **Remote repo:** `origin` → `https://github.com/yohannaftali/orcisf.git`
  (`yohannaftali/orcisf`).
- **Branch strategy:** single `main` branch currently (no deploy pipeline —
  this is primarily a documentation/archive repository, not a running
  service; `src/` is the one actively-developed exception, see below).
  Confirm with the user before assuming a feature-branch workflow is wanted.
- **CI:** `.github/workflows/build-src.yml` builds `src/` on
  windows-latest/macos-latest/ubuntu-latest for every push/PR touching
  `src/**`. Nothing else in the repo has CI (no build step applies to the
  legacy archive or the docs).
- **Releases:** GitHub Releases publish prebuilt GUI binaries. The project
  is currently in **alpha** — tags/releases use a `v0.0.x-alpha` scheme
  (first one: `v0.0.1-alpha`). Bump the patch number for each subsequent
  alpha; move to a `v0.x.0` (beta) or `v1.0.0` scheme only once the user
  explicitly decides the project has left alpha. **Only create a release
  after explicit user confirmation** — either the user asks directly, or
  the `reviewer` skill proposes one (per its Step 8: only when the change
  includes an actual `src/` code change, never for docs/tooling-only
  changes) and the user confirms. A release is a user-visible,
  hard-to-quietly-undo action either way.

## Tracked Issues
| ID | Title | Status | Last Checked |
|----|-------|--------|--------------|
| #1 | epic(src): port ORCISF to a modern cross-platform GUI application | closed | 2026-08-11 |
| #2 | feat(src): choose GUI stack & scaffold cross-platform CMake build (Win/macOS/Linux) | closed | 2026-08-10 |
| #3 | feat(src): port structural-analysis + RC-design + Flexible-Polyhedron engine as a headless library | closed | 2026-08-10 |
| #4 | feat(src): multi-threaded optimization core with configurable core count + cancellable progress | closed | 2026-08-10 |
| #5 | feat(src): 3D viewport + example-folder loader (render structure & per-member results) | closed | 2026-08-10 |
| #6 | feat(src): interactive 3D structure editor (import, drag-and-drop edit, numeric entry) | closed | 2026-08-10 |
| #7 | feat(src): load (pembebanan) input GUI (toolbar-driven, CAD-style 3D placement) | closed | 2026-08-10 |
| #8 | feat(src): reinforcement detailing drawings per beam/column | closed | 2026-08-10 |
| #9 | feat(src): PDF + legacy-text export of results | closed | 2026-08-10 |
| #10 | fix(src): CI build-src.yml failing on Linux and Windows targets | closed | 2026-08-10 |
| #11 | feat(src): add File > New Data action, rename Open Folder... to Open Data... | closed | 2026-08-11 |
| #12 | fix(src): orcisf_gui.exe opens an extra console window on Windows | closed | 2026-08-11 |
| #13 | epic(src): more user-friendly GUI (icon toolbar, view presets, re-optimize, editing guidance) | closed | 2026-08-11 |
| #14 | feat(src): add a configurable icon toolbar below the menu bar | closed | 2026-08-11 |
| #15 | feat(src): add Default/Design/Optimization view-layout presets | closed | 2026-08-11 |
| #16 | feat(src): re-optimize using the last best result | closed | 2026-08-11 |
| #17 | feat(src): add a Regenerate Seed button to the Run panel | closed | 2026-08-11 |
| #18 | feat(src): AutoCAD-style in-progress guidance while adding joints/members | closed | 2026-08-11 |
| #19 | feat(src): custom borderless window chrome + modern ImGui theme (cross-platform) | closed | 2026-08-11 |
| #20 | epic(src): joints/members list panel, 2D plane-locked drawing, UCS icon | closed | 2026-08-11 |
| #21 | feat(src): add a Joints/Members list panel (editable, delete-with-cascade-warning) | closed | 2026-08-11 |
| #22 | feat(src): 2D plane-locked drawing (X-Y/X-Z/Y-Z orthographic views + adjustable offset) | closed | 2026-08-11 |
| #23 | feat(src): UCS icon overlay in the viewport | closed | 2026-08-11 |
| #24 | fix(src): 2D plane offset control unreachable from the viewport (move it under the UCS icon) | ready-for-review | 2026-08-11 |
| #25 | fix(src): Run panel dataset-path field is redundant; write each run into a timestamped output subfolder | ready-for-review | 2026-08-11 |
| #26 | feat(src): wire up the app icon set (icons/) for Windows/macOS/Linux builds | ready-for-review | 2026-08-11 |
| #27 | fix(src): title bar Minimize/Maximize/Close buttons not flush to window right edge | closed (not reproducible) | 2026-08-11 |
| #28 | feat(src): Alt-mnemonic menu navigation + icon before each panel title | ready-for-review | 2026-08-11 |
| #29 | chore(src): interactively re-verify RunPanel dataset gating (#25) and 2D plane offset control (#22/#24) with a real loaded dataset | closed | 2026-08-11 |
| #30 | feat(src): implement real Alt-mnemonic menu navigation (Dear ImGui has no built-in "&" parsing) | ready-for-review | 2026-08-11 |
| #31 | fix(src): application is not DPI-aware -- UI too small on high-DPI monitors in multi-monitor setups | ready-for-review | 2026-08-11 |
| #32 | chore(src): add one-shot build scripts (build.ps1 for Windows, build.sh for macOS/Linux) | ready-for-review | 2026-08-11 |

Epic #1 tracks #2–#9. Chosen stack (see #1 for rationale): Dear ImGui
(docking) + GLFW + OpenGL3, ImGuizmo (3D manipulation), ImPlot (charts),
nativefiledialog-extended (native file/folder dialogs), libharu/HPDF (PDF
export), CMake + vcpkg (Win/macOS/Linux build).

**Epic #1 complete: #2 through #9 have all landed.** The GUI port now
covers the full pipeline the legacy console program did (load/create a
dataset, view it in 3D, edit geometry and loads interactively, run the
threaded optimizer with live progress, inspect results including
reinforcement detailing drawings, export a PDF report or the full legacy
text file set) — see each numbered section above (`gui/viewport/`,
`gui/editor/`, `engine/`, `gui/detailing/` + `src/report/`) for what
landed under which issue and what's deliberately still simplified or
out of scope. "Complete" means the acceptance criteria as written are
met and interactively verified where this environment allows it, **not**
that the port is a pixel-perfect or feature-complete replacement for the
legacy program — known gaps (e.g. per-DOF restraint editing, arrowed
CAD-style dimension lines, wiring edited in-memory geometry directly into
a `Run` without a save/reload step) are called out in their respective
sections above. Treat any *new* work here as a fresh issue against this
now-shipped baseline, not as "finishing" epic #1.

**Epic #13 complete (#14, #15, #16, #17, #18 all landed):** icon
toolbar, Default/Design/Optimization view-layout presets, re-optimize
from last best result (engine-level change, empirically verified — see
its own "read before touching" section above), the Regenerate Seed
button, and AutoCAD-style click-to-place guidance for Add Joint/Connect
Joints.

**Epic #20 complete (#21, #22, #23 all landed):** the Joints/Members
list panel (editable, cascade-delete-with-warning), 2D plane-locked
orthographic drawing (X-Y/X-Z/Y-Z with adjustable offset), and the UCS
icon overlay — see each issue's own "read before touching" section above.

---

## Validation (before pushing)

- **Docs-only changes** (this file, `README.md`, `CHANGE_HISTORY.md`): just
  proofread — there's no build step for them.
- **Legacy source changes** (`Optimasi Beton/Source/`): there is no modern way
  to compile-check them in this environment (Borland C++ 5.02 only). At
  minimum, review the change against the surrounding style and note in the
  commit/PR that it is **untested** (no compiler available) unless you have
  independently verified it against a Borland toolchain.
- **`src/` (GUI port) work:** build via `cmake --preset <os>-release &&
  cmake --build --preset <os>-release` from `src/` (needs `VCPKG_ROOT` set —
  see `src/README.md`). A clean vcpkg build compiles every dependency from
  source (no binary cache configured) and can take a long time on first
  configure. **CI is the authoritative cross-platform check**
  (`.github/workflows/build-src.yml`, Windows/macOS/Linux matrix) — if you
  can't fully build locally in your environment (e.g. no vcpkg/compiler
  available), say so explicitly rather than claiming it was verified, and
  let CI confirm the other platforms.
  - **On this project's Windows dev environment, a local toolchain does
    exist even though it's not on the default `PATH`**: CMake at
    `C:\Qt\Tools\CMake_64\bin\cmake.exe`, MSVC via
    `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`
    (run `VC\Auxiliary\Build\vcvars64.bat` to set up the environment),
    Ninja bundled with that MSVC install. An earlier agent session in
    this repo incorrectly reported "no cmake toolchain available" without
    checking beyond the default `PATH` — check for these before assuming
    a build genuinely can't happen locally.
  - **`src/build.ps1`/`src/build.sh`** (issue #32) wrap all of the
    above into one command for a human contributor (or CI) — deliberately
    placed in `src/`, alongside `vcpkg.json`/`CMakeLists.txt`/
    `CMakePresets.json`, not the repo root, since they only ever build
    this one component; a `vcpkg/` checkout is still expected one level
    up at the repo root (matching `src/README.md`'s bootstrap step), so
    each script derives the repo root as its own parent directory. The
    `builder` agent skill (renamed from `rebuild`) just *calls* these
    scripts rather than re-deriving toolchain paths inline itself --
    they're the single source of truth for this logic now, so update
    only `build.ps1`/`build.sh` if a new fallback toolchain location is
    ever discovered (the skill needs no matching edit; it delegates).
    `build.ps1` was interactively verified in this
    environment (locates `C:\Qt\Tools\CMake_64\bin\cmake.exe` and the
    BuildTools `vcvars64.bat` exactly as documented above, builds both
    `orcisf_gui.exe` and `engine/orcisf_cli.exe` -- note `orcisf_cli`
    lands in a nested `engine/` subdirectory of the build dir, since
    it's defined in `src/engine/CMakeLists.txt`, not the top-level one).
    `build.sh` mirrors the same logic but could not be executed locally
    (no macOS/Linux toolchain in this environment) -- CI is relied on
    to confirm it.
  - **Both scripts self-bootstrap vcpkg if it's missing** (clone to
    `<repo root>/vcpkg` + run `bootstrap-vcpkg.bat`/`.sh -disableMetrics`,
    then set `VCPKG_ROOT` for the rest of that run), rather than just
    erroring and telling the user to do it by hand -- added after a real
    user hit exactly that error on first run from a fresh clone. Only
    triggers when neither `VCPKG_ROOT` nor an existing `<repo root>/
    vcpkg/vcpkg(.exe)` is found; requires `git` on `PATH` (fails with a
    clear message if it's missing, same as every other missing-tool
    case). Interactively verified end-to-end on Windows: with no
    `VCPKG_ROOT` set and no prior checkout, `build.ps1` cloned + bootstrapped
    vcpkg at `<repo root>/vcpkg` and then proceeded to build successfully.

---

## Change Log Policy

- Use **this file (`AGENTS.md`)** for the *current* architecture, structure, and
  working rules only.
- Put **chronological / dated implementation history** in
  [`CHANGE_HISTORY.md`](CHANGE_HISTORY.md) — one dated entry per notable change.
- **When `AGENTS.md` grows large**, compact it: summarize/replace stale detail
  here and move the removed history into `CHANGE_HISTORY.md`. Record the
  compaction date below.
- Every agent that makes a structural/architectural change **must** update both
  files in the same change.

_Last architecture/state compaction: 2026-08-09 (initial AGENTS.md authored)._
