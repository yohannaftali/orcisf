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

// Issue #71: the four internal-force components this port can diagram --
// same four (and the same order) `ForceDiagramSample`/`ForceDiagramPanel`'s
// stacked 2D plots already use. Lives here, alongside ViewPlane/
// LoadPlacementMode, for the same reason those do: `EditorOptions` below
// needs it, and this header is deliberately dependency-free so both the
// editor and viewport layers can include it (see ViewPlane's comment).
//
// Deliberately NOT six components: `engine::MemberForces` does carry the
// minor-axis pair too (`shear_z`/`moment_y`, i.e. SAP2000's V3/M2), but
// `gui::ComputeForceDiagram()` (#61, whose V(x)/M(x) formulas were
// carefully unit-verified) only samples the major axis. Adding V3/M2 means
// extending that verified sampling code, so it's a deliberate follow-up,
// not something to bolt on here.
enum class ForceComponent { Axial, Shear, Moment, Torsion };

inline constexpr ForceComponent kAllForceComponents[4] = {ForceComponent::Axial, ForceComponent::Shear,
                                                           ForceComponent::Moment, ForceComponent::Torsion};

// Issue #71: 3D-viewport force-diagram overlay state. All four components
// are independently toggleable and render *simultaneously* -- that was the
// core of the issue's follow-up report ("show 4 type force diagrams in 3d
// drawed structure of each member, we can toggle each force"): the first
// implementation had a single-selection combo box, which structurally
// couldn't show more than one at a time. Each component is drawn in its own
// plane/color so several at once stay readable -- see
// gui/viewport/SceneModel.h's ComputeForceDiagramPlacement().
struct ForceDiagramOptions {
    bool show_axial = false;
    bool show_shear = false;
    bool show_moment = false;
    bool show_torsion = false;

    // Filled 3D diagram plane (SAP2000-style, the default) vs. outline
    // curve only. The outline is always drawn; this adds the filled
    // surface between the curve and the member's own axis.
    bool filled = true;

    // Draw each diagram's peak numeric value as a text label in the
    // viewport (ViewportPanel's overlay, since SceneRenderer has no text
    // capability). Off by default -- with several components and members
    // enabled at once, labels get dense fast.
    bool values = false;

    // Magnitude -> meters-of-offset factor. 1e-5 keeps a typical
    // 50,000-100,000 N*m moment (or 100,000-200,000 N axial force) to a
    // ~0.5-2m offset, comparable to the member's own cross-section size.
    // NOTE: a previous 1e-4 default was a real bug -- it put the diagram
    // 5-20m off an ordinary ~6m member, entirely outside the framed view,
    // which read as "the feature does nothing". Sanity-check any new
    // default against real .str/ANALYSIS_RESULTS magnitudes.
    float scale = 1e-5f;

    // true = every member with force data (a whole-structure overview, the
    // SAP2000-style default); false = only the currently selected member.
    bool all_members = true;

    bool Enabled(ForceComponent c) const {
        switch (c) {
            case ForceComponent::Axial: return show_axial;
            case ForceComponent::Shear: return show_shear;
            case ForceComponent::Moment: return show_moment;
            case ForceComponent::Torsion: return show_torsion;
        }
        return false;
    }

    bool AnyEnabled() const { return show_axial || show_shear || show_moment || show_torsion; }
};

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

    // Issue #60: deformed-shape overlay (epic #58). Only takes effect
    // when the loaded scene actually has analysis results (SceneModel::
    // has_deformation) -- see ViewportPanel::Draw's overlay control,
    // which disables the toggle otherwise rather than silently ignoring
    // it. Scale exaggerates real (millimeter-to-centimeter-scale)
    // structural displacements so they're visible at building scale.
    bool show_deformed_shape = false;
    float deformation_scale = 50.f;

    // Issue #71: 3D-viewport force diagram overlay -- see ForceDiagramOptions
    // above and gui/viewport/SceneModel.h's ComputeForceDiagramPlacement().
    ForceDiagramOptions force_diagram;
};

} // namespace orcisf::gui
