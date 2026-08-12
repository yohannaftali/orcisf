[🏠 Home](../README.md#algorithms)

# A. Loads

How ORCISF reads load input from the user, computes self-weight, and
assembles everything into the global load vector consumed by the
structural-analysis stage ([B. Structural Analysis](structural-analysis.md)).

Every formula below is implemented twice in this repository: the original
1998–1999 Borland C++ source (`Optimasi Beton/Source/Pembebanan.hpp`), and a
line-by-line modern C++ port (`src/engine/`) that keeps the same variable
names and 1-based array indexing on purpose, so the two can be checked
against each other directly. Citations below point at both.

---

## 1. What a user actually enters

The legacy format supports exactly **two** kinds of load, both stored in the
`.bbn` file:

1. **A uniform distributed load on a member** (`W`, N/m, transverse to the
   member, gravity-sense positive) — e.g. a beam carrying a slab or wall.
2. **A 6-DOF generalized action at a joint** (`AJ`, "arah 1..6" = Fx, Fy, Fz,
   Mx, My, Mz in the global axes) — this single mechanism covers point loads,
   applied moments, and lateral/wind loads. There is no separate "point load"
   or "wind load" type in the format; a wind load is just a joint action with
   the horizontal components set.

Console entry: `load_data()`
([`Pembebanan.hpp:10`](../Optimasi%20Beton/Source/Pembebanan.hpp#L10)).
Modern GUI entry: `EditableStructure::SetMemberLoad()` /
`SetJointLoad()` ([`EditableStructure.cpp`](../src/gui/editor/EditableStructure.cpp)),
wired to the Loads menu's click-to-place modes and the Loads panel's editable
table (see `AGENTS.md`'s `gui/editor/` notes on issue #7).

### Member load → fixed-end forces

A uniform load isn't stored as a single number — it's immediately converted
to the member's 12 local fixed-end force/moment components (`AML[1..12]`),
the standard beam fixed-end-force formulas for a prismatic member under a
uniform transverse load `w` over length `L`:

```
AML[2]  =  w·L/2        (shear, end 1)
AML[6]  =  w·L²/12       (moment, end 1)
AML[8]  =  w·L/2        (shear, end 2)
AML[12] = −w·L²/12       (moment, end 2)
```

(`AML[1]`, `[3]`, `[4]`, `[5]`, `[7]`, `[9]`, `[10]`, `[11]` — axial/torsion/
the other bending plane — are zero for this load type, since it's purely
transverse.)

Legacy: `load_data()`
([`Pembebanan.hpp:44-55`](../Optimasi%20Beton/Source/Pembebanan.hpp#L44-L55)).
Modern: `EditableStructure::SetMemberLoad()`'s `ComputeUdlFixedEndForces()`
helper ([`EditableStructure.cpp`](../src/gui/editor/EditableStructure.cpp))
— re-derives these 12 components from the member's *current* length, so
editing a joint position after a load is set keeps the fixed-end forces
correct.

---

## 2. Self-weight

Self-weight is **never stored in the `.bbn` file** — it's recomputed fresh
from the member's cross-section every time the structure is actually
analyzed, by `berat_sendiri()`
([`Pembebanan.hpp:277-321`](../Optimasi%20Beton/Source/Pembebanan.hpp#L277-L321))
/ `BeratSendiri()`
([`StructuralAnalysis.cpp:283`](../src/engine/src/StructuralAnalysis.cpp#L283)).

For each member, the same "kemiringan" (inclination) check used everywhere
else in this codebase (`periksa_batang()`/`PeriksaBatang()` — see
[B. Structural Analysis §2](structural-analysis.md#2-member-inclination-kemiringan))
decides whether it's treated as a beam or a column:

- **Beam** (`CXZ > 0.001`, i.e. not vertical): unit weight of reinforced
  concrete (**24000 N/m³**) times the current trial cross-section's width
  and height gives a distributed load in N/m:

  ```
  W_Balok = 24000 · b · h        (N/m, b/h in meters)
  ```

  This is *added* to the member's `W` (so a beam's self-weight and any
  user-entered distributed load are combined into one value), and the same
  fixed-end-force formulas from §1 are re-applied on top of whatever the
  user's load already contributed to `AML[2/6/8/12]`.

- **Column** (vertical): self-weight becomes a single downward *point*
  force, applied directly as a joint action at the column's bottom joint
  (`JJ`) rather than a distributed member load — a column doesn't bend
  under its own axial weight the way a beam sags under a transverse one:

  ```
  P_Kolom = −24000 · b · h · L        (N, negative = downward in the
                                        legacy sign convention)
  AJ[6·JJ − 4] += P_Kolom              (arah 2 = global Y, this project's
                                        vertical axis)
  ```

**Never double-count this.** Because self-weight is computed fresh on every
real analysis/optimization run, the raw `.bbn` file must never contain it.
This port's GUI editor is deliberately careful about this split — see
`AGENTS.md`'s note on `ReadLoads()` (real runs, always re-derives
self-weight via `BeratSendiri()`) vs. `ReadLoadsRaw()`/`WriteLoads()` (the
GUI's load editor, strictly self-weight-free, so re-saving never bakes
self-weight into the file where it would get added a second time on the
next real run).

---

## 3. Assembling the global load vector

Once every member has its 12 local `AML` components (user load + self-weight)
and every joint has its 6 `AJ` components, `beban()`
([`Pembebanan.hpp:208-272`](../Optimasi%20Beton/Source/Pembebanan.hpp#L208-L272))
/ `Beban()`
([`StructuralAnalysis.cpp:244`](../src/engine/src/StructuralAnalysis.cpp#L244))
combines them into the single global combined-load vector `AC` that the
stiffness-method solve in
[B. Structural Analysis](structural-analysis.md) actually uses:

1. **Rotate each member's local fixed-end forces into global axes** and
   negate them (a fixed-end force *resists* the applied load, so its
   *equivalent joint load* contribution is the opposite sign), using the
   same `R11..R33` member rotation matrix built by
   [`kekakuan_batang()`/`KekakuanBatang()`](structural-analysis.md#3-stiffness-matrix-assembly):
   accumulated per member into `AE` (equivalent joint loads).
2. **Combine** at every global DOF: `AC[ID[i]] = AJ[i] + AE[i]` — the
   user/self-weight joint actions (`AJ`) plus the rotated-and-summed member
   equivalent loads (`AE`), remapped through the free-DOF equation numbering
   `ID[]` that [B. Structural Analysis §4](structural-analysis.md#4-solving-the-system-modified-choleski)
   also uses.

This `AC` vector is exactly the right-hand side the modified-Choleski solve
factors and solves against.

---

[🏠 Home](../README.md#algorithms)
