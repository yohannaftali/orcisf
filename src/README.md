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

cd src
cmake --preset windows-release   # or macos-release / linux-release
cmake --build --preset windows-release
```

The first configure will build every dependency from source via vcpkg
(no prebuilt binary cache is set up yet) — expect this to take a while on a
clean machine. CI (`.github/workflows/build-src.yml`) builds all three OS
targets on every push/PR touching `src/`.

## Layout

```
src/
├── vcpkg.json           # dependency manifest
├── CMakeLists.txt
├── CMakePresets.json     # windows-{debug,release}, macos-{debug,release}, linux-{debug,release}
├── app/
│   ├── main.cpp          # GLFW/OpenGL3/ImGui bootstrap + render loop
│   └── Application.{h,cpp}  # owns the docking layout + panels
└── gui/
    ├── Toolbar.{h,cpp}         # top menu bar (placeholders; real actions land in #4/#7)
    ├── ViewportPanel.{h,cpp}   # 3D view (placeholder; real rendering lands in #5)
    ├── PropertiesPanel.{h,cpp} # selection editor (placeholder; lands in #6/#7)
    └── LogPanel.{h,cpp}        # run/status log (functional; detailed calc log lands in #3)
```

Everything under `gui/` beyond the docking skeleton is intentionally a
placeholder in this scaffold — see the linked issues for the real
implementations.
