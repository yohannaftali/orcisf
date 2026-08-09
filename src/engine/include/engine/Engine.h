#pragma once

#include <atomic>
#include <string>

#include "engine/LegacyIO.h"
#include "engine/Optimizer.h"
#include "engine/StructureData.h"

// Public facade tying LegacyIO + StructuralAnalysis + Optimizer together,
// matching the legacy program's "Mengoptimasi struktur" menu option
// (Tampilan.hpp) end to end: load a dataset, run the optimizer, write
// every legacy output file plus the new detailed log.
namespace orcisf::engine {

// Runs a full optimization pass against `generic_dataset_path` (e.g.
// "Optimasi Beton/Example/Apl1-1/aplikasi", no extension -- see
// LegacyIO::MakeDatasetPaths) and writes .opt/.str/.kdl/.inf/.his plus the
// new detailed per-generation log.
//
// `sd` is left populated with the final state (population, analysis
// results) after the call, so a caller (e.g. the GUI) can inspect/render
// it without re-reading the just-written files.
void RunFullOptimization(StructureData& sd, const std::string& generic_dataset_path,
                          const OptimizationOptions& options, const ProgressCallback& on_progress = nullptr,
                          const std::atomic<bool>* cancel = nullptr);

// Loads a dataset (geometry + discrete tables + loads) without running an
// optimization -- e.g. for the GUI's "open Example/Apl1-1 and render it in
// 3D" use case (issue #5).
void LoadDatasetForViewing(StructureData& sd, const std::string& generic_dataset_path);

} // namespace orcisf::engine
