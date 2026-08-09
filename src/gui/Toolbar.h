#pragma once

#include <functional>

#include "gui/editor/Selection.h"

namespace orcisf::gui {

// Top toolbar. "File > Open Folder..." (issue #5) and the "Edit" menu
// (issue #6: Undo/Redo, Add Joint, Connect Mode, Snap to Grid) are wired;
// everything else (Run/View menu items, load-type selection for issue #7)
// is still a placeholder.
class Toolbar {
public:
    void Draw(bool can_undo, bool can_redo, EditorOptions& options);

    void SetOnOpenFolder(std::function<void()> callback);
    void SetOnUndo(std::function<void()> callback);
    void SetOnRedo(std::function<void()> callback);
    // Adds a new joint at a sensible default position (the current scene's
    // bounding-sphere center, or the origin if nothing is loaded) -- the
    // user then drags it into place via the viewport's gizmo or types
    // exact coordinates in the Properties panel.
    void SetOnAddJoint(std::function<void()> callback);

private:
    std::function<void()> on_open_folder_;
    std::function<void()> on_undo_;
    std::function<void()> on_redo_;
    std::function<void()> on_add_joint_;
};

} // namespace orcisf::gui
