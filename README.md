# ORCISF

**O**ptimizing **R**einforced **C**oncrete **I**n **S**pace **F**rame structure

A 1998–1999 undergraduate-thesis (Tugas Akhir S1) engineering program by
**Yohan Naftali** (7712/TS, Program Studi Teknik Sipil, peminatan Struktur,
**Universitas Atma Jaya Yogyakarta**; advisors Dr. Ir. FX. Nurwadji Wibowo,
M.Sc. and Ir. Ch. Arief Sudibyo) that analyzes a 3D reinforced-concrete
building frame and then automatically optimizes every beam and column's
dimensions and reinforcement to minimize material cost, subject to strength
and serviceability constraints.

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

This repository is primarily an **archive** of the original 1999 thesis
deliverable: source code, the compiled Windows binary, sample datasets, and
the full written manuscript (in Indonesian). It is not under active
development as a modern application — see [`src/`](src/) in `AGENTS.md`, which
is reserved (currently empty) for a possible future modernized/ported version.

## Repository contents

```
orcisf/
├── AGENTS.md              # architecture & working rules (read this first)
├── CHANGE_HISTORY.md       # dated project history
├── src/                    # reserved, currently empty
└── Optimasi Beton/         # the original 1998-1999 thesis deliverable
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

## Running it

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

The source (`Optimasi Beton/Source/`) targets **Borland C++ 5.02** and uses
pre-standard, DOS-console-era headers — it will not compile unmodified with a
modern compiler. Rebuilding as originally written requires Borland C++ 5.02 (or
a compatible Borland/Turbo C++ toolchain) and the project file
`Optimasi Beton/Source/ORCISF.ide`. See [`AGENTS.md`](AGENTS.md) for details
and for guidance on porting to a modern toolchain.

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

## License

No license file is currently included in this repository.

## Author

**Yohan Naftali** — original author (1998–1999), Universitas Atma Jaya
Yogyakarta.
