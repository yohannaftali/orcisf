#include "engine/Optimizer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <stdexcept>
#include <thread>
#include <utility>

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

// Issue #16: copies a previous run's best design vector into the current
// slot verbatim (no RNG draw) -- the seed is trusted to already contain
// valid discrete-table indices for this dataset's bounds since the size
// check in AcakVariabel() below only passes when jum_balok/jum_kolom
// (hence the layout) matches exactly.
void SeedStrukturAwal(StructureData& sd, const std::vector<int>& seed_var_b, const std::vector<int>& seed_var_k) {
    for (size_t i = 0; i < seed_var_b.size(); ++i) {
        sd.var_b[sd.no_struktur][static_cast<int>(i)] = seed_var_b[i];
    }
    for (size_t i = 0; i < seed_var_k.size(); ++i) {
        sd.var_k[sd.no_struktur][static_cast<int>(i)] = seed_var_k[i];
    }
}

void AcakVariabel(StructureData& sd, Rng& rng, const OptimizationOptions& options) {
    LoadBatasAtas(sd);

    // See OptimizationOptions::seed_from_previous_best's comment -- a size
    // mismatch means the seed was captured against a differently-shaped
    // dataset (edited geometry) and is silently ignored here.
    bool use_seed = options.seed_from_previous_best &&
                     static_cast<int>(options.seed_var_b.size()) == 12 * sd.jum_balok &&
                     static_cast<int>(options.seed_var_k.size()) == 5 * sd.jum_kolom;

    for (int iav = 0; iav < sd.JSTD; ++iav) {
        sd.no_struktur = iav;
        if (iav == 1) {
            CariStrukturAwal(sd);
            continue;
        }
        if (iav == 0 && use_seed) {
            SeedStrukturAwal(sd, options.seed_var_b, options.seed_var_k);
            continue;
        }
        Randomisasi(sd, rng);
    }
}

// Issue #77: forces every beam's tumpuan bar slots (indices 6-9: DIA1tum/
// NL1tum/DIA2tum/NL2tum) to mirror its lapangan slots (indices 2-5:
// DIA1lap/NL1lap/DIA2lap/NL2lap) in-place. `Arr` is either
// `LegacyArray<int>&` (sd.var_b[no_struktur]) or `int*` (var_b_cb, the
// trial-search scratch buffer) -- both support operator[]. Deliberately a
// pure function of (arr, jum_balok): calling it twice on values that
// already match is a no-op, which is what lets #78's snap below run after
// this without breaking the mirror (see OptimizationOptions::
// symmetric_beam_reinforcement's header comment).
template <typename Arr>
void MirrorBeamTumpuan(Arr& var_b, int jum_balok) {
    for (int i = 0; i < jum_balok; ++i) {
        var_b[6 + 12 * i] = var_b[2 + 12 * i];
        var_b[7 + 12 * i] = var_b[3 + 12 * i];
        var_b[8 + 12 * i] = var_b[4 + 12 * i];
        var_b[9 + 12 * i] = var_b[5 + 12 * i];
    }
}

// Issue #78: nearest table index to `idx` (within [0, count)) whose
// resolved value is >= 4, and -- when `even_only` -- also even. Assumes
// `table` is ascending by value (the convention every discrete table in
// this port already relies on, e.g. the adaptive stirrup-tightening loop
// in EvaluateCandidateFull below counting `cari_S` down while comparing
// values). Linear scan is deliberate: these tables are always small
// (typically well under 20 entries), and this runs on a hot path
// (EvaluateTrial, once per search trial per generation), so simplicity
// beats a binary-search-with-eligibility-mask that would only pay off on
// tables orders of magnitude larger than any real dataset uses. Falls
// back to the highest-value eligible index if `idx` itself is out of
// range, and to index 0 if the table has no entry >= 4 at all (a
// malformed/too-small discrete table -- not this function's job to
// validate, just to not crash).
int SnapBarCountIndex(const LegacyArray<float>& table, int count, int idx, bool even_only) {
    auto eligible = [&](int i) {
        float v = Isi(i, table);
        if (v < 4.f) return false;
        if (!even_only) return true;
        int iv = static_cast<int>(v + 0.5f);
        return iv % 2 == 0;
    };
    if (idx >= 0 && idx < count && eligible(idx)) return idx;

    int best = -1;
    int best_dist = count + 1;
    for (int i = 0; i < count; ++i) {
        if (!eligible(i)) continue;
        int dist = (i > idx) ? (i - idx) : (idx - i);
        if (dist < best_dist) {
            best_dist = dist;
            best = i;
        }
    }
    return best >= 0 ? best : 0;
}

