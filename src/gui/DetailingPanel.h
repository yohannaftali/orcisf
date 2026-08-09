#pragma once

#include "gui/editor/Selection.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::gui {

// Issue #8: docked panel showing the reinforcement detailing (concrete
// outline, main bars, stirrups, dimension labels) for whichever member is
// currently selected. View-only -- reads the same Selection/SceneModel
// ViewportPanel/PropertiesPanel already use, no editing capability (#8's
// acceptance criteria doesn't call for editable detailing, just readable
// drawings of the optimizer's chosen design).
class DetailingPanel {
public:
    void Draw(bool* open, const SceneModel& scene, const Selection& selection);
};

} // namespace orcisf::gui
