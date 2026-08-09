#include "engine/CostConstraint.h"

#include <cmath>

#include "engine/BeamDesign.h"
#include "engine/ColumnDesign.h"
#include "engine/StructuralAnalysis.h"

namespace orcisf::engine {

float KendalaHarga(StructureData& sd, const LegacyArray<int>& var_b_nya, LegacyArray<int>& var_k_nya) {
    float kendalanya = 0.f;
    float harganya = 0.f;

    for (int iKH = 0; iKH < sd.jum_balok; ++iKH) {
        PeriksaBatang(sd, sd.no_balok[iKH]);

        sd.B = Isi(var_b_nya[0 + 12 * iKH], sd.sisi_d_B);
        sd.H = Isi(var_b_nya[1 + 12 * iKH], sd.sisi_d_H);

        sd.DIA1lap = Isi(var_b_nya[2 + 12 * iKH], sd.DIA_d);
        sd.NL1lap = Isi(var_b_nya[3 + 12 * iKH], sd.NL_d);
        sd.DIA2lap = Isi(var_b_nya[4 + 12 * iKH], sd.DIA_d);
        sd.NL2lap = Isi(var_b_nya[5 + 12 * iKH], sd.NL_d);

        sd.DIA1tum = Isi(var_b_nya[6 + 12 * iKH], sd.DIA_d);
        sd.NL1tum = Isi(var_b_nya[7 + 12 * iKH], sd.NL_d);
        sd.DIA2tum = Isi(var_b_nya[8 + 12 * iKH], sd.DIA_d);
        sd.NL2tum = Isi(var_b_nya[9 + 12 * iKH], sd.NL_d);

        sd.DIAS = Isi(var_b_nya[10 + 12 * iKH], sd.DIAS_d);
        sd.Jarak_S = Isi(var_b_nya[11 + 12 * iKH], sd.JS_d);
        sd.L = sd.EL[sd.no_balok[iKH]];

        if (std::fabs(sd.GESER_KI[sd.no_balok[iKH]]) > sd.GESER_KA[sd.no_balok[iKH]]) {
            sd.VU = std::fabs(sd.GESER_KI[sd.no_balok[iKH]] / kTeta);
        } else {
            sd.VU = std::fabs(sd.GESER_KA[sd.no_balok[iKH]] / kTeta);
        }

        // Daerah lapangan (midspan)
        sd.DIA1 = sd.DIA1lap;
        sd.NL1 = sd.NL1lap;
        sd.DIA2 = sd.DIA2lap;
        sd.NL2 = sd.NL2lap;
        sd.MU = std::fabs(sd.MLAP[sd.no_balok[iKH]] / kTeta);

        DesignBeam(sd);
        kendalanya += sd.kendala;
        harganya += sd.harga;

        Lendutan(sd, sd.no_balok[iKH]);
        kendalanya += sd.kendala_lendutan;

        // Daerah tumpuan (support)
        sd.DIA1 = sd.DIA1tum;
        sd.NL1 = sd.NL1tum;
        sd.DIA2 = sd.DIA2tum;
        sd.NL2 = sd.NL2tum;

        if (std::fabs(sd.MTUM_KI[sd.no_balok[iKH]]) > std::fabs(sd.MTUM_KA[sd.no_balok[iKH]])) {
            sd.MU = std::fabs(sd.MTUM_KI[sd.no_balok[iKH]] / kTeta);
        } else {
            sd.MU = std::fabs(sd.MTUM_KA[sd.no_balok[iKH]] / kTeta);
        }

        DesignBeam(sd);
        kendalanya += sd.kendala;
        harganya += sd.harga;
    }

    for (int iKH = 0; iKH < sd.jum_kolom; ++iKH) {
        PeriksaBatang(sd, sd.no_kolom[iKH]);

        sd.sisi = Isi(var_k_nya[0 + 5 * iKH], sd.sisi_d_K);
        sd.DIA = Isi(var_k_nya[1 + 5 * iKH], sd.DIA_d);
        sd.N_DIA = Isi(var_k_nya[2 + 5 * iKH], sd.NL_d);
        sd.DIAS = Isi(var_k_nya[3 + 5 * iKH], sd.DIAS_d);
        sd.Jarak_S = Isi(var_k_nya[4 + 5 * iKH], sd.JS_d);

        sd.PU = std::fabs(sd.PK[sd.no_kolom[iKH]] / kTeta);
        sd.MUX = sd.MKX[sd.no_kolom[iKH]] / kTeta;
        sd.MUY = sd.MKY[sd.no_kolom[iKH]] / kTeta;
        sd.VU = std::fabs(sd.GK[sd.no_kolom[iKH]] / kTeta);
        sd.L = sd.EL[sd.no_kolom[iKH]];

        DesignColumn(sd);

        // Tighten stirrup spacing to the smallest discrete value <= Sref,
        // mutating the candidate design in place, then re-run the design
        // with that adjusted spacing.
        for (int cari_S = sd.nvk[4 + 5 * iKH] - 1; cari_S > 0; --cari_S) {
            if (Isi(cari_S, sd.JS_d) <= sd.Sref) {
                var_k_nya[4 + 5 * iKH] = cari_S;
                sd.Jarak_S = Isi(cari_S, sd.JS_d);
                break;
            }
        }
        DesignColumn(sd);

        // If the slenderness constraint is violated, bump the column side
        // up to the smallest discrete value satisfying it, mutating the
        // candidate design in place, then re-run the design once more.
        if (sd.kendala_kelangsingan > 0) {
            sd.L = sd.EL[sd.no_kolom[iKH]];
            float sisi_baru = 0.75f * sd.L * 1000.f / (22.f * std::sqrt(1.f / 12.f));
            for (int cari_sisi = var_k_nya[0 + 5 * iKH]; cari_sisi < sd.nvk[0 + 5 * iKH]; ++cari_sisi) {
                float sisi_cari = Isi(cari_sisi, sd.sisi_d_K);
                if (sisi_cari > sisi_baru) {
                    var_k_nya[0 + 5 * iKH] = cari_sisi;
                    sd.sisi = Isi(cari_sisi, sd.sisi_d_K);
                    break;
                }
            }
            DesignColumn(sd);
        }

        kendalanya += sd.kendala;
        harganya += sd.harga;
    }

    return sd.finalti / (harganya + sd.finalti * kendalanya);
}

} // namespace orcisf::engine
