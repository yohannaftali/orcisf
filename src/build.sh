#!/usr/bin/env bash
# build.sh -- one-shot macOS/Linux build of src/ (orcisf_gui, orcisf_cli).
#
# Locates cmake/a C++20 compiler/ninja/vcpkg even when not on PATH,
# configures the matching preset (macos-release/linux-release) if it
# hasn't been configured yet, and builds. Run from anywhere -- paths are
# resolved relative to this script's own location -- src/ -- not the
# caller's cwd.
#
# Lives in src/ (alongside vcpkg.json/CMakeLists.txt/CMakePresets.json)
# rather than the repo root, since it only ever builds this one component.

set -euo pipefail

src_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(dirname "$src_dir")"

os="$(uname -s)"
case "$os" in
    Darwin) preset="macos-release" ;;
    Linux)  preset="linux-release" ;;
    *)
        echo "error: unsupported OS '$os' -- this script only handles macOS (Darwin) and Linux. Use build.ps1 on Windows." >&2
        exit 1
        ;;
esac

build_dir="$src_dir/build/$preset"

echo "== build.sh: locating toolchain (preset: $preset) =="

find_cmake() {
    if command -v cmake >/dev/null 2>&1; then
        command -v cmake
        return 0
    fi
    for c in /usr/local/bin/cmake /opt/homebrew/bin/cmake /opt/local/bin/cmake; do
        if [ -x "$c" ]; then
            echo "$c"
            return 0
        fi
    done
    return 1
}

find_ninja() {
    if command -v ninja >/dev/null 2>&1; then
        command -v ninja
        return 0
    fi
    for c in /usr/local/bin/ninja /opt/homebrew/bin/ninja /opt/local/bin/ninja; do
        if [ -x "$c" ]; then
            echo "$c"
            return 0
        fi
    done
    return 1
}

cmake_bin="$(find_cmake)" || {
    echo "error: cmake not found on PATH or in common install locations (/usr/local/bin, /opt/homebrew/bin, /opt/local/bin)." >&2
    echo "Install CMake >= 3.21 (e.g. 'brew install cmake' on macOS, or your distro's package manager on Linux), then retry." >&2
    exit 1
}
echo "  cmake: $cmake_bin"

ninja_bin="$(find_ninja)" || {
    echo "error: ninja not found on PATH or in common install locations." >&2
    echo "Install Ninja (e.g. 'brew install ninja' on macOS, or your distro's package manager on Linux), then retry." >&2
    exit 1
}
echo "  ninja: $ninja_bin"

if ! command -v c++ >/dev/null 2>&1 && ! command -v clang++ >/dev/null 2>&1 && ! command -v g++ >/dev/null 2>&1; then
    echo "error: no C++ compiler found (checked c++, clang++, g++ on PATH)." >&2
    echo "Install a C++20 compiler (Xcode Command Line Tools on macOS, gcc/clang on Linux), then retry." >&2
    exit 1
fi

if [ -z "${VCPKG_ROOT:-}" ]; then
    if [ -x "$repo_root/vcpkg/vcpkg" ]; then
        export VCPKG_ROOT="$repo_root/vcpkg"
    fi
fi
if [ -z "${VCPKG_ROOT:-}" ]; then
    # No existing checkout and no VCPKG_ROOT override -- bootstrap one at
    # <repo root>/vcpkg automatically, matching src/README.md's one-time
    # setup steps, instead of just telling the user to run them by hand.
    if ! command -v git >/dev/null 2>&1; then
        echo "error: VCPKG_ROOT is not set, no vcpkg/ checkout was found at the repo root, and git is not on PATH to clone one automatically." >&2
        echo "Install git, or clone vcpkg and export VCPKG_ROOT yourself (see src/README.md), then retry." >&2
        exit 1
    fi

    echo "== VCPKG_ROOT not set and no vcpkg/ checkout found -- cloning one to $repo_root/vcpkg =="
    git clone https://github.com/microsoft/vcpkg "$repo_root/vcpkg"

    echo "== bootstrapping vcpkg =="
    "$repo_root/vcpkg/bootstrap-vcpkg.sh" -disableMetrics

    export VCPKG_ROOT="$repo_root/vcpkg"
fi
echo "  VCPKG_ROOT: $VCPKG_ROOT"

cd "$src_dir"

if [ ! -f "$build_dir/build.ninja" ]; then
    echo "== configuring ($preset) -- first configure builds every vcpkg dependency from source and can take a long time =="
    "$cmake_bin" --preset "$preset"
else
    echo "== build directory already configured, skipping configure step =="
fi

echo "== building orcisf_gui orcisf_cli =="
"$cmake_bin" --build --preset "$preset" --target orcisf_gui orcisf_cli

# orcisf_gui is defined in src/CMakeLists.txt (top-level target dir);
# orcisf_cli is defined in src/engine/CMakeLists.txt (nested target dir) --
# their output paths mirror that same nesting under the build directory.
gui_bin="$build_dir/orcisf_gui"
cli_bin="$build_dir/engine/orcisf_cli"
echo "== build succeeded =="
echo "  orcisf_gui: $gui_bin"
echo "  orcisf_cli: $cli_bin"
