[🏠 Home](../README.md#algorithms)

# B. Structural Analysis

How ORCISF analyzes the 3D space frame: cross-section properties, member
inclination, global stiffness matrix assembly, the modified-Choleski solve,
and how results (displacements, member forces, support reactions) are
recovered. Developed from Weaver & Gere's direct-stiffness method for 3D
space frames.

Source: `Optimasi Beton/Source/Struktur.hpp` and `Solver.hpp` (original,
June 1999), ported line-by-line to `src/engine/src/StructuralAnalysis.cpp`
and `StiffnessSolver.cpp`. See [A. Loads](loads.md) for how the load vector
`AC` used in §4 gets built.

---

## 1. Cross-section properties (`Inersia`)

Before anything else, every member's cross-section dimensions are resolved
from the current trial design (the discrete `b`/`h` tables — see
[E. Optimization §1](optimization.md#1-discrete-design-variables)) and its
area/inertia properties computed:

```
AX = b · h                     (cross-section area, m²)
YI = h·b³/12                   (bending moment of inertia, local y-axis)
ZI = b·h³/12                   (bending moment of inertia, local z-axis)
```

`XI`, the **torsional constant**, is different — it needs an approximation,
since there's no simple closed form for a solid rectangle's torsional
stiffness (the exact St. Venant solution is an infinite series). This
codebase uses the two-branch Roark's/Saint-Venant approximation, valid to
within ~4% at any aspect ratio, branching on which side is longer:

```
if b ≤ h:
   XI = h·b³ · [ 1/3 − 0.21·(b/h)·(1 − b⁴/(12h⁴)) ]

else (b > h — b is the long side):
   a = b/2, c = h/2            (half-dimensions; Roark's own a/b notation
                                 requires a ≥ c)
   XI = a·c³ · [ 16/3 − 3.36·(c/a)·(1 − c⁴/(12a⁴)) ]
```

(Roark's Formulas for Stress & Strain, Warren C. Young, 1989, p.348.) Both
branches compute the *same physical quantity* — algebraically substituting
`a = b/2`, `c = h/2` into the second form reduces it to the first form with
the labels swapped, so which branch runs never changes the answer, only
which side happens to be longer.

Legacy: `inersia()`
([`Struktur.hpp:23-90`](../Optimasi%20Beton/Source/Struktur.hpp#L23-L90)).
Modern: `Inersia()`
([`StructuralAnalysis.cpp:99`](../src/engine/src/StructuralAnalysis.cpp#L99)).

---

## 2. Member inclination ("kemiringan")

Every member's orientation is characterized by its **direction cosines** —
how much of its length lies along each global axis:

```
L  = √[ (Xₖ−Xⱼ)² + (Yₖ−Yⱼ)² + (Zₖ−Zⱼ)² ]     (member length)
CX = (Xₖ−Xⱼ)/L,   Cy = (Yₖ−Yⱼ)/L,   CZ = (Zₖ−Zⱼ)/L
CXZ = |√(CX² + CZ²)|                            (horizontal-plane projection)
```

`CXZ` is the single value this entire codebase branches on to decide
**beam vs. column**: `CXZ > 0.001` means the member has a meaningful
horizontal projection (a beam); `CXZ ≤ 0.001` means it's vertical (a
column). This exact check governs cross-section lookup (§1), self-weight
direction ([A. Loads §2](loads.md#2-self-weight)), the rotation matrix
below, and the beam/column split feeding
[E. Optimization](optimization.md#1-discrete-design-variables)'s design
variable count.

Legacy: `periksa_batang()`
([`Struktur.hpp:458-468`](../Optimasi%20Beton/Source/Struktur.hpp#L458-L468)).
Modern: `PeriksaBatang()`
([`StructuralAnalysis.cpp:10`](../src/engine/src/StructuralAnalysis.cpp#L10)).

---

## 3. Stiffness matrix assembly

### 3a. Local stiffness + rotation to global axes

Each member's 12×12 local stiffness matrix (`SM`) is built from 8 constants
combining axial (`E·AX`), torsional (`G·XI`), and both bending stiffnesses
(`E·YI`, `E·ZI`) over its length — the standard 3D beam-column stiffness
formulation. It's then rotated into global coordinates via the 3×3 rotation
matrix `R11..R33` derived from the direction cosines in §2 (with a
gimbal-lock-safe special case for exactly-vertical members, since the usual
formula divides by `CXZ`).

Legacy: `isi_matrik_kekakuan()`
([`Struktur.hpp:486-557`](../Optimasi%20Beton/Source/Struktur.hpp#L486-L557)),
the rotation-matrix half of `kekakuan_batang()`
([`Struktur.hpp:110-166`](../Optimasi%20Beton/Source/Struktur.hpp#L110-L166)).
Modern: `IsiMatrikKekakuan()` and the rotation-matrix section of
`KekakuanBatang()`
([`StructuralAnalysis.cpp:35`](../src/engine/src/StructuralAnalysis.cpp#L35),
[`:135`](../src/engine/src/StructuralAnalysis.cpp#L135)).

### 3b. Free-DOF numbering and banded superposition

Every joint has 6 global DOF (3 translation + 3 rotation); a restrained DOF
(`JRL[i]==1`, from the joint's support condition) is excluded from the
system actually being solved. `ID[]` maps every global DOF number to its
position in the reduced, free-DOF-only equation numbering — restrained DOFs
get pushed to the *end* of the numbering, so the solved system only ever
covers the first `N` (free DOF count) equations.

Each member's rotated 12×12 stiffness contributes into the *global*
`N × NB` banded matrix `SFF` (only the upper band is stored — `NB`, the
bandwidth, is derived from the largest joint-number gap `|JK−JJ|` across all
members, since a smaller bandwidth means a smaller, faster-to-factor
matrix). This is the classic direct-stiffness superposition step: each
member's local DOF contributes to the global equations its two joints'
free DOFs correspond to, accumulated additively (multiple members sharing a
joint all add into the same rows/columns).

Legacy: `kekakuan_batang()`
([`Struktur.hpp:95-261`](../Optimasi%20Beton/Source/Struktur.hpp#L95-L261)).
Modern: `KekakuanBatang()`
([`StructuralAnalysis.cpp:135`](../src/engine/src/StructuralAnalysis.cpp#L135)).

---

## 4. Solving the system (modified Choleski)

The assembled system `SFF · DF = AC` is symmetric, positive-definite (a
physically stable structure's stiffness matrix always is), and banded
(most entries are zero, since a member can only connect two joints, not
all of them) — exactly the conditions a **banded Choleski factorization**
is designed for. This codebase uses the *modified* variant (`LDLᵀ`, not
`LLᵀ`) specifically so it never needs a square root inside the inner loop.

- **`banfac(N, NB, SFF)`** factors `SFF` in place, band-by-band. If it ever
  hits a non-positive pivot, the structure is unstable (a genuine geometric
  or support-configuration problem, not a numerical fluke) — the legacy
  console program prints an error and exits; this port raises
  `UnstableStructureError` instead.
- **`bansol(N, NB, SFF, AC, DF)`** does the forward-then-backward
  substitution sweep against the factored matrix to solve for the free
  joint displacement vector `DF`.

Legacy: `banfac()`/`bansol()`
([`Solver.hpp:12-130`](../Optimasi%20Beton/Source/Solver.hpp#L12-L130)).
Modern: `BandedFactorize()`/`BandedSolve()`
([`StiffnessSolver.cpp`](../src/engine/src/StiffnessSolver.cpp),
declared in
[`StiffnessSolver.h`](../src/engine/include/engine/StiffnessSolver.h)).

---

## 5. Recovering results

`hasil()`/`Hasil()` turns the raw free-DOF displacement vector `DF` back
into everything downstream code actually needs:

1. **Full displacement vector** `DJ` (all `ND` global DOFs, restrained ones
   forced to zero, free ones pulled from `DF` via the same `ID[]` numbering
   from §3b).
2. **Member-end forces** `AM = AML + AMD` — the fixed-end forces from
   [A. Loads §1](loads.md#member-load--fixed-end-forces) plus the forces
   induced by the now-known joint displacements (`AMD = SMRT · DJ`, the
   local stiffness matrix applied to the member's own 12 displacement
   components).
3. **Per-region beam forces** (feeding [C. Beam](beam.md)): midspan moment
   `MLAP` (from end moments plus the simple-beam mid-span moment
   `w·L²/8`), support moments `MTUM_KI`/`MTUM_KA`, shears `GESER_KI`/
   `GESER_KA`.
4. **Per-member column forces** (feeding [D. Column](column.md)): axial
   `PK`, biaxial moments `MKX`/`MKY`, shear `GK` (the larger of the two end
   shears).
5. **Support reactions** `AR` — accumulated the same way member stiffness
   was assembled in §3b, but only at restrained DOFs, then adjusted by
   subtracting the original applied joint/equivalent loads
   (`AR -= AJ + AE`) so the final value is a true reaction, not a raw
   stiffness-times-displacement product.

Legacy: `hasil()`
([`Struktur.hpp:266-453`](../Optimasi%20Beton/Source/Struktur.hpp#L266-L453)).
Modern: `Hasil()`
([`StructuralAnalysis.cpp:309`](../src/engine/src/StructuralAnalysis.cpp#L309)).

---

## 6. The driver

`struktur()`/`Struktur()` ties §3–5 together in five calls:

```
KekakuanBatang(sd);              // §3: assemble SFF
BandedFactorize(N, NB, SFF);     // §4a: factor
Beban(sd);                       // A.§3: assemble AC
BandedSolve(N, NB, SFF, AC, DF); // §4b: solve
Hasil(sd);                       // §5: recover results
```

Legacy: [`Struktur.hpp:11-18`](../Optimasi%20Beton/Source/Struktur.hpp#L11-L18).
Modern: `Struktur()`
([`StructuralAnalysis.cpp:388`](../src/engine/src/StructuralAnalysis.cpp#L388)).
This whole sequence runs once per candidate design during optimization —
see [E. Optimization](optimization.md) for how often that actually is.

---

[🏠 Home](../README.md#algorithms)
