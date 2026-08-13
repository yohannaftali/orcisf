#include "engine/StructuralAnalysis.h"

#include <cmath>
#include <cstdlib>

#include "engine/StiffnessSolver.h"

namespace orcisf::engine {

void PeriksaBatang(StructureData& sd, int perb) {
    sd.EL[perb] = static_cast<float>(std::sqrt(std::pow(sd.X[sd.JK[perb]] - sd.X[sd.JJ[perb]], 2) +
                                                std::pow(sd.Y[sd.JK[perb]] - sd.Y[sd.JJ[perb]], 2) +
                                                std::pow(sd.Z[sd.JK[perb]] - sd.Z[sd.JJ[perb]], 2)));
    sd.CX = (sd.X[sd.JK[perb]] - sd.X[sd.JJ[perb]]) / sd.EL[perb];
    sd.Cy = (sd.Y[sd.JK[perb]] - sd.Y[sd.JJ[perb]]) / sd.EL[perb];
    sd.CZ = (sd.Z[sd.JK[perb]] - sd.Z[sd.JJ[perb]]) / sd.EL[perb];
    sd.CXZ = std::fabs(std::sqrt(sd.CX * sd.CX + sd.CZ * sd.CZ));
}

void IndeksBatang(StructureData& sd, int ib) {
    sd.IM[1] = 6 * sd.JJ[ib] - 5;
    sd.IM[2] = 6 * sd.JJ[ib] - 4;
    sd.IM[3] = 6 * sd.JJ[ib] - 3;
    sd.IM[4] = 6 * sd.JJ[ib] - 2;
    sd.IM[5] = 6 * sd.JJ[ib] - 1;
    sd.IM[6] = 6 * sd.JJ[ib];
    sd.IM[7] = 6 * sd.JK[ib] - 5;
    sd.IM[8] = 6 * sd.JK[ib] - 4;
    sd.IM[9] = 6 * sd.JK[ib] - 3;
    sd.IM[10] = 6 * sd.JK[ib] - 2;
    sd.IM[11] = 6 * sd.JK[ib] - 1;
    sd.IM[12] = 6 * sd.JK[ib];
}

void IsiMatrikKekakuan(StructureData& sd, int imk) {
    for (int is = 1; is <= 12; ++is) {
        for (int js = 1; js <= 12; ++js) {
            sd.SM[is][js] = 0.f;
        }
    }

    float SCM1A = sd.E * sd.AX[imk] / sd.EL[imk];
    float SCM1B = sd.G * sd.XI[imk] / sd.EL[imk];
    float SCM2Y = 4.0f * sd.E * sd.YI[imk] / sd.EL[imk];
    float SCM3Y = 1.5f * SCM2Y / sd.EL[imk];
    float SCM4Y = 2.0f * SCM3Y / sd.EL[imk];
    float SCM2Z = 4.0f * sd.E * sd.ZI[imk] / sd.EL[imk];
    float SCM3Z = 1.5f * SCM2Z / sd.EL[imk];
    float SCM4Z = 2.0f * SCM3Z / sd.EL[imk];

    sd.SM[1][1] = SCM1A;
    sd.SM[1][7] = -SCM1A;
    sd.SM[2][2] = SCM4Z;
    sd.SM[2][6] = SCM3Z;
    sd.SM[2][8] = -SCM4Z;
    sd.SM[2][12] = SCM3Z;
    sd.SM[3][3] = SCM4Y;
    sd.SM[3][5] = -SCM3Y;
    sd.SM[3][9] = -SCM4Y;
    sd.SM[3][11] = -SCM3Y;
    sd.SM[4][4] = SCM1B;
    sd.SM[4][10] = -SCM1B;
    sd.SM[5][5] = SCM2Y;
    sd.SM[5][9] = SCM3Y;
    sd.SM[5][11] = SCM2Y / 2.0f;
    sd.SM[6][6] = SCM2Z;
    sd.SM[6][8] = -SCM3Z;
    sd.SM[6][12] = SCM2Z / 2.0f;
    sd.SM[7][7] = SCM1A;
    sd.SM[8][8] = SCM4Z;
    sd.SM[8][12] = -SCM3Z;
    sd.SM[9][9] = SCM4Y;
    sd.SM[9][11] = SCM3Y;
    sd.SM[10][10] = SCM1B;
    sd.SM[11][11] = SCM2Y;
    sd.SM[12][12] = SCM2Z;

    for (int is = 1; is <= 11; ++is) {
        for (int js = is + 1; js <= 12; ++js) {
            sd.SM[js][is] = sd.SM[is][js];
        }
    }

    for (int is = 1; is <= 4; ++is) {
        for (int js = 1; js <= 12; ++js) {
            sd.SMRT[js][3 * is - 2] = sd.SM[js][3 * is - 2] * sd.R11[imk] +
                                       sd.SM[js][3 * is - 1] * sd.R21[imk] +
                                       sd.SM[js][3 * is] * sd.R31[imk];
            sd.SMRT[js][3 * is - 1] = sd.SM[js][3 * is - 2] * sd.R12[imk] +
                                       sd.SM[js][3 * is - 1] * sd.R22[imk] +
                                       sd.SM[js][3 * is] * sd.R32[imk];
            sd.SMRT[js][3 * is] = sd.SM[js][3 * is - 2] * sd.R13[imk] +
                                   sd.SM[js][3 * is - 1] * sd.R23[imk] +
                                   sd.SM[js][3 * is] * sd.R33[imk];
        }
    }
}

void Inersia(StructureData& sd) {
    sd.js_balok = 0;
    sd.js_kolom = 0;
    for (int iin = 1; iin <= sd.M; ++iin) {
        PeriksaBatang(sd, iin);

        if (sd.CXZ > 0.001f) {
            sd.b[iin] = Isi(sd.var_b[sd.no_struktur][0 + 12 * sd.js_balok], sd.sisi_d_B) / 1000.f;
            sd.h[iin] = Isi(sd.var_b[sd.no_struktur][1 + 12 * sd.js_balok], sd.sisi_d_H) / 1000.f;
            sd.js_balok++;
        } else {
            sd.b[iin] = Isi(sd.var_k[sd.no_struktur][0 + 5 * sd.js_kolom], sd.sisi_d_K) / 1000.f;
            sd.h[iin] = sd.b[iin];
            sd.js_kolom++;
        }

        sd.AX[iin] = sd.b[iin] * sd.h[iin];

        if (sd.b[iin] <= sd.h[iin]) {
            sd.XI[iin] = static_cast<float>(
                ((1.f / 3.f) -
                 0.21f * sd.b[iin] / sd.h[iin] *
                     (1.f - std::pow(sd.b[iin], 4) / (12.f * std::pow(sd.h[iin], 4)))) *
                sd.h[iin] * std::pow(sd.b[iin], 3));
        } else {
            // Roark's Formulas for Stress & Strain, Warren C. Young 1989, p.348
            float _a = 0.5f * sd.b[iin];
            float _b = 0.5f * sd.h[iin];
            sd.XI[iin] = static_cast<float>(
                _a * std::pow(_b, 3) *
                ((16.f / 3.f) - (3.36f * _b / _a) * (1.f - std::pow(_b, 4) / (12.f * std::pow(_a, 4)))));
        }

        sd.YI[iin] = static_cast<float>(sd.h[iin] * std::pow(sd.b[iin], 3) / 12.f);
        sd.ZI[iin] = static_cast<float>(sd.b[iin] * std::pow(sd.h[iin], 3) / 12.f);
    }
}

void KekakuanBatang(StructureData& sd) {
    sd.NB = 0;
    sd.IR = 0;
    sd.IC = 0;
    for (int ir = 1; ir <= sd.M; ++ir) {
        sd.R11[ir] = 0.f; sd.R12[ir] = 0.f; sd.R13[ir] = 0.f;
        sd.R21[ir] = 0.f; sd.R22[ir] = 0.f; sd.R23[ir] = 0.f;
        sd.R31[ir] = 0.f; sd.R32[ir] = 0.f; sd.R33[ir] = 0.f;
    }

    for (int ir = 1; ir <= sd.M; ++ir) {
        if (6 * (std::abs(sd.JK[ir] - sd.JJ[ir]) + 1) > sd.NB) {
            sd.NB = 6 * (std::abs(sd.JK[ir] - sd.JJ[ir]) + 1);
        }
        PeriksaBatang(sd, ir);
        if (sd.IA[ir] != 0) {
            sd.XPS = sd.XP[ir] - sd.X[sd.JJ[ir]];
            sd.YPS = sd.YP[ir] - sd.Y[sd.JJ[ir]];
            sd.ZPS = sd.ZP[ir] - sd.Z[sd.JJ[ir]];
        }

        if (sd.CXZ <= 0.001f) {
            sd.R11[ir] = 0.f;    sd.R12[ir] = sd.Cy; sd.R13[ir] = 0.f;
            sd.R21[ir] = -sd.Cy; sd.R22[ir] = 0.f;   sd.R23[ir] = 0.f;
            sd.R31[ir] = 0.f;    sd.R32[ir] = 0.f;   sd.R33[ir] = 1.f;
            if (sd.IA[ir] == 0) {
                continue;
            }
            sd.COSA = (-sd.XPS * sd.Cy) / std::sqrt(sd.XPS * sd.XPS + sd.ZPS * sd.ZPS);
            sd.SINA = (-sd.ZPS) / std::sqrt(sd.XPS * sd.XPS + sd.ZPS * sd.ZPS);
            sd.R21[ir] = -sd.Cy * sd.COSA;
            sd.R23[ir] = sd.SINA;
            sd.R31[ir] = sd.Cy * sd.SINA;
            sd.R33[ir] = sd.COSA;
            continue;
        }

        sd.R11[ir] = sd.CX;                    sd.R12[ir] = sd.Cy;             sd.R13[ir] = sd.CZ;
        sd.R21[ir] = (-sd.CX * sd.Cy) / sd.CXZ; sd.R22[ir] = sd.CXZ;           sd.R23[ir] = (-sd.Cy * sd.CZ) / sd.CXZ;
        sd.R31[ir] = (-sd.CZ) / sd.CXZ;         sd.R32[ir] = 0.f;              sd.R33[ir] = sd.CX / sd.CXZ;
        if (sd.IA[ir] == 0) {
            continue;
        }
        sd.YPG = sd.R21[ir] * sd.XPS + sd.R22[ir] * sd.YPS + sd.R23[ir] * sd.ZPS;
        sd.ZPG = sd.R31[ir] * sd.XPS + sd.R32[ir] * sd.YPS + sd.R33[ir] * sd.ZPS;
        sd.COSA = sd.YPG / std::sqrt(sd.YPG * sd.YPG + sd.ZPG * sd.ZPG);
        sd.SINA = sd.ZPG / std::sqrt(sd.YPG * sd.YPG + sd.ZPG * sd.ZPG);
        sd.R21[ir] = (-sd.CX * sd.Cy * sd.COSA - sd.CZ * sd.SINA) / sd.CXZ;
        sd.R22[ir] = sd.CXZ * sd.COSA;
        sd.R23[ir] = (-sd.Cy * sd.CZ * sd.COSA + sd.CX * sd.SINA) / sd.CXZ;
        sd.R31[ir] = (sd.CX * sd.Cy * sd.SINA - sd.CZ * sd.COSA) / sd.CXZ;
        sd.R32[ir] = -sd.CXZ * sd.SINA;
        sd.R33[ir] = (sd.Cy * sd.CZ * sd.SINA + sd.CX * sd.COSA) / sd.CXZ;
    }

    int N1 = 0;
    for (int ir = 1; ir <= sd.ND; ++ir) {
        N1 += sd.JRL[ir];
        if (sd.JRL[ir] <= 0) {
            sd.ID[ir] = ir - N1;
            continue;
        }
        sd.ID[ir] = sd.N + N1;
    }

    for (int ir = 1; ir <= sd.N; ++ir) {
        for (int jr = 1; jr <= sd.NB; ++jr) {
            sd.SFF[ir][jr] = 0.f;
        }
    }

    for (int ir = 1; ir <= sd.M; ++ir) {
        PeriksaBatang(sd, ir);
        IsiMatrikKekakuan(sd, ir);

        for (int jr = 1; jr <= 4; ++jr) {
            for (int kr = 3 * jr - 2; kr <= 12; ++kr) {
                sd.SMS[3 * jr - 2][kr] = sd.R11[ir] * sd.SMRT[3 * jr - 2][kr] +
                                          sd.R21[ir] * sd.SMRT[3 * jr - 1][kr] +
                                          sd.R31[ir] * sd.SMRT[3 * jr][kr];
                sd.SMS[3 * jr - 1][kr] = sd.R12[ir] * sd.SMRT[3 * jr - 2][kr] +
                                          sd.R22[ir] * sd.SMRT[3 * jr - 1][kr] +
                                          sd.R32[ir] * sd.SMRT[3 * jr][kr];
                sd.SMS[3 * jr][kr] = sd.R13[ir] * sd.SMRT[3 * jr - 2][kr] +
                                      sd.R23[ir] * sd.SMRT[3 * jr - 1][kr] +
                                      sd.R33[ir] * sd.SMRT[3 * jr][kr];
            }
        }

        IndeksBatang(sd, ir);

        for (int jr = 1; jr <= kMD; ++jr) {
            if (sd.JRL[sd.IM[jr]] == 0) {
                for (int kr = jr; kr <= kMD; ++kr) {
                    if (sd.JRL[sd.IM[kr]] == 0) {
                        sd.IR = sd.ID[sd.IM[jr]];
                        sd.IC = sd.ID[sd.IM[kr]];
                        if (sd.IR >= sd.IC) {
                            std::swap(sd.IR, sd.IC);
                        }
                        sd.IC = sd.IC - sd.IR + 1;
                        sd.SFF[sd.IR][sd.IC] = sd.SFF[sd.IR][sd.IC] + sd.SMS[jr][kr];
                    }
                }
            }
        }
    }
}

void Beban(StructureData& sd) {
    for (int ib = 1; ib <= sd.ND; ++ib) {
        sd.AC[sd.ID[ib]] = 0.f;
    }

    for (int ib = 1; ib <= sd.M; ++ib) {
        PeriksaBatang(sd, ib);
        if (sd.CXZ <= 0.001f) {
            continue;
        }
        IndeksBatang(sd, ib);
        for (int jb = 1; jb <= 4; ++jb) {
            sd.AE[sd.IM[3 * jb - 2]] = 0.f;
            sd.AE[sd.IM[3 * jb - 1]] = 0.f;
            sd.AE[sd.IM[3 * jb]] = 0.f;
        }
    }

    for (int ib = 1; ib <= sd.M; ++ib) {
        PeriksaBatang(sd, ib);
        if (sd.CXZ <= 0.001f) {
            continue;
        }
        IndeksBatang(sd, ib);
        for (int jb = 1; jb <= 4; ++jb) {
            sd.AE[sd.IM[3 * jb - 2]] = sd.AE[sd.IM[3 * jb - 2]] - sd.R11[ib] * sd.AML[3 * jb - 2][ib] -
                                        sd.R21[ib] * sd.AML[3 * jb - 1][ib] - sd.R31[ib] * sd.AML[3 * jb][ib];
            sd.AE[sd.IM[3 * jb - 1]] = sd.AE[sd.IM[3 * jb - 1]] - sd.R12[ib] * sd.AML[3 * jb - 2][ib] -
                                        sd.R22[ib] * sd.AML[3 * jb - 1][ib] - sd.R32[ib] * sd.AML[3 * jb][ib];
            sd.AE[sd.IM[3 * jb]] = sd.AE[sd.IM[3 * jb]] - sd.R13[ib] * sd.AML[3 * jb - 2][ib] -
                                    sd.R23[ib] * sd.AML[3 * jb - 1][ib] - sd.R33[ib] * sd.AML[3 * jb][ib];
        }
    }

    for (int ib = 1; ib <= sd.ND; ++ib) {
        sd.AC[sd.ID[ib]] = sd.AJ[ib] + sd.AE[ib];
    }
}

void BeratSendiri(StructureData& sd) {
    sd.js_balok = 0;
    sd.js_kolom = 0;
    for (int ibs = 1; ibs <= sd.M; ++ibs) {
        PeriksaBatang(sd, ibs);

        if (sd.CXZ > 0.001f) {
            sd.b[ibs] = Isi(sd.var_b[sd.no_struktur][0 + 12 * sd.js_balok], sd.sisi_d_B);
            sd.h[ibs] = Isi(sd.var_b[sd.no_struktur][1 + 12 * sd.js_balok], sd.sisi_d_H);
            sd.W_Balok[ibs] = 24000.f * sd.b[ibs] * sd.h[ibs] * 1.E-6f;
            sd.W[ibs] += sd.W_Balok[ibs];
            sd.AML[2][ibs] += sd.W_Balok[ibs] * sd.EL[ibs] / 2.f;
            sd.AML[6][ibs] = static_cast<float>(sd.AML[6][ibs] + sd.W_Balok[ibs] * std::pow(sd.EL[ibs], 2) / 12.f);
            sd.AML[8][ibs] += sd.W_Balok[ibs] * sd.EL[ibs] / 2.f;
            sd.AML[12][ibs] = static_cast<float>(sd.AML[12][ibs] + -sd.W_Balok[ibs] * std::pow(sd.EL[ibs], 2) / 12.f);
            sd.js_balok++;
        } else {
            sd.b[ibs] = Isi(sd.var_k[sd.no_struktur][0 + 5 * sd.js_kolom], sd.sisi_d_K);
            sd.h[ibs] = sd.b[ibs];
            sd.P_Kolom[ibs] = -24000.f * sd.b[ibs] * sd.h[ibs] * sd.EL[ibs] * 1.E-6f;
            sd.AJ[6 * sd.JJ[ibs] - 4] += sd.P_Kolom[ibs];
            sd.js_kolom++;
        }
    }
}

void Hasil(StructureData& sd) {
    int ihs = sd.N + 1;
    for (int jhs = 1; jhs <= sd.ND; ++jhs) {
        if (sd.JRL[sd.ND - jhs + 1] == 0) {
            ihs = ihs - 1;
            sd.DJ[sd.ND - jhs + 1] = 0.f;
        } else {
            sd.DJ[sd.ND - jhs + 1] = 0.f;
        }
    }

    ihs = sd.N + 1;
    for (int jhs = 1; jhs <= sd.ND; ++jhs) {
        if (sd.JRL[sd.ND - jhs + 1] == 0) {
            ihs = ihs - 1;
            sd.DJ[sd.ND - jhs + 1] = sd.DF[ihs];
        } else {
            sd.DJ[sd.ND - jhs + 1] = 0.f;
        }
    }

    for (int ih = 1; ih <= sd.M; ++ih) {
        PeriksaBatang(sd, ih);
        IsiMatrikKekakuan(sd, ih);
        IndeksBatang(sd, ih);

        for (int jh = 1; jh <= kMD; ++jh) {
            sd.AMD[jh] = 0.f;
            for (int kh = 1; kh <= kMD; ++kh) {
                sd.AMD[jh] = sd.AMD[jh] + sd.SMRT[jh][kh] * sd.DJ[sd.IM[kh]];
            }
            sd.AM[ih][jh] = sd.AML[jh][ih] + sd.AMD[jh];
        }

        PeriksaBatang(sd, ih);

        if (sd.CXZ > 0.001f) {
            sd.MLAP[ih] = static_cast<float>(-sd.AM[ih][6] + 0.125f * sd.W[ih] * std::pow(sd.EL[ih], 2));
            sd.MTUM_KI[ih] = -sd.AM[ih][6];
            sd.MTUM_KA[ih] = sd.AM[ih][12];
            sd.GESER_KI[ih] = sd.AM[ih][2];
            sd.GESER_KA[ih] = -sd.AM[ih][8];
        } else {
            sd.PK[ih] = std::fabs(sd.AM[ih][7]);
            sd.MKX[ih] = std::fabs(sd.AM[ih][10]);
            sd.MKY[ih] = std::fabs(sd.AM[ih][12]);
            sd.GK[ih] = (std::fabs(sd.AM[ih][8]) > std::fabs(sd.AM[ih][9])) ? std::fabs(sd.AM[ih][8])
                                                                             : std::fabs(sd.AM[ih][9]);
        }

        for (int jh = 1; jh <= 4; ++jh) {
            if (sd.JRL[sd.IM[3 * jh - 2]] == 1) sd.AR[sd.IM[3 * jh - 2]] = 0.f;
            if (sd.JRL[sd.IM[3 * jh - 1]] == 1) sd.AR[sd.IM[3 * jh - 1]] = 0.f;
            if (sd.JRL[sd.IM[3 * jh]] == 1) sd.AR[sd.IM[3 * jh]] = 0.f;
        }

        for (int jh = 1; jh <= 4; ++jh) {
            int J1 = 3 * jh - 2, J2 = 3 * jh - 1, J3 = 3 * jh;
            int I1 = sd.IM[J1], I2 = sd.IM[J2], I3 = sd.IM[J3];
            if (sd.JRL[I1] == 1) {
                sd.AR[I1] = sd.AR[I1] + sd.R11[ih] * sd.AMD[J1] + sd.R21[ih] * sd.AMD[J2] + sd.R31[ih] * sd.AMD[J3];
            }
            if (sd.JRL[I2] == 1) {
                sd.AR[I2] = sd.AR[I2] + sd.R12[ih] * sd.AMD[J1] + sd.R22[ih] * sd.AMD[J2] + sd.R32[ih] * sd.AMD[J3];
            }
            if (sd.JRL[I3] == 1) {
                sd.AR[I3] = sd.AR[I3] + sd.R13[ih] * sd.AMD[J1] + sd.R23[ih] * sd.AMD[J2] + sd.R33[ih] * sd.AMD[J3];
            }
        }
    }

    for (int ih = 1; ih <= sd.ND; ++ih) {
        if (sd.JRL[ih] == 0) {
            continue;
        }
        sd.AR[ih] = sd.AR[ih] - sd.AJ[ih] - sd.AE[ih];
    }
}

void Struktur(StructureData& sd) {
    KekakuanBatang(sd);
    BandedFactorize(sd.N, sd.NB, sd.SFF);
    Beban(sd);
    BandedSolve(sd.N, sd.NB, sd.SFF, sd.AC, sd.DF);
    Hasil(sd);
}

void IsiElemenBalokFields(StructureData& sd, int no_el_balok) {
    sd.B = Isi(sd.var_b[sd.no_struktur][0 + 12 * no_el_balok], sd.sisi_d_B);
    sd.H = Isi(sd.var_b[sd.no_struktur][1 + 12 * no_el_balok], sd.sisi_d_H);

    sd.DIA1lap = Isi(sd.var_b[sd.no_struktur][2 + 12 * no_el_balok], sd.DIA_d);
    sd.NL1lap = Isi(sd.var_b[sd.no_struktur][3 + 12 * no_el_balok], sd.NL_d);
    sd.DIA2lap = Isi(sd.var_b[sd.no_struktur][4 + 12 * no_el_balok], sd.DIA_d);
    sd.NL2lap = Isi(sd.var_b[sd.no_struktur][5 + 12 * no_el_balok], sd.NL_d);

    sd.DIA1tum = Isi(sd.var_b[sd.no_struktur][6 + 12 * no_el_balok], sd.DIA_d);
    sd.NL1tum = Isi(sd.var_b[sd.no_struktur][7 + 12 * no_el_balok], sd.NL_d);
    sd.DIA2tum = Isi(sd.var_b[sd.no_struktur][8 + 12 * no_el_balok], sd.DIA_d);
    sd.NL2tum = Isi(sd.var_b[sd.no_struktur][9 + 12 * no_el_balok], sd.NL_d);

    sd.DIAS = Isi(sd.var_b[sd.no_struktur][10 + 12 * no_el_balok], sd.DIAS_d);
    sd.Jarak_S = Isi(sd.var_b[sd.no_struktur][11 + 12 * no_el_balok], sd.JS_d);
    sd.L = sd.EL[sd.no_balok[no_el_balok]];

    if (std::fabs(sd.GESER_KI[sd.no_balok[no_el_balok]]) > std::fabs(sd.GESER_KA[sd.no_balok[no_el_balok]])) {
        sd.VU = std::fabs(sd.GESER_KI[sd.no_balok[no_el_balok]] / kTeta);
    } else {
        sd.VU = std::fabs(sd.GESER_KA[sd.no_balok[no_el_balok]] / kTeta);
    }
}

void ElemenLapangan(StructureData& sd, int no_el_balok) {
    sd.DIA1 = sd.DIA1lap;
    sd.NL1 = sd.NL1lap;
    sd.DIA2 = sd.DIA2lap;
    sd.NL2 = sd.NL2lap;
    sd.MU = std::fabs(sd.MLAP[sd.no_balok[no_el_balok]] / kTeta);
}

void ElemenTumpuan(StructureData& sd, int no_el_balok) {
    sd.DIA1 = sd.DIA1tum;
    sd.NL1 = sd.NL1tum;
    sd.DIA2 = sd.DIA2tum;
    sd.NL2 = sd.NL2tum;

    if (std::fabs(sd.MTUM_KI[sd.no_balok[no_el_balok]]) > std::fabs(sd.MTUM_KA[sd.no_balok[no_el_balok]])) {
        sd.MU = std::fabs(sd.MTUM_KI[sd.no_balok[no_el_balok]] / kTeta);
    } else {
        sd.MU = std::fabs(sd.MTUM_KA[sd.no_balok[no_el_balok]] / kTeta);
    }
}

void IsiElemenKolomFields(StructureData& sd, int no_el_kolom) {
    sd.sisi = Isi(sd.var_k[sd.no_struktur][0 + 5 * no_el_kolom], sd.sisi_d_K);
    sd.DIA = Isi(sd.var_k[sd.no_struktur][1 + 5 * no_el_kolom], sd.DIA_d);
    sd.N_DIA = Isi(sd.var_k[sd.no_struktur][2 + 5 * no_el_kolom], sd.NL_d);
    sd.DIAS = Isi(sd.var_k[sd.no_struktur][3 + 5 * no_el_kolom], sd.DIAS_d);
    sd.Jarak_S = Isi(sd.var_k[sd.no_struktur][4 + 5 * no_el_kolom], sd.JS_d);

    sd.PU = -sd.PK[sd.no_kolom[no_el_kolom]] / kTeta;
    sd.MUX = sd.MKX[sd.no_kolom[no_el_kolom]] / kTeta;
    sd.MUY = sd.MKY[sd.no_kolom[no_el_kolom]] / kTeta;
    sd.VU = std::fabs(sd.GK[sd.no_kolom[no_el_kolom]] / kTeta);
    sd.L = sd.EL[sd.no_kolom[no_el_kolom]];
}

} // namespace orcisf::engine
