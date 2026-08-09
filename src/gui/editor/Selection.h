#pragma once

namespace orcisf::gui {

enum class SelectionKind { None, Joint, Member };

// Shared selection state: what's picked in the 3D viewport, read/written
// by ViewportPanel (picking, gizmo drag) and read by PropertiesPanel
// (numeric editing, delete button) -- both act on the same joint/member.
struct Selection {
    SelectionKind kind = SelectionKind::None;
    int id = -1; // no_joint or no_batang, depending on kind

    void Clear() {
        kind = SelectionKind::None;
        id = -1;
    }
};

// Editor-wide interaction options, toggled from the Toolbar and read by
// ViewportPanel's click handling.
struct EditorOptions {
    // While true, clicking joints in the viewport connects them with a new
    // member (issue #6's "add members ... directly in the 3D viewport")
    // instead of just selecting -- first click picks the start joint,
    // second click (a different joint) creates the member and stays in
    // connect mode so the user can chain several members without
    // re-toggling the button.
    bool connect_mode = false;
    int connect_first_joint = -1;

    bool snap_to_grid = false;
    float grid_size_m = 0.5f;
};

} // namespace orcisf::gui
