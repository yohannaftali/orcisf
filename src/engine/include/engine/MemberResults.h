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

    // Beam only: moment demand/capacity (N*m) and constraint value for
    // each design region -- lapangan (midspan) and tumpuan (support).
    // kendala_* is 0 when every check in that region is satisfied, >0
    // when violated (same convention as the optimizer's fitness formula).
    float lapangan_MU = 0.f, lapangan_FMU = 0.f, lapangan_kendala = 0.f;
    float tumpuan_MU = 0.f, tumpuan_FMU = 0.f, tumpuan_kendala = 0.f;
    float kendala_lendutan = 0.f;

    // Column only: axial force + biaxial moment demand/capacity (N, N*m)
    // and the aggregate constraint value across every column check
    // (gaya/rasio-penulangan/jarak-tulangan/sengkang/kelangsingan).
    float axial_demand = 0.f, axial_capacity = 0.f;
    float moment_x_demand = 0.f, moment_y_demand = 0.f, moment_capacity = 0.f;
    float kendala_kolom = 0.f;

    // Reinforcement, for issue #8's detailing drawings. All diameters/
    // spacings in mm, cover in mm. Beam: tension/compression bars differ
    // between lapangan (midspan) and tumpuan (support) regions, matching
    // the legacy DIA1lap/NL1lap/... vs DIA1tum/NL1tum/... split (1 =
    // tension, 2 = compression -- see IsiElemenBalokFields). Column: a
    // single symmetric arrangement of N_DIA bars of diameter `dia` evenly
    // spaced around all four sides (4*N_DIA - 4 bars total, matching the
    // legacy `WriteFinalResults()` display), with jarak_antar_tulangan
    // the on-center spacing DesignColumn() checked against the code
    // minimum.
    float cover_mm = 0.f; // selimut_balok or selimut_kolom (run-wide, copied per member for a self-contained struct)
    // Beam. Bar counts are float, matching the legacy NL1/NL2 fields
    // exactly (discrete-table lookups resolve to float even for "count"
    // tables) -- always integral in practice, format with %.0f.
    float lap_dia_tarik = 0.f, lap_dia_tekan = 0.f;
    float lap_n_tarik = 0.f, lap_n_tekan = 0.f;
    float tum_dia_tarik = 0.f, tum_dia_tekan = 0.f;
    float tum_n_tarik = 0.f, tum_n_tekan = 0.f;
    float stirrup_dia = 0.f, stirrup_spacing = 0.f; // shared by both regions (DIAS/Jarak_S are per-member, not per-region)
    // Column:
    float col_dia = 0.f;
    float col_n_dia = 0.f; // N_DIA -- total bars around the perimeter is 4*N_DIA-4
    float col_bar_spacing = 0.f; // jarak_antar_tulangan

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
