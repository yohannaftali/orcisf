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

**Current state (#2 scaffold + #3 engine port + #4 threaded core land; #5–#9 not started):**
```
src/
├── vcpkg.json / CMakeLists.txt / CMakePresets.json
├── app/
│   ├── main.cpp             # GLFW/OpenGL3/ImGui bootstrap + render loop
│   └── Application.{h,cpp}  # docking layout (Viewport | Properties/RunPanel / Log)
├── gui/
│   ├── Toolbar.{h,cpp}          # menu bar — placeholder items only
│   ├── ViewportPanel.{h,cpp}    # 3D view — placeholder, real work is #5
│   ├── PropertiesPanel.{h,cpp}  # selection editor — placeholder, #6/#7
│   ├── RunPanel.{h,cpp}         # #4: dataset/options form, worker-thread-count
│   │                            # slider, Run/Cancel, live progress bar — runs
│   │                            # engine::RunFullOptimization on a background
│   │                            # std::thread so the UI never blocks
│   └── LogPanel.{h,cpp}         # run/status log — functional; detailed
│                                # calc log to disk is done, see engine/
└── engine/                      # #3/#4: headless analysis/design/optimizer
    ├── README.md                # full architecture + validation writeup — read before touching this dir
    ├── include/engine/*.h, src/*.cpp   # one pair per ported legacy file, see README's table
    └── tools/orcisf_cli.cpp     # headless CLI: `info`/`equilibrium`/`optimize [worker_threads] [rng_seed] [quiet]`
```
`ViewportPanel`/`PropertiesPanel`/`Toolbar` are intentionally inert
placeholders — **do not build real features into them under issue #2**;
that belongs to their respective linked issues (#5–#9). `RunPanel` (#4) is
the first GUI panel that actually calls into `orcisf_engine`. Read the
relevant issue before starting work here — each has detailed acceptance
criteria. As each sub-issue lands, extend this section (new subsystems,
data model, how the engine/GUI/export layers connect) rather than
replacing it wholesale, per the Change Log Policy.

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

## Tracked Issues
| ID | Title | Status | Last Checked |
|----|-------|--------|--------------|
| #1 | epic(src): port ORCISF to a modern cross-platform GUI application | open | 2026-08-09 |
| #2 | feat(src): choose GUI stack & scaffold cross-platform CMake build (Win/macOS/Linux) | closed | 2026-08-09 |
| #3 | feat(src): port structural-analysis + RC-design + Flexible-Polyhedron engine as a headless library | closed | 2026-08-09 |
| #4 | feat(src): multi-threaded optimization core with configurable core count + cancellable progress | closed | 2026-08-10 |
| #5 | feat(src): 3D viewport + example-folder loader (render structure & per-member results) | open | 2026-08-09 |
| #6 | feat(src): interactive 3D structure editor (import, drag-and-drop edit, numeric entry) | open | 2026-08-09 |
| #7 | feat(src): load (pembebanan) input GUI (toolbar-driven, CAD-style 3D placement) | open | 2026-08-09 |
| #8 | feat(src): reinforcement detailing drawings per beam/column | open | 2026-08-09 |
| #9 | feat(src): PDF + legacy-text export of results | open | 2026-08-09 |

Epic #1 tracks #2–#9. Chosen stack (see #1 for rationale): Dear ImGui
(docking) + GLFW + OpenGL3, ImGuizmo (3D manipulation), ImPlot (charts),
nativefiledialog-extended (native file/folder dialogs), libharu/HPDF (PDF
export), CMake + vcpkg (Win/macOS/Linux build). **#2's scaffold has
landed** (see `src/`, "`src/` — modern cross-platform GUI port" above) —
build/window/docking skeleton only, no engine/viewport/editor/export
functionality yet; #3–#9 are still open.

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
