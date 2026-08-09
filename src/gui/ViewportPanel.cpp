#include "gui/ViewportPanel.h"

#include <cmath>
#include <cstdint>

#include <imgui.h>

namespace orcisf::gui {

using namespace math3d;

void ViewportPanel::FrameScene(const SceneModel& scene) { camera_.FrameBounds(scene.bounds_center, scene.bounds_radius); }

void ViewportPanel::Draw(bool* open, const SceneModel& scene, int& selected_member) {
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

    unsigned int texture = renderer_.Render(scene, camera_, width, height, selected_member);
    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(texture)), avail, ImVec2(0, 1), ImVec2(1, 0));

    ImVec2 image_min = ImGui::GetItemRectMin();
    ImVec2 image_size = ImGui::GetItemRectSize();
    bool hovered = ImGui::IsItemHovered();
    ImGuiIO& io = ImGui::GetIO();

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        orbiting_ = true;
        drag_pixels_ = 0.f;
    }
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
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
                Vec3 origin, dir;
                camera_.ScreenRay(ndc_x, ndc_y, aspect, origin, dir);
                selected_member = PickMember(scene, origin, dir);
            }
        }
    }
    if (panning_) {
        camera_.Pan(io.MouseDelta.x, io.MouseDelta.y);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            panning_ = false;
        }
    }
    if (hovered && io.MouseWheel != 0.f) {
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