// Applies #78's min-4 (and, when `even_only`, even-only) bar-count snap
// to every beam bar-count slot (3/5/7/9, all reading the shared NL_d
// table -- see AGENTS.md's "Discrete design variables") and every column
// bar-count slot (2). Unlike symmetric_beam_reinforcement's mirror, this
// always runs (min-4 applies regardless of the option) -- only the
// evenness half is conditional on `even_only`.
template <typename ArrB, typename ArrK>
void SnapBarCounts(const StructureData& sd, ArrB& var_b, int jum_balok, ArrK& var_k, int jum_kolom, bool even_only) {
    for (int i = 0; i < jum_balok; ++i) {
        for (int slot : {3, 5, 7, 9}) {
            int& v = var_b[slot + 12 * i];
            v = SnapBarCountIndex(sd.NL_d, sd.nNL, v, even_only);
        }
    }
    for (int i = 0; i < jum_kolom; ++i) {
        int& v = var_k[2 + 5 * i];
        v = SnapBarCountIndex(sd.NL_d, sd.nNL, v, even_only);
    }
}

// ---- Shared per-candidate evaluation body (Polyhedron.hpp's inline
// first-generation / end-of-generation loop body). See CostConstraint.h's
// header comment for why this deliberately differs from KendalaHarga(). ----

void EvaluateCandidateFull(StructureData& sd, int no_struktur, const OptimizationOptions& options) {
    sd.no_struktur = no_struktur;
    // LegacyArray2D::operator[] returns a temporary `int*` prvalue, which
    // can't bind directly to Arr& below -- name it first so it's an
    // lvalue the template can take by reference.
    int* row_b = sd.var_b[no_struktur];
    int* row_k = sd.var_k[no_struktur];
    if (options.symmetric_beam_reinforcement) {
        MirrorBeamTumpuan(row_b, sd.jum_balok);
    }
    SnapBarCounts(sd, row_b, sd.jum_balok, row_k, sd.jum_kolom, options.even_bar_count_only);
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

// One trial of cari_baru()'s search loop, extracted so it can be driven
// either sequentially (CariBaru, below) or in parallel across independent
// `iterasi_var` values (CariBaruParallel). Each `iterasi_var` maps
// deterministically to one candidate (varnew_f/varnew depend only on
// iterasi_var, sd.varnew_asli/arah/TS/no_TS_terjauh, none of which change
// across trials within one cari_baru() call), so trials have no
// inter-iteration data dependency -- the only thing that made the
// original loop "sequential" was accumulating a single running best,
// which is a plain reduction. `valid=false` mirrors the legacy `lompat`
// early-`continue` (candidate identical to the midpoint TM): the caller
// must not treat it as a candidate for the running best.
struct TrialResult {
    bool valid = false;
    float fitcb = 0.f;
};

TrialResult EvaluateTrial(StructureData& sd, int iterasi_var, const OptimizationOptions& options) {
    sd.varplus[sd.no_TS_terjauh] = iterasi_var + 1.f;
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
            sd.varnew_f[icb] = sd.varnew_asli[icb] + (iterasi_var + 1.f) * sd.arah[icb];
        }
    }

    for (int icb = 0; icb < sd.JVD; ++icb) {
        sd.varnew[icb] = Konversi(sd.varnew_f[icb]);
    }

    bool lompat = true;
    for (int ilp = 0; ilp < sd.JVD; ++ilp) {
        if (sd.varnew[ilp] != sd.TM[ilp]) {
            lompat = false;
            break;
        }
    }
    if (lompat) {
        return {};
    }

    PeriksaBatas(sd);
    Unnormalisasi(sd, sd.varnew, sd.var_b_cb, sd.var_k_cb);
    if (options.symmetric_beam_reinforcement) {
        MirrorBeamTumpuan(sd.var_b_cb, sd.jum_balok);
    }
    SnapBarCounts(sd, sd.var_b_cb, sd.jum_balok, sd.var_k_cb, sd.jum_kolom, options.even_bar_count_only);
    float fitcb = KendalaHarga(sd, sd.var_b_cb, sd.var_k_cb);
    return {true, fitcb};
}

