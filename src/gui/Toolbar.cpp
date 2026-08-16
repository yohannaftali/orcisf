#include "gui/Toolbar.h"

#include <imgui.h>

namespace orcisf::gui {

namespace {

// Issue #30: real VS Code/Win32-style Alt-mnemonic menu access -- issue
// #28 first tried this by prefixing labels with "&", which turned out to
// be a Win32/Qt/wxWidgets convention Dear ImGui's stock BeginMenu() does
// NOT implement (confirmed by running a build: the literal "&" was shown,
// unparsed -- see AGENTS.md's issue #28/#30 notes). This hand-rolls it
// instead, entirely via ImGui's public API (no imgui_internal.h):
//  1. Underline is drawn as a manual ImDrawList overlay under the
//     mnemonic character, positioned via CalcTextSize() on the label's
//     prefix/character substrings -- only while Alt is currently held
//     (matching the Windows convention of hiding accelerators otherwise).
//  2. Alt+<letter> opens the menu via the documented ImGui idiom for
//     programmatically opening a popup: call ImGui::OpenPopup(label)
//     with the *exact* same label text BeginMenu(label) uses for its own
//     ID, in the same ID-stack scope (both are direct children of the
//     main menu bar here), on the same frame just before BeginMenu() --
//     ImGui recognizes it as already-open when BeginMenu() runs right
//     after.
bool BeginMnemonicMenu(const char* label, int mnemonic_index, ImGuiKey mnemonic_key, bool alt_held) {
    if (alt_held && ImGui::IsKeyChordPressed(ImGuiMod_Alt | mnemonic_key)) {
        ImGui::OpenPopup(label);
    }

    bool opened = ImGui::BeginMenu(label);

    if (alt_held) {
        ImVec2 item_min = ImGui::GetItemRectMin();
        ImVec2 item_max = ImGui::GetItemRectMax();
        // Main-menu-bar items are padded by FramePadding.x on each side
        // before the label text starts (not publicly exposed as a single
        // constant, but this matches Dear ImGui's own menu-bar item
        // layout) -- close enough for a cosmetic underline; a 1-2px
        // misalignment here is not a functional bug.
        float text_x = item_min.x + ImGui::GetStyle().FramePadding.x;
        ImVec2 pre_size = ImGui::CalcTextSize(label, label + mnemonic_index);
        ImVec2 char_size = ImGui::CalcTextSize(label + mnemonic_index, label + mnemonic_index + 1);
        float underline_x0 = text_x + pre_size.x;
        float underline_x1 = underline_x0 + char_size.x;
        float underline_y = item_max.y - ImGui::GetStyle().FramePadding.y - 1.f;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(underline_x0, underline_y), ImVec2(underline_x1, underline_y),
                                             ImGui::GetColorU32(ImGuiCol_Text), 1.5f);
    }

    return opened;
}

} // namespace

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
                    bool can_export_inf, ViewLayoutPreset current_layout, EditorOptions& options,
                    IconVisibility& icon_visibility, const PanelVisibility& panel_visibility) {
    // Issue #30: real Alt-mnemonic menu access, see BeginMnemonicMenu()'s
    // comment above for how (issue #28's "&"-prefix attempt didn't work --
    // Dear ImGui doesn't parse that convention, see AGENTS.md). Letters:
    // File=F, Edit=E, "View Plane"=P (not V -- already claimed by the
    // separate "View" layout-preset menu), Loads=L, Run=R, View=V.
    bool alt_held = ImGui::IsKeyDown(ImGuiMod_Alt);
    if (ImGui::BeginMainMenuBar()) {
        if (BeginMnemonicMenu("File", 0, ImGuiKey_F, alt_held)) {
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
        if (BeginMnemonicMenu("Edit", 0, ImGuiKey_E, alt_held)) {
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
        if (BeginMnemonicMenu("View Plane", 5, ImGuiKey_P, alt_held)) {
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
        if (BeginMnemonicMenu("Loads", 0, ImGuiKey_L, alt_held)) {
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
        if (BeginMnemonicMenu("Run", 0, ImGuiKey_R, alt_held)) {
            ImGui::MenuItem("Optimize...", nullptr, false, false);
            ImGui::MenuItem("Cancel", nullptr, false, false);
            ImGui::EndMenu();
        }
        // Issue #37: restructured into three labeled sections, separated by
        // Separator() -- Menubar (per-icon-toolbar-button visibility),
        // Subwindows (per-panel show/hide, doubling as the way to reopen a
        // panel closed via its now-fixed tab close button), and Layout
        // (the pre-existing #15 view-layout presets, relocated here
        // unchanged from three flat top-level items into their own
        // submenu).
        if (BeginMnemonicMenu("View", 0, ImGuiKey_V, alt_held)) {
            if (ImGui::BeginMenu("Menubar")) {
                ImGui::MenuItem("New Data", nullptr, &icon_visibility.new_data);
                ImGui::MenuItem("Open Data...", nullptr, &icon_visibility.open_data);
                ImGui::MenuItem("Save", nullptr, &icon_visibility.save);
                ImGui::MenuItem("Undo", nullptr, &icon_visibility.undo);
                ImGui::MenuItem("Redo", nullptr, &icon_visibility.redo);
                ImGui::MenuItem("Add Joint", nullptr, &icon_visibility.add_joint);
                ImGui::MenuItem("Connect Joints", nullptr, &icon_visibility.connect_joints);
                ImGui::MenuItem("Run", nullptr, &icon_visibility.run);
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Subwindows")) {
                if (panel_visibility.viewport) ImGui::MenuItem("Viewport", nullptr, panel_visibility.viewport);
                if (panel_visibility.detailing) ImGui::MenuItem("Detailing", nullptr, panel_visibility.detailing);
                if (panel_visibility.force_diagram)
                    ImGui::MenuItem("Force Diagrams", nullptr, panel_visibility.force_diagram);
                if (panel_visibility.properties) ImGui::MenuItem("Properties", nullptr, panel_visibility.properties);
                if (panel_visibility.optimization)
                    ImGui::MenuItem("Optimization", nullptr, panel_visibility.optimization);
                if (panel_visibility.joints) ImGui::MenuItem("Joints", nullptr, panel_visibility.joints);
                if (panel_visibility.members) ImGui::MenuItem("Members", nullptr, panel_visibility.members);
                if (panel_visibility.loads) ImGui::MenuItem("Loads", nullptr, panel_visibility.loads);
                if (panel_visibility.log) ImGui::MenuItem("Log", nullptr, panel_visibility.log);
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Layout")) {
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
