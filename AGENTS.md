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
│   ├── DockTabIcons.{h,cpp} # #35: draws each panel's PanelIcons.cpp icon
│   │                        # directly on its own dock tab button (before
│   │                        # the title), via imgui_internal.h's
│   │                        # ImGuiDockNode/TabBar -- see its own AGENTS.md
│   │                        # section before touching
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
│   ├── PanelIcons.{h,cpp}       # #35 (was #28 Part 2): the 7 panels' hand-
│   │                            # drawn icon shapes, DrawPanelIcon(icon, dl,
│   │                            # origin, color, size) -- actually drawn onto
│   │                            # each dock tab by app/DockTabIcons.cpp, not
│   │                            # by these panels themselves
│   ├── PanelTitles.{h,cpp}      # #35: PanelWindowTitle()/PanelWindowId() +
│   │                            # the kXxxId stable-identity constants every
│   │                            # panel's Begin() and Application.cpp's
│   │                            # DockBuilderDockWindow() both reference
│   ├── Toolbar.{h,cpp}          # menu bar; File > Open Folder... (#5), the
│   │                            # Edit menu (#6: Undo/Redo, Add Joint, Connect
│   │                            # Joints, Snap to Grid), the Loads menu +
│   │                            # File > Save Loads (.bbn) (#7), and File >
│   │                            # Export PDF.../Export Text... (#9) are wired;
│   │                            # #37: View > Menubar/Subwindows/Layout
│   ├── IconVisibility.h         # #37: 8-bool per-icon-toolbar-button
│   │                            # visibility, written by Toolbar's View >
│   │                            # Menubar, read by IconToolbar::Draw()
│   ├── PanelVisibility.h        # #37: 8 bool* (one per panel), written by
│   │                            # Toolbar's View > Subwindows -- also how a
│   │                            # panel closed via its tab close button
│   │                            # gets reopened
│   ├── IconToolbar.{h,cpp}      # #14: icon-button row docked below the menu
│   │                            # bar (New/Open/Save/Undo/Redo/Add Joint/
│   │                            # Connect/Run) -- fixed curated set, hand-drawn
│   │                            # ImDrawList icons, no icon-font dependency;
│   │                            # #37: each button's visibility gated on an
│   │                            # IconVisibility, hidden slots skip cleanly
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
│   ├── JointsPanel.{h,cpp}      # #36 (split from JointsMembersPanel, #21):
│   │                            # joints table, cascade-delete-with-warning
│   ├── MembersPanel.{h,cpp}     # #36 (split from JointsMembersPanel, #21):
│   │                            # members table
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
- **Verification status:** geometry unit-tested standalone; full rendering
  pipeline interactively confirmed against a real `Example/Apl1-1` run,
  matching `orcisf_cli`/`MEMBER_RESULTS` exactly (details:
  `CHANGE_HISTORY.md`, 2026-08-10, issue #8).

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
- **Verification status:** round-trip and PDF/text export exercised
  standalone against a real `Example/Apl1-1` run (byte/field-identical),
  then the full GUI path (real NFD dialogs) reproduced the same output;
  re-read via `orcisf_cli info`/`equilibrium` gave a perfect (0.0)
  equilibrium residual (details: `CHANGE_HISTORY.md`, 2026-08-10, issue #9).

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
- **Verification status (#6):** joint/member picking, numeric position
  entry, Restrained checkbox, Delete/Add Joint, Undo, and Validation panel
  all exercised end-to-end interactively. Actual click-drag on the
  ImGuizmo gizmo itself was not (sub-pixel targeting too unreliable to
  script) -- `Manipulate()`'s output feeds the same verified
  `MoveJoint()` call, so remaining risk is isolated to ImGuizmo's own
  hit-testing (details: `CHANGE_HISTORY.md`, 2026-08-10, issue #6).
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
- **Verification status (#7):** full load-editor workflow exercised
  end-to-end against a real `Example/Apl1-1` scratch copy, including an
  actual File > Save Loads (.bbn) click whose output was diffed
  byte-for-byte against the GUI state; `ReadLoadsRaw()`/`WriteLoads()`
  round-trip and `SetMemberLoad()`'s UDL formula also unit-tested
  standalone (details: `CHANGE_HISTORY.md`, 2026-08-10, issue #7).

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
- **Real bug caught and fixed:** `ImGui::MenuItem`'s shortcut-text
  parameter does not bind the key chord by itself -- `Ctrl+S` was
  display-only. Fixed with an explicit
  `ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)` check in
  `Application::OnFrame()`, gated on the same `can_export_text` condition
  the menu item uses.
- **Verification status:** build/launch confirmed; `WriteInfPreview()`
  diffed numerically identical to the checked-in `aplikasi.inf` (only
  whitespace/column formatting differs); full interactive pass (New Data,
  Save, General section, Ctrl+S) confirmed via synthesized input +
  screenshots (details: `CHANGE_HISTORY.md`, 2026-08-11, issue #11).

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
- **Verification status:** all 8 icons confirmed interactively (dimming,
  tooltips, Add Joint correctly bootstrapping a structure and cascading
  enabled-state to Save/Undo) (details: `CHANGE_HISTORY.md`, 2026-08-11,
  issue #14).

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
  selection/undo state.
- **Verification status:** all three presets confirmed interactively --
  correct panel arrangement, View menu checkmark tracking, and a
  Design -> Optimization -> Default round-trip landing pixel-identical to
  the original layout, geometry/selection/undo state untouched throughout
  (details: `CHANGE_HISTORY.md`, 2026-08-11, issue #15).

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
- **Verification status:** compiled cleanly; `orcisf_cli optimize`
  against a scratch `Example/Data01` confirmed a real
  `<folder>/<timestamp>/` output subfolder and (via checksum) the
  dataset's own top-level `.opt` genuinely untouched. GUI launch
  confirmed; dataset-path-sync/Run-panel display itself not
  click-tested interactively (details: `CHANGE_HISTORY.md`, 2026-08-11,
  issue #25).

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
- **Verification status:** compiled successfully on Windows (`.rc`
  resource compiles and links, runtime icon PNGs land next to the built
  `.exe`, app launches without crashing). Icon's actual on-screen
  appearance not confirmed via screenshot this pass. macOS bundle
  packaging and Linux `.desktop`/install rules have no local toolchain to
  verify against -- CI is the first real cross-platform check for those
  two (details: `CHANGE_HISTORY.md`, 2026-08-11, issue #26).

**Issue #27 (title bar buttons "not flush to window edge") — resolved
under #31, correcting an earlier wrong "cannot reproduce" verdict:**
`ImGui::SameLine(offset_x)` measures from the *content* origin and
silently adds the enclosing group's `GroupOffset.x` (== `WindowPadding.x`
for a menu bar), so the button cluster really was shifted past the
window's right edge (14px at 100%, 28px at 200%). Fixed under #31 by
positioning `CustomTitleBar`'s button cluster with absolute screen
coordinates (`SetCursorScreenPos()`) instead of `SameLine(offset_x)` --
see the #31 section below. A separate DPI-measurement-tooling bug in the
original bug-report screenshot (mixing DPI-aware `GetWindowRect()` with
non-DPI-aware `.NET Screen.Bounds`) made the first investigation
conclude "not reproducible"; that conclusion was wrong (details:
`CHANGE_HISTORY.md`, 2026-08-11, issue #27).

**Panel icon headers (issue #28 Part 2) — superseded by issue #35, see
below; kept here only for the historical reasoning:** the original
approach (`gui::DrawPanelIconHeader()`, a hand-drawn icon + label row
drawn as the first thing inside each panel's content area, since Dear
ImGui has no public API to embed a glyph inside a dock tab's own label)
was confirmed working via screenshots, but the user clarified the actual
ask was an icon on the dock **tab button** itself, not a duplicate-title
content row. `DrawPanelIconHeader()` and its 7 call sites were removed
entirely and replaced by the `gui/PanelTitles.{h,cpp}` +
`app/DockTabIcons.{h,cpp}` mechanism described below (details:
`CHANGE_HISTORY.md`, 2026-08-11, issues #28/#35).

**Part 1 (Alt-mnemonics) was reverted -- Dear ImGui does NOT parse
`&`-prefixed labels.** The original #28 implementation added
`&File`/`&Edit`/etc. assuming Dear ImGui would strip the `&` and
underline the following letter once `ImGuiConfigFlags_NavEnableKeyboard`
was set; it does not -- the literal `&` rendered unparsed. Reverted to
plain labels (`File`, `Edit`, `View Plane`, `Loads`, `Run`, `View`). Real
Alt+letter mnemonic support needed to be hand-rolled -- scoped as issue
**#30** (details: `CHANGE_HISTORY.md`, 2026-08-11, issue #28 Part 1).

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
- **Verification status:** compiled cleanly. Interactive confirmation
  (Alt held -> underlines appear; Alt+letter -> menu opens) was not
  completed -- automation repeatedly landed on the user's own active
  browser window and was stopped rather than fight for control of their
  session. The underline-position math and `OpenPopup`-before-`BeginMenu`
  idiom are standard Dear ImGui techniques, but per #28's lesson, treat
  as unconfirmed until a real interactive pass succeeds (details:
  `CHANGE_HISTORY.md`, 2026-08-11, issue #30).

**`gui/PanelTitles.{h,cpp}` + `app/DockTabIcons.{h,cpp}` (issue #35,
correcting #28 Part 2) — read before touching any panel's `ImGui::Begin()`
call, any `DockBuilderDockWindow()` call, or the dock-tab icon mechanism:**
- **The technique**: every panel's window title is now
  `<N leading spaces><display label>###<stable_id>`, built by
  `gui::PanelWindowTitle(stable_id, display_label)`, not a plain string
  literal. The leading spaces reserve exactly enough horizontal room in
  the tab for `DockTabIcons.cpp` to draw an icon in front of the label
  without overlapping it; `N` is recomputed every frame from
  `ImGui::CalcTextSize(" ")` against the *current* font, so it always
  matches whatever DPI scale (issue #31) is active right now.
- **Why the leading-space count can safely change every frame without
  breaking docking**: Dear ImGui's `"###stable_id"` marker (see
  `ImHashSkipUncontributingPrefix()` in `imgui_internal.h`) makes
  everything *before* the `###` -- the variable-length spaces, the
  display label -- not contribute to the window's ID hash at all; only
  `stable_id` does. `gui::kViewportId`/`kDetailingId`/`kPropertiesId`/
  `kRunOptimizationId`/`kJointsMembersId`/`kLoadsId`/`kLogId`
  (`PanelTitles.h`) are the single source of truth for these stable
  strings -- `Application.cpp`'s `DockBuilderDockWindow()` calls and
  `DockTabIcons.cpp`'s `FindWindowByName()` calls both reference the
  *same* constant via `gui::PanelWindowId(stable_id)` (just the
  `"###stable_id"` part, no display text), so a typo can't silently
  desync a panel from its own dock-layout/icon wiring the way two
  independently-typed string literals could.
- **This sidesteps the alternative (baking a custom glyph into the font
  atlas) on purpose**, not just for convenience: this project's issue
  #31 already rebuilds the font atlas live on every DPI change
  (`style.FontScaleDpi`), and Dear ImGui does not own/regenerate a
  manually-drawn custom rect's pixel content across that kind of
  rebuild -- a one-shot `AddCustomRectFontGlyph()` bake (obsoleted in
  this project's pinned 1.92.8 anyway, superseded by a custom
  `ImFontLoader`) would need to be redrawn on every rescale, which is
  real additional complexity a draw-list overlay (redrawn every frame
  regardless, like every other hand-drawn icon in this codebase) doesn't
  have. This reasoning -- not just "avoid a new dependency" -- is why
  `imgui_internal.h`'s dock-tab-bar structures were chosen over the
  font-glyph route once both were actually weighed against this specific
  codebase's DPI architecture.
- **`app/DockTabIcons.cpp`'s `DrawDockTabIcons()`** walks a fixed table of
  `(stable_id, PanelIcon)` pairs, looks up each window via
  `ImGui::FindWindowByName(gui::PanelWindowId(stable_id).c_str())`, reads
  its `ImGuiWindow::DockNode->TabBar` (skipping windows that aren't
  currently docked/active or whose node has no tab bar -- e.g. a
  single-window node with `ImGuiDockNodeFlags_AutoHideTabBar`), finds its
  own `ImGuiTabItem` within that bar (`tab.Window == window`), and draws
  the icon at `tab_bar->BarRect.Min + (tab.Offset + FramePadding.x, half
  the bar height)` on the foreground draw list -- matching Dear ImGui's
  own tab-item label layout (`TabItemLabelAndCloseButton()`) closely
  enough that the icon sits flush against the label with no gap or
  overlap. Called once per frame from the very end of
  `Application::OnFrame()`, after every panel's `Draw()` (so each
  window's tab-bar geometry is final for the frame) and before
  `ImGui::Render()`.
- **Lives under `app/`, not `gui/`**, since it operates on dock-node/
  tab-bar internals that are an Application-level concern -- the same
  boundary `Application.cpp`'s own `BuildDockspace()` (issue #15)
  already draws by being the only other file in this port that includes
  `imgui_internal.h`. `PanelIcons.{h,cpp}` (now just the per-icon
  `DrawPanelIcon()` primitive, parameterized on `size` instead of a
  fixed 16px constant) and `PanelTitles.{h,cpp}` stay under `gui/` since
  they don't touch dock internals themselves.
- **Verification status:** compiled cleanly; confirmed via real
  screenshots at 100% and 200% `ORCISF_UI_SCALE` -- all 7 panels' dock
  tabs show correctly positioned icons, dock placement pixel-identical to
  before, no duplicate title row (details: `CHANGE_HISTORY.md`,
  2026-08-11, issue #35). Tab close-button *functionality* was found
  broken separately -- see issue #37 below. A 2026-08-12 `tester` pass
  couldn't independently re-check DPI-scaled icon sizing (no forced
  rescale that session) and flagged it UNVERIFIED; the user then
  manually tested it on a real high-DPI monitor at their office the same
  day and confirmed icons resize correctly -- treat this criterion as
  fully closed out now (details: `CHANGE_HISTORY.md`, 2026-08-12).

**Issue #29 (interactive retest of RunPanel dataset gating + plane-offset
overlay) — resolved and closed after three attempts:**
- Both acceptance criteria confirmed with a real dataset loaded via File
  > Open Data...: **RunPanel dataset gating (#25)** — Dataset field
  populated and Run button renders enabled once a real dataset is loaded.
  **2D plane-lock offset overlay (#22/#24)** — View Plane > X-Y correctly
  shows an orthographic top-down projection with an editable
  "Z offset (m)" field and updated UCS icon; this was never actually
  broken, just unreached in earlier attempts.
- **Durable automation lesson**: on this high-DPI dev machine, always
  call `SetProcessDPIAware()` in any PowerShell/automation script *before*
  any `SetCursorPos`/`GetWindowRect` call, and size screenshot capture
  bitmaps explicitly (not via `.NET`'s `Screen.Bounds`) — a DPI-unaware
  automation script silently operates in virtualized logical-pixel
  coordinates while screenshots are physical-pixel, a 2x mismatch on a
  200%-scaled monitor that produces misleading, hard-to-diagnose click
  failures. Two earlier attempts misdiagnosed this exact mismatch as a
  WinUI3/XAML dialog resisting synthetic input; it wasn't.
- Actually typing a new numeric Z-offset value and confirming it moves a
  placed joint was not independently re-verified (interrupted by a
  focus-stealing notification toast) — the overlay's presence as a live
  ImGui input is strong but not conclusive evidence.
- Full three-attempt debugging narrative: `CHANGE_HISTORY.md`, 2026-08-11,
  issue #29.

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
- **Verification status:** toggling Add Joint mode and click-to-place
  (including staying active across repeated placements) confirmed
  interactively. Connect Joints' first-click-vs-second-click hint text
  was code-reviewed but not re-verified live (interrupted by unrelated
  focus-stealing; details: `CHANGE_HISTORY.md`, 2026-08-11, issue #18).

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
- **Verification status:** code follows `LoadsPanel`/`PropertiesPanel`'s
  proven patterns and cross-checked API; compiled successfully. Interactive
  exercise of the cascade-delete modal specifically was not re-checked
  (details: `CHANGE_HISTORY.md`, 2026-08-11, issue #21).

**Superseded by issue #36 (2026-08-11): `JointsMembersPanel` was split
into `gui/JointsPanel.{h,cpp}` and `gui/MembersPanel.{h,cpp}`, two
independent panels.** `DrawJointsTable()`/the cascade-delete modal moved
into `JointsPanel` unchanged; `DrawMembersTable()` moved into `MembersPanel`
unchanged (both dropped their `ImGui::SeparatorText("Joints"/"Members")`
call, since the panel's own dock tab -- see issue #35 -- already labels
it, same reasoning as #35's content-row-header removal). Everything
above this note about the *logic* (the `SpanAllColumns` pattern, the
`TouchingMembers()` + modal ID-stack scoping, `EditableStructure`'s
cascade behavior) still applies verbatim to `JointsPanel` -- only the
window/file split changed, not the code within either table. `#36` also
reordered the shared dock group's tab order to Joints, Members, Loads
(previously Loads, Joints/Members) -- see `Application.cpp`'s `OnFrame()`
comment on *why* that reorder had to happen via `Draw()` call order, not
via `DockBuilderDockWindow()` call order (a wrong assumption caught by
screenshotting a mismatch, not by reading Dear ImGui's source).
**Verification status for #36:** compiled cleanly; real interactive data
flow confirmed (`JointsPanel`/`MembersPanel` tables correctly reflecting
a scratch structure, tab order correct). The cascade-delete modal's
earlier unverified/apparently-broken status (Delete-button clicks
producing no effect, root cause unknown at the time) is now resolved:
a `tester` pass on 2026-08-12, run *after* issue #41 landed, confirmed
the modal fires correctly and cascades/compacts indices as designed --
almost certainly a side effect of #41's `ImGuiSelectableFlags_
AllowOverlap` fix (the Delete `SmallButton` sits in a column
`SpanAllColumns` would have blocked the same way it blocked `InputFloat`
cells), not a separately-applied fix. Details: `CHANGE_HISTORY.md`,
2026-08-11 (issue #36) and 2026-08-12 (tester pass).

**Docked-panel close-button bug + View menu Menubar/Subwindows/Layout
sections + "Run Optimization" -> "Optimization" rename (issue #37) — read
before touching `Application::OnFrame()`'s per-panel `Draw()` calls,
`Toolbar.cpp`'s View menu, or `IconToolbar.cpp`'s per-button visibility:**
- **Root cause of the close-button bug**: `Application::OnFrame()` called
  every panel's `Draw(&panel_open_, ...)` unconditionally every frame,
  trusting `ImGui::Begin(name, open)` to no-op once `*open` went false --
  true for floating windows, but Dear ImGui's docked-tab-bar bookkeeping
  does not remove a window from its dock node purely because `*p_open`
  went false while `Begin()` keeps getting called each frame; the tab
  lingers until the caller stops calling `Begin()` for it. **Fix**: every
  panel's `Draw()` call in `OnFrame()` is now wrapped in
  `if (panel_open_) { ... }` -- skip the call entirely once closed. This
  bug predates #35/#36 (confirmed to reproduce even with a fully static
  window title, ruling out #35's dynamic-title mechanism as the cause).
  Root-cause hunt details: `CHANGE_HISTORY.md`, 2026-08-11, issue #37.
- **`gui/IconVisibility.h`** (new): a plain 8-bool struct, one per
  `IconToolbar` button (`new_data`/`open_data`/`save`/`undo`/`redo`/
  `add_joint`/`connect_joints`/`run`), all defaulting to visible. Owned
  by `Application` (`icon_visibility_`), written by `Toolbar`'s new View
  > Menubar checkable `MenuItem`s, read by `IconToolbar::Draw()`.
- **`gui/PanelVisibility.h`** (new): 8 `bool*` fields (one per panel,
  including `optimization` for `run_open_` -- the pointer name doesn't
  need to track the #37 display rename), rebuilt fresh each frame in
  `Application::OnFrame()` from the same `*_open_` fields each panel's
  `Draw()` call is already gated on above -- not new state, just a
  read/write *view* over the existing fields so `Toolbar` doesn't need
  8 separate parameters or any Application-specific knowledge. Passed to
  `Toolbar::Draw()` as `const PanelVisibility&`; the pointers themselves
  are non-const so View > Subwindows' `MenuItem(label, nullptr, bool*)`
  can toggle them directly (open **and** close -- this is also the
  practical way to reopen a panel closed via its now-fixed tab close
  button).
- **`IconToolbar::Draw()`'s reflow logic**: a hidden button's slot is
  skipped entirely (not left as a blank gap). Implemented via a small
  `Sep(bool double_gap)` lambda that only emits `ImGui::SameLine()` when
  something has already been drawn this row (tracked by a `first` bool)
  -- so whichever button ends up being the first *visible* one never
  gets a leading gap, regardless of which button that turns out to be.
  Verified interactively: unchecking "New Data" in View > Menubar
  removed it from the toolbar with the remaining icons shifting left
  flush, no gap.
- **View menu restructure**: `Toolbar.cpp`'s `View` top-level menu (still
  opened via the existing Alt+V mnemonic, issue #30) now contains three
  `ImGui::BeginMenu()` submenus separated by `ImGui::Separator()`:
  **Menubar** (the 8 `IconVisibility` toggles), **Subwindows** (the 8
  `PanelVisibility` toggles), **Layout** (issue #15's pre-existing
  Default/Design/Optimization preset items, functionally unchanged, just
  moved from flat top-level `View` items into their own submenu).
- **Rename scope, deliberately narrow**: only the *display* text changed
  (`RunPanel.cpp`'s `PanelWindowTitle(kRunOptimizationId, "Optimization")`
  call, plus `ViewportPanel.cpp`'s "no dataset loaded" hint text) --
  `gui::kRunOptimizationId` (the dock/window *identity* string) is
  unchanged on purpose, so no `DockBuilderDockWindow()` call or
  `DockTabIcons.cpp` table entry needed touching. `IconToolbar.cpp`'s Run
  button tooltip ("Run Optimization") was deliberately left as-is -- it
  describes the *action* the button performs (a verb phrase), not the
  panel's title (a noun label), and the issue's own request was
  specifically to rename the panel/tab, not every occurrence of the
  phrase. Note the resulting naming overlap this creates: there is now
  both an "Optimization" **panel** (Subwindows) and an "Optimization"
  **layout preset** (Layout) -- both call themselves that on purpose (a
  panel and a preset are different kinds of things a user picks from
  different submenus), not a naming bug.
- **Verification status:** close-button fix, View > Subwindows
  close/reopen cycle, View > Menubar toggle reflow, View > Layout
  presets, and the "Optimization" rename all confirmed via real
  screenshots for `Viewport`/`Log`. The remaining 6 panels' close
  buttons go through the identical gating fix but weren't individually
  re-clicked (low risk; details: `CHANGE_HISTORY.md`, 2026-08-11,
  issue #37).

**Preset tab grouping fix (issue #38, follow-up to #15/#36/#37) — read
before touching `BuildDefaultLayout()`/`BuildDesignLayout()`:** Properties,
Optimization, and Log are now tabbed together in one right-side dock node
in both the Default and Design presets (Log used to sit in the bottom
Joints/Members/Loads strip in Default; Design used to split Optimization+Log
into a separate `dock_bottom_right` node while Properties sat alone --
that extra split is gone). Tab order (Properties -> Optimization -> Log)
needed no `OnFrame()` `Draw()`-call reordering -- the existing call order
(Viewport, Properties, Joints, Members, Loads, Detailing, Run, Log)
already puts these three in the right relative sequence, per the
`DockBuilderDockWindow()`-order-doesn't-matter convention from #36.
`BuildOptimizationLayout()` is untouched. **Verified interactively** via
screenshots of all three presets: Default (Properties/Optimization/Log
grouped, bottom strip is Joints/Members/Loads only), Design (same
right-side grouping, no leftover empty node), Optimization (unchanged).

**Row `Selectable` click-blocking fix (issue #41) — read before touching
any `ImGui::Selectable(...)` row in `JointsPanel.cpp`/`MembersPanel.cpp`/
`LoadsPanel.cpp`, or adding a new editable table row anywhere in
`gui/`:**
- **Root cause was `ImGuiSelectableFlags_SpanAllColumns` alone, not a
  row-height mismatch** — the first hypothesis (tried and shipped before
  testing) assumed `Selectable()`'s default unframed height being
  shorter than an `InputFloat` cell's `GetFrameHeight()` left a gap at
  the bottom of each row where clicks "fell through" to the widget
  underneath. Testing (synthetic clicks at multiple Y positions per row,
  every row, then typing and checking whether the field's value changed)
  showed the height-only fix changed nothing — every click still landed
  on the Selectable. Root cause was only found by adding direct
  `ImGui::GetHoveredID()`/`ImGui::GetActiveID()` logging (temporarily,
  to the app's own `LogPanel`) and, as a controlled diagnostic,
  temporarily removing `ImGuiSelectableFlags_SpanAllColumns` entirely —
  that alone fixed click routing to the `InputFloat`, proving
  `SpanAllColumns` unconditionally claims every click across the whole
  row regardless of later-column widgets, at any Y. The real, correct
  fix is `ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap`
  together (`AllowOverlap` is documented in `imgui.h` as letting
  subsequent widgets win hit-testing over an earlier overlapping one) —
  the height fix (`ImVec2(0.f, ImGui::GetFrameHeight())`) is still kept
  alongside it, but only for the cosmetic row-highlight-covers-the-whole-row
  effect, not for click routing.
- **Applied to all four `Selectable()` call sites**: `JointsPanel.cpp`
  (one, holds the canonical comment other files point back to),
  `MembersPanel.cpp` (one — currently inert since that table's other
  cells are still read-only `Text`, not editable widgets, but issue #39
  adds editable fields to this exact row and must not reintroduce the
  bug), `LoadsPanel.cpp` (two, member-loads and joint-loads tables).
- **What was verified**: rebuilt and directly instrumented (the same
  `GetHoveredID()`/`GetActiveID()` logging, left in only long enough to
  confirm, then fully removed before commit) against `JointsPanel` —
  clicking directly on an `InputFloat` cell activates it persistently
  (`ActiveID` survives multiple post-release frame checks, field shows a
  live text-selection highlight and accepts typed input), while clicking
  the non-widget Joint-number column still selects the row transiently
  (`ActiveID` reverts to 0 immediately on mouse-up, 3D viewport gizmo
  moves to the clicked joint). `MembersPanel`/`LoadsPanel` received the
  byte-identical fix (verified by direct code comparison, not
  independently re-instrumented per-file) and were visually spot-checked
  in a running build (Loads menu, table rendering) without a regression.
- **The temporary `ActiveID`/`HoveredID` debug logging (and its
  `#include <cstdio>`) was fully removed from `Application::OnFrame()`
  before this fix was committed** — if you need the same diagnostic
  technique again for a future ImGui click-routing bug, re-add it
  the same way (log to `LogPanel`, gated behind a `static bool` armed by
  `ImGui::IsMouseClicked()`) and remove it again before committing,
  don't leave it in.

**Editable member type/endpoints, Add Member/Add Joint buttons, viewport
number labels (issue #39) — read before touching `EditableStructure::
SetMemberEndpoints()`/`SetMemberTypeOverride()`, `MembersPanel.cpp`/
`JointsPanel.cpp`'s new top-right buttons, or `ViewportPanel.cpp`'s
`DrawEntityLabels()`:**
- **A member's beam/column type is purely geometry-derived (`CXZ`,
  `periksa_batang()`) throughout `engine::StructuralAnalysis`/`Optimizer`
  (design-variable count `JVD`, self-weight direction, the RC design
  path) — this port deliberately does NOT let the Members panel's type
  dropdown change that.** The dropdown (`EditableStructure::
  SetMemberTypeOverride()`/`GetMemberTypeOverride()`, 0/1/2 =
  Auto/Beam/Column, a new GUI-only `std::array<int, kMak>` inside
  `EditableStructure`, shifted in lockstep with `JJ`/`JK`/`IA` on
  Add/DeleteMember) only affects this port's own display (folded into
  `MemberVisual::is_beam` via a new optional `BuildSceneModel()`
  parameter — viewport color, Properties/Detailing panel labels) — a
  `Run` always uses the real geometry-derived classification, never this
  override. This was a deliberate, documented engineering call (not an
  oversight): silently letting a GUI dropdown change what the ported 1999
  algorithm's core classification computes would risk corrupting
  analysis/optimization results for a cosmetic convenience, exactly the
  kind of "don't fix a deliberate deviation" risk this file's `engine/`
  section warns about elsewhere. Instead, `EditableStructure::Validate()`
  gained a check: whenever a member's override disagrees with its actual
  geometry, it surfaces as a Properties-panel validation issue (e.g.
  *"Member 1's selected type (Column) doesn't match its geometry (would
  classify as Beam) -- a Run always uses the geometry-derived type"*) so
  the user is warned rather than silently misled. **Confirmed
  interactively**: setting a horizontal test member's override to
  "Column" immediately updated the Properties panel title to "Batang 1
  (Kolom)" and produced exactly that validation warning.
- **`EditableStructure::SetMemberEndpoints(member_id, joint_a, joint_b)`**
  (new) backs the Members panel's editable Joint A/Joint B cells —
  validates `joint_a != joint_b` and both in `[1, NJ]` *before* touching
  `sd_.JJ`/`sd_.JK`, returning `false` (arrays untouched) on any invalid
  input, so an in-progress/bad edit (e.g. a joint number that doesn't
  exist) can never corrupt the compacted-index invariant. **Confirmed
  interactively**: retargeting a member's Joint A from 2 to 1 correctly
  reconnected it in the viewport and updated its computed length in
  Properties; typing an out-of-range value (99, with only 3 joints
  present) left Joint A/B and the computed length completely unchanged —
  the edit was silently rejected, not applied-then-caught.
- **"+ Add Member"/"+ Add Joint" (top-right of `MembersPanel`/
  `JointsPanel`, above the table header)** insert a *real* member/joint
  immediately via the existing `AddMember()`/`AddJoint()` (never a
  "pending" row — the fixed-size legacy `JJ`/`JK`/`X`/`Y`/`Z` arrays have
  no concept of one), which the user then edits in place. Add Member
  defaults to connecting the *last two* existing joints (disabled with a
  tooltip if fewer than 2 joints exist, since `AddMember()` itself
  already rejects `joint_a == joint_b`); Add Joint defaults to the
  current joints' centroid (origin for an empty structure) — same
  "produce something visible/editable, not a silent no-op" reasoning
  issue #7 used for load-placement defaults. Both are additional entry
  points alongside the pre-existing toolbar/viewport-click Add Joint flow
  (#14/#18) and viewport connect-mode for members, not a replacement.
  **Confirmed interactively**: from a blank structure, placed 3 joints
  via viewport clicks, then Add Member correctly created a member
  connecting joints 2 and 3.
- **Viewport joint/member number labels (`ViewportPanel.cpp`'s new
  `DrawEntityLabels()`)** are hand-drawn `ImDrawList` text (matching this
  project's established icon-overlay approach, e.g. the UCS icon from
  #23) but unlike that icon's direction-only dot-product projection, a
  label needs a real world-to-screen point: `proj * view * point` (the
  same matrices `SceneRenderer` already renders with) evaluated on the
  CPU, perspective-divided, then mapped into the viewport image's pixel
  rect. Points behind the eye (`w <= ~0`) or well outside the NDC range
  are skipped rather than drawn off-screen. **Confirmed interactively**:
  `J1`/`J2`/`J3` labels tracked each joint's cube correctly at its
  on-screen position, and `M1` rendered at the connecting member's
  midpoint, through multiple geometry edits (retargeting, add/delete).
- **What was NOT part of this issue's scope**: the type-override array
  is intentionally excluded from `UndoStack`'s `GeometrySnapshot` (it
  lives in `EditableStructure`, not `engine::StructureData`, and
  `GeometrySnapshot` only ever mutates `sd` directly) — an Undo/Redo
  after setting a type override can leave the override pointing at a
  member index that geometry undo just shifted. Documented here as a
  known, deliberate scope limit rather than silently unhandled; revisit
  only if a future issue specifically needs override/undo consistency.

**Per-DOF joint restraint editing (issue #40) — read before touching
`PropertiesPanel.cpp`'s `kPreset*` arrays, `EditableStructure::
SetJointDof()`, or `ViewportPanel.cpp`'s `ClassifyRestraintPreset()`:**
- **The Roller preset restrains UY (vertical translation), not UZ, even
  though issue #40's own acceptance-criteria text says "UZ (Z assumed
  vertical)".** This project's vertical/up axis is Y everywhere else
  (`Camera::WorldUp()` = `{0,1,0}`, `orcisf_cli`'s equilibrium check
  documents "arah 2 = Y", every other viewport/gizmo/camera computation)
  — the issue text's "Z assumed vertical" would only be correct under a
  Z-up convention this codebase doesn't use. Implementing it literally
  would have produced a support type that silently contradicts "up"
  everywhere else in the GUI. Corrected deliberately, not a misreading —
  `PropertiesPanel.cpp` and `ViewportPanel.cpp`'s `kPresetRoller`/Roller
  classification both use UY, with a comment explaining why. If a future
  dataset genuinely needs Z-up, this would need revisiting alongside the
  rest of the codebase's Y-up assumption, not in isolation here.
- **`EditableStructure::SetJointDof(joint_id, dof_index, restrained)` /
  `GetJointDof()`** (new) set/read a single `JRL` flag directly — `dof_index`
  0..5 in the legacy "arah 1..6" order (UX, UY, UZ, RX, RY, RZ), the same
  order `SetJointLoad()`'s `actions[6]` already uses. The old
  `SetJointRestrained()` (all-6-at-once) is kept, unchanged, since
  `JointsPanel`'s existing single "Restrained" summary checkbox and
  `AddJoint()`'s "free by default" bootstrap still use it — issue #40
  didn't ask to change that entry point, only to add a finer-grained one
  alongside it (`AGENTS.md`'s own `gui/editor/` note anticipated exactly
  this as a future refinement, not a replacement).
- **`PropertiesPanel.cpp`'s `DrawJointProperties()`** replaced the single
  "Restrained (fixed support)" checkbox with 6 independent `UX`/`UY`/
  `UZ`/`RX`/`RY`/`RZ` checkboxes (always read fresh from `JRL` via
  `GetJointDof()`, never cached) plus four preset buttons (Fixed/Pinned/
  Roller/Free) that call a shared `ApplyRestraintPreset()` helper — both
  are views onto the same `JRL` state, so they can't drift out of sync
  with each other by construction (no separate "which preset is active"
  state is stored anywhere). Preset buttons are plain text buttons with
  hover tooltips describing the intended icon glyph, not hand-drawn
  `ImDrawList` icons like `IconToolbar`'s buttons — a deliberate, minor
  scope trim (the issue's icon descriptions are cosmetic; the actual
  preset behavior is what the acceptance criteria are really checking).
- **`ViewportPanel.cpp`'s `ClassifyRestraintPreset()`** classifies a
  joint's 6 flags into `"Fixed"`/`"Pinned"`/`"Roller"`/`"Free"`/`"Custom"`
  (exact same preset definitions as `PropertiesPanel.cpp`'s `kPreset*`
  arrays — keep both in sync if a preset's definition ever changes) and
  `DrawEntityLabels()` (issue #39) appends it to the *selected* joint's
  `J#` label only (e.g. `"J1 [Pinned]"`), reusing the exact same
  world-to-screen projection #39 already built rather than adding a
  second one.
- **What was verified interactively** (screenshots at each step, same
  session as #39's verification): placed a joint, confirmed its default
  state showed `[Free]` in both the viewport label and all-unchecked
  Properties checkboxes; clicked **Pinned** and confirmed UX/UY/UZ
  checked (RX/RY/RZ not), the viewport label updated live to
  `[Pinned]`, and `JointsPanel`'s pre-existing single "Restrained"
  summary checkbox correctly became checked (confirming the old
  all-or-nothing entry point still works, unmodified, alongside the new
  one); clicked **Roller** and confirmed only UY was checked (the Y-up
  correction, not UZ) with the label updating to `[Roller]`; manually
  checked `RX` on top of Roller's `UY` and confirmed the label correctly
  fell back to `[Custom]` for a combination matching none of the four
  presets.

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
- **Real sign bug found and fixed**: the Y-Z (elevation) plane's
  `ortho_right_` was initially `{0,0,1}`, which makes `Cross(right,forward)`
  (the up vector every `Camera` method derives) work out to world **-Y**
  -- since Y is this project's vertical axis, that would render the
  elevation view upside-down. Fixed to `{0,0,-1}` (caught by hand-deriving
  the cross product, no build toolchain available that session; details:
  `CHANGE_HISTORY.md`, 2026-08-10, issue #22).
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
- **Verification status:** cross-product/up-vector math hand-derived and
  re-checked; compiled successfully in a later session. The three plane
  views' actual on-screen rendering and click-to-place-on-locked-plane
  behavior were not interactively exercised in a running app.

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
- **Verification status:** compiled cleanly; empirically exercised via a
  standalone scratch program against `Example/Data01` -- a seeded
  re-optimization produced lower cost than the baseline (confirming the
  seed takes effect) and `worker_threads=4` reproduced a bit-identical
  result to `worker_threads=1` (confirming #4's determinism guarantee
  holds with seeding active). Full numbers: `engine/README.md`'s
  "Re-optimize from last best" section. The GUI checkbox itself was
  reviewed but not interactively clicked through.

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
- **Verification status:** dot-product projection math hand-checked for
  reference orientations; compiled successfully and app launched without
  crashing. Live rotation-with-camera behavior not pixel-tested
  interactively.

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
- **Verification status:** compiled cleanly, app launched without
  crashing. Not interactively click-tested -- whether dragging the
  slider avoids triggering orbit, and whether a placed joint's locked
  coordinate matches a nonzero slider value, are the issue's own
  acceptance criteria and still need a real interactive/screenshot pass.

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
  per-frame `ImGui::GetIO().MouseDelta`.** A delta-based version was
  tried first and visibly jittered: `glfwSetWindowPos()` moves the window
  but not the physical cursor, so the cursor's window-relative position
  (what GLFW's mouse callback and ImGui's `MouseDelta` report) silently
  shifts by the exact opposite of the move on the next frame -- a
  self-inflicted feedback oscillation. The fix
  (`dragging_`/`drag_start_cursor_screen_*_`/`drag_start_window_*_`
  members) recomputes `glfwGetWindowPos() + glfwGetCursorPos()` fresh
  every frame and references everything back to the drag-start anchor,
  which has no feedback path. Don't revert to the delta-based version
  without re-testing this (details: `CHANGE_HISTORY.md`, 2026-08-11,
  issue #19).
- This whole mechanism (whichever version) is deliberate Phase 0 scope
  (see the issue's platform-agnostic-baseline rationale) -- Phase 1+
  replaces it with real native dragging per platform. It's a no-op on
  Wayland (`glfwGetPlatform() == GLFW_PLATFORM_WAYLAND`), surfaced via a
  disabled-looking drag zone + tooltip rather than failing silently.
- **Verification status:** borderless window, title text, Minimize/
  Maximize/Close icons (including the Maximize<->Restore swap), and the
  drag mechanism all confirmed interactively. macOS/Linux not verified in
  this environment (no access) -- verify via a real build there before
  treating Phase 0 as fully done on those platforms.

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
- **Verification status:** compiled cleanly; visually confirmed via
  real DPI-aware screenshots at 100%/150%/200% (chrome scaling,
  title-bar button visibility/centering, no panel clipping); live
  rescaling exercised through the real per-frame path across multiple
  100%<->200% round trips with no compounding. A later `tester` pass
  re-confirmed criteria 1-4 with fresh evidence and closed the issue on
  the user's call; **criterion 5 (an actual two-physical-monitor drag)
  was explicitly left UNVERIFIED** -- only one physical monitor is
  available in this environment. If a real dual-monitor drag is ever
  found to misbehave, treat it as a new report (details:
  `CHANGE_HISTORY.md`, 2026-08-11, issue #31).

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
| #31 | fix(src): application is not DPI-aware -- UI too small on high-DPI monitors in multi-monitor setups | closed | 2026-08-11 |
| #32 | chore(src): add one-shot build scripts (build.ps1 for Windows, build.sh for macOS/Linux) | ready-for-review | 2026-08-11 |
| #35 | fix(src): move panel icons onto the dock tab button, remove the in-content icon+title header row (#28 correction) | ready-for-review (tester: all criteria PASS, 1 via user's manual DPI check) | 2026-08-12 |
| #36 | feat(src): split Joints/Members panel into separate Joints and Members panels; order Loads tab immediately after them | ready-for-review (tester: all criteria PASS, 1 via user's manual DPI check) | 2026-08-12 |
| #37 | feat(src): View menu Menubar/Subwindows/Layout sections + fix tab close button + rename "Run Optimization" panel | ready-for-review (tester: all criteria PASS, 1 via user's manual DPI check) | 2026-08-12 |
| #38 | fix(src): move Log tab into right-side Properties/Optimization group; fix Design preset's Properties/Optimization/Log tab order | ready-for-review (tester: all criteria PASS, 1 via user's manual DPI check) | 2026-08-12 |
| #39 | feat(src): editable member type/joint endpoints + Add Member/Add Joint buttons, with joint/member labels in viewport | ready-for-review (tester: all criteria PASS, 1 via user's manual DPI check) | 2026-08-12 |
| #40 | feat(src): per-DOF joint restraint editing (6 checkboxes + Fixed/Pinned/Roller/Free quick-support buttons) | ready-for-review (tester: all criteria PASS, 1 via user's manual DPI check) | 2026-08-12 |
| #41 | fix(src): row-selection Selectable overlaps/covers editable input cells in Joints/Members/Loads tables | ready-for-review (tester: all criteria PASS, 1 via user's manual DPI check) | 2026-08-12 |

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

_Last architecture/state compaction: 2026-08-12 (moved narrative/debugging
history for issues #16-#37 into CHANGE_HISTORY.md, condensed per-issue
sections to current rules/gotchas only)._