// Legacy trial count: `do{...iterasi_var++;}while(iterasi_var<X)` (X =
// fabs(TS[no_TS_terjauh])*3) runs iterasi_var = 0,1,...,ceil(X)-1 (or
// exactly once if X<=0, since a do-while always runs its body once before
// the first check).
int TrialCount(const StructureData& sd) {
    float x = std::fabs(sd.TS[sd.no_TS_terjauh]) * 3.f;
    return std::max(1, static_cast<int>(std::ceil(x)));
}

void CariBaru(StructureData& sd, const OptimizationOptions& options) {
    sd.iterasi_var = 0;
    NormalisasiInt(sd, sd.varnew_asli, sd.var_b_jelek, sd.var_k_jelek);
    sd.fitcb_best = 0.f;

    int n_trials = TrialCount(sd);
    for (int iterasi_var = 0; iterasi_var < n_trials; ++iterasi_var) {
        sd.iterasi_var = iterasi_var;
        TrialResult result = EvaluateTrial(sd, iterasi_var, options);
        if (result.valid && result.fitcb > sd.fitcb_best) {
            sd.fitcb_best = result.fitcb;
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
    }
    sd.iterasi_var = n_trials;
}

// ---- Parallel evaluation (issue #4) ----
//
// `workers` holds `worker_threads - 1` extra StructureData clones (the
// caller's `sd` itself is always lane 0). Every analysis/design function
// mutates shared scratch state on its StructureData argument (SFF, SM,
// SMRT, AM, DF, the per-element B/H/DIA*/sisi/... scalars, kendala_*,
// harga, ...), so two candidates cannot be evaluated concurrently against
// the *same* StructureData without racing -- hence one full private copy
// per lane. A plain `StructureData` copy (all members are std::vector-
// backed) is a correct, if not maximally cache-friendly, way to give each
// lane an independent, consistent snapshot of the read-only problem
// definition (geometry/loads/discrete tables) and the current population/
// search state; at ~10-15 MB per copy this is negligible next to the
// per-generation trial-evaluation work it unlocks.

// Splits `count` independent items [0, count) across `1 + workers.size()`
// lanes (lane 0 = the calling thread using `sd` directly, lanes 1.. =
// worker threads using their own clone), running `work(laneStructureData,
// item)` for each item, then joins every thread before returning. Returns
// the per-lane [lo,hi) ranges actually used (ranges[0] is lane 0's,
// handled by the calling thread) so the caller can merge results back
// knowing exactly which lane computed which items.
template <typename Work>
std::vector<std::pair<int, int>> RunOnLanes(StructureData& sd, std::vector<StructureData>& workers, int count,
                                             Work&& work) {
    int n_lanes = 1 + static_cast<int>(workers.size());
    n_lanes = std::max(1, std::min(n_lanes, count));

    int chunk = (count + n_lanes - 1) / n_lanes;
    std::vector<std::pair<int, int>> ranges(static_cast<size_t>(n_lanes));
    for (int lane = 0; lane < n_lanes; ++lane) {
        int lo = lane * chunk;
        int hi = std::min(count, lo + chunk);
        ranges[static_cast<size_t>(lane)] = {lo, hi};
    }

    if (n_lanes <= 1) {
        for (int i = ranges[0].first; i < ranges[0].second; ++i) {
            work(sd, i);
        }
        return ranges;
    }

    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(n_lanes) - 1);
    for (int lane = 1; lane < n_lanes; ++lane) {
        auto [lo, hi] = ranges[static_cast<size_t>(lane)];
        StructureData& local = workers[static_cast<size_t>(lane) - 1];
        threads.emplace_back([&work, &local, lo, hi]() {
            for (int i = lo; i < hi; ++i) {
                work(local, i);
            }
        });
    }
    for (int i = ranges[0].first; i < ranges[0].second; ++i) {
        work(sd, i);
    }
    for (auto& th : threads) {
        th.join();
    }
    return ranges;
}

