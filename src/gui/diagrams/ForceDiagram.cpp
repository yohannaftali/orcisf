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
    // derivation (matches Hasil()'s MTUM_KI/GESER_KI exactly).
    float m_start_nmm = -forces.moment_z_a;
    float v_start_n = forces.shear_y_a;
    float w_per_mm = forces.w_total_n_per_m / 1000.f; // N/m -> N/mm, to match x in mm
    float length_mm = length_m * 1000.f;

    num_stations = std::max(2, num_stations);
    out.samples.reserve(static_cast<size_t>(num_stations));
    for (int i = 0; i < num_stations; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(num_stations - 1);
        float x_mm = t * length_mm;

        ForceDiagramSample s;
        s.x_m = t * length_m;
        s.n_n = forces.axial_a; // no distributed axial load in this load model -- constant
        s.v_n = v_start_n - w_per_mm * x_mm;
        s.m_nmm = m_start_nmm + v_start_n * x_mm - 0.5f * w_per_mm * x_mm * x_mm;
        s.t_nmm = forces.torsion_a; // no distributed torque in this load model -- constant

        if (out.has_section) {
            float axial_stress_mpa = s.n_n / area_mm2;
            float bending_stress_mpa = s.m_nmm / section_modulus_mm3;
            s.sigma_top_mpa = axial_stress_mpa + bending_stress_mpa;
            s.sigma_bottom_mpa = axial_stress_mpa - bending_stress_mpa;
        }

        out.samples.push_back(s);
    }

    return out;
}

} // namespace orcisf::gui
