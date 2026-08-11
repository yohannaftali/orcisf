// Small headless validation/batch CLI for orcisf_engine -- also doubles as
// the "future CLI/batch tool" mentioned in issue #3's Objective. Not part
// of the GUI; used here to validate the engine port against real
// Optimasi Beton/Example/* datasets without needing the GUI built.
#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>

#include "engine/Engine.h"
#include "engine/MemberResults.h"
#include "engine/StructuralAnalysis.h"

using namespace orcisf::engine;

namespace {

int CmdInfo(const std::string& dataset) {
    StructureData sd;
    LoadDatasetForViewing(sd, dataset);
    std::cout << "ISN=" << sd.ISN << "\n";
    std::cout << "M=" << sd.M << " NJ=" << sd.NJ << " NR=" << sd.NR << " NRJ=" << sd.NRJ << "\n";
    std::cout << "E=" << sd.E << " G=" << sd.G << "\n";
    std::cout << "FC=" << sd.FC << " FY=" << sd.FY << " FYS=" << sd.FYS << "\n";
    std::cout << "ND=" << sd.ND << " N=" << sd.N << "\n";
    std::cout << "nsisi_B=" << sd.nsisi_B << " nsisi_H=" << sd.nsisi_H << " nsisi_K=" << sd.nsisi_K
              << " nDIA=" << sd.nDIA << " nNL=" << sd.nNL << " nDIAS=" << sd.nDIAS << " nJS=" << sd.nJS << "\n";
    for (int i = 1; i <= sd.NJ; ++i) {
        std::cout << "  joint " << i << ": (" << sd.X[i] << ", " << sd.Y[i] << ", " << sd.Z[i] << ")\n";
    }
    for (int i = 1; i <= sd.M; ++i) {
        std::cout << "  member " << i << ": " << sd.JJ[i] << " -> " << sd.JK[i] << "\n";
    }
    return 0;
}

// Assigns every discrete design-variable slot its *smallest* index (0), runs
// a full analysis, and checks global equilibrium (sum of support reactions
// should balance sum of applied joint loads + member self-weight) as an
// independent correctness check on the ported stiffness solver -- this does
// not depend on reproducing the legacy binary's (unreproducible, RNG-based)
// output, only on Newton's third law / statics.
int CmdEquilibrium(const std::string& dataset) {
    StructureData sd;
    LoadDatasetForViewing(sd, dataset);

    OptimizationOptions opt;
    opt.fak_kali = 1;
    opt.fak_plus = 0;
    PrepareOptimization(sd, opt);

    sd.no_struktur = 0;
    for (int b = 0; b < sd.jum_balok; ++b) {
        for (int j = 0; j < 12; ++j) sd.var_b[0][j + 12 * b] = sd.nvb[j + 12 * b] / 2; // mid-range size
    }
    for (int k = 0; k < sd.jum_kolom; ++k) {
        for (int j = 0; j < 5; ++j) sd.var_k[0][j + 5 * k] = sd.nvk[j + 5 * k] / 2;
    }

    Inersia(sd);
    Struktur(sd);

    // Vertical (global axis 2, i.e. Y-up per the .inp convention) global
    // equilibrium check: sum of applied joint loads (AJ, which berat_sendiri()
    // already folded column self-weight into) plus the resultant of every
    // beam's distributed self-weight/applied load (W) must balance the sum
    // of support reactions.
    double sum_AJ = 0.0, sum_beam_resultant = 0.0, sum_reactions = 0.0;
    for (int j = 1; j <= sd.NJ; ++j) {
        sum_AJ += sd.AJ[6 * j - 4]; // arah 2 = Y
    }
    for (int i = 1; i <= sd.M; ++i) {
        PeriksaBatang(sd, i);
        if (sd.CXZ > 0.001f) { // beams only; column self-weight is in AJ
            sum_beam_resultant += -static_cast<double>(sd.W[i]) * sd.EL[i]; // W acts downward (-Y)
        }
    }
    for (int j = 1; j <= sd.NJ; ++j) {
        sum_reactions += sd.AR[6 * j - 4];
    }

    double applied_total = sum_AJ + sum_beam_resultant;
    std::cout << "Sum applied load, arah Y (joint AJ + beam self-weight resultant): " << applied_total << "\n";
    std::cout << "Sum support reactions, arah Y:                                    " << sum_reactions << "\n";
    double diff = applied_total + sum_reactions; // reactions oppose applied load, should cancel to ~0
    std::cout << "Residual (should be ~0): " << diff << "\n";
    return (std::fabs(diff) < 1.0) ? 0 : 1; // 1 N tolerance
}

int CmdOptimize(const std::string& dataset, float harga_beton, float harga_besi, float selimut_kolom,
                 float selimut_balok, float finalti, int iter_mak, int fak_plus, int fak_kali,
                 unsigned int worker_threads, unsigned int rng_seed, bool quiet) {
    StructureData sd;
    OptimizationOptions opt;
    opt.harga_beton = harga_beton;
    opt.harga_besi = harga_besi;
    opt.selimut_kolom = selimut_kolom;
    opt.selimut_balok = selimut_balok;
    opt.finalti = finalti;
    opt.j_iterasi_mak = iter_mak;
    opt.fak_plus = fak_plus;
    opt.fak_kali = fak_kali;
    opt.worker_threads = worker_threads;
    opt.rng_seed = rng_seed;

    ProgressCallback cb = [quiet](const ProgressInfo& info) -> bool {
        if (!quiet) {
            std::printf("gen %d/%d  fitness=%.6g  harga=%.6g  kendala=%.6g  t=%.2fs%s\n", info.generation,
                         info.max_generation, info.best_fitness, info.best_harga, info.best_kendala,
                         info.elapsed_seconds, info.converged ? "  [CONVERGED]" : "");
        }
        return true;
    };

    std::string output_path = RunFullOptimization(sd, dataset, opt, cb);

    std::cout << "Output written to: " << output_path << "\n";
    std::cout << "Done. JVD=" << sd.JVD << " JSTD=" << sd.JSTD << " worker_threads=" << worker_threads
              << " rng_seed=" << rng_seed << "\n";
    std::cout << "Best fitness=" << sd.fitstr[sd.JSTD - 1] << " harga=" << sd.hargastr[sd.JSTD - 1]
              << " kendala=" << sd.kendalastr[sd.JSTD - 1] << "\n";

    // Dump every population slot's fitness + design variables so two runs
    // (e.g. worker_threads=1 vs N with the same rng_seed) can be diffed
    // for exact equality -- see engine/README.md's threading validation.
    std::cout << "POPULATION_DUMP\n";
    for (int i = 0; i < sd.JSTD; ++i) {
        std::cout << i << " fit=" << sd.fitstr[i] << " kdl=" << sd.kendalastr[i] << " hrg=" << sd.hargastr[i]
                  << " b=";
        for (int b = 0; b < sd.jum_balok; ++b) {
            for (int j = 0; j < 12; ++j) std::cout << sd.var_b[i][j + 12 * b] << ",";
        }
        std::cout << " k=";
        for (int k = 0; k < sd.jum_kolom; ++k) {
            for (int j = 0; j < 5; ++j) std::cout << sd.var_k[i][j + 5 * k] << ",";
        }
        std::cout << "\n";
    }

    std::cout << "MEMBER_RESULTS\n";
    for (const MemberResult& r : ComputeMemberResults(sd)) {
        std::cout << "batang=" << r.no_batang << " type=" << (r.is_beam ? "balok" : "kolom") << " w=" << r.width
                   << " h=" << r.height << " harga=" << r.harga << " kendala=" << r.Kendala()
                   << " cover=" << r.cover_mm;
        if (r.is_beam) {
            std::cout << " lap_tarik=" << r.lap_n_tarik << "D" << r.lap_dia_tarik
                       << " lap_tekan=" << r.lap_n_tekan << "D" << r.lap_dia_tekan
                       << " tum_tarik=" << r.tum_n_tarik << "D" << r.tum_dia_tarik
                       << " tum_tekan=" << r.tum_n_tekan << "D" << r.tum_dia_tekan
                       << " sengkang=" << r.stirrup_dia << "-" << r.stirrup_spacing;
        } else {
            std::cout << " tul_utama=" << (4 * r.col_n_dia - 4) << "D" << r.col_dia
                       << " jarak_tul=" << r.col_bar_spacing
                       << " sengkang=" << r.stirrup_dia << "-" << r.stirrup_spacing;
        }
        std::cout << "\n";
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage:\n"
                  << "  orcisf_cli info <dataset-generic-path>\n"
                  << "  orcisf_cli equilibrium <dataset-generic-path>\n"
                  << "  orcisf_cli optimize <dataset-generic-path> <harga_beton> <harga_besi> <selimut_kolom> "
                     "<selimut_balok> <finalti> <iter_mak> <fak_plus> <fak_kali> [worker_threads] [rng_seed] "
                     "[quiet:0|1]\n";
        return 2;
    }

    std::string cmd = argv[1];
    std::string dataset = argv[2];

    try {
        if (cmd == "info") {
            return CmdInfo(dataset);
        }
        if (cmd == "equilibrium") {
            return CmdEquilibrium(dataset);
        }
        if (cmd == "optimize") {
            if (argc < 11) {
                std::cerr << "optimize needs 9 args after dataset\n";
                return 2;
            }
            unsigned int worker_threads = (argc > 11) ? static_cast<unsigned int>(std::stoul(argv[11])) : 1u;
            unsigned int rng_seed = (argc > 12) ? static_cast<unsigned int>(std::stoul(argv[12])) : 0u;
            bool quiet = (argc > 13) && std::string(argv[13]) == "1";
            return CmdOptimize(dataset, std::stof(argv[3]), std::stof(argv[4]), std::stof(argv[5]),
                                std::stof(argv[6]), std::stof(argv[7]), std::stoi(argv[8]), std::stoi(argv[9]),
                                std::stoi(argv[10]), worker_threads, rng_seed, quiet);
        }
        std::cerr << "Unknown command: " << cmd << "\n";
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
