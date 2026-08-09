#pragma once

#include <vector>

#include "engine/StructureData.h"

// New in this port (issue #5's "per-member results viewable in 3D"
// acceptance criterion): captures the same per-member final-design
// numbers LegacyIO's WriteFinalResults() writes to the .opt/.kdl text
// files, but into memory instead, for the GUI viewport/properties panel.
namespace orcisf::engine {

struct MemberResult {
    int no_batang = 0; // 1-based, matches sd.JJ[no_batang]/JK[no_batang]
    bool is_beam = true;

    // Final cross-section (mm). Beam: width x height. Column: side x side
    // (width == height == sisi).
    float width = 0.f, height = 0.f;

    float harga = 0.f; // cost contribution (Rp.)

    // Beam only: moment demand/capacity (Nmm) and constraint value for
    // each design region -- lapangan (midspan) and tumpuan (support).
    // kendala_* is 0 when every check in that region is satisfied, >0
    // when violated (same convention as the optimizer's fitness formula).
    float lapangan_MU = 0.f, lapangan_FMU = 0.f, lapangan_kendala = 0.f;
    float tumpuan_MU = 0.f, tumpuan_FMU = 0.f, tumpuan_kendala = 0.f;
    float kendala_lendutan = 0.f;

    // Column only: axial force + biaxial moment demand/capacity (N, Nmm)
    // and the aggregate constraint value across every column check
    // (gaya/rasio-penulangan/jarak-tulangan/sengkang/kelangsingan).
    float axial_demand = 0.f, axial_capacity = 0.f;
    float moment_x_demand = 0.f, moment_y_demand = 0.f, moment_capacity = 0.f;
    float kendala_kolom = 0.f;

    // Worst-case constraint value across every check for this member --
    // 0 = fully satisfied (a reasonable "green" in a color-coded view),
    // >0 = violated (the total the optimizer's fitness formula would
    // penalize). Beams: max of lapangan/tumpuan/lendutan; columns:
    // kendala_kolom directly.
    float Kendala() const;
};

// Computes final-design results for every beam/column using the current
// best (JSTD-1) structure, the same precondition WriteFinalResults has:
// call after an optimization run has populated sd.var_b/var_k and sorted
// sd.fitstr/kendalastr/hargastr. Mutates sd's scratch design-calculation
// fields the same way WriteFinalResults does (both are read-only with
// respect to var_b/var_k/fitstr themselves).
std::vector<MemberResult> ComputeMemberResults(StructureData& sd);

} // namespace orcisf::engine
