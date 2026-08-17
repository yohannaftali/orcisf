#include "gui/diagrams/ForceDiagram.h"

#include <algorithm>

namespace orcisf::gui {

using engine::MemberForces;

ForceDiagram ComputeForceDiagram(const MemberForces& forces, float length_m, float width_mm, float height_mm,
                                  int num_stations) {
    ForceDiagram out;
    out.has_section = width_mm > 1e-3f && height_mm > 1e-3f;
    float area_mm2 = width_mm * height_mm;
    float section_modulus_mm3 = width_mm * height_mm * height_mm / 6.f;

    // See ForceDiagram.h's header comment for the sign-convention
    // derivation (matches Hasil()'s MTUM_KI/GESER_KI exactly) and the
    // unit-correction narrative (everything here stays in meters/N*m --
    // no millimeter conversion for x/w, that was the bug).
    float m_start_nm = -forces.moment_z_a;
    float v_start_n = forces.shear_y_a;
    float w_n_per_m = forces.w_total_n_per_m;

    num_stations = std::max(2, num_stations);
    out.samples.reserve(static_cast<size_t>(num_stations));
    for (int i = 0; i < num_stations; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(num_stations - 1);
        float x_m = t * length_m;

        ForceDiagramSample s;
        s.x_m = x_m;
        s.n_n = forces.axial_a; // no distributed axial load in this load model -- constant
        s.v_n = v_start_n - w_n_per_m * x_m;
        s.m_nm = m_start_nm + v_start_n * x_m - 0.5f * w_n_per_m * x_m * x_m;
        s.t_nm = forces.torsion_a; // no distributed torque in this load model -- constant

        if (out.has_section) {
            // M is N*m; convert to N*mm (*1000) before dividing by the
            // section modulus (mm^3) to get a result in N/mm^2 = MPa.
            float m_nmm = s.m_nm * 1000.f;
            float axial_stress_mpa = s.n_n / area_mm2;
            float bending_stress_mpa = m_nmm / section_modulus_mm3;
            s.sigma_top_mpa = axial_stress_mpa + bending_stress_mpa;
            s.sigma_bottom_mpa = axial_stress_mpa - bending_stress_mpa;
        }

        out.samples.push_back(s);
    }

    return out;
}

} // namespace orcisf::gui
