#include "engine/Engine.h"

#include <fstream>
#include <memory>

namespace orcisf::engine {

void LoadDatasetForViewing(StructureData& sd, const std::string& generic_dataset_path) {
    DatasetPaths paths = MakeDatasetPaths(generic_dataset_path);
    ReadDataset(sd, paths);
    ReadLoads(sd, paths.bbn);
}

void RunFullOptimization(StructureData& sd, const std::string& generic_dataset_path,
                          const OptimizationOptions& options, const ProgressCallback& on_progress,
                          const std::atomic<bool>* cancel) {
    DatasetPaths paths = MakeDatasetPaths(generic_dataset_path);
    ReadDataset(sd, paths);
    ReadLoads(sd, paths.bbn);

    PrepareOptimization(sd, options);

    auto his = std::make_shared<std::ofstream>(paths.his);
    auto log_detail = std::make_shared<std::ofstream>(paths.log_detail);

    *log_detail << "Log kalkulasi detail -- superset dari " << paths.his << "\n"
                << "Setiap generasi: seluruh populasi (indeks 0=terjelek .. JSTD-1=terbaik),\n"
                << "fitness/kendala/harga, dan variabel desain balok (12 slot/balok) + kolom (5 slot/kolom).\n\n";

    ProgressCallback progress_wrapper = [&](const ProgressInfo& info) -> bool {
        *his << "Generasi " << info.generation << " | fitness terbaik " << info.best_fitness << " | harga "
             << info.best_harga << " | kendala " << info.best_kendala << " | waktu " << info.elapsed_seconds
             << " detik";
        if (info.converged) {
            *his << " | KONVERGEN";
        }
        *his << "\n";
        if (on_progress) {
            return on_progress(info);
        }
        return true;
    };

    DetailLogCallback detail_writer = [&](int generation, const StructureData& s) {
        *log_detail << "=== Generasi " << generation << " ===\n";
        for (int i = 0; i < s.JSTD; ++i) {
            *log_detail << "  [" << i << "] fitness=" << s.fitstr[i] << " kendala=" << s.kendalastr[i]
                        << " harga=" << s.hargastr[i] << "\n";
            *log_detail << "      balok:";
            for (int b = 0; b < s.jum_balok; ++b) {
                *log_detail << " (";
                for (int j = 0; j < 12; ++j) {
                    *log_detail << s.var_b[i][j + 12 * b] << (j < 11 ? "," : "");
                }
                *log_detail << ")";
            }
            *log_detail << "\n      kolom:";
            for (int k = 0; k < s.jum_kolom; ++k) {
                *log_detail << " (";
                for (int j = 0; j < 5; ++j) {
                    *log_detail << s.var_k[i][j + 5 * k] << (j < 4 ? "," : "");
                }
                *log_detail << ")";
            }
            *log_detail << "\n";
        }
    };

    RunOptimization(sd, options, progress_wrapper, detail_writer, cancel);

    his->close();
    log_detail->close();

    WriteFinalResults(sd, paths);
}

} // namespace orcisf::engine
