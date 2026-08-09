#include "engine/MemberResults.h"

#include <algorithm>

#include "engine/BeamDesign.h"
#include "engine/ColumnDesign.h"
#include "engine/StructuralAnalysis.h"

namespace orcisf::engine {

float MemberResult::Kendala() const {
    if (is_beam) {
        return std::max({lapangan_kendala, tumpuan_kendala, kendala_lendutan});
    }
    return kendala_kolom;
}

std::vector<MemberResult> ComputeMemberResults(StructureData& sd) {
    std::vector<MemberResult> results;
    results.reserve(static_cast<size_t>(sd.jum_balok + sd.jum_kolom));

    // Mirrors WriteFinalResults' (LegacyIO.cpp / CETAK.HPP's cetak_akhir())
    // per-member loop exactly -- same "best structure" slot, same call
    // order -- just capturing into MemberResult instead of writing text.
    sd.no_struktur = sd.JSTD - 1;

    for (int oio = 0; oio < sd.jum_balok; ++oio) {
        MemberResult r;
        r.no_batang = sd.no_balok[oio];
        r.is_beam = true;

        PeriksaBatang(sd, r.no_batang);
        IsiElemenBalokFields(sd, oio);
        r.width = sd.B;
        r.height = sd.H;

        ElemenLapangan(sd, oio);
        DesignBeam(sd);
        r.lapangan_MU = sd.MU;
        r.lapangan_FMU = sd.FMU;
        r.lapangan_kendala = sd.kendala;
        float harga_lapangan = sd.harga;
        Lendutan(sd, r.no_batang);
        r.kendala_lendutan = sd.kendala_lendutan;

        ElemenTumpuan(sd, oio);
        DesignBeam(sd);
        r.tumpuan_MU = sd.MU;
        r.tumpuan_FMU = sd.FMU;
        r.tumpuan_kendala = sd.kendala;
        r.harga = harga_lapangan + sd.harga;

        results.push_back(r);
    }

    for (int oio = 0; oio < sd.jum_kolom; ++oio) {
        MemberResult r;
        r.no_batang = sd.no_kolom[oio];
        r.is_beam = false;

        PeriksaBatang(sd, r.no_batang);
        IsiElemenKolomFields(sd, oio);
        DesignColumn(sd);

        r.width = sd.sisi;
        r.height = sd.sisi;
        r.axial_demand = sd.PK[r.no_batang] / 0.8f; // matches .opt's "Gaya aksial" display
        r.axial_capacity = sd.FPU;
        r.moment_x_demand = sd.MKX[r.no_batang] / 0.8f;
        r.moment_y_demand = sd.MKY[r.no_batang] / 0.8f;
        r.moment_capacity = sd.FMU;
        r.kendala_kolom = sd.kendala;
        r.harga = sd.harga;

        results.push_back(r);
    }

    return results;
}

} // namespace orcisf::engine
