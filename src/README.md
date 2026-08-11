# ORCISF GUI (`/src`)

Modern, cross-platform (Windows 11 / macOS / Linux) GUI port of ORCISF.
Tracked as GitHub epic [#1](https://github.com/yohannaftali/orcisf/issues/1),
this scaffold is [#2](https://github.com/yohannaftali/orcisf/issues/2). See
`AGENTS.md` at the repo root for the full architecture writeup.

The legacy Borland C++ program under `../Optimasi Beton/Source/` is left
untouched — this is an independent rewrite, not an in-place port.

## Stack

- [Dear ImGui](https://github.com/ocornut/imgui) (docking) + [GLFW](https://www.glfw.org/) + OpenGL3 — application shell/panels
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) — 3D translate/rotate/scale manipulation
- [ImPlot](https://github.com/epezent/implot) — run-time charts
- [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended) — native Open/Save/Select-Folder dialogs
- [libharu (HPDF)](https://github.com/libharu/libharu) — PDF export
- CMake + [vcpkg](https://vcpkg.io/) manifest mode — dependency management & cross-platform build

## Building

Requires: CMake ≥ 3.21, Ninja, a C++20 compiler, and vcpkg.

```sh
# one-time: get vcpkg and point VCPKG_ROOT at it
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh   # or bootstrap-vcpkg.bat on Windows
export VCPKG_ROOT=$(pwd)/vcpkg   # PowerShell: $env:VCPKG_ROOT = "$PWD/vcpkg"
```

**Quickest path**: run the one-shot build script at the repo root — it
detects the OS, locates CMake/MSVC/vcpkg (even if not on `PATH`),
configures the matching preset if needed, and builds `orcisf_gui`/
`orcisf_cli`:

```powershell
# Windows (PowerShell)
.\build.ps1
```

```sh
# macOS / Linux
./build.sh
```

**Manual control**, if you'd rather drive CMake yourself:

```sh
cd src
cmake --preset windows-release   # or macos-release / linux-release
cmake --build --preset windows-release
```

The first configure will build every dependency from source via vcpkg
(no prebuilt binary cache is set up yet) — expect this to take a while on a
clean machine. CI (`.github/workflows/build-src.yml`) builds all three OS
targets on every push/PR touching `src/`.

## Layout

Epic #1 (issues #2–#12) has landed the full pipeline: load/create a
dataset, view and edit it in 3D, run the threaded optimizer, inspect
results including reinforcement detailing, and export a PDF report or the
full legacy text file set. `src/`'s top-level shape:

```
src/
├── vcpkg.json / CMakeLists.txt / CMakePresets.json
├── app/          # main.cpp (GLFW/OpenGL3/ImGui bootstrap), Application.{h,cpp}
│                 # (docking layout, owns the loaded dataset + editor state)
├── gui/          # Toolbar, ViewportPanel, PropertiesPanel, LoadsPanel,
│                 # DetailingPanel, RunPanel, LogPanel + viewport/editor/
│                 # detailing subsystems
├── report/       # PDF + legacy-text export
└── engine/       # headless analysis/design/optimizer library + CLI
```

`src/`'s `File` menu: `New Data` (prompts for a save location, starts a
blank editable dataset), `Open Data...` (load an existing one),
`Save`/`Save As...`, `Save Loads (.bbn)`, `Export PDF.../Export
Text.../Export INF Preview...`.

See [`AGENTS.md`](../AGENTS.md) at the repo root for the full
per-subsystem architecture writeup — this file stays intentionally
high-level; don't duplicate AGENTS.md's detail here.
