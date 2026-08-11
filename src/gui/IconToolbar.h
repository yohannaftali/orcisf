#pragma once

#include <functional>

#include "gui/IconVisibility.h"
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
    //
    // Issue #31: a function of the current DPI scale, not a compile-time
    // constant. It was `static constexpr float kHeight = 36.f`, which on a
    // 200% monitor left the toolbar half the height of the menu bar above
    // it (with correspondingly tiny 28px buttons) while every other metric
    // around it had doubled. Application::OnFrame() shrinks the dockspace
    // work area by exactly this, so the two can never disagree.
    static float Height();

    // Base (100%-scale) row height. Only exists so the scaling is stated
    // in one place; call Height() everywhere else.
    static constexpr float kBaseHeight = 36.f;

    void SetOnNewData(std::function<void()> callback);
    void SetOnOpenFolder(std::function<void()> callback);
    void SetOnSave(std::function<void()> callback);
    void SetOnUndo(std::function<void()> callback);
    void SetOnRedo(std::function<void()> callback);
    void SetOnAddJoint(std::function<void()> callback);
    void SetOnRun(std::function<void()> callback);

    // Issue #37: visibility gates which buttons actually draw (a hidden
    // one's slot is skipped, the row reflows) -- see IconVisibility.h.
    void Draw(bool can_undo, bool can_redo, bool can_save, bool can_run, EditorOptions& options,
              const IconVisibility& visibility);

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
