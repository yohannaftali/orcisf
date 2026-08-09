#include "engine/BeamDesign.h"

#include <cmath>

namespace orcisf::engine {

namespace {

// Port of balok::analisa(): flexural capacity + reinforcement-ratio checks.
void Analisa(StructureData& sd) {
    if (sd.FC <= 30.f) {
        sd.BT1 = 0.85f;
    } else {
        sd.BT1 = 0.85f - 0.008f * (sd.FC - 30.f);
    }
    if (sd.BT1 < 0.65f) {
        sd.BT1 = 0.65f;
    }

    sd.DS = sd.selimut_balok + 0.5f * sd.DIA1;
    sd.D = sd.H - sd.DS;
    float FS = sd.FY;
    float EPSY = sd.FY / 200000.f;

    float FS1 = FS;

    float ARM = (sd.AS - sd.AS1) * sd.FY / (0.85f * sd.FC * sd.B);
    if (ARM == 0) {
        ARM = kLimitNol;
    }

    float EPS1 = 0.003f * (ARM - sd.BT1 * sd.DS) / ARM;
    float EPS = 0.003f * (sd.BT1 * sd.D - ARM) / ARM;

    if (EPS < EPSY || EPS1 < EPSY) {
        float ASOL = 0.85f * sd.FC * sd.B;
        float BSOL = 600.f * sd.AS1 - sd.AS * sd.FY;
        float CSOL = -600.f * sd.BT1 * sd.AS1 * sd.DS;
        float DSOL = BSOL * BSOL - 4.f * ASOL * CSOL;
        if (DSOL <= 0) {
            DSOL = 0.f;
        }
        float ARMS = -BSOL + std::sqrt(DSOL);
        if (ARMS <= 0 && DSOL >= 0) {
            ARMS = -BSOL - std::sqrt(DSOL);
        }
        ARM = 0.5f * (ARMS / ASOL);
        if (ARM <= 0) {
            ARM = kLimitNol;
        }

        EPS1 = 0.003f * (ARM - sd.BT1 * sd.DS) / ARM;
        EPS = 0.003f * (sd.BT1 * sd.D - ARM) / ARM;
    }

    if (EPS > EPSY) {
        FS = sd.FY;
    }

    FS1 = EPS1 * 200000.f;
    if (FS1 > sd.FY) {
        FS1 = sd.FY;
    }
    if (FS1 < 0) {
        FS1 = 0;
    }

    float RH1 = sd.AS1 / (sd.B * sd.D);
    float RHB = 0.75f * (0.85f * sd.FC * sd.BT1 / sd.FY) * (600.f / (600.f + FS)) + RH1 * FS1 * FS1 / sd.FY;
    float RHO = sd.AS / (sd.B * sd.D);
    float RMIN = 1.4f / sd.FY;

    sd.kendala_rho_b = RHO / RHB - 1.f;
    if (sd.kendala_rho_b < 0) {
        sd.kendala_rho_b = 0.f;
    }

    sd.kendala_rho_m = RMIN / RHO - 1.f;
    if (sd.kendala_rho_m < 0) {
        sd.kendala_rho_m = 0.f;
    }

    sd.kendala_rho = sd.kendala_rho_b + sd.kendala_rho_m;

    sd.FMU = kTeta * (0.85f * sd.FC * ARM * sd.B * (sd.D - ARM / 2.f) + sd.AS1 * FS1 * (sd.D - sd.DS)) / 1000.f;

    if (sd.FMU > 0) {
        sd.kendala_M = sd.MU / sd.FMU - 1.f;
    } else {
        sd.kendala_M = 0.f;
    }
    if (sd.kendala_M < 0) {
        sd.kendala_M = 0.f;
    }
}

// Port of balok::sengkang_balok(): shear reinforcement / stirrup-spacing check.
void SengkangBalok(StructureData& sd) {
    sd.DS = sd.selimut_balok + 0.5f * sd.DIA1;
    sd.D = sd.H - sd.DS;

    sd.AV = 0.25f * kPi * (sd.DIAS * sd.DIAS);

    sd.VC = (1.f / 6.f) * std::sqrt(sd.FC) * sd.B * sd.D;
    float VS = sd.VU / kTeta - sd.VC;

    float Smak[4];
    if (VS <= 0) {
        VS = kLimitNol;
    }
    Smak[0] = 3.f * sd.AV * sd.FYS / sd.B;
    Smak[1] = sd.AV * sd.FYS * sd.D / VS;
    Smak[2] = 0.5f * sd.D;
    Smak[3] = 600.f;
    if (VS > (1.f / 3.f) * std::sqrt(sd.FC) * sd.B * sd.D) {
        Smak[2] = 0.25f * sd.D;
        Smak[3] = 300.f;
    }
    float SmakS = Smak[0];
    for (int sk = 1; sk < 4; ++sk) {
        if (SmakS > Smak[sk]) {
            SmakS = Smak[sk];
        }
    }

    sd.Sref = SmakS;
    sd.kendala_sb = sd.Jarak_S / SmakS - 1.f;
    if (sd.kendala_sb < 0.f) {
        sd.kendala_sb = 0.f;
    }
}

} // namespace

void DesignBeam(StructureData& sd) {
    sd.kendala = 0.f;
    sd.harga = 0.f;

    sd.AS = 0.25f * kPi * std::pow(sd.DIA1, 2) * sd.NL1;
    sd.AS1 = 0.25f * kPi * std::pow(sd.DIA2, 2) * sd.NL2;

    Analisa(sd);
    SengkangBalok(sd);
    sd.kendala = sd.kendala_sb + sd.kendala_rho + sd.kendala_M;

    sd.volume_beton = sd.B * sd.H * 0.5f * sd.L / 1.E6f;
    sd.berat_besi = (sd.AS + sd.AS1) * 0.5f * sd.L * kBjBesi / 1.E6f;
    sd.berat_sengkang = 0.5f * (std::fabs(sd.L / (sd.Jarak_S / 1000.f)) - 1.f) *
                        (2.f * ((sd.B - 2.f * sd.selimut_balok) / 1000.f + 2.f * (sd.H - 2.f * sd.selimut_balok) / 1000.f)) *
                        (kPi / 4.f) * std::pow(sd.DIAS / 1000.f, 2) * kBjBesi;

    sd.harga = sd.volume_beton * sd.harga_beton + sd.berat_besi * sd.harga_besi + sd.berat_sengkang * sd.harga_besi;
}

void Lendutan(StructureData& sd, int no_batang_l) {
    sd.n = 2.E5f / (sd.E * 1.E-6f);
    sd.fr = 0.7f * std::sqrt(sd.FC);

    sd.AS = 0.25f * kPi * std::pow(sd.DIA1, 2) * sd.NL1;
    sd.AS1 = 0.25f * kPi * std::pow(sd.DIA2, 2) * sd.NL2;

    sd.LGN = (sd.n * sd.AS / sd.B) *
             (std::sqrt(1.f + (2.f * sd.B * (sd.H - sd.selimut_balok)) / (sd.n * sd.AS)) - 1.f);

    sd.Icr = (1.f / 3.f) * sd.B * std::pow(sd.LGN, 3) +
             sd.n * sd.AS * std::pow((sd.H - sd.selimut_balok) - sd.LGN, 2);

    sd.Ig = (1.f / 12.f) * sd.B * std::pow(sd.H, 3);

    sd.Mcr = sd.fr * sd.Ig / (0.5f * sd.H);

    float mlap_term = sd.MLAP[no_batang_l] * 1.e3f;
    sd.Ie = std::pow(sd.Mcr / mlap_term, 3) * sd.Ig + (1.f - std::pow(sd.Mcr / mlap_term, 3)) * sd.Icr;

    sd.LENDUTAN = ((5.f * std::pow(sd.L * 1000.f, 2)) / (48.f * (sd.E * 1.E-6f) * sd.Ie)) *
                  std::fabs(sd.MLAP[no_batang_l]) * 1000.f;

    sd.Lambda = 2.f / (1.f + 50.f * sd.AS1 / (sd.B * sd.H));
    sd.LENDUTAN = (1.f + sd.Lambda) * sd.LENDUTAN;

    sd.LENDUTAN_IJIN = (sd.L * 1000.f) / 180.f;

    sd.kendala_lendutan = sd.LENDUTAN / sd.LENDUTAN_IJIN - 1.f;
    if (sd.kendala_lendutan < 0) {
        sd.kendala_lendutan = 0.f;
    }
}

} // namespace orcisf::engine
