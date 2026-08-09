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
