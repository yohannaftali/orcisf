#pragma once

#include <functional>

#include "gui/editor/EditableStructure.h"
#include "gui/editor/Selection.h"
#include "gui/editor/UndoStack.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::gui {

// Issue #36 (split from JointsMembersPanel, issue #21): tabular list of
// every member in the loaded dataset, deletable, mirroring LoadsPanel's
// row-click-syncs-Selection pattern. No modal/confirmation state of its
// own -- deleting a member never cascades to anything else, unlike
// deleting a joint (see JointsPanel).
class MembersPanel {
public:
    void Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
              const std::function<void()>& on_geometry_changed);
};

} // namespace orcisf::gui