// Copies one candidate slot's design variables + evaluation results from
// `src` to `dst` (used to merge a worker lane's results back into the
// master after RunOnLanes joins).
void CopyCandidateSlot(StructureData& dst, const StructureData& src, int slot) {
    dst.fitstr[slot] = src.fitstr[slot];
    dst.kendalastr[slot] = src.kendalastr[slot];
    dst.hargastr[slot] = src.hargastr[slot];
    for (int b = 0; b < dst.jum_balok; ++b) {
        for (int j = 0; j < 12; ++j) {
            dst.var_b[slot][j + 12 * b] = src.var_b[slot][j + 12 * b];
        }
    }
    for (int k = 0; k < dst.jum_kolom; ++k) {
        for (int j = 0; j < 5; ++j) {
            dst.var_k[slot][j + 5 * k] = src.var_k[slot][j + 5 * k];
        }
    }
}

// Parallel version of `for (iop = lo; iop < hi; ++iop) EvaluateCandidateFull(sd, iop);`.
void EvaluatePopulationParallel(StructureData& sd, std::vector<StructureData>& workers, int lo, int hi,
                                 const OptimizationOptions& options) {
    if (workers.empty() || hi - lo <= 1) {
        for (int iop = lo; iop < hi; ++iop) {
            EvaluateCandidateFull(sd, iop, options);
        }
        return;
    }

    for (auto& w : workers) {
        w = sd; // full sync: problem definition + current population state
    }

    auto ranges = RunOnLanes(sd, workers, hi - lo, [lo, &options](StructureData& local, int i) {
        EvaluateCandidateFull(local, lo + i, options);
    });

    // ranges[0] was computed by `sd` itself -- nothing to merge for it.
    for (size_t lane = 1; lane < ranges.size(); ++lane) {
        const StructureData& local = workers[lane - 1];
        for (int i = ranges[lane].first; i < ranges[lane].second; ++i) {
            CopyCandidateSlot(sd, local, lo + i);
        }
    }
}

// Parallel version of cari_baru()'s trial-search loop (Baru.hpp): each
// `iterasi_var` in [0, TrialCount(sd)) is evaluated independently (see
// EvaluateTrial's comment), so this partitions that range across lanes and
// reduces each lane's local best into the global fitcb_best/
// var_b_cb_best/var_k_cb_best -- this is the "heavy iteration" loop that
// dominates wall-clock time (it runs every generation, potentially many
// trials each), unlike the population evaluation above which only runs
// once (generation 0) plus a single extra candidate per later generation.
// Copies *only* what a worker's EvaluateTrial() calls actually read beyond
// what was already fixed for the whole run at worker-pool creation time
// (geometry/loads/discrete tables/nvb/nvk/JVD/JSTD/jum_balok/jum_kolom --
// none of which change generation to generation): this generation's search
// direction (TM/TS/arah/no_TS_terjauh, all JVD-sized) and var_b_jelek/
// var_k_jelek (from which each worker locally re-derives varnew_asli, one
// cheap JVD-sized loop, rather than also syncing varnew_asli itself).
//
// Deliberately *not* a full `dst = src` struct copy: that was measured to
// cost far more than the trial-search work it was protecting for datasets
// where TrialCount() is small (every field on StructureData, including the
// large mak*mak scratch matrices no trial actually needs synced -- each
// worker fully overwrites its own SFF/SM/SMRT/AM/etc. from scratch inside
// Inersia()/Struktur(), so their *previous* contents are irrelevant).
void SyncSearchState(StructureData& dst, const StructureData& src) {
    for (int i = 0; i < src.JVD; ++i) {
        dst.TM[i] = src.TM[i];
        dst.TS[i] = src.TS[i];
        dst.arah[i] = src.arah[i];
    }
    dst.no_TS_terjauh = src.no_TS_terjauh;
    for (int b = 0; b < src.jum_balok; ++b) {
        for (int j = 0; j < 12; ++j) {
            dst.var_b_jelek[j + 12 * b] = src.var_b_jelek[j + 12 * b];
        }
    }
    for (int k = 0; k < src.jum_kolom; ++k) {
        for (int j = 0; j < 5; ++j) {
            dst.var_k_jelek[j + 5 * k] = src.var_k_jelek[j + 5 * k];
        }
    }
    dst.fitcb_best = 0.f;
}

// Below this many trials per worker lane, thread-spawn + sync overhead
// isn't worth paying even with the cheap targeted sync above -- fall back
// to the sequential path (identical result either way).
constexpr int kMinTrialsPerWorker = 8;

