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
// LegacyIO::MakeDatasetPaths), reading the dataset's own .inp/.isd/.idl/
// .ijl/.ids/.ijs/.bbn input files as always.
//
// Issue #25: output (.opt/.str/.kdl/.inf/.his plus the detailed
// per-generation log) is written into a fresh subfolder named after the
// run's start date/time (`YYYY-MM-DD.HH.MM`, e.g. "2026-08-11.13.40"),
// created next to `generic_dataset_path`'s own folder -- not overwritten
// in place -- so results from separate runs (including a
// re-optimize-from-last-best continuation, issue #16) don't collide and
// a history of past attempts is kept. Returns the generic *output* path
// those files were actually written to (same folder+basename-no-extension
// convention as `generic_dataset_path`, just inside that new subfolder);
// callers that need to locate them afterward (e.g. issue #9's text/PDF
// export, which copies .his/.log.txt from wherever the run actually wrote
// them) must use this return value, not `generic_dataset_path` itself.
// Two runs started within the same minute against the same dataset reuse
// the same subfolder (last one wins) -- an accepted minor edge case, not
// otherwise handled.
//
// `sd` is left populated with the final state (population, analysis
// results) after the call, so a caller (e.g. the GUI) can inspect/render
// it without re-reading the just-written files.
std::string RunFullOptimization(StructureData& sd, const std::string& generic_dataset_path,
                                 const OptimizationOptions& options, const ProgressCallback& on_progress = nullptr,
                                 const std::atomic<bool>* cancel = nullptr);

// Loads a dataset (geometry + discrete tables + loads) without running an
// optimization -- e.g. for the GUI's "open Example/Apl1-1 and render it in
// 3D" use case (issue #5).
void LoadDatasetForViewing(StructureData& sd, const std::string& generic_dataset_path);

} // namespace orcisf::engine
