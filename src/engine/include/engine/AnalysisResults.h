#pragma once

#include <vector>

#include "engine/StructureData.h"

// New in this port (issue #59, part of epic #58 -- FE results
// visualization): exposes the same raw member end forces (AM), joint
// displacements (DJ), and support reactions (AR) `Hasil()` already
// computes, captured into memory for the GUI instead of only ever being
// written to the `.str` text file (LegacyIO's WriteStrukturSection()).
namespace orcisf::engine {

// Local end forces for one member, Weaver & Gere space-frame convention:
// at each end, the 6 components are axial, shear-y, shear-z, torsion
// (about local x), moment-y, moment-z -- read directly from
// sd.AM[no_batang][1..12] (index 1-6 = end A/"start"/JJ, 7-12 = end
// B/"end"/JK), the same field Hasil() already populates and
// WriteStrukturSection() already writes to the .str file. Axial/shear in
// N, torsion/moment in **N*m** (not Nmm -- see the correction note
// below; this contradicts MemberResult's own "(N, Nmm)" comment for the
// same AM-derived quantities, which is itself mislabeled -- tracked as
// a documentation-only issue against that pre-existing, already-shipped
// file rather than fixed here, see the note).
//
// **Unit correction (issue #61's actual root cause, found 2026-08-17):**
// every field derived from `sd.AM`/`AR`/`AJ`/`AE` that represents a
// moment or torsion is in **N*m** (newton-meters), the same consistent
// SI unit family (N, m) the whole analysis engine uses throughout
// (`E`/`G` in Pa, `X`/`Y`/`Z`/`EL` in meters, forces in N) -- *not*
// Nmm as a pre-existing comment on `MemberResult` (issue #5) claimed
// and this file's own comments copied without independently verifying.
// Confirmed two ways: (1) `BeratSendiri()`'s self-weight fixed-end
// moment (`sd.AML[6][i] += W_Balok*EL^2/12`) uses `W` in N/m and `EL` in
// m with no unit-conversion factor anywhere, giving a result that can
// only be dimensionally consistent as N*m; (2) the legacy `MLAP`
// formula (`Hasil()`: `MLAP = -AM[6] + 0.125*W*EL^2`, W in N/m, EL in
// m) only reproduces its own real, checked-in output value when
// `-AM[6]` is treated as N*m -- treating it as Nmm (as this file
// originally assumed) makes the two terms mismatch by a factor of
// ~1000-1600x, which is exactly the bug `gui::ComputeForceDiagram()`
// had. The legacy `.opt` file's own "Nmm" print label
// (`WriteFinalResults()`: `"Momen kiri : " << MTUM_KI/0.8f << " Nmm"`)
// is therefore *also* mislabeled -- a pre-existing issue predating this
// epic, filed separately (see AGENTS.md's #61 note) rather than touched
// here, since the underlying *numbers* in that already-shipped,
// already-validated code are self-consistent within their own formulas
// regardless of the print label's wording; only a new consumer doing
// independent dimensional analysis (this force-diagram code) was
// actually affected.
struct MemberForces {
    int no_batang = 0; // 1-based, matches sd.JJ[no_batang]/JK[no_batang]

    float axial_a = 0.f, shear_y_a = 0.f, shear_z_a = 0.f;
    float torsion_a = 0.f, moment_y_a = 0.f, moment_z_a = 0.f;

    float axial_b = 0.f, shear_y_b = 0.f, shear_z_b = 0.f;
    float torsion_b = 0.f, moment_y_b = 0.f, moment_z_b = 0.f;

