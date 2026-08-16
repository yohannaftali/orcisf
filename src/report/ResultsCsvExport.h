#pragma once

#include <string>

#include "engine/AnalysisResults.h"

// Issue #62 (epic #58): spreadsheet-compatible export of ResultsPanel's
// tables. Same `report::` namespace as TextExport.{h,cpp}/PdfExport.{h,cpp}
// (see TextExport.h's comment on why not `export`).
namespace orcisf::report {

// Writes joint displacements, member end forces, and support reactions
// (plus the global-equilibrium totals) as a single CSV file at `csv_path`
// -- one section per table, blank line between sections, a header row per
// section. Returns an empty string on success, or a human-readable error
// message.
std::string WriteResultsCsv(const engine::AnalysisResults& analysis, const std::string& csv_path);

} // namespace orcisf::report
