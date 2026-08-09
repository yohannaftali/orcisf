#pragma once

#include <functional>
#include <string>
#include <vector>

#include "gui/editor/EditableStructure.h"
#include "gui/editor/Selection.h"
#include "gui/editor/UndoStack.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::gui {

// Docked panel for the current selection (issue #5) and, when a dataset is
// editable, numeric geometry editing + validation feedback (issue #6):
// exact X/Y/Z entry and a restrained toggle for a selected joint, a delete
// button for a selected joint/member, and a always-visible list of
// blocking validation issues (EditableStructure::Validate()) so problems
// are surfaced before the user tries to run an analysis.
class PropertiesPanel {
public:
    void Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
              UndoStack* undo, const std::vector<std::string>& validation_issues,
              const std::function<void()>& on_geometry_changed);
};

} // namespace orcisf::gui
