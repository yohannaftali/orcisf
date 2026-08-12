[🏠 Home](../README.md#algorithms)

# E. Optimization

How ORCISF searches for a minimum-cost, constraint-satisfying design using
the **"Flexible Polyhedron"** method — a generalized-simplex, derivative-free
direct-search optimizer in the Nelder–Mead family.

Source: `Optimasi Beton/Source/Polyhedron.hpp` (driver, original, June
1999) plus its companion files (`Penormalan.hpp`, `Pengacakan.hpp`,
`Diskritisasi.hpp`, `Kendala.hpp`, `Telusur.hpp`, `Baru.hpp`,
`Pengurutan.hpp`), ported to `src/engine/src/Optimizer.cpp` and
`CostConstraint.cpp`. Every design this method evaluates is scored using
[C. Beam](beam.md) and [D. Column](column.md), each of which needs a fresh
run of [B. Structural Analysis](structural-analysis.md).

---

## 1. Discrete design variables

Cross-sections and reinforcement aren't continuous — the user supplies
**discrete candidate tables** per structure (widths/heights/column sides,
bar diameters, bar counts, stirrup spacings — the `.isd`/`.idl`/`.ijl`/
`.ids`/`.ijs` files). A design variable never stores an actual physical
value; it stores an **index into one of these tables**, resolved via
`isi()`/`Isi()`. This is why the optimizer's internal state looks like it's
manipulating small integers, not millimeters or reinforcement ratios.

Every member is first classified beam or column by
[B. Structural Analysis §2](structural-analysis.md#2-member-inclination-kemiringan)'s
`CXZ` check, giving:

```
JVD = 12·jum_balok + 5·jum_kolom       (total design-variable count)
```

- **12 per beam**: width, height, then 4 diameter/count pairs (tension +
  compression steel, at *both* the [midspan and support regions](beam.md#1-two-independent-regions-lapangan-vs-tumpuan)),
  then stirrup diameter and spacing.
- **5 per column**: side length, bar diameter, bar count per side, stirrup
  diameter, stirrup spacing.

The population size is `JSTD = JVD·fak_kali + fak_plus` (both user-tunable
factors) — a generalized simplex needs more vertices than dimensions to
have a meaningful "search direction," unlike the classic Nelder-Mead
simplex, which uses exactly `n+1` vertices for an `n`-dimensional problem.

Legacy: `optimasi()`
([`Polyhedron.hpp:10-61`](../Optimasi%20Beton/Source/Polyhedron.hpp#L10-L61)).
Modern: `PrepareOptimization()`
([`Optimizer.cpp:749`](../src/engine/src/Optimizer.cpp#L749)).
Rounding a continuous search value back to a valid discrete index is
`konversi()`/`Konversi()`
([`Diskritisasi.hpp`](../Optimasi%20Beton/Source/Diskritisasi.hpp),
[`Optimizer.cpp`](../src/engine/src/Optimizer.cpp)) — ordinary
round-to-nearest.

---

## 2. The fitness function

A single scalar combining cost and constraint violation:

```
fitness = finalti / (harga + finalti · kendala)
```

`harga` is the total material cost (concrete + steel, summed over every
beam region and column — see [C. Beam §5](beam.md#5-cost) /
[D. Column §5](column.md#5-cost)). `kendala` is the total constraint
violation (every individual `kendala_*` term from beam flexure/shear/
deflection and column ratio/slenderness/interaction/shear is ≥0 by
construction — see those pages — and they're all summed together here).
`finalti` is a user-supplied penalty factor.

Why this particular form, not simply `1/harga` with a separate feasibility
filter: a design with `kendala = 0` (fully feasible) scores
`fitness = finalti/harga` — pure cost-ranking among feasible designs. A
design with any violation gets that violation *added to the cost*,
scaled by `finalti`, so a large enough penalty factor makes even a
slightly-infeasible design score far worse than any feasible one, while
still giving the search a smooth gradient to follow *toward* feasibility
instead of a hard cliff.

Evaluating this fitness for one candidate design means a full structural
re-analysis: read the trial design's variables, run
[Struktur()](structural-analysis.md#6-the-driver), then loop every beam
(both regions) and column, accumulating cost and constraint.

Legacy: `Kendala_Harga()`
([`Kendala.hpp:6-204`](../Optimasi%20Beton/Source/Kendala.hpp#L6-L204)).
Modern: `KendalaHarga()`
([`CostConstraint.cpp:11`](../src/engine/src/CostConstraint.cpp#L11)).

---

## 3. The Flexible Polyhedron driver loop

### 3a. Initial population

`JSTD` candidate designs ("struktur 0..JSTD-1") are generated: slot 1 is a
deliberately extreme all-maximum-index reference design
(`cari_struktur_awal()`/`CariStrukturAwal()`), every other slot is
uniform-random within each variable's discrete range
(`randomisasi()`/`Randomisasi()`). Each is evaluated (§2) and the whole
population sorted by fitness — **ascending**, so index `0` is the current
*worst* design and index `JSTD-1` is the current *best*
(`sort()`/`Sort()`, an unmodified bubble sort).

### 3b. Search direction (`penelusuran`/`Penelusuran`)

Classic Nelder-Mead centroid step, generalized to this codebase's discrete,
two-part (beam+column) variable layout: compute the centroid `M` of every
point *except the worst* (indices `1..JSTD-1`), then the search direction
is `XS = M − worst_point`. The **coordinate with the largest
`|XS|`** is picked as the "farthest" direction to move first.

### 3c. New-point search (`cari_baru`/`CariBaru`)

Not a single reflect/expand step like classic Nelder-Mead — a **discrete
walk along the search direction**: starting from the worst point, step
along `XS` (each coordinate scaled proportionally to how far it needs to
move relative to the farthest coordinate), rounding to valid discrete
indices at each step, evaluating fitness (§2) at every step, and keeping
whichever step scored best. The walk runs for up to
`3 × |farthest coordinate's XS|` steps — proportional to how far the
search direction actually points, so a small move takes fewer trial
evaluations than a large one.

### 3d. Replace or shrink

If the best point found in §3c beats the *current worst* design's fitness,
it **replaces** the worst point (`ganti_baru()`/`GantiBaru()`) — this is
this method's role of both "reflect" and "expand" combined into one
adaptive step, rather than classic Nelder-Mead's separate reflect/expand/
contract cases. If nothing better was found, the whole polyhedron
**shrinks**: every point except the best moves halfway toward the best
(`penyusutan()`/`Penyusutan()`) — the direct analog of Nelder-Mead's
shrink step.

### 3e. Convergence

The loop stops when either:
- the best fitness stops improving between two successive generations
  (`fitstr[JSTD-1] == fitstr[JSTD-JVD-1]`, comparing against a point
  `JVD` generations back rather than just the immediately prior one — a
  short plateau isn't enough to call it converged), or
- too many consecutive shrinks happen without the recorded best fitness
  changing (an acceleration/stall-detection check), or
- the generation count hits the user-configured `j_iterasi_mak` (the
  legacy manual recommends **10,000+** for good results).

Legacy driver: `optimasi()`
([`Polyhedron.hpp:260-437`](../Optimasi%20Beton/Source/Polyhedron.hpp#L260-L437)).
Modern: `RunOptimization()`
([`Optimizer.cpp:776`](../src/engine/src/Optimizer.cpp#L776)). Search
direction: `penelusuran()`/`Penelusuran()`
([`Telusur.hpp`](../Optimasi%20Beton/Source/Telusur.hpp),
[`Optimizer.cpp:314`](../src/engine/src/Optimizer.cpp#L314)). New-point
search/replace/shrink: `cari_baru()`/`ganti_baru()`/`penyusutan()`
([`Baru.hpp`](../Optimasi%20Beton/Source/Baru.hpp)) /
`CariBaru()`/`GantiBaru()`/`Penyusutan()`
([`Optimizer.cpp:473`](../src/engine/src/Optimizer.cpp#L473),
[`:717`](../src/engine/src/Optimizer.cpp#L717),
[`:730`](../src/engine/src/Optimizer.cpp#L730)).

### 3f. A reactive special case: slenderness

If a trial column design violates the slenderness constraint
([D. Column §2](column.md#2-slenderness)), the driver doesn't just record
the violation and move on — it actively searches the column's own discrete
side-length table for the next larger size that would satisfy `KL/r ≤ 22`,
and re-evaluates with that size before accepting the candidate. This is a
targeted repair step layered on top of the general search, specific to this
one constraint.

---

## 4. Re-optimize from last best result

An optional mode (this port's issue #16) seeds the initial population's
worst slot with a *previous* run's best design instead of a random one —
since the search only ever replaces the worst point with something that
scored strictly better (§3d), seeding with an already-good design can only
ever match or improve on it, never make the final result worse. See
`AGENTS.md`'s issue #16 notes and
[`src/engine/README.md`](../src/engine/README.md)'s "Re-optimize from last
best" section for the full empirical validation.

---

## 5. Multi-threading

The modern port (issue #4) parallelizes the two hottest loops — initial
population evaluation (§3a) and the per-generation trial search (§3c) —
across worker threads, each with its own cloned structural-analysis
scratch state, using a fork-join pattern per hot loop rather than a
persistent thread pool. Determinism is preserved: the same RNG seed
produces bit-identical results regardless of worker-thread count, since
threading only changes *which trials run in parallel*, never the sequence
of random draws or which trial ultimately wins each comparison. Full
details (why a full `StructureData` copy per generation was tried and
rejected as ~3× slower, the exact synchronization strategy, and the
verification method) are in `AGENTS.md`'s issue #4 notes — not duplicated
here since that's already the authoritative, verified writeup.

---

[🏠 Home](../README.md#algorithms)
