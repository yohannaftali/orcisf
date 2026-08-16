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
// WriteStrukturSection() already writes to the .str file.
struct MemberForces {
    int no_batang = 0; // 1-based, matches sd.JJ[no_batang]/JK[no_batang]

    float axial_a = 0.f, shear_y_a = 0.f, shear_z_a = 0.f;
    float torsion_a = 0.f, moment_y_a = 0.f, moment_z_a = 0.f;

    float axial_b = 0.f, shear_y_b = 0.f, shear_z_b = 0.f;
    float torsion_b = 0.f, moment_y_b = 0.f, moment_z_b = 0.f;
};

// One joint's 6-DOF displacement (translations in mm, rotations in
// radians -- the same units DJ already carries), read from
// sd.DJ[6*no_joint-5+dof], dof 0..5 = UX,UY,UZ,RX,RY,RZ ("arah 1..6"),
// the same convention JRL/AJ/AR use throughout this port.
struct JointDisplacement {
    int no_joint = 0; // 1-based
    float ux = 0.f, uy = 0.f, uz = 0.f;
    float rx = 0.f, ry = 0.f, rz = 0.f;
};

// One restrained joint's 6-DOF reaction (force in N, moment in Nmm),
// read from sd.AR[6*no_joint-5+dof]. Only populated for joints with at
// least one restrained DOF -- Hasil() only ever writes an AR entry when
// the corresponding sd.JRL flag is 1 (see StructuralAnalysis.cpp), so an
// unrestrained joint's reaction is not a meaningful physical quantity.
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
