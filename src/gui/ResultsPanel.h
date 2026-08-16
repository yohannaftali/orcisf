#pragma once

#include "engine/AnalysisResults.h"
#include "gui/editor/Selection.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::gui {

// Issue #62 (epic #58): docked panel showing the "Display > Show Tables"
// equivalent -- joint displacements, member end forces, and support
// reactions in spreadsheet-style tables (issue #52's TableView), plus a
// visible global-equilibrium check (total reactions vs. total applied
// load). Row selection in the Joints/Members tables syncs `Selection`
// the same way every other results table in this project already does.
// View-only, no editing -- reads `analysis` (issue #59's
// engine::AnalysisResults, null when no completed run's results are
// loaded).
class ResultsPanel {
public:
    void Draw(bool* open, const SceneModel& scene, Selection& selection, const engine::AnalysisResults* analysis);
};

} // namespace orcisf::gui
