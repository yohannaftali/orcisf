#include "gui/Toolbar.h"

#include <imgui.h>

namespace orcisf::gui {

void Toolbar::SetOnNewData(std::function<void()> callback) { on_new_data_ = std::move(callback); }
void Toolbar::SetOnSave(std::function<void()> callback) { on_save_ = std::move(callback); }
void Toolbar::SetOnSaveAs(std::function<void()> callback) { on_save_as_ = std::move(callback); }
void Toolbar::SetOnOpenFolder(std::function<void()> callback) { on_open_folder_ = std::move(callback); }
void Toolbar::SetOnUndo(std::function<void()> callback) { on_undo_ = std::move(callback); }
void Toolbar::SetOnRedo(std::function<void()> callback) { on_redo_ = std::move(callback); }
void Toolbar::SetOnAddJoint(std::function<void()> callback) { on_add_joint_ = std::move(callback); }
void Toolbar::SetOnSaveLoads(std::function<void()> callback) { on_save_loads_ = std::move(callback); }
void Toolbar::SetOnExportText(std::function<void()> callback) { on_export_text_ = std::move(callback); }
void Toolbar::SetOnExportPdf(std::function<void()> callback) { on_export_pdf_ = std::move(callback); }
void Toolbar::SetOnExportInf(std::function<void()> callback) { on_export_inf_ = std::move(callback); }

void Toolbar::Draw(bool can_undo, bool can_redo, bool can_save, bool can_export_text, bool can_export_pdf,
                    bool can_export_inf, EditorOptions& options) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Data")) {
                if (on_new_data_) on_new_data_();
            }
            if (ImGui::MenuItem("Open Data...")) {
                if (on_open_folder_) on_open_folder_();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", "Ctrl+S", false, can_export_text)) {
                if (on_save_) on_save_();
            }
            if (ImGui::MenuItem("Save As...", nullptr, false, can_export_text)) {
                if (on_save_as_) on_save_as_();
            }
            if (ImGui::MenuItem("Save Loads (.bbn)", nullptr, false, can_save)) {
                if (on_save_loads_) on_save_loads_();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Export PDF...", nullptr, false, can_export_pdf)) {
                if (on_export_pdf_) on_export_pdf_();
            }
            if (ImGui::MenuItem("Export Text...", nullptr, false, can_export_text)) {
                if (on_export_text_) on_export_text_();
            }
            if (ImGui::MenuItem("Export INF Preview (Koordinat/Batang/Pengekang)...", nullptr, false,
                                 can_export_inf)) {
                if (on_export_inf_) on_export_inf_();
            }
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
                options.load_mode = LoadPlacementMode::None;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Snap to Grid", nullptr, options.snap_to_grid)) {
                options.snap_to_grid = !options.snap_to_grid;
            }
            ImGui::SetNextItemWidth(120.f);
            ImGui::SliderFloat("Grid size (m)", &options.grid_size_m, 0.1f, 2.f, "%.2f");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Loads")) {
            bool none = options.load_mode == LoadPlacementMode::None;
            bool member_load = options.load_mode == LoadPlacementMode::MemberLoad;
            bool joint_load = options.load_mode == LoadPlacementMode::JointLoad;
            if (ImGui::MenuItem("Select (no placement)", nullptr, none)) {
                options.load_mode = LoadPlacementMode::None;
            }
            if (ImGui::MenuItem("Add Member Load (distributed)", nullptr, member_load)) {
                options.load_mode = LoadPlacementMode::MemberLoad;
                options.connect_mode = false;
            }
            if (ImGui::MenuItem("Add Joint Load (point / moment / wind)", nullptr, joint_load)) {
                options.load_mode = LoadPlacementMode::JointLoad;
                options.connect_mode = false;
            }
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
        } else if (options.load_mode == LoadPlacementMode::MemberLoad) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 0.95f, 1.f), "Click a member to add a distributed load");
        } else if (options.load_mode == LoadPlacementMode::JointLoad) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 0.95f, 1.f), "Click a joint to add a point load");
        }
        ImGui::EndMainMenuBar();
    }
}

} // namespace orcisf::gui