void CariBaruParallel(StructureData& sd, std::vector<StructureData>& workers, const OptimizationOptions& options) {
    int n_trials = TrialCount(sd);
    int n_lanes = 1 + static_cast<int>(workers.size());

    if (workers.empty() || n_trials < n_lanes * kMinTrialsPerWorker) {
        CariBaru(sd, options); // identical result, no threading overhead worth paying
        return;
    }

    sd.iterasi_var = 0;
    NormalisasiInt(sd, sd.varnew_asli, sd.var_b_jelek, sd.var_k_jelek);
    sd.fitcb_best = 0.f;

    for (auto& w : workers) {
        SyncSearchState(w, sd);
        NormalisasiInt(w, w.varnew_asli, w.var_b_jelek, w.var_k_jelek);
    }

    // Each lane tracks its own local best directly on its StructureData
    // (fitcb_best/var_b_cb_best/var_k_cb_best, already zeroed by the sync
    // above), exactly like the sequential path -- no shared mutable state
    // touched during dispatch.
    RunOnLanes(sd, workers, n_trials, [&options](StructureData& local, int iterasi_var) {
        local.iterasi_var = iterasi_var;
        TrialResult result = EvaluateTrial(local, iterasi_var, options);
        if (result.valid && result.fitcb > local.fitcb_best) {
            local.fitcb_best = result.fitcb;
            for (int icb = 0; icb < local.jum_balok; ++icb) {
                for (int jcb = 0; jcb < 12; ++jcb) {
                    local.var_b_cb_best[jcb + 12 * icb] = local.var_b_cb[jcb + 12 * icb];
                }
            }
            for (int icb = 0; icb < local.jum_kolom; ++icb) {
                for (int jcb = 0; jcb < 5; ++jcb) {
                    local.var_k_cb_best[jcb + 5 * icb] = local.var_k_cb[jcb + 5 * icb];
                }
            }
        }
    });

    // Reduce: lane 0 is `sd` itself (already holds its own local best);
    // fold in each worker's local best if it beats the running one.
    auto fold_in = [&sd](const StructureData& local) {
        if (local.fitcb_best > sd.fitcb_best) {
            sd.fitcb_best = local.fitcb_best;
            for (int icb = 0; icb < sd.jum_balok; ++icb) {
                for (int jcb = 0; jcb < 12; ++jcb) {
                    sd.var_b_cb_best[jcb + 12 * icb] = local.var_b_cb_best[jcb + 12 * icb];
                }
            }
            for (int icb = 0; icb < sd.jum_kolom; ++icb) {
                for (int jcb = 0; jcb < 5; ++jcb) {
                    sd.var_k_cb_best[jcb + 5 * icb] = local.var_k_cb_best[jcb + 5 * icb];
                }
            }
        }
    };
    for (auto& w : workers) {
        fold_in(w);
    }

    sd.iterasi_var = n_trials;
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

AnalyzeResult AnalyzeFixedDesign(StructureData& sd, const OptimizationOptions& options, const std::vector<int>& var_b,
                                  const std::vector<int>& var_k) {
    PrepareOptimization(sd, options);

    if (static_cast<int>(var_b.size()) != 12 * sd.jum_balok) {
        throw std::invalid_argument("AnalyzeFixedDesign: var_b size " + std::to_string(var_b.size()) +
                                     " does not match 12*jum_balok=" + std::to_string(12 * sd.jum_balok));
    }
    if (static_cast<int>(var_k.size()) != 5 * sd.jum_kolom) {
        throw std::invalid_argument("AnalyzeFixedDesign: var_k size " + std::to_string(var_k.size()) +
                                     " does not match 5*jum_kolom=" + std::to_string(5 * sd.jum_kolom));
    }

    LoadBatasAtas(sd);
    for (size_t i = 0; i < var_b.size(); ++i) {
        int upper = sd.nvb[static_cast<int>(i)];
        if (var_b[i] < 0 || var_b[i] >= upper) {
            throw std::invalid_argument("AnalyzeFixedDesign: var_b[" + std::to_string(i) +
                                         "]=" + std::to_string(var_b[i]) + " out of range [0," +
                                         std::to_string(upper) + ")");
        }
    }
    for (size_t i = 0; i < var_k.size(); ++i) {
        int upper = sd.nvk[static_cast<int>(i)];
        if (var_k[i] < 0 || var_k[i] >= upper) {
            throw std::invalid_argument("AnalyzeFixedDesign: var_k[" + std::to_string(i) +
                                         "]=" + std::to_string(var_k[i]) + " out of range [0," +
                                         std::to_string(upper) + ")");
        }
    }

    sd.no_struktur = 0;
    for (size_t i = 0; i < var_b.size(); ++i) sd.var_b[0][static_cast<int>(i)] = var_b[i];
    for (size_t i = 0; i < var_k.size(); ++i) sd.var_k[0][static_cast<int>(i)] = var_k[i];

    Inersia(sd);
    Struktur(sd);

    // ComputeMemberResults() (like WriteFinalResults()) always reads
    // whichever slot sd.JSTD-1 points at ("the best structure" convention).
    // This function only ever populates slot 0, so force JSTD=1 to make
    // that convention land on the fixed design just written, regardless of
    // whatever JSTD PrepareOptimization()'s fak_kali/fak_plus computed --
    // safe because fitstr/hargastr/kendalastr/var_b/var_k are all
    // fixed-kMak-sized LegacyArray(2D)s, never resized by JSTD (see
    // StructureData.h's LegacyArray comment), so this touches no array
    // bounds, just which slot downstream readers select.
    sd.JSTD = 1;

    AnalyzeResult result;
    result.member_results = ComputeMemberResults(sd);
    result.analysis = ComputeAnalysisResults(sd);
    return result;
}

void RunOptimization(StructureData& sd, const OptimizationOptions& options, const ProgressCallback& on_progress,
                      const DetailLogCallback& on_detail, const std::atomic<bool>* cancel) {
    // Issue #65: the convergence check below reads sd.fitstr[sd.JSTD -
    // sd.JVD - 1] -- with fak_kali=1/fak_plus=0 (both reachable via the Run
    // panel's plain ImGui::InputInt fields, and via orcisf_cli's optimize
    // command-line args), JSTD == JVD, making that index -1 -- an
    // out-of-bounds LegacyArray access (UB, size_t-cast from -1). The
    // legacy algorithm's convergence check itself assumes a population
    // strictly larger than JVD (comparing the best candidate against the
    // one JVD+1 slots below it), so this isn't just an index guard: JSTD
    // <= JVD is genuinely too small a population for optimasi() to
    // function as designed. Fail loudly and early instead of reading
    // garbage memory -- RunPanel/orcisf_cli both already catch
    // std::exception around this call.
    if (sd.JSTD <= sd.JVD) {
        throw std::invalid_argument("RunOptimization: JSTD (" + std::to_string(sd.JSTD) + ") must be greater than "
                                     "JVD (" + std::to_string(sd.JVD) +
                                     ") -- increase fak_kali and/or fak_plus (JSTD = JVD*fak_kali + fak_plus).");
    }
    // Issue #75 (found instrumenting #63): the opposite failure mode from
    // #65 above -- var_b/var_k/fitstr/kendalastr/hargastr are fixed
    // kMak(825)-sized LegacyArray(2D)s (see StructureData.h), never resized
    // by JSTD, so a JSTD that's too *large* (a big fak_kali on a dataset
    // with enough beams/columns -- e.g. fak_kali=20 on an 8-member dataset
    // already exceeds it) writes past the end of every one of those arrays.
    // Confirmed as a real, deterministic segfault via orcisf_cli before this
    // guard was added. Same fail-loudly-and-early treatment as #65.
    if (sd.JSTD > kMak) {
        throw std::invalid_argument("RunOptimization: JSTD (" + std::to_string(sd.JSTD) +
                                     ") exceeds the maximum population size (" + std::to_string(kMak) +
                                     ") -- reduce fak_kali and/or fak_plus (JSTD = JVD*fak_kali + fak_plus).");
    }

    Rng rng(options.rng_seed != 0 ? options.rng_seed : std::random_device{}());
    AcakVariabel(sd, rng, options);

    // `workers` holds worker_threads-1 extra StructureData clones; `sd`
    // itself is always lane 0. Created once (not per-generation) since a
    // clone already contains the full population -- see EvaluatePopulation/
    // CariBaruParallel's "full sync" comments for why a per-generation
    // *sync* (cheap, ~10-15MB memcpy-equivalent) is still needed even
    // though the clones themselves are created just this once.
    std::vector<StructureData> workers;
    if (options.worker_threads > 1) {
        workers.assign(options.worker_threads - 1, sd);
    }

    EvaluatePopulationParallel(sd, workers, 0, sd.JSTD, options);
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
        CariBaruParallel(sd, workers, options);

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

        EvaluateCandidateFull(sd, 0, options);
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
