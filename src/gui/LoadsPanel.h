#pragma once

#include <functional>

#include "gui/editor/EditableStructure.h"
#include "gui/editor/Selection.h"
#include "gui/editor/UndoStack.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::gui {

// Issue #7's "load-schedule panel (table)": lists every member/joint that
// currently has a nonzero raw load, editable inline, with a delete button
// per row and a click-to-select (syncs `selection` so the 3D viewport
// highlights/frames the same entity the Properties panel would show).
class LoadsPanel {
public:
    void Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
              UndoStack* undo, const std::function<void()>& on_geometry_changed);
};

} // namespace orcisf::gui
