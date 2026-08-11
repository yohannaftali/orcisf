# build.ps1 -- one-shot Windows build of src/ (orcisf_gui, orcisf_cli).
#
# Locates CMake/MSVC/vcpkg even when not on PATH (see .claude/skills/builder/
# SKILL.md, which documents the same fallback locations for an AI agent),
# configures the windows-release preset if it hasn't been configured yet,
# and builds. Run from anywhere (paths are resolved relative to this
# script's own location -- src/ -- not the caller's current directory).
#
# Lives in src/ (alongside vcpkg.json/CMakeLists.txt/CMakePresets.json)
# rather than the repo root, since it only ever builds this one component.

$ErrorActionPreference = "Stop"

$SrcDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $SrcDir
$Preset = "windows-release"
$BuildDir = Join-Path $SrcDir "build\$Preset"

function Find-CMake {
    $cmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    $candidates = @(
        "C:\Program Files\CMake\bin\cmake.exe",
        "C:\Qt\Tools\CMake_64\bin\cmake.exe"
    )
    foreach ($c in $candidates) {
        if (Test-Path $c) { return $c }
    }
    return $null
}

function Find-VcVars64 {
    $vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $installPath = & $vswhere -latest -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath 2>$null
        if ($installPath) {
            $vcvars = Join-Path $installPath "VC\Auxiliary\Build\vcvars64.bat"
            if (Test-Path $vcvars) { return $vcvars }
        }
    }

    $editions = @("BuildTools", "Community", "Professional", "Enterprise")
    foreach ($edition in $editions) {
        $vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2022\$edition\VC\Auxiliary\Build\vcvars64.bat"
        if (Test-Path $vcvars) { return $vcvars }
    }
    return $null
}

Write-Host "== build.ps1: locating toolchain ==" -ForegroundColor Cyan

$CMake = Find-CMake
if (-not $CMake) {
    Write-Error "cmake.exe not found on PATH or in common install locations (C:\Program Files\CMake\bin, C:\Qt\Tools\CMake_64\bin). Install CMake >= 3.21 or add it to PATH, then retry."
    exit 1
}
Write-Host "  cmake: $CMake"

$VcVars = Find-VcVars64
if (-not $VcVars) {
    Write-Error "MSVC vcvars64.bat not found (checked vswhere.exe and C:\Program Files (x86)\Microsoft Visual Studio\2022\{BuildTools,Community,Professional,Enterprise}). Install Visual Studio 2022 Build Tools with the 'Desktop development with C++' workload, then retry."
    exit 1
}
Write-Host "  MSVC env: $VcVars"

if (-not $env:VCPKG_ROOT) {
    $candidate = Join-Path $RepoRoot "vcpkg"
    if (Test-Path (Join-Path $candidate "vcpkg.exe")) {
        $env:VCPKG_ROOT = $candidate
    }
}
if (-not $env:VCPKG_ROOT) {
    # No existing checkout and no VCPKG_ROOT override -- bootstrap one at
    # <repo root>/vcpkg automatically, matching src/README.md's one-time
    # setup steps, instead of just telling the user to run them by hand.
    $Git = Get-Command git -ErrorAction SilentlyContinue
    if (-not $Git) {
        Write-Error "VCPKG_ROOT is not set, no vcpkg/ checkout was found at the repo root, and git is not on PATH to clone one automatically. Install git, or clone vcpkg and set `$env:VCPKG_ROOT` yourself (see src/README.md), then retry."
        exit 1
    }

    $candidate = Join-Path $RepoRoot "vcpkg"
    Write-Host "== VCPKG_ROOT not set and no vcpkg/ checkout found -- cloning one to $candidate ==" -ForegroundColor Cyan
    git clone https://github.com/microsoft/vcpkg $candidate
    if ($LASTEXITCODE -ne 0) {
        Write-Error "git clone of vcpkg failed (exit $LASTEXITCODE)."
        exit 1
    }

    Write-Host "== bootstrapping vcpkg ==" -ForegroundColor Cyan
    & (Join-Path $candidate "bootstrap-vcpkg.bat") -disableMetrics
    if ($LASTEXITCODE -ne 0) {
        Write-Error "vcpkg bootstrap failed (exit $LASTEXITCODE)."
        exit 1
    }

    $env:VCPKG_ROOT = $candidate
}
Write-Host "  VCPKG_ROOT: $env:VCPKG_ROOT"

# vcvars64.bat only affects its own cmd.exe process, so configure/build run
# inside a single `cmd /c "vcvars64.bat && ..."` invocation to inherit it.
$NeedsConfigure = -not (Test-Path (Join-Path $BuildDir "build.ninja"))

Push-Location $SrcDir
try {
    if ($NeedsConfigure) {
        Write-Host "== configuring ($Preset) -- first configure builds every vcpkg dependency from source and can take a long time ==" -ForegroundColor Cyan
        $configureCmd = "call `"$VcVars`" && `"$CMake`" --preset $Preset"
        cmd /c $configureCmd
        if ($LASTEXITCODE -ne 0) {
            Write-Error "cmake --preset $Preset failed (exit $LASTEXITCODE)."
            exit 1
        }
    }
    else {
        Write-Host "== build directory already configured, skipping configure step ==" -ForegroundColor Cyan
    }

    Write-Host "== building orcisf_gui orcisf_cli ==" -ForegroundColor Cyan
    $buildCmd = "call `"$VcVars`" && `"$CMake`" --build --preset $Preset --target orcisf_gui orcisf_cli"
    cmd /c $buildCmd
    if ($LASTEXITCODE -ne 0) {
        Write-Error "cmake --build failed (exit $LASTEXITCODE). If linking failed with LNK1104 on orcisf_gui.exe, a previous run may still be holding the file open -- check 'Get-Process orcisf_gui' and stop it, then retry."
        exit 1
    }
}
finally {
    Pop-Location
}

# orcisf_gui is defined in src/CMakeLists.txt (top-level target dir);
# orcisf_cli is defined in src/engine/CMakeLists.txt (nested target dir) --
# their output paths mirror that same nesting under the build directory.
$GuiExe = Join-Path $BuildDir "orcisf_gui.exe"
$CliExe = Join-Path $BuildDir "engine\orcisf_cli.exe"
Write-Host "== build succeeded ==" -ForegroundColor Green
Write-Host "  orcisf_gui: $GuiExe"
Write-Host "  orcisf_cli: $CliExe"
