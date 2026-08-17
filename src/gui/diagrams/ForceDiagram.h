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
// m_nm/t_nm in N*m (bending moment about local z / torsion about local
// x) -- matching engine::MemberForces' own convention (see that struct's
// header comment: these are N*m, not Nmm -- a units correction made
// while fixing this exact file, see ComputeForceDiagram()'s comment).
// sigma_top_mpa/sigma_bottom_mpa (N/mm^2 = MPa) are only meaningful when
// the owning ForceDiagram::has_section is true.
struct ForceDiagramSample {
    float x_m = 0.f;
    float n_n = 0.f, v_n = 0.f, m_nm = 0.f, t_nm = 0.f;
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
// V_start = shear_y_a (matching GESER_KI, no negation). `x`/`length_m`
// are in meters and `w` (`forces.w_total_n_per_m`) is in N/m throughout
// -- **do not convert to millimeters anywhere in this formula.**
//
// *** ROOT CAUSE OF A CONFIRMED BUG, FIXED 2026-08-17 -- read before
// reintroducing a millimeter conversion here ***. The very first
// version of this function converted x/w to millimeters before
// evaluating the formula, on the assumption that `MemberForces`'
// moment/torsion fields were in Nmm (matching a pre-existing, as it
// turns out mislabeled, comment on `engine::MemberResult`). A `tester`
// pass (2026-08-17) mechanically confirmed this was wrong by building a
// temporary instrumented `orcisf_cli` and comparing this formula's
// M(x) against real `MTUM_KI`/`MLAP`/`MTUM_KA` values from a live run:
// with the mm conversion, M(x) was off by 134x-1638x depending on
// station -- not a rounding gap, a real formula bug. Root cause: `AM`'s
// moment/torsion components (and therefore `MemberForces::moment_*`/
// `torsion_*`) are actually in **N*m**, the same SI unit family (N, m)
// this engine uses everywhere else -- confirmed independently via
// `BeratSendiri()`'s self-weight fixed-end-moment formula
// (`W_Balok*EL^2/12`, dimensionally only consistent as N*m given W in
// N/m and EL in m) and by reproducing the legacy `MLAP` formula's real
// checked-in value by hand. Once x/w are kept in their natural units
// (meters, N/m -- no conversion at all) and `M`/`T` are treated as
// N*m throughout, `M(length_m)` reproduces the real `MTUM_KA`/
// `moment_z_b` value to float32 precision, confirming the underlying
// V(x)/M(x) formula itself was *always* correct -- only the unit
// bookkeeping around it was wrong. See `engine::MemberForces`'s own
// header comment for the fuller unit-correction narrative (it also
// affects `MemberResult`'s pre-existing "Nmm" labels elsewhere in this
// codebase -- tracked as a separate, out-of-scope documentation issue,
// not touched here).
//
// `width_mm`/`height_mm` (0 if unavailable, e.g. no completed design for
// this member yet) enable the extreme-fiber axial+bending stress columns
// via sigma = N/A +- M/S for a rectangular section (S = width*height^2/6,
// bending about local z -- matching M(x)'s own axis). `M` must be
// converted from N*m to N*mm (`*1000`) before dividing by `S` (in mm^3)
// to get a result in N/mm^2 = MPa -- see the .cpp for exactly where.
ForceDiagram ComputeForceDiagram(const engine::MemberForces& forces, float length_m, float width_mm, float height_mm,
                                  int num_stations = 21);

} // namespace orcisf::gui
