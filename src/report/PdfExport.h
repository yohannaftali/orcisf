#pragma once

#include <string>
#include <vector>

#include "engine/MemberResults.h"
#include "engine/StructureData.h"

namespace orcisf::report {

// Issue #9's "Download as PDF": input summary, the optimization run's
// fitness/cost/constraint summary, a per-member dimensions/reinforcement/
// forces table, and (issue #8) a reinforcement detailing drawing page per
// member -- built by reusing gui::BuildDetailingDrawing() directly (see
// its header comment: this is exactly why the layout/render split in
// issue #8 exists) with a second renderer that draws the same
// DetailingDrawing with HPDF calls instead of ImDrawList calls.
// `results` must be non-empty (i.e. an optimization has actually
// completed) -- callers should not offer "Download as PDF" otherwise.
// Returns an empty string on success, or a human-readable error message.
std::string WritePdfReport(const engine::StructureData& sd, const std::vector<engine::MemberResult>& results,
                            const std::string& pdf_path);

} // namespace orcisf::report
