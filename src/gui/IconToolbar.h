#pragma once

#include <functional>

#include "gui/editor/Selection.h"

namespace orcisf::gui {

// Issue #14: an icon-button row docked below the main menu bar, reusing the
// same actions Toolbar's File/Edit menu already exposes (New Data, Open
// Data, Save, Undo, Redo, Add Joint, Connect Joints, Run) so common actions
// don't require opening a menu every time -- Office/Adobe/AutoCAD-style.
//
// Scoped-down delivery: a fixed, curated button set rather than a fully
// user-customizable Quick Access Toolbar (drag-to-reorder/add-remove) --
// judged too large to bundle with the initial icon set; a future issue can
// add customization on top of this without changing how buttons are drawn.
//
// Icons are hand-drawn with ImDrawList primitives (lines/shapes) rather
// than a bundled icon font, matching this project's preference for small,
// dependency-free GUI code (see SceneRenderer's hand-rolled GL primitives)
// -- no vcpkg.json change needed.
class IconToolbar {
public:
    // Full row height (button + vertical padding) -- Application reserves
    // this much space below the main menu bar for the dockspace so panels
    // don't render underneath the toolbar.
    static constexpr float kHeight = 36.f;

    void SetOnNewData(std::function<void()> callback);
    void SetOnOpenFolder(std::function<void()> callback);
    void SetOnSave(std::function<void()> callback);
    void SetOnUndo(std::function<void()> callback);
    void SetOnRedo(std::function<void()> callback);
    void SetOnAddJoint(std::function<void()> callback);
    void SetOnRun(std::function<void()> callback);

    void Draw(bool can_undo, bool can_redo, bool can_save, bool can_run, EditorOptions& options);

private:
    std::function<void()> on_new_data_;
    std::function<void()> on_open_folder_;
    std::function<void()> on_save_;
    std::function<void()> on_undo_;
    std::function<void()> on_redo_;
    std::function<void()> on_add_joint_;
    std::function<void()> on_run_;
};

} // namespace orcisf::gui