    // Issue #61: this member's total distributed transverse load (N/m,
    // self-weight-inclusive -- the same value WriteFinalResults()'s
    // "Beban Total" line prints), read from sd.W[no_batang] at the same
    // moment the end forces above are captured. Needed to derive N(x)/
    // V(x)/M(x) force diagrams along the span analytically from the end
    // forces + this single uniform load (this port's load model never
    // has more than one distributed load per member -- see AGENTS.md's
    // `gui/editor/` note on the legacy `.bbn` format's two load
    // categories) without a second, separately-timed StructureData read
    // (sd.W is zeroed by Application::OnRunResult() shortly after a run,
    // for the load editor's sake -- see LegacyIO.h's ReadLoads() comment
    // -- so this must be captured here, not read again later).
    float w_total_n_per_m = 0.f;
};

// One joint's 6-DOF displacement, read from sd.DJ[6*no_joint-5+dof], dof
// 0..5 = UX,UY,UZ,RX,RY,RZ ("arah 1..6"), the same convention
// JRL/AJ/AR use throughout this port. Translations (ux,uy,uz) are in
// meters, the same unit X/Y/Z joint coordinates already use -- verified
// by hand against a real run (Apl1-1, issue #59): joint 5's DJ2 =
// -0.000169 matches a column's P*L/(A*E) axial deflection
// (217200N * 5m / (0.25m^2 * 2.5743e10 Pa) = -1.688e-4 m) to 3 significant
// figures, ruling out mm (which would be 1000x too large) or any other
// unit. Rotations (rx,ry,rz) are in radians, standard for a stiffness
// formulation's rotational DOFs.
struct JointDisplacement {
    int no_joint = 0; // 1-based
    float ux = 0.f, uy = 0.f, uz = 0.f;
    float rx = 0.f, ry = 0.f, rz = 0.f;
};

// One restrained joint's 6-DOF reaction, read from
// sd.AR[6*no_joint-5+dof]. Only populated for joints with at least one
// restrained DOF -- Hasil() only ever writes an AR entry when the
// corresponding sd.JRL flag is 1 (see StructuralAnalysis.cpp), so an
// unrestrained joint's reaction is not a meaningful physical quantity.
// Force in N, moment in N*m (same AM-derived convention as MemberForces
// above -- see that struct's comment for the unit correction).
struct JointReaction {
    int no_joint = 0; // 1-based
    float fx = 0.f, fy = 0.f, fz = 0.f;
    float mx = 0.f, my = 0.f, mz = 0.f;
};

struct AnalysisResults {
    std::vector<MemberForces> member_forces;      // one per member, 1..sd.M
    std::vector<JointDisplacement> displacements;  // one per joint, 1..sd.NJ
    std::vector<JointReaction> reactions;          // restrained joints only

    // Sums a reaction component (0=Fx,1=Fy,2=Fz,3=Mx,4=My,5=Mz, "arah
    // 1..6") across every restrained joint -- e.g. TotalReaction(1)
    // matches orcisf_cli's `equilibrium` command's "Sum support
    // reactions, arah Y" total exactly (same sd.AR values, same
    // restrained-joint set -- orcisf_cli.cpp now calls this directly
    // rather than summing AR a second, independent way).
    float TotalReaction(int dof_0based) const;

    // Issue #62: total applied load per global DOF (same 0..5 "arah 1..6"
    // indexing as TotalReaction above) -- sum of every joint's AJ plus,
    // for dof=1 (Y, this port's vertical axis) only, every beam member's
    // self-weight/UDL resultant (-W*EL, downward) -- matching
    // orcisf_cli's `equilibrium` command's "Sum applied load, arah Y"
    // computation exactly, generalized to all 6 DOF (no distributed load
    // ever acts in the other 5 directions in this port's load model, see
    // AGENTS.md's `gui/editor/` note). Compare against TotalReaction(dof)
    // for the same dof for a global-equilibrium check -- the two should
    // be equal and opposite (sum to ~0).
    float total_applied_load[6] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
};

// Captures sd.AM/DJ/AR into GUI-friendly structs.
//
// IMPORTANT -- read before assuming this reflects "the final optimized
// structure": like WriteFinalResults()'s `.str` section (see
// engine/README.md's "Deliberate deviations"), this reads whichever
// structure's analysis happens to be "frozen" in sd.AM/DJ/AR at the time
// it's called -- for a completed optimization run, that is the
// *last-evaluated* population candidate during the search, not
// necessarily the best (JSTD-1) structure. This is intentional, not a
// bug to fix here: it is the exact same characteristic
// ComputeMemberResults()'s own force-derived fields (MU/FMU/
// axial_demand/moment_x_demand, all traced back to the same
// Hasil()-computed MLAP/MTUM_KI/PK/MKX/MKY) already have. Re-running
// Struktur() here for slot JSTD-1 specifically would make this
// function's numbers *diverge* from what PropertiesPanel/DetailingPanel/
// PDF export already display for the same run -- worse than the
// existing legacy characteristic, not a fix for it. Call after
// Struktur() (directly, or via a completed optimization run) has
// populated sd's analysis fields -- no new analysis pass here.
AnalysisResults ComputeAnalysisResults(const StructureData& sd);

} // namespace orcisf::engine
