---
name: rebuild
description: >
  Rebuilds the src/ GUI port (orcisf_gui, orcisf_cli, orcisf_engine) for
  whichever OS the agent is currently running on, after code changes have
  landed (a fix, an issue implementation, a review pass). Use this whenever
  you're about to verify/test a change to src/ and need a fresh binary --
  "rebuild", "build it", "does this compile", "verify the fix", "run it and
  check" — not just when the user says "build" literally. Locates the
  toolchain (CMake/compiler/vcpkg) even when it isn't on the default PATH,
  builds via the matching CMake preset, and reports pass/fail plus the
  built binary's path. Do not use for the legacy Optimasi Beton/Source/
  program (no modern toolchain can build it, see AGENTS.md) or for
  anything outside src/.
---

# Rebuild Skill

You are a build specialist for this repo's `src/` GUI port. Your job is
to get a fresh, correct build of `orcisf_gui`/`orcisf_cli` on whichever
OS this session is running on, using whatever toolchain is actually
available — even if it isn't on the default shell `PATH` — and report
back clearly whether it succeeded.

Follow these steps in order on every invocation.

---

## Step 1 — Detect the OS and the right CMake preset

`src/CMakePresets.json` defines `windows-release`, `macos-release`,
`linux-release` (each gated on `hostSystemName`). Match the current OS:

| OS | Preset |
|---|---|
| Windows | `windows-release` |
| macOS | `macos-release` |
| Linux | `linux-release` |

Check whether `src/build/<preset>/` already exists (a prior session may
have configured it) — if so, you can usually skip straight to Step 3
(the build step reconfigures automatically if `CMakeLists.txt`/
`vcpkg.json` changed).

---

## Step 2 — Locate the toolchain (don't assume it's on `PATH`)

**Never conclude "no toolchain available" from a `PATH`-only check
(`which cmake`, `Get-Command cmake`) failing.** This project has been
caught making that mistake before (see `AGENTS.md`'s Validation section)
— a real toolchain existed but wasn't on the default `PATH`.

### Windows
1. `cmake`: check `PATH` first; if missing, search common install
   locations (e.g. `C:\Program Files\CMake\bin\cmake.exe`, IDE-bundled
   copies like `C:\Qt\Tools\CMake_64\bin\cmake.exe`, or wherever a prior
   session's `AGENTS.md`/`CHANGE_HISTORY.md` notes it was found — check
   those files first, they may already record the exact path).
2. MSVC: locate via `vswhere.exe`
   (`C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe`)
   or by checking `C:\Program Files (x86)\Microsoft Visual Studio\2022\
   BuildTools` / `...\Community` / `...\Professional`. Set up the
   environment by running `VC\Auxiliary\Build\vcvars64.bat` before
   invoking `cmake`/`ninja` (via `cmd /c "<vcvars64.bat> && <command>"`
   from PowerShell, since `vcvars64.bat` only affects its own process).
3. `VCPKG_ROOT`: check if already set, or look for a `vcpkg/` checkout at
   the repo root or a scratch location a prior session may have cloned
   (see `src/README.md` for the one-time bootstrap if none exists yet).

### macOS / Linux
1. `cmake`, a C++20 compiler (clang/gcc), and `ninja` are usually on
   `PATH` if installed via the system package manager or Homebrew — check
   `PATH` first here, but still search `/usr/local/bin`,
   `/opt/homebrew/bin` (Apple Silicon Homebrew), and `/opt/*/bin` if not.
2. `VCPKG_ROOT` — same as Windows: check env var, then a local `vcpkg/`
   checkout.

If a toolchain genuinely cannot be found after checking beyond `PATH`,
say so explicitly and stop — don't claim a build was verified that
wasn't attempted.

---

## Step 3 — Build

```sh
# with VCPKG_ROOT set and the toolchain environment active (vcvars64.bat on Windows):
cmake --build src/build/<preset> --target orcisf_gui orcisf_cli
```

If `src/build/<preset>/` doesn't exist yet, configure first:

```sh
cmake --preset <preset>
cmake --build --preset <preset> --target orcisf_gui orcisf_cli
```

The first configure on a clean machine builds every vcpkg dependency
from source (no binary cache) and can take a long time — warn the user
before starting if `vcpkg_installed/` doesn't already exist.

**Windows-specific gotcha**: if linking fails with `LNK1104: cannot open
file 'orcisf_gui.exe'`, a previous build's process is still running and
holding the file open. Check `Get-Process orcisf_gui` and stop it
(`Stop-Process -Force`) before retrying — this is not a build error to
debug, just a leftover process to clean up.

---

## Step 4 — Report

State clearly:
- Which preset/OS was built, and the toolchain paths used (so the next
  session doesn't have to rediscover them — consider recording a
  genuinely new toolchain location in `AGENTS.md`'s Validation section
  if the one already documented there was wrong/outdated).
- Pass/fail, with the actual compiler errors if it failed (don't
  paraphrase them away).
- The built binary's path (e.g. `src/build/windows-release/orcisf_gui.exe`).
- Whether you also launched/smoke-tested it, and what you saw — per
  this project's validation conventions, distinguish "compiled" from
  "ran without crashing" from "visually confirmed correct" (they are
  different levels of verification, don't conflate them in your report).

Do not commit or push anything as part of this skill — building is a
local verification step, not a code change.
