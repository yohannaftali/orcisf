[🏠 Home](../README.md#algorithms)

# D. Column

How ORCISF designs a reinforced-concrete column: slenderness, biaxial
axial-moment (P-M) interaction capacity, and stirrup requirements. Based on
**SK SNI T-15-1991-03**, with the biaxial method from Hulse & Mosley,
*Reinforced Concrete Design by Computer* (1986), p.163.

Source: `Optimasi Beton/Source/Kolom.hpp` (class `kolom`, original, July
1999), ported to `src/engine/src/ColumnDesign.cpp`. Forces used here (`PK`,
`MKX`/`MKY`, `GK`) come from
[B. Structural Analysis §5](structural-analysis.md#5-recovering-results).
Every column in this codebase is modeled with a **square** cross-section
(`sisi × sisi`) and bars distributed evenly around all four faces.

---

## 1. Reinforcement ratio and bar spacing

```
Aₛ,total = (4·N_DIA − 4) · (π/4) · DIA²    (N_DIA bars per side; corners
                                              shared between adjacent sides,
                                              hence 4N−4 unique bars)
ρ = Aₛ,total / sisi²
```

**Constraint**: two clamped terms, summed — `ρ` must stay within
`[0.01, 0.08]` (SNI's minimum/maximum column reinforcement ratio).

Bar spacing along one face is also checked against the larger of `1.5·DIA`
or `40mm` (SNI's minimum clear spacing between longitudinal bars).

Legacy: `kolom::rho()` / `kolom::jarak_tulangan()`
([`Kolom.hpp:131-192`](../Optimasi%20Beton/Source/Kolom.hpp#L131-L192)).
Modern: `Rho()` / `JarakTulangan()`
([`ColumnDesign.cpp:62`](../src/engine/src/ColumnDesign.cpp#L62),
[`:79`](../src/engine/src/ColumnDesign.cpp#L79)).

---

## 2. Slenderness

```
KL/r = 0.5·L·1000 / (sisi·√(1/12))       (K = 0.5, an assumed effective-
                                            length factor for a braced frame)
```

**Constraint**: SNI requires `KL/r ≤ 22` for a column to be treated as
short (i.e. without a separate slenderness/second-order-moment
magnification analysis, which this codebase doesn't otherwise implement) —
`kendala_kelangsingan = (KL/r)/22 − 1`, clamped to ≥0. When this triggers,
the optimizer's driver loop (`optimasi()`/`RunOptimization()`) responds by
searching for a *larger* discrete side dimension rather than accepting the
violation — see
[E. Optimization](optimization.md#3-the-flexible-polyhedron-driver-loop)'s
notes on this reactive re-sizing step.

Legacy: `kolom::kelangsingan()`
([`Kolom.hpp:197-214`](../Optimasi%20Beton/Source/Kolom.hpp#L197-L214)).
Modern: `Kelangsingan()`
([`ColumnDesign.cpp:101`](../src/engine/src/ColumnDesign.cpp#L101)).

---

## 3. Biaxial axial-moment (P-M) interaction

A column rarely sees pure axial load or pure uniaxial bending — it sees
axial force *and* moment about both local axes simultaneously. Rather than
building the full 3D interaction surface, this codebase uses **Hulse &
Mosley's equivalent-uniaxial-moment method**: fold the *smaller* of the two
demand moments into the larger one via a shape factor, then solve a single
uniaxial P-M interaction problem.

1. **Shape factor** (empirical, symmetric reinforcement):

   ```
   β = 0.3 + (0.7/0.6)·(0.6 − Pu/(sisi²·f'c))       (floor: β ≥ 0.3)
   ```

2. **Equivalent uniaxial moment** — whichever of `MNX`/`MNY` is larger
   becomes the "primary" axis, with the smaller folded in:

   ```
   MOX = MNX + MNY·(1−β)/β      if MNX > MNY
   MOY = MNY + MNX·(1−β)/β      otherwise
   eks = |MOX or MOY| / PN        (equivalent eccentricity, mm)
   ```

3. **Solve for the neutral-axis depth `c`** that makes the section's actual
   capacity eccentricity match `eks`, via the **false-position (regula
   falsi) root-finding method** — not a closed form, since the trial
   capacity function (§3a below) involves summing forces over a discrete
   set of bar layers, which has no simple algebraic inverse. The search:
   - starts by bounding `c` between the balanced-condition depth
     `CB = 600·d/(fy+600)` and a coarse bracket (`[40mm, sisi−40mm]`, then
     widened if the sign doesn't bracket a root);
   - runs up to 100 iterations of false-position, converging when the
     trial function's residual falls within ±1mm (then a tighter ±10mm
     pass for a refined answer);
   - accepts the discrete-election of layer count `N_DIA`'s implied
     symmetric bar pattern as-is (`ASdi` = corner-row vs. interior-row
     steel area per layer) rather than an arbitrary continuous
     distribution.

### 3a. The trial capacity function (`hitung_kolom`/`HitungKolom`)

For a candidate neutral-axis depth `Ccoba`, sums the concrete compression
block plus every bar layer's contribution (tension or compression,
depending which side of the neutral axis it falls on, with the standard
strain-compatibility formula `fs = 600·(c−d_i)/c`, clamped to `±fy`, and a
`0.85·f'c` deduction for bars embedded within the compression block):

```
PNcoba = 0.85·β1·f'c·Ccoba·sisi + Σᵢ Fsᵢ
MNcoba = 0.85·β1·f'c·Ccoba·sisi·(sisi/2 − β1·Ccoba/2) + Σᵢ Fsᵢ·(sisi/2 − dᵢ)
```

returning `eks − MNcoba/PNcoba` — the false-position search in §3 drives
this residual to zero.

Legacy: `kolom::analisa()` / `kolom::hitung_kolom()`
([`Kolom.hpp:219-651`](../Optimasi%20Beton/Source/Kolom.hpp#L219-L651)).
Modern: `Analisa()` / `HitungKolom()`
([`ColumnDesign.cpp:112`](../src/engine/src/ColumnDesign.cpp#L112),
[`:15`](../src/engine/src/ColumnDesign.cpp#L15)).

**Constraints**: `kendala_pn = PN/PNcoba − 1` and
`kendala_mn = (MOX or MOY)/MNcoba − 1` (both clamped to ≥0) — nonzero
whenever the demand axial force or equivalent moment exceeds what the
converged neutral-axis solution can actually carry. A separate `kendala_po`
guards the minimum-eccentricity axial capacity limit `PO` before the
iterative search even runs.

---

## 4. Shear design (stirrups)

Simpler than the beam case (§3 of [C. Beam](beam.md)) — the maximum stirrup
spacing is just the smallest of three fixed SNI limits:

```
Smax = min( sisi,  16·DIA,  48·DIAS )
```

**Constraint**: `kendala_sengkang = (actual spacing)/Smax − 1`, clamped to
≥0.

Legacy: `kolom::sengkang_kolom()`
([`Kolom.hpp:656-688`](../Optimasi%20Beton/Source/Kolom.hpp#L656-L688)).
Modern: `SengkangKolom()`
([`ColumnDesign.cpp:266`](../src/engine/src/ColumnDesign.cpp#L266)).

---

## 5. Cost

Concrete volume (`sisi²·L`), longitudinal steel weight (`Aₛ,total·L`), and
stirrup steel weight, each priced by the user-supplied unit prices — same
role as [C. Beam §5](beam.md#5-cost), feeding
[E. Optimization §2](optimization.md#2-the-fitness-function)'s fitness
function.

---

[🏠 Home](../README.md#algorithms)
