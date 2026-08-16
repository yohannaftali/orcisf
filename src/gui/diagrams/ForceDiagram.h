#pragma once

#include <vector>

#include "engine/AnalysisResults.h"

// Issue #61 (epic #58): pure-geometry force-diagram computation, no ImGui/
// ImPlot dependency -- same DetailingLayout/DetailingPanel (#8) split of
// "one layout/computation function, a separate renderer" this project
// already established, so a future second renderer (e.g. a PDF export of
// force diagrams, mirroring #9's reuse of DetailingLayout) wouldn't need
// to recompute anything.
namespace orcisf::gui {

// One sampled station along a member's span (local x, 0 = end A/JJ, up to
// `length_m` = end B/JK). n_n/v_n in N (axial / shear about local y),
// m_nmm/t_nmm in Nmm (bending moment about local z / torsion about local
// x) -- matching engine::MemberForces' own mixed-unit convention.
// sigma_top_mpa/sigma_bottom_mpa (N/mm^2 = MPa) are only meaningful when
// the owning ForceDiagram::has_section is true.
struct ForceDiagramSample {
    float x_m = 0.f;
    float n_n = 0.f, v_n = 0.f, m_nmm = 0.f, t_nmm = 0.f;
    float sigma_top_mpa = 0.f, sigma_bottom_mpa = 0.f;
};

struct ForceDiagram {
    std::vector<ForceDiagramSample> samples; // ascending x_m, first = 0, last = length_m
    bool has_section = false;                // false when width_mm/height_mm weren't available
};

// Computes N(x)/V(x)/M(x)/T(x) along a member's span from its two
// end-force sets plus its single uniform distributed load
// (engine::MemberForces, issue #59), using standard beam-diagram
// relations -- valid because this port's load model never has more than
// one uniform member load and no arbitrary intermediate point load (the
// legacy `.bbn` format's only two load categories, see AGENTS.md's
// `gui/editor/` note): N and T are constant; V(x) = V_start - w*x;
// M(x) = M_start + V_start*x - w*x^2/2, with M_start = -moment_z_a
// (matching MTUM_KI's own negation of AM's raw start moment) and
// V_start = shear_y_a (matching GESER_KI, no negation).
//
// *** KNOWN UNVERIFIED GAP -- read before trusting this for anything
// beyond a rough visual diagram *** -- see AGENTS.md's #61 notes for the
// full numeric trail: V(x) was independently confirmed exact against a
// real run (V(length_m) computed this way matched -GESER_KA/shear_y_b
// precisely), and M_start/M_end taken directly from moment_z_a/
// moment_z_b independently match MTUM_KI/MTUM_KA exactly -- but
// integrating the *verified* V(x) from the *verified* M_start does NOT
// reproduce the *verified* M_end for the one real case checked
// (off by ~25x, not a rounding-scale discrepancy). Since V(x) alone
// checks out but the V-M integral relationship doesn't, the likely
// explanation is something in how self-weight timing/staleness affects
// which `w` value is actually consistent with this particular M/V pair
// (see AGENTS.md), not a sign error (every sign permutation was tried
// by hand and none matched either) -- but this was NOT root-caused
// tonight. Treat M(x)'s *interior* shape (not just its two endpoints,
// which are exact) as provisional until re-verified.
//
// `width_mm`/`height_mm` (0 if unavailable, e.g. no completed design for
// this member yet) enable the extreme-fiber axial+bending stress columns
// via sigma = N/A +- M/S for a rectangular section (S = width*height^2/6,
// bending about local z -- matching M(x)'s own axis).
ForceDiagram ComputeForceDiagram(const engine::MemberForces& forces, float length_m, float width_mm, float height_mm,
                                  int num_stations = 21);

} // namespace orcisf::gui
