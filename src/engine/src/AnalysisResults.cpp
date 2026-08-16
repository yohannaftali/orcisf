#include "engine/AnalysisResults.h"

#include <cmath>

#include "engine/LegacyIO.h"

namespace orcisf::engine {

float AnalysisResults::TotalReaction(int dof_0based) const {
    float total = 0.f;
    for (const JointReaction& r : reactions) {
        switch (dof_0based) {
            case 0: total += r.fx; break;
            case 1: total += r.fy; break;
            case 2: total += r.fz; break;
            case 3: total += r.mx; break;
            case 4: total += r.my; break;
            default: total += r.mz; break;
        }
    }
    return total;
}

AnalysisResults ComputeAnalysisResults(const StructureData& sd) {
    AnalysisResults out;

    out.member_forces.reserve(static_cast<size_t>(sd.M));
    for (int i = 1; i <= sd.M; ++i) {
        MemberForces f;
        f.no_batang = i;
        f.axial_a = sd.AM[i][1];
        f.shear_y_a = sd.AM[i][2];
        f.shear_z_a = sd.AM[i][3];
        f.torsion_a = sd.AM[i][4];
        f.moment_y_a = sd.AM[i][5];
        f.moment_z_a = sd.AM[i][6];
        f.axial_b = sd.AM[i][7];
        f.shear_y_b = sd.AM[i][8];
        f.shear_z_b = sd.AM[i][9];
        f.torsion_b = sd.AM[i][10];
        f.moment_y_b = sd.AM[i][11];
        f.moment_z_b = sd.AM[i][12];
        f.w_total_n_per_m = sd.W[i];
        out.member_forces.push_back(f);
    }

    out.displacements.reserve(static_cast<size_t>(sd.NJ));
    for (int j = 1; j <= sd.NJ; ++j) {
        JointDisplacement d;
        d.no_joint = j;
        d.ux = sd.DJ[6 * j - 5];
        d.uy = sd.DJ[6 * j - 4];
        d.uz = sd.DJ[6 * j - 3];
        d.rx = sd.DJ[6 * j - 2];
        d.ry = sd.DJ[6 * j - 1];
        d.rz = sd.DJ[6 * j];
        out.displacements.push_back(d);
    }

    // Reuse ComputeRestraintSummary()'s restrained-joint list rather than
    // re-deriving which joints qualify -- keeps this in lockstep with
    // WriteStructureFile()/the Properties panel's own restraint-derived
    // counts (issue #11).
    RestraintSummary restraint = ComputeRestraintSummary(sd);
    out.reactions.reserve(restraint.restrained_joints.size());
    for (int j : restraint.restrained_joints) {
        JointReaction r;
        r.no_joint = j;
        r.fx = sd.AR[6 * j - 5];
        r.fy = sd.AR[6 * j - 4];
        r.fz = sd.AR[6 * j - 3];
        r.mx = sd.AR[6 * j - 2];
        r.my = sd.AR[6 * j - 1];
        r.mz = sd.AR[6 * j];
        out.reactions.push_back(r);
    }

    // Issue #62: total applied load, all 6 DOF -- see the header comment
    // on total_applied_load for why only dof=1 (Y) gets a beam-self-weight
    // term.
    for (int j = 1; j <= sd.NJ; ++j) {
        for (int dof = 0; dof < 6; ++dof) {
            out.total_applied_load[dof] += sd.AJ[6 * j - 5 + dof];
        }
    }
    for (int i = 1; i <= sd.M; ++i) {
        // Geometry-only beam/column classification (mirrors PeriksaBatang's
        // CXZ>0.001 check without needing a mutable StructureData& to call
        // it -- the same inline approach gui::BuildSceneModel() already
        // uses, see SceneModel.cpp).
        float dx = sd.X[sd.JK[i]] - sd.X[sd.JJ[i]];
        float dy = sd.Y[sd.JK[i]] - sd.Y[sd.JJ[i]];
        float dz = sd.Z[sd.JK[i]] - sd.Z[sd.JJ[i]];
        float len = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (len > 1e-6f) {
            float cx = dx / len, cz = dz / len;
            bool is_beam = std::fabs(std::sqrt(cx * cx + cz * cz)) > 0.001f;
            if (is_beam) {
                out.total_applied_load[1] += -sd.W[i] * sd.EL[i];
            }
        }
    }

    return out;
}

} // namespace orcisf::engine
