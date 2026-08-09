#include "gui/ViewportPanel.h"

#include <cmath>
#include <cstdint>

#include <imgui.h>
#include <ImGuizmo.h>

namespace orcisf::gui {

using namespace math3d;

void ViewportPanel::FrameScene(const SceneModel& scene) { camera_.FrameBounds(scene.bounds_center, scene.bounds_radius); }

void ViewportPanel::HandlePicking(const SceneModel& scene, Selection& selection, EditableStructure* editable,
                                   UndoStack* undo, EditorOptions& options,
                                   const std::function<void()>& on_geometry_changed, float ndc_x, float ndc_y,
                                   float aspect) {
    Vec3 origin, dir;
    camera_.ScreenRay(ndc_x, ndc_y, aspect, origin, dir);

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

    ImGuizmo::SetOrthographic(false);
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

    ImVec2 avail = ImGui::GetContentRegionAvail();
    int width = static_cast<int>(avail.x);
    int height = static_cast<int>(avail.y);

    if (width < 4 || height < 4 || scene.Empty()) {
        ImGui::TextDisabled(
            "No dataset loaded.\n"
            "Use File > Open Folder... to load an Optimasi Beton/Example/* dataset,\n"
            "or run an optimization from the Run Optimization panel.");
        ImGui::End();
        return;
    }

    unsigned int texture = renderer_.Render(
        scene, camera_, width, height, selection.kind == SelectionKind::Member ? selection.id : -1);
    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(texture)), avail, ImVec2(0, 1), ImVec2(1, 0));

    ImVec2 image_min = ImGui::GetItemRectMin();
    ImVec2 image_size = ImGui::GetItemRectSize();
    bool hovered = ImGui::IsItemHovered();
    ImGuiIO& io = ImGui::GetIO();

    DrawGizmo(scene, selection, editable, undo, options, on_geometry_changed, image_min.x, image_min.y,
              image_size.x, image_size.y);
    bool gizmo_capturing = ImGuizmo::IsOver() || ImGuizmo::IsUsing();

    if (hovered && !gizmo_capturing && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        orbiting_ = true;
        drag_pixels_ = 0.f;
    }
    if (hovered && !gizmo_capturing && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        panning_ = true;
    }

    if (orbiting_) {
        camera_.Orbit(-io.MouseDelta.x * 0.35f, io.MouseDelta.y * 0.35f);
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
    if (hovered && !gizmo_capturing && io.MouseWheel != 0.f) {
        camera_.Zoom(io.MouseWheel);
    }

    // Keyboard orbit/zoom while the viewport is hovered or focused --
    // issue #5's "mouse + keyboard" camera-control acceptance criterion.
    if (hovered || ImGui::IsWindowFocused()) {
        float dt = io.DeltaTime;
        float orbit_speed = 90.f * dt; // degrees/sec
        if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) camera_.Orbit(-orbit_speed, 0.f);
        if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) camera_.Orbit(orbit_speed, 0.f);
        if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) camera_.Orbit(0.f, orbit_speed);
        if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) camera_.Orbit(0.f, -orbit_speed);
        if (ImGui::IsKeyDown(ImGuiKey_Equal) || ImGui::IsKeyDown(ImGuiKey_KeypadAdd)) camera_.Zoom(dt * 10.f);
        if (ImGui::IsKeyDown(ImGuiKey_Minus) || ImGui::IsKeyDown(ImGuiKey_KeypadSubtract)) camera_.Zoom(-dt * 10.f);
    }

    ImGui::End();
}

} // namespace orcisf::gui
