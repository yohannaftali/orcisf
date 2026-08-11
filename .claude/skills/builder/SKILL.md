---
name: builder
description: >
  Rebuilds the src/ GUI port (orcisf_gui, orcisf_cli, orcisf_engine) for
  whichever OS the agent is currently running on, after code changes have
  landed (a fix, an issue implementation, a review pass). Use this whenever
  you're about to verify/test a change to src/ and need a fresh binary --
  "rebuild", "build it", "does this compile", "verify the fix", "run it and
  check" — not just when the user says "build" literally. Runs the repo's
  own build.ps1/build.sh (issue #32) rather than re-deriving toolchain
  paths by hand, and reports pass/fail plus the built binary's path. Do not
  use for the legacy Optimasi Beton/Source/ program (no modern toolchain
  can build it, see AGENTS.md) or for anything outside src/.
---

# Builder Skill

You are a build specialist for this repo's `src/` GUI port. Your job is
to get a fresh, correct build of `orcisf_gui`/`orcisf_cli` on whichever
OS this session is running on, and report back clearly whether it
succeeded.

**Read `AGENTS.md` first**, even for a task this mechanical — this
project's convention is that every skill reads it before doing anything,
since a small detail there (a currently-known toolchain quirk, a
Windows-specific gotcha already documented in the Validation section)
can change how you interpret a build failure. Don't skip this because
the task feels self-contained.

**Run the repo's own build script rather than re-deriving toolchain
paths yourself** — `build.ps1` (Windows) / `build.sh` (macOS/Linux) at
the repo root already encode the OS-detection, toolchain-location
(CMake/MSVC/vcpkg, including fallback paths when not on `PATH`),
configure-if-needed, and build steps this skill used to spell out
step-by-step. Calling the script directly does the same job in far
fewer tokens than re-discovering those paths inline every invocation.

---

## Step 1 — Run the script

Detect the OS and invoke the matching script from the repo root:

```powershell
# Windows
.\build.ps1
```

```sh
# macOS / Linux
./build.sh
```

Both scripts already:
- Locate CMake/MSVC/vcpkg (checking `PATH` first, then common install
  locations) and fail with a clear, actionable message if a required
  tool genuinely can't be found — trust that message, don't second-guess
  it by manually re-checking `PATH` yourself first.
- Configure the matching preset (`windows-release`/`macos-release`/
  `linux-release`) only if `src/build/<preset>/` doesn't already exist —
  warn the user before waiting if this is a first configure (it builds
  every vcpkg dependency from source and can take a long time).
- Build `orcisf_gui` and `orcisf_cli`, printing the toolchain paths used
  and the final binary locations on success.

If a script's own error output says a tool is missing, that's the
answer — don't fall back to manually searching for the toolchain
yourself unless the script's own fallback logic is what's actually
wrong (in which case, fix the script, don't route around it for this
one run).

**Windows-specific gotcha**: if linking fails with `LNK1104: cannot open
file 'orcisf_gui.exe'`, a previous build's process is still running and
holding the file open. Check `Get-Process orcisf_gui` and stop it
(`Stop-Process -Force`) before retrying — this is not a build error to
debug, just a leftover process to clean up.

---

## Step 2 — Report

State clearly:
- Which preset/OS was built (the script prints this).
- Pass/fail, with the actual compiler errors if it failed (don't
  paraphrase them away).
- The built binaries' paths, as printed by the script — note
  `orcisf_cli` lands in a nested `engine/` subdirectory of the build
  dir (it's defined in `src/engine/CMakeLists.txt`, not the top-level
  one), not the build root alongside `orcisf_gui`.
- Whether you also launched/smoke-tested it, and what you saw — per
  this project's validation conventions, distinguish "compiled" from
  "ran without crashing" from "visually confirmed correct" (they are
  different levels of verification, don't conflate them in your report).

If a genuinely new toolchain fallback location is discovered that
`build.ps1`/`build.sh` don't already check, fix the scripts themselves
(and mirror the change in both, plus `AGENTS.md`'s Validation section)
rather than only noting it in this skill's instructions — the scripts
are the source of truth now, this file is just how an agent invokes them.

Do not commit or push anything as part of this skill — building is a
local verification step, not a code change.
