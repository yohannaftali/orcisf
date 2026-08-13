# ORCISF

**O**ptimizing **R**einforced **C**oncrete **I**n **S**pace **F**rame structure

A 1998–1999 undergraduate-thesis (Tugas Akhir S1) engineering program by
**Yohan Naftali** (7712/TS, Program Studi Teknik Sipil, peminatan Struktur,
**Universitas Atma Jaya Yogyakarta**; advisors Dr. Ir. FX. Nurwadji Wibowo,
M.Sc. and Ir. Ch. Arief Sudibyo) that analyzes a 3D reinforced-concrete
building frame and then automatically optimizes every beam and column's
dimensions and reinforcement to minimize material cost, subject to strength
and serviceability constraints. This repository also hosts a **modern,
cross-platform GUI port** of the same engine (see below).

> For AI coding agents: read [`AGENTS.md`](AGENTS.md) first — it is the single
> source of truth for this repository's architecture and working rules.

---

## What it does

Given a 3D space-frame building model (joints, members, restraints, loads) and
a set of discrete candidate cross-sections/reinforcement, ORCISF:

1. **Analyzes the structure** with the direct-stiffness method for 3D space
   frames (developed from Weaver & Gere), solving for member forces and joint
   displacements with a modified-Choleski banded solver.
2. **Designs every beam and column** against the forces from step 1 (biaxial
   column interaction, beam flexure/shear/deflection), tracking a **cost**
   (concrete + steel, by user-supplied unit prices) and a **constraint
   violation** for each candidate design.
3. **Optimizes** the whole set of beam/column design variables together using
   the **"Flexible Polyhedron"** method — a generalized-simplex, derivative-free
   direct-search optimizer (Nelder–Mead family) — repeating analyze → design →
   evaluate → search for a configurable number of generations (10,000+
   recommended) until the design converges on a minimum-cost, constraint-
   satisfying solution.

Output is a set of plain-text files: final optimized dimensions/reinforcement,
the structural analysis results, constraint-check results, and an
optimization-run history log.

## Status

- **`Optimasi Beton/`** is a historical **archive** of the original 1999
  thesis deliverable: source code, the compiled Windows binary, sample
  datasets, and the full written manuscript (in Indonesian). It is not
  under active development and is kept as-is.
