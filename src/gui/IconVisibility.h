#pragma once

namespace orcisf::gui {

// Issue #37: per-icon show/hide toggles for IconToolbar's fixed 8-button
// row, editable from the View menu's new "Menubar" section. Issue #14's
// own notes anticipated this as the natural follow-up once the initial
// fixed icon set shipped ("a future issue can add customization on top of
// this without changing how IconButton() draws/wires a button") -- this is
// that follow-up, scoped to visibility only (not reordering/adding icons).
// A hidden icon's slot is skipped entirely (the row reflows), not left as
// a blank gap -- see IconToolbar::Draw()'s SameLine handling.
struct IconVisibility {
    bool new_data = true;
    bool open_data = true;
    bool save = true;
    bool undo = true;
    bool redo = true;
    bool add_joint = true;
    bool connect_joints = true;
    bool run = true;
};

} // namespace orcisf::gui
