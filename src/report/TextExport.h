#pragma once

#include <string>

#include "engine/StructureData.h"

// Issue #9's "Download as text" export. Named `report/` (not the issue's
// suggested `/src/export` -- `export` is a reserved C++ word going back to
// the unused C++98 export-templates feature and repurposed for C++20
// modules, so it can't be a namespace identifier).
namespace orcisf::report {

// Writes the complete legacy file set to `dest_generic_path`:
// .inp/.isd/.idl/.ijl/.ids/.ijs/.bbn always (mirrors whatever's currently
// in `sd`, the same "raw load" convention issue #7 established); if
// `has_run_results` is true (an optimization has completed on this exact
// `sd` and no edit has invalidated it since -- same precondition
// engine::WriteFinalResults() has), also writes .opt/.str/.kdl/.inf and
// copies .his/.log_detail from `source_generic_path` (the path the run
// actually wrote them to -- per-generation history that can't be
// regenerated from the final StructureData alone; missing/unreadable
// source files are skipped, not fatal, since a run may not have produced
// them e.g. if cancelled before the first generation).
// Returns an empty string on success, or a human-readable error message.
std::string WriteTextExport(engine::StructureData& sd, const std::string& dest_generic_path, bool has_run_results,
                             const std::string& source_generic_path);

} // namespace orcisf::report