- **`src/`** is an actively-developed, ground-up **modern GUI port**
  (Windows/macOS/Linux desktop app) of the same analysis/design/optimization
  engine, with an interactive 3D editor, threaded optimizer, reinforcement
  detailing drawings, and PDF/legacy-text export. See the
  [**GUI application tutorial**](#gui-application-tutorial) below, or grab a
  prebuilt Windows binary from the
  [Releases page](https://github.com/yohannaftali/orcisf/releases).

## Repository contents

```
orcisf/
├── AGENTS.md               # architecture & working rules (read this first)
├── CHANGE_HISTORY.md       # dated project history
├── src/                    # modern cross-platform GUI port (actively developed)
│   ├── app/                # GLFW/OpenGL3/ImGui bootstrap, docking layout
│   ├── gui/                # panels: Viewport, Properties, Loads, Joints/Members,
│   │                       # Detailing, Run Optimization, Log
│   ├── engine/             # headless analysis/design/optimizer library + CLI
│   └── report/             # PDF + legacy-text export
└── Optimasi Beton/         # the original 1998-1999 thesis deliverable (archive)
    ├── BacaSaya.txt                 # original Indonesian user manual
    ├── orcisf.exe / cw3230.dll      # compiled Win32 console binary + runtime
    ├── Optimasi Struktur Beton.doc  # (2006) — empty, no recoverable content
    ├── Source/                      # Borland C++ 5.02 source code
    ├── Example/                     # sample input/output datasets
    └── Teori/                       # full thesis manuscript (Indonesian, .doc/.pdf)
        └── latex/                   # LaTeX transcription of the thesis (see AGENTS.md)
```

See [`AGENTS.md`](AGENTS.md) for the full breakdown of the source modules, the
input/output file-format convention, and the optimization algorithm.

---

## GUI application tutorial

The modern GUI (`src/`, built as `orcisf_gui`) covers the full legacy
pipeline — load or create a dataset, view and edit it in 3D, apply loads,
run the threaded optimizer, inspect reinforcement detailing, and export a
PDF report or the full legacy text file set.

### Getting the app

- **Prebuilt binary**: download the latest Windows build from the
  [Releases page](https://github.com/yohannaftali/orcisf/releases).
- **Build from source**: see [`src/README.md`](src/README.md) (needs CMake,
  a C++20 compiler, and [vcpkg](https://vcpkg.io/)):
  ```sh
  cd src
  cmake --preset windows-release   # or macos-release / linux-release
  cmake --build --preset windows-release
  ```
  The built app is `src/build/<preset>/orcisf_gui.exe` (or the equivalent
  binary name on macOS/Linux).

### The window

On launch you'll see a custom (borderless) title bar, a menu bar
(**File / Edit / Loads / Run / View**), an icon toolbar below it
(New Data / Open Data / Save / Undo / Redo / Add Joint / Connect Joints /
Run), and a docked set of panels: **Viewport** (3D view), **Detailing**
(2D reinforcement drawings), **Properties**, **Joints/Members**, **Loads**,
**Run Optimization**, and **Log**. Use **View > Default / Design /
Optimization** to switch between layout presets sized for different stages
of the workflow — no panel is ever hidden by a preset, just resized/re-tabbed.

### Coordinate system

ORCISF's 3D viewport is **Y-up**: **Y** is the vertical (height) axis, and
**X** and **Z** form the horizontal ground plane. This matches the
direct-stiffness engine's own convention (self-weight is applied along
"arah 2 = Y") — it isn't just a display choice, so don't confuse it with
tools that treat Z as vertical (Blender/CAD-style X-Y-ground conventions).
A dark-cobalt-blue reference grid on the X-Z ground plane, labeled `X1,
X2, X3...` and `Z1, Z2, Z3...` along its edges (matching the real
coordinates shown in the Joints/Properties panels), is always visible in
the Viewport to help orient a structure at a glance — together with the
small UCS (axis) icon in the corner, which shows the current camera
orientation in every view mode.

### 1. Start a dataset

- **File > New Data** — prompts for a save location, then starts a blank,
  empty structure you build up from scratch.
- **File > Open Data...** — pick a folder containing an existing legacy
  dataset (e.g. one of `Optimasi Beton/Example/Apl1-1/`'s `.inp`-rooted file
  sets) and it loads directly into the 3D viewport.

> Always work against a **copy** of a bundled `Example/` dataset, not the
> original — running/saving overwrites files in place, and on Windows the
> filenames are case-insensitive (`GEDUNG.opt` and `gedung.opt` collide).

### 2. Build/edit the geometry

- **Add Joint** (toolbar icon or Edit menu) is a click-to-place mode: click
  in the Viewport to place a joint; it stays active so you can place several
  in a row. Click the button again (or pick another mode) to stop.
- **Connect Joints** is the same click-to-place pattern for members: click
  a first joint, then a second, to create a member between them.
- Drag a selected joint's translate gizmo in the 3D view to move it, or type
  exact X/Y/Z coordinates in the **Properties** panel.
- The **Joints/Members** panel lists every joint (editable X/Y/Z, a
  restrained toggle, Delete) and member (Delete) in a table — deleting a
  joint that still has members attached shows a confirmation listing which
  members will also be deleted (members always need two joints, so this is
  a cascade, not a partial delete).
- **Edit > Snap to Grid** plus a grid-size slider snaps placements/moves to
  a regular spacing.
- **View > View Plane** locks the viewport to an orthographic X-Y / X-Z /
  Y-Z plane at an adjustable offset (typeable or via slider) along the
  locked axis — useful for drawing a single floor plan or elevation exactly
  on-plane. A small readout at the bottom of the viewport shows which plane
  and offset is active; a UCS (axis) icon in the corner always shows the
  current camera orientation, in every view mode.
- **Properties** panel's "General" section (shown when nothing is selected)
  edits structure-wide fields: title, `E`/`G`/`FC`/`FY`/`FYS` material
  properties.

### 3. Apply loads

Use the **Loads** menu to enter a placement mode, then click in the
viewport:
- **Add Member Load** — click a member to apply a uniform distributed load
  (fine-tune the exact value afterward).
- **Add Joint Load** — click a joint to apply a 6-DOF force/moment (covers
  point loads, moments, and lateral/wind loads — the legacy format doesn't
  distinguish them, they're all just a joint action).

The **Loads** panel lists every member/joint with a nonzero load in an
editable table. **File > Save Loads (.bbn)** writes them back to the
dataset's load file.

### 4. Run the optimizer

Open the **Run Optimization** panel:
1. Enter the dataset's generic path (no extension) if it isn't already
   filled in.
2. Set cost/design parameters: concrete/steel unit prices, column/beam
   cover thickness, penalty factor, max generations, and the population-size
   `fak_plus`/`fak_kali` factors (same meaning as the legacy console
   program's prompts).
3. Under **Performance**, choose how many worker threads to use — more
   threads only changes wall-clock time, never the numeric result, for a
   given RNG seed.
4. Click **Run**. Progress (generation, best fitness/cost/constraint,
   elapsed time) updates live; **Cancel** stops early with the best design
   found so far.
5. Once a run completes against a given dataset path, a **"Re-optimize from
   last best result"** checkbox becomes available — check it before your
   next **Run** to continue refining that design instead of starting from a
   fresh random population.

When a run finishes, the Viewport/Detailing panels automatically refresh
with the new results.

### 5. Inspect results

- The **Viewport** colors members by constraint status and scales their
  cross-section to the optimized dimensions.
- Select a beam or column (click it in the 3D view, or via
  Joints/Members) to see its numeric results in **Properties**, and its
  2D reinforcement drawing (bar counts/diameters/spacing, stirrups,
  dimensions) in **Detailing**.

### 6. Export

From the **File** menu:
- **Export PDF...** — a full report (cover/input summary, per-member
  results table, one detailing page per member), available once a run has
  completed.
- **Export Text...** — the full legacy file set
  (`.inp/.isd/.idl/.ijl/.ids/.ijs/.bbn`, plus `.opt/.str/.kdl/.inf/.his`
  if a completed run exists) to a folder you choose.
- **Export INF Preview...** — just the input-echo portion (`.inf`),
  generated directly from the current in-GUI dataset, independent of a
  completed run.
- **Save** / **Save As...** write the editable dataset back to its legacy
  file set (same formats the original console program reads).

---

## Running the original 1999 console program

`Optimasi Beton/orcisf.exe` is a Win32 console application (requires
`cw3230.dll` in the same folder — a Borland C++ runtime DLL). Run it from a
Windows console in that folder and follow the menu:

1. Enter general + discrete design data (writes `.inp`/`.isd`/`.idl`/`.ijl`/
   `.ids`/`.ijs`).
2. Enter load data (writes `.bbn`).
3. Browse an existing input dataset.
4. Optimize a structure — enter the generic filename (e.g. an
   `Example/...` case), unit prices, cover thickness, penalty factor, and
   iteration count, then let it run.
5. Exit.

All input files are plain ASCII, so they can also be prepared/edited directly
in a text editor. See `Optimasi Beton/BacaSaya.txt` for the original author's
instructions (Indonesian), and `Optimasi Beton/Example/` for ready-to-run
sample datasets.

## Building from source

- **Modern GUI (`src/`)**: see [`src/README.md`](src/README.md) and the
  [GUI application tutorial](#gui-application-tutorial) above.
- **Legacy console program (`Optimasi Beton/Source/`)**: targets
  **Borland C++ 5.02** and uses pre-standard, DOS-console-era headers — it
  will not compile unmodified with a modern compiler. Rebuilding as
  originally written requires Borland C++ 5.02 (or a compatible
  Borland/Turbo C++ toolchain) and the project file
  `Optimasi Beton/Source/ORCISF.ide`. See [`AGENTS.md`](AGENTS.md) for
  details.

## Algorithms

Every formula this application implements — load handling, structural
analysis, beam/column design, and the Flexible Polyhedron optimizer — is
documented in `documentation/`, with each formula cited against both the
original 1999 source and the modern `src/engine/` port:

- [A. Loads](documentation/loads.md) — reading load input, self-weight
  calculation, assembling the global load vector.
- [B. Structural Analysis](documentation/structural-analysis.md) — inertia/
  torsion constants, member inclination ("kemiringan"), stiffness matrix
  assembly, the modified-Choleski solve, and recovering displacements/
  forces/reactions.
- [C. Beam](documentation/beam.md) — flexural, shear, and deflection design.
- [D. Column](documentation/column.md) — slenderness, biaxial P-M
  interaction, and stirrup design.
- [E. Optimization](documentation/optimization.md) — the Flexible
  Polyhedron (Nelder–Mead family) direct-search method.

## Documentation

The full thesis manuscript (Indonesian, legacy MS Word `.doc` format) is under
`Optimasi Beton/Teori/`:

- `Isi/` — chapters BAB I–V (introduction, literature review, methodology,
  results, conclusion), plus the abstract (`Intisari.doc`) and proposal.
- `Daftar/` — table of contents, lists of figures/tables/symbols, references.
- `Lampiran/` — 38 appendices, largely source-code listings and result tables.
- `Optimasi Polyhedron.pdf` / `Optimasi Polyhedron Naftali.pdf` — condensed
  PDF summaries of the optimization method.
- `latex/` — a LaTeX transcription of the thesis (title page through
  bibliography), built by extracting the `.doc` text and reconstructing
  formulas that were embedded as MathType objects by cross-referencing the
  C++ source. `latex/main.pdf` is the compiled result;
  `latex/LEGACY_TRANSCRIPTION_NOTES.md` documents exactly what's verbatim
  vs. reconstructed vs. unrecoverable (figures).

## Releases

Prebuilt binaries of the modern GUI are published on the
[Releases page](https://github.com/yohannaftali/orcisf/releases). The
project is in an early **alpha** stage — expect rough edges, and please
report issues.

## License

No license file is currently included in this repository.

## Author

**Yohan Naftali** — original author (1998–1999), Universitas Atma Jaya
Yogyakarta.
