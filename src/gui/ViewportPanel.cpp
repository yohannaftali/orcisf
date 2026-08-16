#include "gui/ViewportPanel.h"

#include <cmath>
#include <cstdint>

#include <imgui.h>
#include <ImGuizmo.h>

#include <string>

#include "gui/PanelIcons.h"
#include "gui/PanelTitles.h"
#include "gui/UiScale.h"

namespace orcisf::gui {

using namespace math3d;

namespace {

// Issue #23: a small always-visible 3-axis UCS (User Coordinate System)
// indicator, hand-drawn on the foreground draw list rather than a real 3D
// gizmo mesh -- matches this project's stated preference for small,
// dependency-free GUI code (see IconToolbar.cpp's hand-drawn icons,
// AGENTS.md's notes on that file). Each world axis direction is projected
// onto the screen via simple dot products against the camera's current
// Right()/Forward()-derived screen basis (the same up-vector derivation
// Camera::Pan()/ScreenRay() already use internally) -- correct in both
// perspective and orthographic/plane-locked (#22) modes since it only
// depends on camera orientation, not projection type, and needs no matrix
// multiply. Purely a draw-list overlay: it never creates an ImGui item, so
// it can't intercept mouse input meant for orbit/pan/click-to-place.
void DrawUcsIcon(ImDrawList* dl, ImVec2 center, float radius, const Camera& camera) {
    Vec3 right = camera.Right();
    Vec3 forward = camera.Forward();
    Vec3 up = Cross(right, forward).Normalized();

    auto project = [&](const Vec3& axis) {
        float sx = Dot(axis, right);
        float sy = Dot(axis, up); // screen-space up; ImGui's Y grows downward, so flip below
        return ImVec2(center.x + sx * radius, center.y - sy * radius);
    };

    struct AxisSpec {
        Vec3 dir;
        ImU32 color;
        const char* label;
    };
    const AxisSpec axes[3] = {
        {{1.f, 0.f, 0.f}, IM_COL32(230, 70, 70, 255), "X"},
        {{0.f, 1.f, 0.f}, IM_COL32(80, 210, 90, 255), "Y"},
        {{0.f, 0.f, 1.f}, IM_COL32(80, 140, 235, 255), "Z"},
    };

    // Issue #31: the dot/line/label offsets are base (100%-scale) pixel
    // values; without Scaled() the whole icon stays a few physical pixels
    // across on a high-DPI monitor.
    const float label_gap = Scaled(3.f);
    const float label_back = Scaled(12.f);
    const float label_rise = Scaled(7.f);
    dl->AddCircleFilled(center, Scaled(3.f), IM_COL32(230, 230, 230, 255));
    for (const AxisSpec& a : axes) {
        ImVec2 tip = project(a.dir);
        dl->AddLine(center, tip, a.color, Scaled(2.f));
        dl->AddCircleFilled(tip, Scaled(3.f), a.color);
        ImVec2 label_pos(tip.x + (tip.x >= center.x ? label_gap : -label_back), tip.y - label_rise);
        dl->AddText(label_pos, a.color, a.label);
    }
}

// Issue #39: joint/member number labels, so a user cross-referencing the
// Joints/Members tables against the 3D view can tell which dot/segment is
// which row. Reuses this project's established hand-drawn-ImDrawList-
// overlay approach (see DrawUcsIcon above / IconToolbar.cpp) rather than a
// new text-rendering dependency -- ImGui's own AddText already does
// everything needed here. Unlike the UCS icon's direction-only dot-product
// projection, a label needs an actual world-to-screen point, so this does
// a real clip-space projection (proj * view * point, perspective divide)
// -- the same view/projection matrices SceneRenderer already renders with,
// just evaluated on the CPU for these few label anchors instead of the GPU.
// Issue #40: classifies a joint's 6 JRL flags into one of the four
// quick-support presets (matching PropertiesPanel.cpp's kPreset* arrays
// exactly, including the UY-not-UZ Roller correction -- see that file's
// comment on why), or "Custom" for any other combination.
const char* ClassifyRestraintPreset(EditableStructure& editable, int joint_id) {
    bool dof[6];
    for (int i = 0; i < 6; ++i) dof[i] = editable.GetJointDof(joint_id, i);
    if (!dof[0] && !dof[1] && !dof[2] && !dof[3] && !dof[4] && !dof[5]) return "Free";
    if (dof[0] && dof[1] && dof[2] && dof[3] && dof[4] && dof[5]) return "Fixed";
    if (dof[0] && dof[1] && dof[2] && !dof[3] && !dof[4] && !dof[5]) return "Pinned";
    if (!dof[0] && dof[1] && !dof[2] && !dof[3] && !dof[4] && !dof[5]) return "Roller";
    return "Custom";
}

// Issue #39's world-to-screen projection (proj * view * point, perspective
// divide), factored out of DrawEntityLabels so issue #50's grid labels can
// reuse the exact same technique rather than a second projection
// mechanism -- both need a real point-in-space projection, unlike
// DrawUcsIcon's direction-only dot products above. Returns false for
// points behind the eye (w <= ~0) or far enough off to the side that
// drawing them would be pointless -- callers just skip that entity/line
// for this frame.
bool ProjectWorldToScreen(const Mat4& vp, ImVec2 image_min, ImVec2 image_size, const Vec3& world,
                           ImVec2& out_screen) {
    float x = world.x, y = world.y, z = world.z;
    float cx = vp.m[0] * x + vp.m[4] * y + vp.m[8] * z + vp.m[12];
    float cy = vp.m[1] * x + vp.m[5] * y + vp.m[9] * z + vp.m[13];
    float cw = vp.m[3] * x + vp.m[7] * y + vp.m[11] * z + vp.m[15];
    if (cw < 1e-4f) return false;
    float ndc_x = cx / cw;
    float ndc_y = cy / cw;
    if (ndc_x < -1.3f || ndc_x > 1.3f || ndc_y < -1.3f || ndc_y > 1.3f) return false;
    out_screen.x = image_min.x + (ndc_x * 0.5f + 0.5f) * image_size.x;
    out_screen.y = image_min.y + (1.f - (ndc_y * 0.5f + 0.5f)) * image_size.y;
    return true;
}

void DrawEntityLabels(ImDrawList* dl, const SceneModel& scene, const Camera& camera, float aspect, ImVec2 image_min,
                       ImVec2 image_size, const Selection& selection, EditableStructure* editable) {
    Mat4 view = camera.ViewMatrix();
    Mat4 proj = camera.ProjectionMatrix(aspect);
    Mat4 vp = Mat4::Multiply(proj, view);

    auto project = [&](const Vec3& world, ImVec2& out_screen) {
        return ProjectWorldToScreen(vp, image_min, image_size, world, out_screen);
    };

    const ImU32 joint_color = IM_COL32(255, 225, 120, 255);
    const ImU32 member_color = IM_COL32(150, 210, 255, 255);
    const float dx = Scaled(5.f);
    const float dy = Scaled(5.f);

    for (const JointVisual& j : scene.joints) {
        ImVec2 screen;
        if (!project(j.pos, screen)) continue;
        std::string label = "J" + std::to_string(j.no_joint);
        // Issue #40: append the selected joint's restraint preset (e.g.
        // "J3 [Pinned]") so clicking a joint shows its support type
        // directly in the viewport, not just in the Properties panel.
        if (editable && selection.kind == SelectionKind::Joint && selection.id == j.no_joint) {
            label += " [";
            label += ClassifyRestraintPreset(*editable, j.no_joint);
            label += "]";
        }
        dl->AddText(ImVec2(screen.x + dx, screen.y - dy), joint_color, label.c_str());
    }
    for (const MemberVisual& m : scene.members) {
        ImVec2 screen;
        if (!project((m.a + m.b) * 0.5f, screen)) continue;
        std::string label = "M" + std::to_string(m.no_batang);
        dl->AddText(ImVec2(screen.x + dx, screen.y - dy), member_color, label.c_str());
    }
}

// Issue #50: X{n}/Z{n} axis labels for the ground-plane grid --
// SceneRenderer::DrawGrid() draws the actual GL line geometry (a real
// scene element, depth-tested against the structure); this is the
// text-label half, following the same DetailingLayout/DetailingPanel (#8)
// split of "one layout function, multiple renderers" so the lines and
// labels can never disagree about where a given grid line actually is.
// Labels use the real world coordinate (e.g. "X5" = world X=5, matching
// the Joints/Properties panels), not an arbitrary line index.
void DrawGridLabels(ImDrawList* dl, const SceneModel& scene, const Camera& camera, float aspect, ImVec2 image_min,
                     ImVec2 image_size) {
    GroundGridLayout layout = ComputeGroundGridLayout(scene);
    Mat4 vp = Mat4::Multiply(camera.ProjectionMatrix(aspect), camera.ViewMatrix());

    // A lighter tint of DrawGrid's dark-cobalt line color, so the label
    // stays associated with the grid at a glance but is still readable
    // against the dark viewport background.
    const ImU32 label_color = IM_COL32(110, 150, 220, 255);
    float x0 = layout.x_index_min * layout.spacing_m;
    float z0 = layout.z_index_min * layout.spacing_m;

    for (int i = layout.x_index_min; i <= layout.x_index_max; ++i) {
        if (i % layout.label_stride != 0) continue;
        float x = i * layout.spacing_m;
        ImVec2 screen;
        if (!ProjectWorldToScreen(vp, image_min, image_size, Vec3{x, layout.y, z0}, screen)) continue;
        std::string label = "X" + std::to_string(static_cast<int>(std::lround(x)));
        dl->AddText(screen, label_color, label.c_str());
    }
    for (int i = layout.z_index_min; i <= layout.z_index_max; ++i) {
        if (i % layout.label_stride != 0) continue;
        float z = i * layout.spacing_m;
        ImVec2 screen;
        if (!ProjectWorldToScreen(vp, image_min, image_size, Vec3{x0, layout.y, z}, screen)) continue;
        std::string label = "Z" + std::to_string(static_cast<int>(std::lround(z)));
        dl->AddText(screen, label_color, label.c_str());
    }
}

} // namespace

void ViewportPanel::FrameScene(const SceneModel& scene) { camera_.FrameBounds(scene.bounds_center, scene.bounds_radius); }

void ViewportPanel::HandlePicking(const SceneModel& scene, Selection& selection, EditableStructure* editable,
                                   UndoStack* undo, EditorOptions& options,
                                   const std::function<void()>& on_geometry_changed, float ndc_x, float ndc_y,
                                   float aspect) {
    Vec3 origin, dir;
    camera_.ScreenRay(ndc_x, ndc_y, aspect, origin, dir);

    if (options.add_joint_mode && editable) {
        auto snap = [&](float v) { return std::round(v / options.grid_size_m) * options.grid_size_m; };
        bool has_pos = false;
        Vec3 pos;

        if (options.view_plane != ViewPlane::Free) {
            // Issue #22: in a 2D plane-locked view, ScreenRay()'s direction
            // is exactly the locked axis (see Camera::ScreenRay's
            // orthographic branch), so any t along it lands on the locked
            // plane -- camera_.distance is a convenient, always-positive
            // choice. The locked coordinate is then forced to the exact
            // configured offset (not just whatever the ray math produced)
            // so floating-point drift never nudges a point off the plane
            // the user explicitly typed/slid to.
            pos = origin + dir * camera_.distance;
            switch (options.view_plane) {
                case ViewPlane::XY: pos.z = options.plane_offset_xy; break;
                case ViewPlane::XZ: pos.y = options.plane_offset_xz; break;
                case ViewPlane::YZ: pos.x = options.plane_offset_yz; break;
                default: break;
            }
            has_pos = true;
        } else {
            // Places at the click ray's intersection with the horizontal
            // plane through the camera's current orbit target (matches
            // "the depth you're already looking at" rather than a fixed
            // world height, since target starts at the loaded structure's
            // bounds center -- see Camera::FrameBounds -- or the origin
            // for an empty scene). Ray parallel to that plane (dir.y ~ 0)
            // or behind the camera (t <= 0) is a no-op, same graceful-skip
            // pattern PickJoint/PickMember use for a miss.
            constexpr float kMinRayPlaneAngle = 1e-4f;
            if (std::fabs(dir.y) > kMinRayPlaneAngle) {
                float t = (camera_.target.y - origin.y) / dir.y;
                if (t > 0.f) {
                    pos = origin + dir * t;
                    has_pos = true;
                }
            }
        }

        if (has_pos) {
            if (options.snap_to_grid && options.grid_size_m > 1e-6f) {
                // Only snap the two free axes -- snapping the locked axis
                // too could silently shift it away from a plane offset the
                // user explicitly typed/slid to (issue #22).
                switch (options.view_plane) {
                    case ViewPlane::XY: pos.x = snap(pos.x); pos.y = snap(pos.y); break;
                    case ViewPlane::XZ: pos.x = snap(pos.x); pos.z = snap(pos.z); break;
                    case ViewPlane::YZ: pos.y = snap(pos.y); pos.z = snap(pos.z); break;
                    case ViewPlane::Free: pos = {snap(pos.x), snap(pos.y), snap(pos.z)}; break;
                }
            }
            if (undo) undo->PushUndo(editable->SdForUndo());
            int joint_id = editable->AddJoint(pos);
            if (joint_id > 0) {
                selection = {SelectionKind::Joint, joint_id};
                if (on_geometry_changed) on_geometry_changed();
            }
        }
        return;
    }

    if (options.load_mode == LoadPlacementMode::MemberLoad && editable) {
        int hit_member = PickMember(scene, origin, dir);
        if (hit_member < 0) return;
        if (undo) undo->PushUndo(editable->SdForUndo());
        constexpr float kDefaultMemberLoad = 5000.f; // N/m, a reasonable starting point to then fine-tune
        editable->SetMemberLoad(hit_member, kDefaultMemberLoad);
        selection = {SelectionKind::Member, hit_member};
        if (on_geometry_changed) on_geometry_changed();
        return;
    }
    if (options.load_mode == LoadPlacementMode::JointLoad && editable) {
        int hit_joint = PickJoint(scene, origin, dir);
        if (hit_joint < 0) return;
        if (undo) undo->PushUndo(editable->SdForUndo());
        constexpr float kDefaultJointLoad[6] = {0.f, -10000.f, 0.f, 0.f, 0.f, 0.f}; // N, downward
        editable->SetJointLoad(hit_joint, kDefaultJointLoad);
        selection = {SelectionKind::Joint, hit_joint};
        if (on_geometry_changed) on_geometry_changed();
        return;
    }

    if (options.connect_mode && editable) {
        int hit_joint = PickJoint(scene, origin, dir);
        if (hit_joint < 0) return;

        if (options.connect_first_joint < 0) {
            options.connect_first_joint = hit_joint;
            selection = {SelectionKind::Joint, hit_joint};
            return;
        }
        if (hit_joint == options.connect_first_joint) return; // clicked the same joint twice

        if (undo) undo->PushUndo(editable->SdForUndo());
        int new_member = editable->AddMember(options.connect_first_joint, hit_joint);
        options.connect_first_joint = -1;
        if (new_member > 0) {
            selection = {SelectionKind::Member, new_member};
            if (on_geometry_changed) on_geometry_changed();
        }
        return;
    }

    int hit_joint = PickJoint(scene, origin, dir);
    if (hit_joint >= 0) {
        selection = {SelectionKind::Joint, hit_joint};
        return;
    }
    int hit_member = PickMember(scene, origin, dir);
    if (hit_member >= 0) {
        selection = {SelectionKind::Member, hit_member};
        return;
    }
    selection.Clear();
}

void ViewportPanel::DrawGizmo(const SceneModel& scene, Selection& selection, EditableStructure* editable,
                               UndoStack* undo, EditorOptions& options,
                               const std::function<void()>& on_geometry_changed, float image_min_x,
                               float image_min_y, float image_w, float image_h) {
    if (!editable || selection.kind != SelectionKind::Joint) {
        gizmo_was_using_ = false;
        return;
    }
    const JointVisual* jv = nullptr;
    for (const JointVisual& j : scene.joints) {
        if (j.no_joint == selection.id) {
            jv = &j;
            break;
        }
    }
    if (!jv) return;

    // Issue #22: ImGuizmo needs to know when the projection is
    // orthographic (locked-plane view) vs. perspective, or its gizmo
    // hit-testing/rendering math would assume the wrong projection type.
    ImGuizmo::SetOrthographic(camera_.IsOrthographic());
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(image_min_x, image_min_y, image_w, image_h);

    float aspect = image_w / image_h;
    Mat4 view = camera_.ViewMatrix();
    Mat4 proj = camera_.ProjectionMatrix(aspect);
    Mat4 model = Mat4::FromBasis(jv->pos, Vec3{1, 0, 0}, Vec3{0, 1, 0}, Vec3{0, 0, 1}, Vec3{1, 1, 1});

    float snap[3] = {options.grid_size_m, options.grid_size_m, options.grid_size_m};
    bool changed = ImGuizmo::Manipulate(view.m, proj.m, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, model.m, nullptr,
                                         options.snap_to_grid ? snap : nullptr);

    bool using_now = ImGuizmo::IsUsing();
    if (using_now && !gizmo_was_using_ && undo) {
        undo->PushUndo(editable->SdForUndo());
    }
    gizmo_was_using_ = using_now;

    if (changed) {
        Vec3 new_pos{model.m[12], model.m[13], model.m[14]};
        editable->MoveJoint(selection.id, new_pos);
        if (on_geometry_changed) on_geometry_changed();
    }
}

void ViewportPanel::Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
                          UndoStack* undo, EditorOptions& options,
                          const std::function<void()>& on_geometry_changed) {
    if (!ImGui::Begin(PanelWindowTitle(kViewportId, "Viewport").c_str(), open)) {
        ImGui::End();
        return;
    }
    ImVec2 avail = ImGui::GetContentRegionAvail();
    int width = static_cast<int>(avail.x);
    int height = static_cast<int>(avail.y);

    if (width < 4 || height < 4) {
        ImGui::End();
        return;
    }

    // Issue #18: only bail out to the static placeholder when there's no
    // editable dataset at all -- a blank-but-editable structure (0 joints,
    // right after File > New Data or the Add Joint bootstrap) must still
    // render the interactive (empty) 3D view below, or clicking to place
    // the very first joint (add_joint_mode) would have nothing to click
    // on. `editable` is only non-null once *something* (even a blank
    // in-memory structure) has been loaded, see Application::LoadStructure.
    if (!editable && scene.Empty()) {
        ImGui::TextDisabled(
            "No dataset loaded.\n"
            "Use File > Open Folder... to load an Optimasi Beton/Example/* dataset,\n"
            "or run an optimization from the Optimization panel.");
        ImGui::End();
        return;
    }

    // Issue #22: keep the camera's plane-lock state in sync with
    // options.view_plane/plane_offset_* every frame (cheap -- SetViewPlane
    // just assigns a few fields) rather than only reacting to a one-shot
    // toggle event, so a slider drag on the offset control below takes
    // effect immediately without extra plumbing.
    float active_offset = 0.f;
    switch (options.view_plane) {
        case ViewPlane::XY: active_offset = options.plane_offset_xy; break;
        case ViewPlane::XZ: active_offset = options.plane_offset_xz; break;
        case ViewPlane::YZ: active_offset = options.plane_offset_yz; break;
        case ViewPlane::Free: break;
    }
    camera_.SetViewPlane(options.view_plane, active_offset);

    unsigned int texture =
        renderer_.Render(scene, camera_, width, height, selection.kind == SelectionKind::Member ? selection.id : -1,
                          options.show_deformed_shape, options.deformation_scale);
    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(texture)), avail, ImVec2(0, 1), ImVec2(1, 0));

    ImVec2 image_min = ImGui::GetItemRectMin();
    ImVec2 image_size = ImGui::GetItemRectSize();
    bool hovered = ImGui::IsItemHovered();
    ImGuiIO& io = ImGui::GetIO();

    // Issue #23: UCS icon, always visible (free/perspective and
    // plane-locked views alike) in the bottom-left corner. Issue #24
    // raised its vertical position from the very bottom (kUcsBottomGap)
    // so the interactive plane-offset control below has room to render
    // underneath it without running off the bottom of the viewport.
    // Issue #31: DPI-scaled base values -- see gui/UiScale.h. The bottom
    // gap in particular is a layout constant (it reserves room for the
    // plane-offset overlay below), so leaving it unscaled would overlap the
    // two on a high-DPI monitor, not merely shrink them.
    const float ucs_radius = Scaled(22.f);
    const float ucs_margin = Scaled(34.f);      // horizontal distance from the left edge
    const float ucs_bottom_gap = Scaled(96.f);  // vertical distance from the bottom edge
    ImVec2 ucs_center(image_min.x + ucs_margin, image_min.y + image_size.y - ucs_bottom_gap);

    // Issue #51: the UCS icon and both label overlays below used to draw
    // onto GetForegroundDrawList(), which by Dear ImGui's own compositing
    // order (background draw list, then every window in z-order, then the
    // foreground draw list last) renders on top of *every* window -- a
    // menu bar dropdown included. Clipping alone can't fix that (a clip
    // only bounds *where* pixels land, not which layer they composite
    // into), so this now draws on this "Viewport" window's own draw list
    // instead: it participates in normal window z-ordering (correctly
    // drawn *behind* a popup/menu opened afterward the same frame) and is
    // already clipped to the window's own content region by Dear ImGui,
    // which also happens to fix the "label bleeds into a neighboring
    // docked panel" report. The explicit PushClipRect/PopClipRect below is
    // kept anyway for the exact image rect (a hair tighter than the
    // window's full content region, e.g. excludes the plane-offset
    // overlay's own area) rather than relying on the window clip alone.
    ImDrawList* overlay_dl = ImGui::GetWindowDrawList();
    overlay_dl->PushClipRect(image_min, ImVec2(image_min.x + image_size.x, image_min.y + image_size.y), true);

    DrawUcsIcon(overlay_dl, ucs_center, ucs_radius, camera_);

    // Issue #39/#40: joint/member number labels (+ the selected joint's
    // restraint preset), drawn after the UCS icon so they layer on top of
    // it if one ever happens to land nearby.
    DrawEntityLabels(overlay_dl, scene, camera_, image_size.x / image_size.y, image_min, image_size, selection,
                      editable);

    // Issue #50: ground-plane grid axis labels (the GL line geometry
    // itself is drawn inside renderer_.Render() above, as part of the 3D
    // scene). Drawn every frame regardless of view_plane -- the grid is
    // visible in both perspective and the X-Z locked orthographic view.
    DrawGridLabels(overlay_dl, scene, camera_, image_size.x / image_size.y, image_min, image_size);

    overlay_dl->PopClipRect();

    // Issue #24: the plane-offset control used to live only in Toolbar's
    // "View Plane" menu, which had to be reopened for every adjustment --
    // with the default 0.000 offset there was no discoverable way to
    // actually move the locked plane while looking at the 3D view. This
    // is now a real interactive ImGui overlay (not just the old
    // foreground-drawlist read-only text), docked directly under the UCS
    // icon: a typeable field and a slider, both editing the same
    // options.plane_offset_xy/xz/yz field Toolbar's plane *selector* (kept
    // there) and Camera::SetViewPlane/ViewportPanel::HandlePicking already
    // read every frame -- see the top of this function for the sync and
    // the add_joint_mode branch above for where placement consumes it.
    // Positioned via SetCursorScreenPos within this same "Viewport" window
    // (not a separate Begin/End) so it draws on top of the already-submitted
    // Image() and naturally take input priority over it -- ImGui gives a
    // later-drawn widget at the same screen position hover/click priority
    // over an earlier one, so orbit/pan/click-to-place on the rest of the
    // image is unaffected; only actually interacting with this widget
    // captures input.
    // Set below (only relevant while a plane is locked) if the mouse
    // interacted with the offset overlay this frame -- `hovered`, captured
    // above right after Image(), predates these widgets being submitted,
    // so it alone can't tell orbit-click-start (below) to back off when
    // the user is actually dragging the slider/typing in the field.
    bool offset_overlay_capturing = false;

    if (options.view_plane != ViewPlane::Free) {
        float* offset = nullptr;
        const char* axis_label = "";
        switch (options.view_plane) {
            case ViewPlane::XY: offset = &options.plane_offset_xy; axis_label = "Plane X-Y -- Z offset (m)"; break;
            case ViewPlane::XZ: offset = &options.plane_offset_xz; axis_label = "Plane X-Z -- Y offset (m)"; break;
            case ViewPlane::YZ: offset = &options.plane_offset_yz; axis_label = "Plane Y-Z -- X offset (m)"; break;
            case ViewPlane::Free: offset = nullptr; break;
        }
        if (offset) {
            // Issue #31: row offsets derived from the *current* text/frame
            // heights rather than the hand-tuned 20/44/62 pixel literals
            // this used to carry -- those were measured against the 13px
            // font and silently overlapped once the font grew with DPI.
            const float control_width = Scaled(168.f);
            const float gap = Scaled(4.f);
            const float text_h = ImGui::GetTextLineHeight();
            const float frame_h = ImGui::GetFrameHeight();
            ImVec2 pos(image_min.x + Scaled(8.f), ucs_center.y + ucs_radius + Scaled(8.f));
            const float input_y = pos.y + text_h + gap;
            const float slider_y = input_y + frame_h + gap;

            ImDrawList* fg = ImGui::GetForegroundDrawList();
            ImVec2 bg_min(pos.x - Scaled(6.f), pos.y - gap);
            ImVec2 bg_max(pos.x + control_width + Scaled(6.f), slider_y + frame_h + gap);
            fg->AddRectFilled(bg_min, bg_max, IM_COL32(20, 20, 24, 200), Scaled(4.f));

            ImGui::SetCursorScreenPos(pos);
            ImGui::PushID("plane_offset_overlay");
            ImGui::TextColored(ImVec4(1.f, 0.82f, 0.15f, 1.f), "%s", axis_label);
            ImGui::SetCursorScreenPos(ImVec2(pos.x, input_y));
            ImGui::SetNextItemWidth(control_width);
            ImGui::InputFloat("##offset_input", offset, 0.f, 0.f, "%.3f");
            offset_overlay_capturing |= ImGui::IsItemHovered() || ImGui::IsItemActive();
            ImGui::SetCursorScreenPos(ImVec2(pos.x, slider_y));
            ImGui::SetNextItemWidth(control_width);
            ImGui::SliderFloat("##offset_slider", offset, -20.f, 20.f, "%.3f");
            offset_overlay_capturing |= ImGui::IsItemHovered() || ImGui::IsItemActive();
            ImGui::PopID();
        }
    }

    // Issue #60: deformed-shape overlay toggle + scale slider, top-left
    // corner (bottom-left is already the UCS icon + plane-offset control,
    // #23/#24). Same technique as that overlay: real ImGui widgets (not
    // GetForegroundDrawList() -- see issue #51's compositing-layer
    // lesson), positioned with SetCursorScreenPos within this same
    // "Viewport" window so they draw on top of the already-submitted
    // Image() and naturally win input priority over it at that screen
    // position.
    bool deform_overlay_capturing = false;
    {
        const float control_width = Scaled(168.f);
        const float gap = Scaled(4.f);
        const float text_h = ImGui::GetTextLineHeight();
        const float frame_h = ImGui::GetFrameHeight();
        ImVec2 pos(image_min.x + Scaled(8.f), image_min.y + Scaled(8.f));
        const float checkbox_y = pos.y + text_h + gap;
        const float slider_y = checkbox_y + frame_h + gap;

        ImDrawList* fg = ImGui::GetForegroundDrawList();
        ImVec2 bg_min(pos.x - Scaled(6.f), pos.y - gap);
        ImVec2 bg_max(pos.x + control_width + Scaled(6.f), slider_y + frame_h + gap);
        fg->AddRectFilled(bg_min, bg_max, IM_COL32(20, 20, 24, 200), Scaled(4.f));

        ImGui::SetCursorScreenPos(pos);
        ImGui::PushID("deformed_shape_overlay");
        ImGui::TextColored(ImVec4(0.30f, 0.95f, 0.95f, 1.f), "Deformed Shape");

        // Issue #60 AC: disabled (with a tooltip explaining why) rather
        // than silently no-op'd when the loaded scene has no analysis
        // results yet -- same reasoning RunPanel/PDF export already gate
        // on has_run_results_ for (issues #9/#25).
        ImGui::BeginDisabled(!scene.has_deformation);
        ImGui::SetCursorScreenPos(ImVec2(pos.x, checkbox_y));
        ImGui::Checkbox("Show##deformed_toggle", &options.show_deformed_shape);
        if (!scene.has_deformation && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Run an optimization first to compute displacement results.");
        }
        deform_overlay_capturing |= ImGui::IsItemHovered() || ImGui::IsItemActive();
        ImGui::SetCursorScreenPos(ImVec2(pos.x, slider_y));
        ImGui::SetNextItemWidth(control_width);
        ImGui::SliderFloat("##deformed_scale", &options.deformation_scale, 1.f, 500.f, "x%.0f");
        deform_overlay_capturing |= ImGui::IsItemHovered() || ImGui::IsItemActive();
        ImGui::EndDisabled();
        ImGui::PopID();
    }

    DrawGizmo(scene, selection, editable, undo, options, on_geometry_changed, image_min.x, image_min.y,
              image_size.x, image_size.y);
    bool gizmo_capturing = ImGuizmo::IsOver() || ImGuizmo::IsUsing();

    // Issue #24/#60: also gate on !offset_overlay_capturing /
    // !deform_overlay_capturing so a click/drag that starts on either
    // top-of-image overlay never starts an orbit/pan -- see those
    // overlays' own comments above for why `hovered` alone can't tell.
    if (hovered && !gizmo_capturing && !offset_overlay_capturing && !deform_overlay_capturing &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        orbiting_ = true;
        drag_pixels_ = 0.f;
    }
    if (hovered && !gizmo_capturing && !offset_overlay_capturing && !deform_overlay_capturing &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        panning_ = true;
    }

    if (orbiting_) {
        // Issue #22: orbiting would rotate away from the locked plane, so
        // a left-drag pans-in-place (visually a no-op, matching "nothing
        // to orbit to") instead of tilting the view -- pan/zoom still work
        // normally so the user can still navigate within the locked plane.
        if (!camera_.IsOrthographic()) {
            camera_.Orbit(-io.MouseDelta.x * 0.35f, io.MouseDelta.y * 0.35f);
        }
        drag_pixels_ += std::fabs(io.MouseDelta.x) + std::fabs(io.MouseDelta.y);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            orbiting_ = false;
            if (drag_pixels_ < 4.f && hovered && image_size.x > 0 && image_size.y > 0) {
                float px = (io.MousePos.x - image_min.x) / image_size.x;
                float py = (io.MousePos.y - image_min.y) / image_size.y;
                float ndc_x = px * 2.f - 1.f;
                float ndc_y = 1.f - py * 2.f;
                float aspect = image_size.x / image_size.y;
                HandlePicking(scene, selection, editable, undo, options, on_geometry_changed, ndc_x, ndc_y, aspect);
            }
        }
    }
    if (panning_) {
        camera_.Pan(io.MouseDelta.x, io.MouseDelta.y);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            panning_ = false;
        }
    }
    if (hovered && !gizmo_capturing && !offset_overlay_capturing && !deform_overlay_capturing &&
        io.MouseWheel != 0.f) {
        camera_.Zoom(io.MouseWheel);
    }

    // Keyboard orbit/zoom while the viewport is hovered or focused --
    // issue #5's "mouse + keyboard" camera-control acceptance criterion.
    // Orbit keys are skipped in plane-locked mode (issue #22), same
    // reasoning as the mouse-drag orbit guard above; zoom still works.
    if (hovered || ImGui::IsWindowFocused()) {
        float dt = io.DeltaTime;
        float orbit_speed = 90.f * dt; // degrees/sec
        if (!camera_.IsOrthographic()) {
            if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) camera_.Orbit(-orbit_speed, 0.f);
            if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) camera_.Orbit(orbit_speed, 0.f);
            if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) camera_.Orbit(0.f, orbit_speed);
            if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) camera_.Orbit(0.f, -orbit_speed);
        }
        if (ImGui::IsKeyDown(ImGuiKey_Equal) || ImGui::IsKeyDown(ImGuiKey_KeypadAdd)) camera_.Zoom(dt * 10.f);
        if (ImGui::IsKeyDown(ImGuiKey_Minus) || ImGui::IsKeyDown(ImGuiKey_KeypadSubtract)) camera_.Zoom(-dt * 10.f);
    }

    ImGui::End();
}

} // namespace orcisf::gui
