#include "gui/Toolbar.h"

#include <imgui.h>

namespace orcisf::gui {

void Toolbar::SetOnOpenFolder(std::function<void()> callback) { on_open_folder_ = std::move(callback); }
void Toolbar::SetOnUndo(std::function<void()> callback) { on_undo_ = std::move(callback); }
void Toolbar::SetOnRedo(std::function<void()> callback) { on_redo_ = std::move(callback); }
void Toolbar::SetOnAddJoint(std::function<void()> callback) { on_add_joint_ = std::move(callback); }

void Toolbar::Draw(bool can_undo, bool can_redo, EditorOptions& options) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Folder...")) {
                if (on_open_folder_) on_open_folder_();
            }
            ImGui::MenuItem("Save", nullptr, false, false);
            ImGui::MenuItem("Export PDF...", nullptr, false, false);
            ImGui::MenuItem("Export Text...", nullptr, false, false);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, can_undo)) {
                if (on_undo_) on_undo_();
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, can_redo)) {
                if (on_redo_) on_redo_();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Add Joint")) {
                if (on_add_joint_) on_add_joint_();
            }
            if (ImGui::MenuItem("Connect Joints (add member)", nullptr, options.connect_mode)) {
                options.connect_mode = !options.connect_mode;
                options.connect_first_joint = -1;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Snap to Grid", nullptr, options.snap_to_grid)) {
                options.snap_to_grid = !options.snap_to_grid;
            }
            ImGui::SetNextItemWidth(120.f);
            ImGui::SliderFloat("Grid size (m)", &options.grid_size_m, 0.1f, 2.f, "%.2f");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Run")) {
            ImGui::MenuItem("Optimize...", nullptr, false, false);
            ImGui::MenuItem("Cancel", nullptr, false, false);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Viewport", nullptr, false, false);
            ImGui::MenuItem("Properties", nullptr, false, false);
            ImGui::MenuItem("Log", nullptr, false, false);
            ImGui::EndMenu();
        }
        if (options.connect_mode) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.f, 0.82f, 0.15f, 1.f), "Connect mode: click two joints to add a member");
        }
        ImGui::EndMainMenuBar();
    }
}

} // namespace orcisf::gui
