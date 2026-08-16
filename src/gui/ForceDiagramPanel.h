#pragma once

#include "engine/AnalysisResults.h"
#include "gui/editor/Selection.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::gui {

// Issue #61 (epic #58): docked panel showing N/V/M/T force diagrams (via
// ImPlot) for whichever member is currently selected, plus a station
// slider for numeric inspection (the "click a member for details, read
// values at a station" workflow from the epic's request). View-only, no
// editing -- reads the same Selection/SceneModel every other per-member
// panel (Properties, Detailing) already uses, plus `analysis` (issue
// #59's engine::AnalysisResults, null when no completed run's results
// are loaded).
class ForceDiagramPanel {
public:
    void Draw(bool* open, const SceneModel& scene, const Selection& selection, const engine::AnalysisResults* analysis);

private:
    float station_t_ = 0.5f; // 0..1 along the selected member's span, for the numeric readout
};

} // namespace orcisf::gui
