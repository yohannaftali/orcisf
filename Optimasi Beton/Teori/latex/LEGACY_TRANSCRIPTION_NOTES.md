# Transcription notes — LaTeX version of `Optimasi Struktur Beton.doc` / `Teori/`

This directory (`Optimasi Beton/Teori/latex/`) is a LaTeX transcription of the
thesis manuscript. Two source documents were considered:

- **`Optimasi Beton/Optimasi Struktur Beton.doc`** (2006, condensed
  paper/summary) — inspected directly with `antiword` and by manually
  parsing the Word binary FIB/piece-table structure with Python
  (`olefile`). Its main document story is empty (`ccpText = 1`, i.e. only
  the trailing paragraph mark) and it contains no embedded OLE objects or
  recognizable image signatures. **There is no recoverable text in this
  file.** This was confirmed independently by two methods, not assumed.
- **`Optimasi Beton/Teori/Isi/BAB I.doc` … `BAB V.doc`, `Intisari.doc`,
  `Cover/*.doc`, `Daftar/*.doc`** — the actual 1999 thesis manuscript.
  These *do* contain real, extractable content, and are what this LaTeX
  document is built from.

## Method

1. Plain text was extracted from every `.doc` file with `antiword` (0.37).
2. Many formulas in `BAB I.doc`–`BAB IV.doc` are **not** plain text — they
   were inserted as embedded MathType/Equation Editor 3.0 OLE objects
   (`ObjectPool/.../Equation Native` streams inside the compound-file
   `.doc`), which `antiword` can only render as a bare `[pic]` placeholder
   with the equation's number floating nearby (and, because these are
   anchored/floating objects, not always in reading order in the
   `antiword` dump). Verified with `olefile` by listing each `.doc`'s OLE
   directory (`BAB I.doc` has 1 such object, `BAB II.doc` has ~24, etc.).
3. Those equations were **reconstructed**, not guessed, by cross-referencing:
   - the surrounding descriptive paragraphs (which usually name every
     variable in the missing formula, e.g. "pada persamaan di atas RHO
     adalah rasio penulangan, kendala\_rho\_b adalah kendala rasio
     tulangan maksimum..." — enough to reconstruct the exact formula from
     the corresponding source code identifier), and
   - the **actual C++ implementation** in `Optimasi Beton/Source/*.hpp`
     (`Balok.hpp`, `Kolom.hpp`, `Struktur.hpp`, `Solver.hpp`), which is the
     literal, working implementation of the method the thesis describes —
     i.e. the ground truth for what is actually computed, and (b) the
     standard references the text itself cites (Weaver & Gere 1980 for the
     stiffness method; SK SNI T-15-1991-03 for RC design; Hulse & Mosley
     1986 for biaxial columns; Nelder–Mead/flexible-polyhedron literature).
4. Equations that were given as literal, readable text in the `.doc`
   (most of BAB I's problem formulation, and several equilibrium/strain
   equations in BAB II) were transcribed verbatim into LaTeX math mode —
   no reconstruction needed there.
5. **Figures are not recoverable.** Diagrams (Gambar 2-1, 2-2, 4-1 through
   4-6), the console-menu screenshot (Gambar 3-1, reproduced here as a
   monospaced text box instead), and the 12×12 member-stiffness matrix
   (Tabel 2-1, which exists in the source `.doc` *only* as a raster/vector
   image with no extractable text at all) could not be pulled out of the
   legacy binary format. Each is marked in the LaTeX source with a visible
   placeholder box (`\gambarhilang{...}{...}`) or, for Tabel 2-1, an
   explicit reconstruction from the cited Weaver & Gere reference (see the
   comment at the top of `bab/tabel2-1.tex`).

## What this means for reliability

- **Front matter, all narrative/prose text, all data tables (BAB IV), the
  bibliography, and every equation given as plain text in the original**:
  high-confidence, close-to-verbatim transcription.
- **Equation numbers/exact split (BAB II's ~2-7 through 2-25, BAB III's
  3-1 through 3-14)**: the *mathematical content* is cross-checked against
  the shipped C++ implementation and is trustworthy, but the precise
  historical equation *numbering/grouping* in a few spots (noted inline
  with LaTeX comments) is a best-effort reconstruction from position in
  the `antiword` dump plus narrative order, not a literal OCR of the
  original numbering. If exact fidelity to the original numbering matters
  (e.g. for citing this thesis by equation number), verify against the
  original `.doc`/printed thesis.
- **Tabel 2-1 (space-frame member stiffness matrix)**: reconstructed from
  the cited Weaver & Gere (1980) reference, not pulled from the (lost)
  original image. Verify before using it as an engineering reference.
- **Figures**: not recoverable at all; placeholders only.
- **Typesetting**: a handful of long inline formulas/identifiers (e.g. in
  BAB II's stiffness-method derivation, BAB III's constraint list) produce
  minor "Overfull \hbox" warnings — text or math running up to roughly an
  inch past the margin in a few spots. The worst offenders (a >2.7in
  overflow in the beam/column cost formulas, and the Tabel 2-1 matrix) were
  fixed with `\footnotesize`/`landscape`/reflowed line breaks; the rest are
  cosmetic and don't affect content correctness. Re-check `main.log` after
  any edit that adds a long unbroken formula or identifier.

## Compiling

`main.tex` uses `report` class with standard packages only (`amsmath`,
`amssymb`, `graphicx`, `geometry`, `setspace`, `longtable`, `booktabs`,
`enumitem`, `titlesec`, `fancyhdr`, `hyperref`) — no `babel`/`polyglossia`
Indonesian language pack dependency (Indonesian caption words are set
directly via `\renewcommand`). Compile with `pdflatex` (run it twice for
the table of contents to resolve), e.g.:

```
pdflatex main.tex
pdflatex main.tex
```

**Verified:** compiled end-to-end with Tectonic 0.17.0 (a self-contained
XeTeX-based engine) — `main.pdf`, 73 pages, no errors, only cosmetic
overfull-\verb|\hbox| warnings from a few long unbreakable words/wide
tables. `main.pdf` in this directory is that compiled output, checked in
so it can be read without a local LaTeX install; recompile with the
commands above if you edit the source.
