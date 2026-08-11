#pragma once

namespace orcisf::gui {

enum class SelectionKind { None, Joint, Member };

// Issue #7's toolbar-driven load placement modes -- "None" leaves
// clicking as plain select (issue #5/#6 behavior), the other two make a
// click place a default load onto whatever's clicked (then edited
// numerically in Properties/the load schedule panel), matching the
// legacy .bbn format's only two load categories (Pembebanan.hpp): a
// uniform distributed load on a member, or a 6-DOF generalized action
// (force/moment) at a joint -- which also covers "point load" and
// "wind/lateral load", both just a joint force in a particular direction.
enum class LoadPlacementMode { None, MemberLoad, JointLoad };

// Issue #22's AutoCAD-style 2D plane-locked drawing: `Free` is the normal
// perspective/orbit camera; the other three switch the viewport to an
// orthographic view looking straight down the named axis (XY locks Z, XZ
// locks Y, YZ locks X) at that axis's current offset (`EditorOptions`'s
// `plane_offset_*` fields below). Lives here (not in Camera.h) so
// EditorOptions can reference it without the viewport/editor layers
// depending on each other in the other direction -- Camera.h includes this
// header for the same reason `gui/viewport/SceneModel.h` already sits
// alongside editor-facing code in this project's layering.
enum class ViewPlane { Free, XY, XZ, YZ };

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
    // Issue #18: while true, clicking empty space in the viewport places a
    // new joint at the click's ray/ground-plane intersection instead of
    // the old "Add Joint always instantly creates one at the scene center"
    // behavior -- matches Connect Joints/the load-placement modes' own
    // click-to-place pattern (and, like them, stays active after each
    // placement so several joints can be added without re-toggling).
    bool add_joint_mode = false;

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

    LoadPlacementMode load_mode = LoadPlacementMode::None;

    // Issue #22: which 2D plane (if any) the viewport is locked to, and
    // each plane's independently-remembered offset along its locked axis
    // (meters) -- switching XY -> XZ -> XY preserves XY's offset rather
    // than resetting it. `ViewportPanel::HandlePicking()`'s add_joint_mode
    // branch forces the locked axis of any placed point to the relevant
    // offset below whenever view_plane != Free.
    ViewPlane view_plane = ViewPlane::Free;
    float plane_offset_xy = 0.f; // XY plane's Z
    float plane_offset_xz = 0.f; // XZ plane's Y
    float plane_offset_yz = 0.f; // YZ plane's X
};

} // namespace orcisf::gui
