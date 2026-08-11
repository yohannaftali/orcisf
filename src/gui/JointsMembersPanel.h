#pragma once

#include <functional>
#include <vector>

#include "gui/editor/EditableStructure.h"
#include "gui/editor/Selection.h"
#include "gui/editor/UndoStack.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::gui {

// Issue #21: tabular list of every joint and member in the loaded dataset,
// inline-editable (joint X/Y/Z, restraint toggle) and deletable, mirroring
// LoadsPanel's existing row-click-syncs-Selection / inline-InputFloat
// pattern rather than introducing a new one. Row-level movement reuses the
// existing ImGuizmo gizmo / Properties panel numeric fields (issue #6) --
// this panel is not a second way to drag a joint.
//
// Deleting a joint that still has members attached already cascades to
// delete those members too (EditableStructure::DeleteJoint()) -- that
// silent behavior predates this panel. What's new here is a confirmation
// modal shown before the delete happens whenever it would take members
// down with it, so the user isn't surprised by a silent cascade.
class JointsMembersPanel {
public:
    void Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
              const std::function<void()>& on_geometry_changed);

private:
    // Set while the "delete this joint and its N members?" modal is open;
    // -1 when no delete is pending confirmation.
    int pending_delete_joint_ = -1;
    std::vector<int> pending_delete_joint_members_;
};

} // namespace orcisf::gui
