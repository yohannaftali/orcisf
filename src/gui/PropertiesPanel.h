#pragma once

#include "gui/viewport/SceneModel.h"

namespace orcisf::gui {

// Docked panel for the current selection. For now (issue #5) that's a
// clicked/hovered member from the 3D viewport, showing its dimensions and
// (if an optimization has run) demand/capacity + constraint results.
// Numeric joint/member/load *editing* is issue #6/#7.
class PropertiesPanel {
public:
    void Draw(bool* open, const SceneModel& scene, int selected_member);
};

} // namespace orcisf::gui
