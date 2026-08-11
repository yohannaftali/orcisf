#include "gui/ViewportPanel.h"

#include <cmath>
#include <cstdint>

#include <imgui.h>
#include <ImGuizmo.h>

#include "gui/PanelIcons.h"

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

    dl->AddCircleFilled(center, 3.f, IM_COL32(230, 230, 230, 255));
    for (const AxisSpec& a : axes) {
        ImVec2 tip = project(a.dir);
        dl->AddLine(center, tip, a.color, 2.f);
        dl->AddCircleFilled(tip, 3.f, a.color);
        ImVec2 label_pos(tip.x + (tip.x >= center.x ? 3.f : -12.f), tip.y - 7.f);
        dl->AddText(label_pos, a.color, a.label);
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
    if (!ImGui::Begin("Viewport", open)) {
        ImGui::End();
        return;
    }
    DrawPanelIconHeader(PanelIcon::Viewport, "Viewport");

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
            "or run an optimization from the Run Optimization panel.");
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

    unsigned int texture = renderer_.Render(
        scene, camera_, width, height, selection.kind == SelectionKind::Member ? selection.id : -1);
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
    constexpr float kUcsRadius = 22.f;
    constexpr float kUcsMargin = 34.f;      // horizontal distance from the left edge
    constexpr float kUcsBottomGap = 96.f;   // vertical distance from the bottom edge
    ImVec2 ucs_center(image_min.x + kUcsMargin, image_min.y + image_size.y - kUcsBottomGap);
    DrawUcsIcon(ImGui::GetForegroundDrawList(), ucs_center, kUcsRadius, camera_);

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
            constexpr float kControlWidth = 168.f;
            ImVec2 pos(image_min.x + 8.f, ucs_center.y + kUcsRadius + 8.f);

            ImDrawList* fg = ImGui::GetForegroundDrawList();
            ImVec2 bg_min(pos.x - 6.f, pos.y - 4.f);
            ImVec2 bg_max(pos.x + kControlWidth + 6.f, pos.y + 62.f);
            fg->AddRectFilled(bg_min, bg_max, IM_COL32(20, 20, 24, 200), 4.f);

            ImGui::SetCursorScreenPos(pos);
            ImGui::PushID("plane_offset_overlay");
            ImGui::TextColored(ImVec4(1.f, 0.82f, 0.15f, 1.f), "%s", axis_label);
            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + 20.f));
            ImGui::SetNextItemWidth(kControlWidth);
            ImGui::InputFloat("##offset_input", offset, 0.f, 0.f, "%.3f");
            offset_overlay_capturing |= ImGui::IsItemHovered() || ImGui::IsItemActive();
            ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + 44.f));
            ImGui::SetNextItemWidth(kControlWidth);
            ImGui::SliderFloat("##offset_slider", offset, -20.f, 20.f, "%.3f");
            offset_overlay_capturing |= ImGui::IsItemHovered() || ImGui::IsItemActive();
            ImGui::PopID();
        }
    }

    DrawGizmo(scene, selection, editable, undo, options, on_geometry_changed, image_min.x, image_min.y,
              image_size.x, image_size.y);
    bool gizmo_capturing = ImGuizmo::IsOver() || ImGuizmo::IsUsing();

    // Issue #24: also gate on !offset_overlay_capturing so a click/drag
    // that starts on the plane-offset overlay (which sits on top of the
    // image at that screen position) never starts an orbit/pan -- see the
    // overlay's own comment above for why `hovered` alone can't tell.
    if (hovered && !gizmo_capturing && !offset_overlay_capturing && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        orbiting_ = true;
        drag_pixels_ = 0.f;
    }
    if (hovered && !gizmo_capturing && !offset_overlay_capturing && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
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
    if (hovered && !gizmo_capturing && !offset_overlay_capturing && io.MouseWheel != 0.f) {
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
