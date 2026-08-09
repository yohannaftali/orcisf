#include "engine/Optimizer.h"

#include <chrono>
#include <cmath>
#include <random>

#include "engine/BeamDesign.h"
#include "engine/ColumnDesign.h"
#include "engine/CostConstraint.h"
#include "engine/Discretization.h"
#include "engine/StructuralAnalysis.h"

namespace orcisf::engine {

namespace {

// ---- Port of Penormalan.hpp ----

void Unnormalisasi(const StructureData& sd, const LegacyArray<int>& var_nor, LegacyArray<int>& var_bv,
                    LegacyArray<int>& var_kv) {
    int novar = 0;
    for (int isum = 0; isum < sd.jum_balok; ++isum) {
        for (int jsum = 0; jsum < 12; ++jsum) {
            var_bv[jsum + 12 * isum] = var_nor[novar];
            ++novar;
        }
    }
    for (int isum = 0; isum < sd.jum_kolom; ++isum) {
        for (int jsum = 0; jsum < 5; ++jsum) {
            var_kv[jsum + 5 * isum] = var_nor[novar];
            ++novar;
        }
    }
}

void NormalisasiFloat(const StructureData& sd, LegacyArray<float>& var_nor, const LegacyArray<float>& var_bv,
                       const LegacyArray<float>& var_kv) {
    int novar = 0;
    for (int isum = 0; isum < sd.jum_balok; ++isum) {
        for (int jsum = 0; jsum < 12; ++jsum) {
            var_nor[novar] = var_bv[jsum + 12 * isum];
            ++novar;
        }
    }
    for (int isum = 0; isum < sd.jum_kolom; ++isum) {
        for (int jsum = 0; jsum < 5; ++jsum) {
            var_nor[novar] = var_kv[jsum + 5 * isum];
            ++novar;
        }
    }
}

void NormalisasiInt(const StructureData& sd, LegacyArray<int>& var_nor, const LegacyArray<int>& var_bv,
                     const LegacyArray<int>& var_kv) {
    int novar = 0;
    for (int isum = 0; isum < sd.jum_balok; ++isum) {
        for (int jsum = 0; jsum < 12; ++jsum) {
            var_nor[novar] = var_bv[jsum + 12 * isum];
            ++novar;
        }
    }
    for (int isum = 0; isum < sd.jum_kolom; ++isum) {
        for (int jsum = 0; jsum < 5; ++jsum) {
            var_nor[novar] = var_kv[jsum + 5 * isum];
            ++novar;
        }
    }
}

// ---- Port of Pengacakan.hpp ----

// Legacy `random(n)`: uniform integer in [0, n). Not bit-reproducible
// against Borland's RNG (which also seeds from wall-clock time via
// `randomize()`, so even the *original* binary can't reproduce a past run)
// -- see issue #3's discussion. std::mt19937 gives the same *statistical*
// behavior with a modern, well-distributed generator.
struct Rng {
    std::mt19937 engine;
    explicit Rng(unsigned int seed) : engine(seed) {}
    int operator()(int n) {
        if (n <= 1) return 0;
        std::uniform_int_distribution<int> dist(0, n - 1);
        return dist(engine);
    }
};

void LoadBatasAtas(StructureData& sd) {
    for (int isinv = 0; isinv < sd.jum_balok; ++isinv) {
        sd.nvb[0 + 12 * isinv] = sd.nsisi_B;
        sd.nvb[1 + 12 * isinv] = sd.nsisi_H;
        sd.nvb[2 + 12 * isinv] = sd.nDIA;
        sd.nvb[3 + 12 * isinv] = sd.nNL;
        sd.nvb[4 + 12 * isinv] = sd.nDIA;
        sd.nvb[5 + 12 * isinv] = sd.nNL;
        sd.nvb[6 + 12 * isinv] = sd.nDIA;
        sd.nvb[7 + 12 * isinv] = sd.nNL;
        sd.nvb[8 + 12 * isinv] = sd.nDIA;
        sd.nvb[9 + 12 * isinv] = sd.nNL;
        sd.nvb[10 + 12 * isinv] = sd.nDIAS;
        sd.nvb[11 + 12 * isinv] = sd.nJS;
    }
    for (int isinv = 0; isinv < sd.jum_kolom; ++isinv) {
        sd.nvk[0 + 5 * isinv] = sd.nsisi_K;
        sd.nvk[1 + 5 * isinv] = sd.nDIA;
        sd.nvk[2 + 5 * isinv] = sd.nNL;
        sd.nvk[3 + 5 * isinv] = sd.nDIAS;
        sd.nvk[4 + 5 * isinv] = sd.nJS;
    }
}

void CariStrukturAwal(StructureData& sd) {
    for (int iran = 0; iran < sd.jum_balok; ++iran) {
        for (int nv_b = 0; nv_b < 12; ++nv_b) {
            sd.var_b[sd.no_struktur][nv_b + 12 * iran] = sd.nvb[nv_b] - 1;
            if (nv_b == 5 || nv_b == 9 || nv_b == 11) {
                sd.var_b[sd.no_struktur][nv_b + 12 * iran] = 0;
            }
        }
    }
    for (int iran = 0; iran < sd.jum_kolom; ++iran) {
        for (int nv_k = 0; nv_k < 5; ++nv_k) {
            sd.var_k[sd.no_struktur][nv_k + 5 * iran] = sd.nvk[nv_k] - 1;
        }
    }
}

void Randomisasi(StructureData& sd, Rng& rng) {
    for (int iran = 0; iran < sd.jum_balok; ++iran) {
        for (int nv_b = 0; nv_b < 12; ++nv_b) {
            sd.var_b[sd.no_struktur][nv_b + 12 * iran] = rng(sd.nvb[nv_b]);
        }
    }
    for (int iran = 0; iran < sd.jum_kolom; ++iran) {
        for (int nv_k = 0; nv_k < 5; ++nv_k) {
            sd.var_k[sd.no_struktur][nv_k + 5 * iran] = rng(sd.nvk[nv_k]);
        }
    }
}

void AcakVariabel(StructureData& sd, Rng& rng) {
    LoadBatasAtas(sd);
    for (int iav = 0; iav < sd.JSTD; ++iav) {
        sd.no_struktur = iav;
        if (iav == 1) {
            CariStrukturAwal(sd);
            continue;
        }
        Randomisasi(sd, rng);
    }
}

// ---- Shared per-candidate evaluation body (Polyhedron.hpp's inline
// first-generation / end-of-generation loop body). See CostConstraint.h's
// header comment for why this deliberately differs from KendalaHarga(). ----

void EvaluateCandidateFull(StructureData& sd, int no_struktur) {
    sd.no_struktur = no_struktur;
    sd.kendalastr[no_struktur] = 0.f;
    sd.hargastr[no_struktur] = 0.f;
    sd.fitstr[no_struktur] = 0.f;

    Inersia(sd);
    Struktur(sd);

    for (int jop = 0; jop < sd.jum_balok; ++jop) {
        PeriksaBatang(sd, sd.no_balok[jop]);
        IsiElemenBalokFields(sd, jop);
        ElemenLapangan(sd, jop);
        DesignBeam(sd);

        // Adaptive stirrup tightening -- present here (and in the kolom
        // loop below), but *not* in KendalaHarga()'s beam handling. See
        // CostConstraint.h.
        for (int cari_S = sd.nvb[11 + 12 * jop] - 1; cari_S > 0; --cari_S) {
            if (Isi(cari_S, sd.JS_d) <= sd.Sref) {
                sd.var_b[no_struktur][11 + 12 * jop] = cari_S;
                sd.Jarak_S = Isi(cari_S, sd.JS_d);
                break;
            }
        }
        DesignBeam(sd);

        sd.kendalastr[no_struktur] += sd.kendala;
        sd.hargastr[no_struktur] += sd.harga;

        Lendutan(sd, sd.no_balok[jop]);
        sd.kendalastr[no_struktur] += sd.kendala_lendutan;

        ElemenTumpuan(sd, jop);
        DesignBeam(sd);
        sd.kendalastr[no_struktur] += sd.kendala;
        sd.hargastr[no_struktur] += sd.harga;
    }

    for (int jop = 0; jop < sd.jum_kolom; ++jop) {
        PeriksaBatang(sd, sd.no_kolom[jop]);
        IsiElemenKolomFields(sd, jop);
        DesignColumn(sd);

        for (int cari_S = sd.nvk[4 + 5 * jop] - 1; cari_S > 0; --cari_S) {
            if (Isi(cari_S, sd.JS_d) <= sd.Sref) {
                sd.var_k[no_struktur][4 + 5 * jop] = cari_S;
                sd.Jarak_S = Isi(cari_S, sd.JS_d);
                break;
            }
        }
        DesignColumn(sd);

        if (sd.kendala_kelangsingan > 0) {
            sd.L = sd.EL[sd.no_kolom[jop]];
            float sisi_baru = 0.75f * sd.L * 1000.f / (22.f * std::sqrt(1.f / 12.f));
            for (int cari_sisi = sd.var_k[no_struktur][0 + 5 * jop]; cari_sisi < sd.nvk[0 + 5 * jop]; ++cari_sisi) {
                float sisi_cari = Isi(cari_sisi, sd.sisi_d_K);
                if (sisi_cari > sisi_baru) {
                    sd.var_k[no_struktur][0 + 5 * jop] = cari_sisi;
                    sd.sisi = Isi(cari_sisi, sd.sisi_d_K);
                    break;
                }
            }
            DesignColumn(sd);
        }

        sd.kendalastr[no_struktur] += sd.kendala;
        sd.hargastr[no_struktur] += sd.harga;
    }

    sd.fitstr[no_struktur] = sd.finalti / (sd.hargastr[no_struktur] + sd.finalti * sd.kendalastr[no_struktur]);
}

// ---- Port of Pengurutan.hpp's bubble sort ----

void Sort(StructureData& sd, int n_array) {
    for (int isort = 1; isort < n_array; ++isort) {
        for (int jsort = 0; jsort <= isort; ++jsort) {
            if (sd.fitstr[isort] < sd.fitstr[jsort]) {
                double dummy1 = sd.fitstr[isort];
                float dummy2 = sd.kendalastr[isort];
                float dummy3 = sd.hargastr[isort];
                LegacyArray<int> dummyvarb, dummyvark;
                for (int dvb = 0; dvb < sd.jum_balok; ++dvb) {
                    for (int dnv_b = 0; dnv_b < 12; ++dnv_b) {
                        dummyvarb[dnv_b + 12 * dvb] = sd.var_b[isort][dnv_b + 12 * dvb];
                    }
                }
                for (int dvk = 0; dvk < sd.jum_kolom; ++dvk) {
                    for (int dnv_k = 0; dnv_k < 5; ++dnv_k) {
                        dummyvark[dnv_k + 5 * dvk] = sd.var_k[isort][dnv_k + 5 * dvk];
                    }
                }

                sd.fitstr[isort] = sd.fitstr[jsort];
                sd.kendalastr[isort] = sd.kendalastr[jsort];
                sd.hargastr[isort] = sd.hargastr[jsort];
                for (int dvb = 0; dvb < sd.jum_balok; ++dvb) {
                    for (int dnv_b = 0; dnv_b < 12; ++dnv_b) {
                        sd.var_b[isort][dnv_b + 12 * dvb] = sd.var_b[jsort][dnv_b + 12 * dvb];
                    }
                }
                for (int dvk = 0; dvk < sd.jum_kolom; ++dvk) {
                    for (int dnv_k = 0; dnv_k < 5; ++dnv_k) {
                        sd.var_k[isort][dnv_k + 5 * dvk] = sd.var_k[jsort][dnv_k + 5 * dvk];
                    }
                }

                sd.fitstr[jsort] = static_cast<float>(dummy1);
                sd.kendalastr[jsort] = dummy2;
                sd.hargastr[jsort] = dummy3;
                for (int dvb = 0; dvb < sd.jum_balok; ++dvb) {
                    for (int dnv_b = 0; dnv_b < 12; ++dnv_b) {
                        sd.var_b[jsort][dnv_b + 12 * dvb] = dummyvarb[dnv_b + 12 * dvb];
                    }
                }
                for (int dvk = 0; dvk < sd.jum_kolom; ++dvk) {
                    for (int dnv_k = 0; dnv_k < 5; ++dnv_k) {
                        sd.var_k[jsort][dnv_k + 5 * dvk] = dummyvark[dnv_k + 5 * dvk];
                    }
                }
            }
        }
    }
}

// ---- Port of Telusur.hpp ----

void Penelusuran(StructureData& sd) {
    LegacyArray<float> sum_bestgood_b, sum_bestgood_k;

    for (int nbgw = 1; nbgw < sd.JSTD; ++nbgw) {
        for (int isum = 0; isum < sd.jum_balok; ++isum) {
            for (int jsum = 0; jsum < 12; ++jsum) {
                sum_bestgood_b[jsum + 12 * isum] += sd.var_b[nbgw][jsum + 12 * isum];
            }
        }
        for (int isum = 0; isum < sd.jum_kolom; ++isum) {
            for (int jsum = 0; jsum < 5; ++jsum) {
                sum_bestgood_k[jsum + 5 * isum] += sd.var_k[nbgw][jsum + 5 * isum];
            }
        }
    }

    for (int isum = 0; isum < sd.jum_balok; ++isum) {
        for (int jsum = 0; jsum < 12; ++jsum) {
            sd.XM_b[jsum + 12 * isum] = sum_bestgood_b[jsum + 12 * isum] / (sd.JSTD - 1.f);
        }
    }
    for (int isum = 0; isum < sd.jum_kolom; ++isum) {
        for (int jsum = 0; jsum < 5; ++jsum) {
            sd.XM_k[jsum + 5 * isum] = sum_bestgood_k[jsum + 5 * isum] / (sd.JSTD - 1.f);
        }
    }

    for (int isum = 0; isum < sd.jum_balok; ++isum) {
        for (int jsum = 0; jsum < 12; ++jsum) {
            int idx = jsum + 12 * isum;
            sd.XS_b[idx] = sd.XM_b[idx] - sd.var_b[0][idx];
            if (sd.XS_b[idx] > 0.f) {
                sd.tanda_arah_b[idx] = 1;
            } else if (sd.XS_b[idx] < 0.f) {
                sd.tanda_arah_b[idx] = -1;
            } else {
                sd.tanda_arah_b[idx] = 0;
            }
        }
    }
    for (int isum = 0; isum < sd.jum_kolom; ++isum) {
        for (int jsum = 0; jsum < 5; ++jsum) {
            int idx = jsum + 5 * isum;
            sd.XS_k[idx] = sd.XM_k[idx] - sd.var_k[0][idx];
            if (sd.XS_k[idx] > 0.f) {
                sd.tanda_arah_k[idx] = 1;
            } else if (sd.XS_k[idx] < 0.f) {
                sd.tanda_arah_k[idx] = -1;
            } else {
                sd.tanda_arah_k[idx] = 0;
            }
        }
    }

    for (int isum = 0; isum < sd.jum_balok; ++isum) {
        for (int jsum = 0; jsum < 12; ++jsum) {
            sd.var_b_jelek[jsum + 12 * isum] = sd.var_b[0][jsum + 12 * isum];
        }
    }
    for (int isum = 0; isum < sd.jum_kolom; ++isum) {
        for (int jsum = 0; jsum < 5; ++jsum) {
            sd.var_k_jelek[jsum + 5 * isum] = sd.var_k[0][jsum + 5 * isum];
        }
    }

    for (int isum = 0; isum < sd.JVD; ++isum) {
        sd.TM[isum] = 0.f;
        sd.TS[isum] = 0.f;
        sd.varnew_asli[isum] = 0;
        sd.arah[isum] = 0;
    }
    NormalisasiFloat(sd, sd.TM, sd.XM_b, sd.XM_k);
    NormalisasiFloat(sd, sd.TS, sd.XS_b, sd.XS_k);
    NormalisasiInt(sd, sd.arah, sd.tanda_arah_b, sd.tanda_arah_k);

    sd.no_TS_terjauh = 0;
    for (int cari_ts = 1; cari_ts < sd.JVD; ++cari_ts) {
        if (std::fabs(sd.TS[cari_ts]) > std::fabs(sd.TS[sd.no_TS_terjauh])) {
            sd.no_TS_terjauh = cari_ts;
        }
    }
}

// ---- Port of Baru.hpp ----

void PeriksaBatas(StructureData& sd) {
    for (int ipb = 0; ipb < sd.JVD; ++ipb) {
        if (sd.varnew[ipb] > sd.nvm[ipb] - 1) {
            sd.varnew[ipb] = sd.nvm[ipb] - 1;
        }
        if (sd.varnew[ipb] < 0) {
            sd.varnew[ipb] = 0;
        }
    }
}

void CariBaru(StructureData& sd) {
    sd.iterasi_var = 0;
    NormalisasiInt(sd, sd.varnew_asli, sd.var_b_jelek, sd.var_k_jelek);
    sd.fitcb_best = 0.f;

    do {
        sd.varplus[sd.no_TS_terjauh] = sd.iterasi_var + 1.f;
        sd.varnew_f[sd.no_TS_terjauh] =
            sd.varnew_asli[sd.no_TS_terjauh] + sd.varplus[sd.no_TS_terjauh] * sd.arah[sd.no_TS_terjauh];

        for (int icb = 0; icb < sd.JVD; ++icb) {
            if (icb == sd.no_TS_terjauh) {
                continue;
            }
            if (sd.TS[sd.no_TS_terjauh] != 0) {
                sd.varplus[icb] = std::fabs(sd.TS[icb] / sd.TS[sd.no_TS_terjauh] * sd.varplus[sd.no_TS_terjauh]);
                sd.varnew_f[icb] = sd.varnew_asli[icb] + sd.varplus[icb] * sd.arah[icb];
            } else {
                sd.varnew_f[icb] = sd.varnew_asli[icb] + (sd.iterasi_var + 1.f) * sd.arah[icb];
            }
        }

        for (int icb = 0; icb < sd.JVD; ++icb) {
            sd.varnew[icb] = Konversi(sd.varnew_f[icb]);
        }

        sd.lompat = 0;
        for (int ilp = 0; ilp < sd.JVD; ++ilp) {
            if (sd.varnew[ilp] == sd.TM[ilp]) {
                if (ilp == sd.JVD - 1) {
                    sd.lompat = 1;
                }
            } else {
                sd.lompat = 0;
                break;
            }
        }

        if (sd.lompat == 1) {
            sd.lompat = 0;
            sd.iterasi_var++;
            continue;
        }

        PeriksaBatas(sd);
        Unnormalisasi(sd, sd.varnew, sd.var_b_cb, sd.var_k_cb);

        sd.fitcb = KendalaHarga(sd, sd.var_b_cb, sd.var_k_cb);

        if (sd.fitcb > sd.fitcb_best) {
            sd.fitcb_best = sd.fitcb;
            for (int icb = 0; icb < sd.jum_balok; ++icb) {
                for (int jcb = 0; jcb < 12; ++jcb) {
                    sd.var_b_cb_best[jcb + 12 * icb] = sd.var_b_cb[jcb + 12 * icb];
                }
            }
            for (int icb = 0; icb < sd.jum_kolom; ++icb) {
                for (int jcb = 0; jcb < 5; ++jcb) {
                    sd.var_k_cb_best[jcb + 5 * icb] = sd.var_k_cb[jcb + 5 * icb];
                }
            }
        }
        sd.iterasi_var++;
    } while (sd.iterasi_var < std::fabs(sd.TS[sd.no_TS_terjauh]) * 3);
}

void GantiBaru(StructureData& sd) {
    for (int igbar = 0; igbar < sd.jum_balok; ++igbar) {
        for (int jgbar = 0; jgbar < 12; ++jgbar) {
            sd.var_b[0][jgbar + 12 * igbar] = sd.var_b_cb_best[jgbar + 12 * igbar];
        }
    }
    for (int igbar = 0; igbar < sd.jum_kolom; ++igbar) {
        for (int jgbar = 0; jgbar < 5; ++jgbar) {
            sd.var_k[0][jgbar + 5 * igbar] = sd.var_k_cb_best[jgbar + 5 * igbar];
        }
    }
}

void Penyusutan(StructureData& sd) {
    for (int nkon = 0; nkon < sd.JSTD - 1; ++nkon) {
        for (int igbar = 0; igbar < sd.jum_balok; ++igbar) {
            for (int jgbar = 0; jgbar < 12; ++jgbar) {
                int idx = jgbar + 12 * igbar;
                sd.var_b[nkon][idx] = Konversi(0.5f * (sd.var_b[nkon][idx] + sd.var_b[sd.JSTD - 1][idx]));
            }
        }
        for (int igbar = 0; igbar < sd.jum_kolom; ++igbar) {
            for (int jgbar = 0; jgbar < 5; ++jgbar) {
                int idx = jgbar + 5 * igbar;
                sd.var_k[nkon][idx] = Konversi(0.5f * (sd.var_k[nkon][idx] + sd.var_k[sd.JSTD - 1][idx]));
            }
        }
    }
}

} // namespace

void PrepareOptimization(StructureData& sd, const OptimizationOptions& options) {
    sd.harga_beton = options.harga_beton;
    sd.harga_besi = options.harga_besi;
    sd.selimut_kolom = options.selimut_kolom;
    sd.selimut_balok = options.selimut_balok;
    sd.finalti = options.finalti;
    sd.j_iterasi_mak = options.j_iterasi_mak;
    sd.fak_plus = options.fak_plus;
    sd.fak_kali = options.fak_kali;

    sd.jum_balok = 0;
    sd.jum_kolom = 0;
    for (int iop = 1; iop <= sd.M; ++iop) {
        PeriksaBatang(sd, iop);
        if (sd.CXZ > 0.001f) {
            sd.no_balok[sd.jum_balok] = iop;
            sd.jum_balok++;
        } else {
            sd.no_kolom[sd.jum_kolom] = iop;
            sd.jum_kolom++;
        }
    }

    sd.JVD = 12 * sd.jum_balok + 5 * sd.jum_kolom;
    sd.JSTD = sd.JVD * sd.fak_kali + sd.fak_plus;
}

void RunOptimization(StructureData& sd, const OptimizationOptions& options, const ProgressCallback& on_progress,
                      const DetailLogCallback& on_detail, const std::atomic<bool>* cancel) {
    (void)options; // worker_threads is issue #4's scope; single-threaded here (see Optimizer.h).

    Rng rng(std::random_device{}());
    AcakVariabel(sd, rng);

    for (int iop = 0; iop < sd.JSTD; ++iop) {
        EvaluateCandidateFull(sd, iop);
    }
    Sort(sd, sd.JSTD);
    if (on_detail) {
        on_detail(0, sd);
    }

    auto t_awal = std::chrono::steady_clock::now();
    int generasi = 1;
    sd.jum_susut = 0;
    bool converged = false;

    do {
        Penelusuran(sd);
        CariBaru(sd);

        if (sd.fitcb_best > sd.fitstr[0]) {
            sd.jum_susut = 0;
            GantiBaru(sd);
        } else {
            Penyusutan(sd);
            sd.jum_susut++;
            if (sd.jum_susut == 1) {
                sd.patok_fit = sd.fitstr[sd.JSTD - 1];
            }
        }

        EvaluateCandidateFull(sd, 0);
        Sort(sd, sd.JSTD);

        if (on_detail) {
            on_detail(generasi, sd);
        }

        auto now = std::chrono::steady_clock::now();
        ProgressInfo info;
        info.generation = generasi;
        info.max_generation = sd.j_iterasi_mak;
        info.best_fitness = sd.fitstr[sd.JSTD - 1];
        info.best_harga = sd.hargastr[sd.JSTD - 1];
        info.best_kendala = sd.kendalastr[sd.JSTD - 1];
        info.elapsed_seconds = std::chrono::duration<double>(now - t_awal).count();

        if (sd.fitstr[sd.JSTD - 1] == sd.fitstr[sd.JSTD - sd.JVD - 1]) {
            info.converged = true;
            converged = true;
            if (on_progress) on_progress(info);
            break;
        }

        if (sd.jum_susut == sd.JSTD) {
            if (sd.patok_fit == sd.fitstr[sd.JSTD - 1]) {
                info.converged = true;
                converged = true;
                if (on_progress) on_progress(info);
                break;
            }
            sd.jum_susut = 0;
        }

        if (on_progress) {
            if (!on_progress(info)) {
                break; // caller requested stop
            }
        }
        if (cancel && cancel->load()) {
            break;
        }

        ++generasi;
    } while (generasi < sd.j_iterasi_mak);

    (void)converged;
}

} // namespace orcisf::engine
