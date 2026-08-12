[🏠 Home](../README.md#algorithms)

# C. Beam

How ORCISF designs a reinforced-concrete beam: flexural (bending) capacity
at midspan and support, shear/stirrup requirements, and the deflection
check. Based on **SK SNI T-15-1991-03** (the Indonesian concrete design
code current at the time this was written, 1998–1999).

Source: `Optimasi Beton/Source/Balok.hpp` (class `balok`, original, June
1999), ported to `src/engine/src/BeamDesign.cpp`. Forces used here
(`MLAP`, `MTUM_KI`/`MTUM_KA`, `GESER_KI`/`GESER_KA`) come from
[B. Structural Analysis §5](structural-analysis.md#5-recovering-results).

---

## 1. Two independent regions: lapangan vs. tumpuan

A beam is designed **twice** — once for its midspan ("lapangan") region,
once for its support ("tumpuan") region — because the two see opposite
bending senses: midspan typically sags (positive/sagging moment `MLAP`,
tension at the bottom), while the support typically hogs (negative moment
`MTUM_KI`/`MTUM_KA`, tension at the top). Each region gets its own
independent reinforcement (`DIA1`/`NL1` tension, `DIA2`/`NL2` compression),
and this codebase deliberately tracks two full sets of design variables per
beam for exactly this reason (see
[E. Optimization §1](optimization.md#1-discrete-design-variables)'s
`12 = 2 regions × (B,H shared) + ...` variable count).

The `balok`/`DesignBeam()` constructor is invoked once per region, with the
caller (`Kendala_Harga()`/`KendalaHarga()`) swapping which moment (`MU`) and
which reinforcement set (`DIA1`/`NL1`/`DIA2`/`NL2`) is active before each
call — see [E. Optimization §2](optimization.md#2-the-fitness-function).

---

## 2. Flexural (bending) design

Uses the **Whitney equivalent rectangular stress block** — standard
strength-design flexural theory:

1. **Stress-block depth factor** `β1` (SNI T-15-1991-03 §3.3.2(7)(3)):

   ```
   β1 = 0.85                    if f'c ≤ 30 MPa
   β1 = 0.85 − 0.008·(f'c − 30)  if f'c > 30 MPa   (floor: β1 ≥ 0.65)
   ```

2. **Trial neutral-axis depth** `a` (distance from the compression face to
   the neutral axis), from force equilibrium between tension and
   compression steel:

   ```
   a = (Aₛ − Aₛ′)·fy / (0.85·f'c·b)
   ```

3. **Strain check**: if either the tension or compression steel isn't yet
   at yield strain (`ε < εy = fy/200000`), the code re-solves `a` via the
   quadratic form of strain compatibility instead of the simple force
   formula above — this handles beams where the assumption "both layers of
   steel have yielded" doesn't hold.
4. **Nominal moment capacity**:

   ```
   FMU = θ · [ 0.85·f'c·a·b·(d − a/2) + Aₛ′·fs′·(d − ds) ]
   ```

   (`θ` = strength-reduction factor, `d` = effective depth, `ds` = cover to
   compression steel centroid.)
5. **Constraint**: `kendala_M = MU/FMU − 1` (clamped to ≥0) — nonzero
   whenever the demand moment `MU` (from structural analysis) exceeds the
   section's capacity `FMU`.

Also checked in the same pass: **reinforcement ratio** `ρ = Aₛ/(b·d)`
against both a minimum (`ρmin = 1.4/fy`, ensures a minimally-ductile
section) and a balanced-ratio-derived maximum (`ρb`, ensures the section
fails in a ductile, steel-yielding mode rather than a brittle concrete-crushing
one) — two more constraint terms (`kendala_rho_m`, `kendala_rho_b`) summed
into the same total.

Legacy: `balok::analisa()`
([`Balok.hpp:96-243`](../Optimasi%20Beton/Source/Balok.hpp#L96-L243)).
Modern: `Analisa()`
([`BeamDesign.cpp:10`](../src/engine/src/BeamDesign.cpp#L10)).

---

## 3. Shear design (stirrups)

```
Vc = (1/6)·√f'c · b · d              (concrete's own shear capacity, N)
Vs = Vu/θ − Vc                       (shear steel must resist the rest)
```

The maximum allowed stirrup spacing is the *smallest* of four SNI-mandated
limits (tighter limits kick in when `Vs` exceeds `(1/3)√f'c·b·d`, i.e. when
the section is relying heavily on stirrups):

```
Smax = min( 3·Av·fyt/b,  Av·fyt·d/Vs,  d/2 (or d/4),  600mm (or 300mm) )
```

**Constraint**: `kendala_sb = (actual spacing)/Smax − 1` (clamped to ≥0).

Legacy: `balok::sengkang_balok()`
([`Balok.hpp:248-300`](../Optimasi%20Beton/Source/Balok.hpp#L248-L300)).
Modern: `SengkangBalok()`
([`BeamDesign.cpp:98`](../src/engine/src/BeamDesign.cpp#L98)).

---

## 4. Deflection check

Per SK SNI T-15-1991-03 §3.2.5, using the cracked-transformed-section
method (Reinforced Concrete Mechanics and Design, p.355):

1. **Neutral axis of the cracked transformed section** (`LGN`) and the
   **cracked moment of inertia** `Icr`, vs. the **gross moment of inertia**
   `Ig = b·h³/12`.
2. **Cracking moment** `Mcr = fr·Ig/(0.5h)`, where `fr = 0.7√f'c` is the
   concrete's modulus of rupture.
3. **Effective moment of inertia** (Branson's equation), interpolating
   between `Ig` and `Icr` based on how far the actual moment `MLAP` exceeds
   `Mcr`:

   ```
   Ie = (Mcr/MLAP)³·Ig + [1 − (Mcr/MLAP)³]·Icr
   ```

4. **Instantaneous deflection** (simple-beam midspan formula):
   `δ = 5·L²·|MLAP| / (48·E·Ie)`.
5. **Long-term deflection multiplier** `λ = 2/(1 + 50·Aₛ′/(b·h))` (SNI
   §3.2.5(2.5), accounts for creep/shrinkage over time depending on how
   much compression steel is present) — final deflection is
   `(1+λ)·δ`.
6. **Constraint**: allowable deflection is `L/180`;
   `kendala_lendutan = δ/δ_allow − 1` (clamped to ≥0).

Legacy: `lendutan()`
([`Balok.hpp:306-378`](../Optimasi%20Beton/Source/Balok.hpp#L306-L378)).
Modern: `Lendutan()`
([`BeamDesign.cpp:155`](../src/engine/src/BeamDesign.cpp#L155)).

---

## 5. Cost

Alongside every constraint, the constructor also computes this trial
design's material cost: concrete volume (`0.5·B·H·L`, the 0.5 reflecting
that a beam element spans only half into each adjoining column — see the
legacy source for the exact tributary convention), longitudinal steel
weight, and stirrup steel weight, each priced by the user-supplied unit
prices. This `harga` value feeds directly into
[E. Optimization §2](optimization.md#2-the-fitness-function)'s fitness
function alongside the constraint total from §§2–4 above.

---

[🏠 Home](../README.md#algorithms)
