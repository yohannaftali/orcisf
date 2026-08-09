#include "engine/ColumnDesign.h"

#include <cmath>

namespace orcisf::engine {

namespace {

// Port of kolom::hitung_kolom(): axial force/moment capacity of the column
// section at trial neutral-axis depth Ccoba, returned as (eks - ekscoba) so
// the false-position search below can find its root. PNcoba/MNcoba/ekscoba
// are the function's real outputs, returned via pointers to mirror the
// legacy code's side-effecting-then-returning style (they feed FPU/FMU
// after the search converges).
float HitungKolom(const StructureData& sd, float BT1, float DS, float eks, float Ccoba, float* out_PNcoba,
                   float* out_MNcoba) {
    if (Ccoba == 0) {
        Ccoba = kLimitNol;
    }
    float PNcoba = 0.85f * BT1 * sd.FC * Ccoba * sd.sisi;
    float MNcoba = 0.85f * BT1 * sd.FC * Ccoba * sd.sisi * (sd.sisi / 2.f - BT1 * Ccoba / 2.f);

    for (int ikl = 1; ikl <= static_cast<int>(sd.N_DIA); ++ikl) {
        float di = DS + (sd.N_DIA - ikl) * (sd.sisi - 2.f * DS) / (sd.N_DIA - 1.f);
        if (di == 0.5f * sd.sisi) {
            continue;
        }

        float ASdi;
        if (ikl == 1 || ikl == static_cast<int>(sd.N_DIA)) {
            ASdi = sd.N_DIA * (kPi / 4.f) * std::pow(sd.DIA, 2);
        } else {
            ASdi = 2.f * (kPi / 4.f) * std::pow(sd.DIA, 2);
        }

        float fsi = 600.f * (Ccoba - di) / Ccoba;
        if (fsi > sd.FY) {
            fsi = sd.FY;
        } else if (fsi < -sd.FY) {
            fsi = -sd.FY;
        }
        if (di < BT1 * Ccoba) {
            fsi = fsi - 0.85f * sd.FC;
        }

        float Fsi = fsi * ASdi;
        PNcoba = PNcoba + Fsi;
        MNcoba = MNcoba + Fsi * (sd.sisi / 2.f - di);
    }

    if (PNcoba <= 0) {
        PNcoba = kLimitNol;
    }
    float ekscoba = MNcoba / PNcoba;

    *out_PNcoba = PNcoba;
    *out_MNcoba = MNcoba;
    return eks - ekscoba;
}

// Port of kolom::rho(). Returns ASTOT (needed later for berat_besi).
float Rho(StructureData& sd) {
    float ASTOT = (4.f * sd.N_DIA - 4.f) * (kPi / 4.f) * std::pow(sd.DIA, 2);
    float RHO = ASTOT / std::pow(sd.sisi, 2);

    sd.kendala_r_min = 0.01f / RHO - 1.f;
    if (sd.kendala_r_min < 0.f) {
        sd.kendala_r_min = 0.f;
    }
    sd.kendala_r_mak = RHO / 0.08f - 1.f;
    if (sd.kendala_r_mak < 0.f) {
        sd.kendala_r_mak = 0.f;
    }
    sd.kendala_r = sd.kendala_r_min + sd.kendala_r_mak;
    return ASTOT;
}

// Port of kolom::jarak_tulangan().
void JarakTulangan(StructureData& sd) {
    float min1 = 1.5f * sd.DIA;
    float min2 = 40.f;
    float sisa = sd.sisi - 2.f * sd.selimut_kolom - sd.N_DIA * sd.DIA;

    sd.jarak_antar_tulangan = sisa / (sd.N_DIA - 1.f);
    if (sd.jarak_antar_tulangan == 0) {
        sd.jarak_antar_tulangan = kLimitNol;
    }

    float jarak_min = min1;
    if (min2 < min1) {
        jarak_min = min2;
    }

    sd.kendala_tul = jarak_min / sd.jarak_antar_tulangan - 1.f;
    if (sd.kendala_tul < 0.f) {
        sd.kendala_tul = 0.f;
    }
}

// Port of kolom::kelangsingan().
void Kelangsingan(StructureData& sd) {
    sd._K = 0.5f;
    float rasio_kelangsingan = sd._K * sd.L * 1000.f / (sd.sisi * std::sqrt(1.f / 12.f));
    sd.kendala_kelangsingan = rasio_kelangsingan / 22.f - 1.f;
    if (sd.kendala_kelangsingan < 0) {
        sd.kendala_kelangsingan = 0.f;
    }
}

// Port of kolom::analisa(): biaxial force/moment capacity via false-position
// search over the neutral-axis depth, ASTOT already computed by Rho().
void Analisa(StructureData& sd, float ASTOT) {
    float PN = std::fabs(sd.PU / kTeta);
    float MNX = std::fabs(sd.MUX / kTeta) * 1000.f;
    float MNY = std::fabs(sd.MUY / kTeta) * 1000.f;
    if (PN == 0.f) {
        PN = kLimitNol;
    }

    float PO = kTeta * (0.85f * sd.FC * std::pow(sd.sisi, 2) + ASTOT * sd.FY);

    sd.kendala_po = PN / PO - 1.f;
    if (sd.kendala_po <= 0.f) {
        sd.kendala_po = 0.f;

        float DS = sd.selimut_kolom + 0.5f * sd.DIA;

        float BT1;
        if (sd.FC <= 30.f) {
            BT1 = 0.85f;
        } else {
            BT1 = 0.85f - 0.008f * (sd.FC - 30.f);
        }
        if (BT1 < 0.65f) {
            BT1 = 0.65f;
        }

        // Empirical biaxial shape factor for symmetric reinforcement, Hulse &
        // Mosley (1986), "Reinforced Concrete Design by Computer", p.163.
        float beta = 0.3f + (0.7f / 0.6f) * (0.6f - sd.PU / (sd.sisi * sd.sisi * sd.FC));
        if (beta < 0.3f) {
            beta = 0.3f;
        }

        float eks;
        bool mnx_governs = MNX > MNY;
        if (mnx_governs) {
            float MOX = MNX + MNY * ((1.f - beta) / beta);
            eks = std::fabs(MOX / PN);
        } else {
            float MOY = MNY + MNX * ((1.f - beta) / beta);
            eks = std::fabs(MOY / PN);
        }

        float D = sd.sisi - DS;
        float CB = 600.f * D / (sd.FY + 600.f);

        float PNcoba, MNcoba;
        HitungKolom(sd, BT1, DS, eks, CB, &PNcoba, &MNcoba);

        float trialkiri = 40.f;
        float trialkanan = sd.sisi - 40.f;
        float trialbaru = CB;

        float deki = HitungKolom(sd, BT1, DS, eks, trialkiri, &PNcoba, &MNcoba);
        float deka = HitungKolom(sd, BT1, DS, eks, trialkanan, &PNcoba, &MNcoba);
        float deba = HitungKolom(sd, BT1, DS, eks, trialbaru, &PNcoba, &MNcoba);

        if (deki * deka > 0 && deki != 0 && deka != 0) {
            do {
                if (trialkiri > kLimitNol) {
                    trialkiri = trialkiri / 2.f;
                    deki = HitungKolom(sd, BT1, DS, eks, trialkiri, &PNcoba, &MNcoba);
                } else {
                    trialkanan += 10.f;
                    deka = HitungKolom(sd, BT1, DS, eks, trialkanan, &PNcoba, &MNcoba);
                    if (trialkanan > sd.sisi / 0.85f) {
                        break;
                    }
                }
            } while (deki * deka > 0 && deki != 0 && deka != 0);
        }

        int iterasi_fp = 1;
        do {
            if (deba < 1.f && deba > -1.f) {
                if (PNcoba <= 0) {
                    PNcoba = kLimitNol;
                }
                MNcoba = std::fabs(MNcoba);
                break;
            }
            if (deki * deba < 0.f) {
                trialkanan = trialbaru;
                deka = HitungKolom(sd, BT1, DS, eks, trialkanan, &PNcoba, &MNcoba);
            } else {
                trialkiri = trialbaru;
                deki = HitungKolom(sd, BT1, DS, eks, trialkiri, &PNcoba, &MNcoba);
            }
            trialbaru = (trialkiri + trialkanan) / 2.f;
            deba = HitungKolom(sd, BT1, DS, eks, trialbaru, &PNcoba, &MNcoba);
            ++iterasi_fp;
            if (trialkiri == trialkanan) {
                break;
            }
        } while (iterasi_fp <= 100);

        iterasi_fp = 1;
        do {
            if (trialkiri == trialkanan) {
                break;
            }
            trialbaru = trialkanan - deka * (trialkanan - trialkiri) / (deka - deki);
            deba = HitungKolom(sd, BT1, DS, eks, trialbaru, &PNcoba, &MNcoba);

            if (deba < 10.f && deba > -10.f) {
                if (PNcoba <= 0) {
                    PNcoba = kLimitNol;
                }
                MNcoba = std::fabs(MNcoba);
                break;
            }
            if (deki * deba < 0.f) {
                trialkanan = trialbaru;
                deka = HitungKolom(sd, BT1, DS, eks, trialkanan, &PNcoba, &MNcoba);
            } else {
                trialkiri = trialbaru;
                deki = HitungKolom(sd, BT1, DS, eks, trialkiri, &PNcoba, &MNcoba);
            }
            ++iterasi_fp;
            if (iterasi_fp >= 100) {
                deka = HitungKolom(sd, BT1, DS, eks, (trialkiri + trialkanan) / 2.f, &PNcoba, &MNcoba);
                break;
            }
        } while (deki * deka != 0.f);

        sd.FPU = std::fabs(PNcoba);
        sd.FMU = std::fabs(MNcoba) / 1000.f;

        sd.kendala_pn = PN / PNcoba - 1.f;
        if (sd.kendala_pn < 0) {
            sd.kendala_pn = 0.f;
        }

        if (mnx_governs) {
            float MOX = MNX + MNY * ((1.f - beta) / beta);
            sd.kendala_mn = MOX / MNcoba - 1.f;
        } else {
            float MOY = MNY + MNX * ((1.f - beta) / beta);
            sd.kendala_mn = MOY / MNcoba - 1.f;
        }
        if (sd.kendala_mn < 0) {
            sd.kendala_mn = 0.f;
        }
    } else {
        sd.FPU = PO;
        sd.FMU = 0.f;
        sd.kendala_pn = 0.f;
        sd.kendala_mn = 0.f;
    }

    sd.kendala_gaya = sd.kendala_po + sd.kendala_pn + sd.kendala_mn;
}

// Port of kolom::sengkang_kolom().
void SengkangKolom(StructureData& sd) {
    float Smak[3];
    Smak[0] = sd.sisi;
    Smak[1] = 16.f * sd.DIA;
    Smak[2] = 48.f * sd.DIAS;
    float SmakS = Smak[0];
    for (int sk = 1; sk < 3; ++sk) {
        if (SmakS > Smak[sk]) {
            SmakS = Smak[sk];
        }
    }

    sd.Sref = SmakS;
    sd.kendala_sengkang = sd.Jarak_S / SmakS - 1.f;
    if (sd.kendala_sengkang < 0.f) {
        sd.kendala_sengkang = 0.f;
    }
}

} // namespace

void DesignColumn(StructureData& sd) {
    sd.kendala = 0.f;
    sd.harga = 0.f;

    float ASTOT = Rho(sd);
    JarakTulangan(sd);
    Kelangsingan(sd);
    Analisa(sd, ASTOT);
    SengkangKolom(sd);

    sd.kendala = sd.kendala_sengkang + sd.kendala_r + sd.kendala_tul + sd.kendala_gaya + sd.kendala_kelangsingan;

    sd.volume_beton = sd.sisi * sd.sisi * sd.L / 1.E6f;
    sd.berat_besi = ASTOT * sd.L * kBjBesi / 1.E6f;
    sd.berat_sengkang = (std::fabs(sd.L / (sd.Jarak_S / 1000.f)) - 1.f) * 4.f *
                        ((sd.sisi - 2.f * sd.selimut_kolom) / 1000.f) * (kPi / 4.f) *
                        std::pow(sd.DIAS / 1000.f, 2.f) * kBjBesi;

    sd.harga = sd.volume_beton * sd.harga_beton + sd.berat_besi * sd.harga_besi + sd.berat_sengkang * sd.harga_besi;
}

} // namespace orcisf::engine
