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
void Toolbar::SetOnViewLayout(std::function<void(ViewLayoutPreset)> callback) { on_view_layout_ = std::move(callback); }
void Toolbar::SetTitleBarDrawer(std::function<void()> drawer) { on_title_bar_drawer_ = std::move(drawer); }

void Toolbar::Draw(bool can_undo, bool can_redo, bool can_save, bool can_export_text, bool can_export_pdf,
                    bool can_export_inf, ViewLayoutPreset current_layout, EditorOptions& options) {
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
            if (ImGui::MenuItem("Add Joint (click in viewport to place)", nullptr, options.add_joint_mode)) {
                options.add_joint_mode = !options.add_joint_mode;
                if (options.add_joint_mode) {
                    options.connect_mode = false;
                    options.connect_first_joint = -1;
                    options.load_mode = LoadPlacementMode::None;
                    if (on_add_joint_) on_add_joint_();
                }
            }
            if (ImGui::MenuItem("Connect Joints (add member)", nullptr, options.connect_mode)) {
                options.connect_mode = !options.connect_mode;
                options.connect_first_joint = -1;
                options.add_joint_mode = false;
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
        // Issue #22: 2D plane-locked drawing -- switches the viewport
        // between the normal perspective/orbit camera and an orthographic
        // view locked to X-Y/X-Z/Y-Z, each with its own independently-
        // remembered offset along the locked axis (see EditorOptions'
        // plane_offset_* fields). Just plane *selection* here -- issue #24
        // moved the offset input/slider itself into the viewport (docked
        // under the UCS icon, ViewportPanel::Draw()) since a menu that has
        // to be reopened for every adjustment made the offset effectively
        // undiscoverable/unreachable while actually looking at the locked
        // plane. Don't re-add a second offset editor here; one place to
        // edit `plane_offset_xy`/`_xz`/`_yz` is enough.
        if (ImGui::BeginMenu("View Plane")) {
            if (ImGui::MenuItem("Free (perspective/orbit)", nullptr, options.view_plane == ViewPlane::Free)) {
                options.view_plane = ViewPlane::Free;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("X-Y plane (locks Z)", nullptr, options.view_plane == ViewPlane::XY)) {
                options.view_plane = ViewPlane::XY;
            }
            if (ImGui::MenuItem("X-Z plane (locks Y)", nullptr, options.view_plane == ViewPlane::XZ)) {
                options.view_plane = ViewPlane::XZ;
            }
            if (ImGui::MenuItem("Y-Z plane (locks X)", nullptr, options.view_plane == ViewPlane::YZ)) {
                options.view_plane = ViewPlane::YZ;
            }
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
                options.add_joint_mode = false;
            }
            if (ImGui::MenuItem("Add Joint Load (point / moment / wind)", nullptr, joint_load)) {
                options.load_mode = LoadPlacementMode::JointLoad;
                options.connect_mode = false;
                options.add_joint_mode = false;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Run")) {
            ImGui::MenuItem("Optimize...", nullptr, false, false);
            ImGui::MenuItem("Cancel", nullptr, false, false);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Default", nullptr, current_layout == ViewLayoutPreset::Default)) {
                if (on_view_layout_) on_view_layout_(ViewLayoutPreset::Default);
            }
            if (ImGui::MenuItem("Design", nullptr, current_layout == ViewLayoutPreset::Design)) {
                if (on_view_layout_) on_view_layout_(ViewLayoutPreset::Design);
            }
            if (ImGui::MenuItem("Optimization", nullptr, current_layout == ViewLayoutPreset::Optimization)) {
                if (on_view_layout_) on_view_layout_(ViewLayoutPreset::Optimization);
            }
            ImGui::EndMenu();
        }
        // Issue #18: AutoCAD-command-line-style step-by-step guidance while
        // a click-to-place mode is active. Each of these modes already
        // stays active after a successful placement (ViewportPanel keeps
        // options.add_joint_mode/connect_mode/load_mode set, only clearing
        // per-placement scratch state like connect_first_joint) so the
        // hint text's job is just to say what the *next* click does --
        // the "place another without reopening a menu" behavior itself
        // already exists.
        if (options.add_joint_mode) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.45f, 0.9f, 0.5f, 1.f),
                                "Add Joint: click in the viewport to place a joint (stays active -- "
                                "click the button again or pick another mode to stop)");
        } else if (options.connect_mode) {
            ImGui::Separator();
            if (options.connect_first_joint < 0) {
                ImGui::TextColored(ImVec4(1.f, 0.82f, 0.15f, 1.f), "Connect Joints: click the first joint");
            } else {
                ImGui::TextColored(ImVec4(1.f, 0.82f, 0.15f, 1.f),
                                    "Connect Joints: now click the second joint to create the member");
            }
        } else if (options.load_mode == LoadPlacementMode::MemberLoad) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 0.95f, 1.f),
                                "Add Member Load: click a member to apply a distributed load (stays active)");
        } else if (options.load_mode == LoadPlacementMode::JointLoad) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 0.95f, 1.f),
                                "Add Joint Load: click a joint to apply a point load (stays active)");
        }
        if (on_title_bar_drawer_) on_title_bar_drawer_();
        ImGui::EndMainMenuBar();
    }
}

} // namespace orcisf::gui
