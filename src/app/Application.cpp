#include "app/Application.h"
#include "app/DockTabIcons.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <nfd.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <utility>

#include "engine/Engine.h"
#include "engine/LegacyIO.h"
#include "engine/MemberResults.h"
#include "report/PdfExport.h"
#include "report/ResultsCsvExport.h"
#include "report/TextExport.h"

namespace orcisf::app {

namespace {

// Resolves a user-picked folder (e.g. "Optimasi Beton/Example/Apl1-1") to
// the generic dataset base path LegacyIO expects (folder + basename, no
// extension) by finding whichever file in it has a case-insensitive
// ".inp" extension -- the legacy naming convention uses inconsistent
// casing across datasets (GEDUNG.INP vs aplikasi.inp), see AGENTS.md's
// "Data / File Format Convention".
// Issue #11/follow-up: asks the user for a folder + base filename via a
// single NFD save dialog (folder navigation + typed name, same pattern
// OnExportPdfRequested/OnExportInfRequested already use), returning the
// legacy generic path (extension stripped) or nullopt if cancelled.
std::optional<std::string> PromptForGenericPath(const char* default_name) {
    nfdu8filteritem_t filter{"ORCISF Dataset", "inp"};
    nfdu8char_t* out_path = nullptr;
    nfdsavedialogu8args_t args{};
    args.filterList = &filter;
    args.filterCount = 1;
    args.defaultName = default_name;
    nfdresult_t result = NFD_SaveDialogU8_With(&out_path, &args);
    if (result != NFD_OKAY) {
        return std::nullopt;
    }
    std::filesystem::path chosen = out_path;
    NFD_FreePathU8(out_path);
    chosen.replace_extension();
    return chosen.string();
}

std::optional<std::string> FindDatasetGenericPath(const std::string& folder) {
    namespace fs = std::filesystem;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(folder, ec)) {
        if (ec || !entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
        if (ext == ".inp") {
            fs::path generic = entry.path();
            generic.replace_extension();
            return generic.string();
        }
    }
    return std::nullopt;
}

} // namespace

Application::Application() {
    log_panel_.AddLine("ORCISF GUI scaffold started (issue #2).");
    log_panel_.AddLine("3D viewport (#5), Run panel (#4), the interactive editor (#6), and the load editor (#7) "
                        "are wired to the engine.");

    run_panel_.SetLogSink([this](std::string line) { log_panel_.AddLine(std::move(line)); });
    run_panel_.SetOnResult([this](engine::StructureData sd, std::string dataset_path, std::string output_path) {
        OnRunResult(std::move(sd), std::move(dataset_path), std::move(output_path));
    });
    toolbar_.SetOnNewData([this]() { OnNewDataRequested(); });
    toolbar_.SetOnSave([this]() { OnSaveRequested(); });
    toolbar_.SetOnSaveAs([this]() { OnSaveAsRequested(); });
    toolbar_.SetOnOpenFolder([this]() { OnOpenFolderRequested(); });
    toolbar_.SetOnUndo([this]() { OnUndo(); });
    toolbar_.SetOnRedo([this]() { OnRedo(); });
    toolbar_.SetOnAddJoint([this]() { OnAddJointRequested(); });
    toolbar_.SetOnSaveLoads([this]() { OnSaveLoadsRequested(); });
    toolbar_.SetOnExportText([this]() { OnExportTextRequested(); });
    toolbar_.SetOnExportPdf([this]() { OnExportPdfRequested(); });
    toolbar_.SetOnExportInf([this]() { OnExportInfRequested(); });
    toolbar_.SetOnExportResultsCsv([this]() { OnExportResultsCsvRequested(); });
    toolbar_.SetOnViewLayout([this](gui::ViewLayoutPreset preset) { current_layout_ = preset; });
    toolbar_.SetTitleBarDrawer([this]() {
        if (window_) title_bar_.Draw(window_, "ORCISF - Optimasi Beton Bertulang Pada Struktur Portal Ruang");
    });

    icon_toolbar_.SetOnNewData([this]() { OnNewDataRequested(); });
    icon_toolbar_.SetOnOpenFolder([this]() { OnOpenFolderRequested(); });
    icon_toolbar_.SetOnSave([this]() { OnSaveRequested(); });
    icon_toolbar_.SetOnUndo([this]() { OnUndo(); });
    icon_toolbar_.SetOnRedo([this]() { OnRedo(); });
    icon_toolbar_.SetOnAddJoint([this]() { OnAddJointRequested(); });
    icon_toolbar_.SetOnRun([this]() { run_panel_.TriggerRun(); });
}

void Application::SetWindow(GLFWwindow* window) { window_ = window; }

void Application::OnNewDataRequested() {
    // Asks where to put the new dataset up front (follow-up to #11) rather
    // than leaving loaded_dataset_path_ empty until the user happens to hit
    // Save -- a blank in-memory-only structure with no path is easy to lose
    // (e.g. Open Data replacing it) with no obvious way to save it.
    // Cancelling the dialog leaves whatever was previously loaded alone.
    std::optional<std::string> generic = PromptForGenericPath("struktur.inp");
    if (!generic) return;

    LoadStructure(engine::StructureData{}, nullptr, *generic);
    std::string err = report::WriteTextExport(loaded_sd_, *generic, /*has_run_results=*/false, "");
    if (err.empty()) {
        log_panel_.AddLine("Started a new structure at: " + *generic);
    } else {
        log_panel_.AddLine("Started a new structure, but failed to write initial files: " + err);
    }
}

void Application::OnSaveRequested() {
    if (!editable_) return;
    if (loaded_dataset_path_.empty()) {
        OnSaveAsRequested();
        return;
    }
    std::string err =
        report::WriteTextExport(loaded_sd_, loaded_dataset_path_, has_run_results_, last_run_output_path_);
    if (err.empty()) {
        log_panel_.AddLine("Saved: " + loaded_dataset_path_);
    } else {
        log_panel_.AddLine("Save failed: " + err);
    }
}

void Application::OnSaveAsRequested() {
    if (!editable_) return;
    std::string default_name = "struktur.inp";
    if (!loaded_dataset_path_.empty()) {
        default_name = std::filesystem::path(loaded_dataset_path_).filename().string() + ".inp";
    }
    std::optional<std::string> generic = PromptForGenericPath(default_name.c_str());
    if (!generic) return;

    std::string source = last_run_output_path_;
    std::string err = report::WriteTextExport(loaded_sd_, *generic, has_run_results_, source);
    if (err.empty()) {
        loaded_dataset_path_ = *generic;
        log_panel_.AddLine("Saved as: " + loaded_dataset_path_);
    } else {
        log_panel_.AddLine("Save As failed: " + err);
    }
}

void Application::LoadStructure(engine::StructureData sd, const std::vector<engine::MemberResult>* results,
                                 std::string dataset_path, const engine::AnalysisResults* analysis) {
    loaded_sd_ = std::move(sd);
    loaded_dataset_path_ = std::move(dataset_path);
    editable_.emplace(loaded_sd_);
    undo_stack_.Clear();
    selection_.Clear();
    editor_options_.connect_mode = false;
    editor_options_.connect_first_joint = -1;

    has_run_results_ = (results != nullptr);
    current_results_ = results ? *results : std::vector<engine::MemberResult>{};
    current_analysis_ = analysis ? *analysis : engine::AnalysisResults{};

    scene_ = gui::BuildSceneModel(loaded_sd_, results, loaded_dataset_path_, &editable_->MemberTypeOverrides(),
                                   analysis);
    validation_issues_ = editable_->Validate();
    viewport_panel_.FrameScene(scene_);
}

void Application::RebuildSceneAfterEdit() {
    if (!editable_) return;
    has_run_results_ = false;
    current_results_.clear();
    current_analysis_ = engine::AnalysisResults{};
    scene_ = gui::BuildSceneModel(loaded_sd_, nullptr, loaded_dataset_path_, &editable_->MemberTypeOverrides());
    validation_issues_ = editable_->Validate();
}

void Application::OnOpenFolderRequested() {
    nfdu8char_t* out_path = nullptr;
    nfdresult_t result = NFD_PickFolderU8(&out_path, nullptr);

    if (result == NFD_OKAY) {
        std::string folder = out_path;
        NFD_FreePathU8(out_path);

        std::optional<std::string> generic = FindDatasetGenericPath(folder);
        if (!generic) {
            log_panel_.AddLine("No .inp dataset file found in: " + folder);
            return;
        }
        try {
            engine::StructureData sd;
            engine::DatasetPaths paths = engine::MakeDatasetPaths(*generic);
            engine::ReadDataset(sd, paths);
            // Raw (self-weight-free) loads, not engine::LoadDatasetForViewing's
            // ReadLoads() -- the editor must round-trip exactly what the user
            // set, not ReadLoads()'s self-weight-inflated in-memory state
            // (see engine::ReadLoadsRaw()'s comment).
            engine::ReadLoadsRaw(sd, paths.bbn);
            LoadStructure(std::move(sd), nullptr, *generic);
            log_panel_.AddLine("Loaded dataset: " + *generic);
        } catch (const std::exception& e) {
            log_panel_.AddLine(std::string("Failed to load dataset: ") + e.what());
        }
    } else if (result == NFD_ERROR) {
        log_panel_.AddLine(std::string("Folder dialog error: ") + NFD_GetError());
    }
}

void Application::OnRunResult(engine::StructureData sd, std::string dataset_path, std::string output_path) {
    std::vector<engine::MemberResult> results = engine::ComputeMemberResults(sd);
    // Issue #60: captured here (before the load-zeroing loop below,
    // though that loop only touches W/AML/AJ, not AM/DJ/AR) so the
    // deformed-shape overlay's data comes from the exact same sd state
    // ComputeMemberResults() just read -- see AnalysisResults.h's header
    // comment on why this deliberately does not re-run Struktur().
    engine::AnalysisResults analysis = engine::ComputeAnalysisResults(sd);

    // RunFullOptimization's sd came through engine::ReadLoads(), which
    // bakes self-weight into W/AJ (see engine::ReadLoadsRaw()'s comment on
    // LegacyIO.h) -- there's no cheap way to separate that back into "raw
    // user load" vs. "self-weight" per member/joint (self-weight's joint
    // contribution can be shared across multiple columns at that joint).
    // Rather than show self-weight-inflated values as if they were
    // editable user loads (issue #7's load editor/schedule would then be
    // showing numbers the user never entered), clear them: post-run
    // editing starts from a clean load slate. The .bbn file on disk (the
    // actual source of truth) is untouched by this -- only this in-memory
    // copy, which the editor treats independently of what RunPanel used.
    for (int i = 1; i <= sd.M; ++i) {
        sd.W[i] = 0.f;
        for (int j = 1; j <= 12; ++j) sd.AML[j][i] = 0.f;
    }
    for (int i = 1; i <= 6 * sd.NJ; ++i) sd.AJ[i] = 0.f;

    LoadStructure(std::move(sd), &results, dataset_path, &analysis);
    last_run_output_path_ = output_path; // issue #25: LoadStructure() doesn't touch this, set it after
    log_panel_.AddLine("Viewport updated with results from: " + dataset_path + " (output: " + output_path + ")");
}

void Application::OnAddJointRequested() {
    // Issue #18: Add Joint is now a click-to-place mode (toggled by the
    // caller, see Toolbar/IconToolbar), not an instant single-shot add --
    // this only handles the "make sure there's something to click into"
    // bootstrap step. The actual placement happens in
    // ViewportPanel::HandlePicking() when options.add_joint_mode is set,
    // via editable_->AddJoint() directly at the click's 3D position.
    if (!editable_) {
        // No dataset loaded yet -- start a blank in-memory structure the
        // user can build up from scratch (issue #6's editor isn't only for
        // editing an imported file).
        LoadStructure(engine::StructureData{}, nullptr, "");
        log_panel_.AddLine("Started a new blank structure.");
    }
    log_panel_.AddLine("Add Joint mode: click in the viewport to place a joint. Click the button again to stop.");
}

void Application::OnUndo() {
    if (!editable_ || !undo_stack_.CanUndo()) return;
    undo_stack_.Undo(loaded_sd_);
    selection_.Clear();
    RebuildSceneAfterEdit();
}

void Application::OnRedo() {
    if (!editable_ || !undo_stack_.CanRedo()) return;
    undo_stack_.Redo(loaded_sd_);
    selection_.Clear();
    RebuildSceneAfterEdit();
}

void Application::OnSaveLoadsRequested() {
    if (!editable_ || loaded_dataset_path_.empty()) return;
    try {
        engine::DatasetPaths paths = engine::MakeDatasetPaths(loaded_dataset_path_);
        engine::WriteLoads(loaded_sd_, paths.bbn);
        log_panel_.AddLine("Saved loads to: " + paths.bbn);
    } catch (const std::exception& e) {
        log_panel_.AddLine(std::string("Failed to save loads: ") + e.what());
    }
}

void Application::OnExportTextRequested() {
    if (!editable_) return;

    nfdu8char_t* out_path = nullptr;
    nfdresult_t result = NFD_PickFolderU8(&out_path, nullptr);
    if (result != NFD_OKAY) {
        if (result == NFD_ERROR) log_panel_.AddLine(std::string("Folder dialog error: ") + NFD_GetError());
        return;
    }
    std::string folder = out_path;
    NFD_FreePathU8(out_path);

    // Reuse the loaded dataset's own base filename in the chosen folder
    // (e.g. "aplikasi"), matching the generic-name convention everywhere
    // else in this project -- falls back to "struktur" for a blank
    // in-memory structure that was never loaded from a path.
    std::string basename = "struktur";
    if (!loaded_dataset_path_.empty()) {
        basename = std::filesystem::path(loaded_dataset_path_).filename().string();
    }
    std::string dest_generic = (std::filesystem::path(folder) / basename).string();

    std::string err = report::WriteTextExport(loaded_sd_, dest_generic, has_run_results_, last_run_output_path_);
    if (err.empty()) {
        log_panel_.AddLine("Exported legacy text file set to: " + folder);
    } else {
        log_panel_.AddLine("Text export failed: " + err);
    }
}

void Application::OnExportPdfRequested() {
    if (!has_run_results_) return;

    nfdu8filteritem_t filter{"PDF", "pdf"};
    nfdu8char_t* out_path = nullptr;
    nfdsavedialogu8args_t args{};
    args.filterList = &filter;
    args.filterCount = 1;
    args.defaultName = "report.pdf";
    nfdresult_t result = NFD_SaveDialogU8_With(&out_path, &args);
    if (result != NFD_OKAY) {
        if (result == NFD_ERROR) log_panel_.AddLine(std::string("Save dialog error: ") + NFD_GetError());
        return;
    }
    std::string pdf_path = out_path;
    NFD_FreePathU8(out_path);

    std::string err = report::WritePdfReport(loaded_sd_, current_results_, pdf_path);
    if (err.empty()) {
        log_panel_.AddLine("Exported PDF report to: " + pdf_path);
    } else {
        log_panel_.AddLine("PDF export failed: " + err);
    }
}

void Application::OnExportInfRequested() {
    if (!editable_) return;

    nfdu8filteritem_t filter{"INF", "inf"};
    nfdu8char_t* out_path = nullptr;
    nfdsavedialogu8args_t args{};
    args.filterList = &filter;
    args.filterCount = 1;
    args.defaultName = "struktur.inf";
    nfdresult_t result = NFD_SaveDialogU8_With(&out_path, &args);
    if (result != NFD_OKAY) {
        if (result == NFD_ERROR) log_panel_.AddLine(std::string("Save dialog error: ") + NFD_GetError());
        return;
    }
    std::string inf_path = out_path;
    NFD_FreePathU8(out_path);

    try {
        engine::WriteInfPreview(loaded_sd_, inf_path);
        log_panel_.AddLine("Exported .inf preview to: " + inf_path);
    } catch (const std::exception& e) {
        log_panel_.AddLine(std::string("INF export failed: ") + e.what());
    }
}

void Application::OnExportResultsCsvRequested() {
    if (!has_run_results_) return; // same precondition as PDF export -- see current_analysis_'s own comment

    nfdu8filteritem_t filter{"CSV", "csv"};
    nfdu8char_t* out_path = nullptr;
    nfdsavedialogu8args_t args{};
    args.filterList = &filter;
    args.filterCount = 1;
    args.defaultName = "hasil.csv";
    nfdresult_t result = NFD_SaveDialogU8_With(&out_path, &args);
    if (result != NFD_OKAY) {
        if (result == NFD_ERROR) log_panel_.AddLine(std::string("Save dialog error: ") + NFD_GetError());
        return;
    }
    std::string csv_path = out_path;
    NFD_FreePathU8(out_path);

    std::string err = report::WriteResultsCsv(current_analysis_, csv_path);
    if (err.empty()) {
        log_panel_.AddLine("Exported results CSV to: " + csv_path);
    } else {
        log_panel_.AddLine("Results CSV export failed: " + err);
    }
}

namespace {

// Issue #15's "Default" preset -- exactly the single fixed layout this app
// always had (no regression for existing users): Viewport/Detailing
// large+center, Properties/Optimization/Log (renamed from "Run
// Optimization", issue #37; regrouped with Log here per issue #38 -- Log
// used to sit in the bottom strip instead) tabbed on the right,
// Joints/Members/Loads tabbed along the bottom (issue #36 split what used
// to be a single combined Joints/Members tab into two).
void BuildDefaultLayout(ImGuiID dock_main) {
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.25f, nullptr, &dock_main);

    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kViewportId).c_str(), dock_main);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kDetailingId).c_str(), dock_main);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kForceDiagramId).c_str(), dock_main); // issue #61
    // Issue #38: Properties, Optimization, Log tabbed together here -- tab
    // order (Properties -> Optimization -> Log) comes from OnFrame()'s
    // Draw() call order, not from the order they're listed below (see the
    // comment there, established by issue #36).
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kPropertiesId).c_str(), dock_right);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kRunOptimizationId).c_str(), dock_right);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kLogId).c_str(), dock_right);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kResultsId).c_str(), dock_right); // issue #62
    // Issue #36: tab order (Joints, Members, Loads) comes from OnFrame().
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kJointsId).c_str(), dock_bottom);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kMembersId).c_str(), dock_bottom);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kLoadsId).c_str(), dock_bottom);
}

// "Design" preset: focuses on geometry/load input -- Viewport+Detailing
// stay large, Loads gets a bigger bottom strip. Properties/Optimization/Log
// are tabbed together in a single right-side column (issue #38 -- Properties
// used to be alone in this column with Optimization+Log split into a
// separate bottom-right node; that extra split is gone) -- still present,
// just tabbed in a less prominent corner rather than hidden, so nothing a
// Design-focused user needs is ever unreachable.
void BuildDesignLayout(ImGuiID dock_main) {
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.22f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.32f, nullptr, &dock_main);

    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kViewportId).c_str(), dock_main);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kDetailingId).c_str(), dock_main);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kForceDiagramId).c_str(), dock_main); // issue #61
    // Issue #38: tab order (Properties -> Optimization -> Log) comes from
    // OnFrame()'s Draw() call order, same convention as #36 below.
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kPropertiesId).c_str(), dock_right);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kRunOptimizationId).c_str(), dock_right);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kLogId).c_str(), dock_right);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kResultsId).c_str(), dock_right); // issue #62
    // Issue #36: tab order (Joints, Members, Loads) comes from OnFrame().
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kJointsId).c_str(), dock_bottom);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kMembersId).c_str(), dock_bottom);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kLoadsId).c_str(), dock_bottom);
}

// "Optimization" preset (a view-layout preset name, distinct from the
// panel of the same name below -- issue #37's rename made these two
// "Optimization"s coexist; see AGENTS.md's #37 notes): focuses on
// running -- the Optimization panel + Log get the large primary area,
// everything else (Viewport/Detailing/Properties/Loads) moves to a
// secondary column, tabbed in pairs so all four stay one click away
// instead of disappearing.
void BuildOptimizationLayout(ImGuiID dock_main) {
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.35f, nullptr, &dock_main);
    ImGuiID dock_right_bottom =
        ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.5f, nullptr, &dock_right);

    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kRunOptimizationId).c_str(), dock_main);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kLogId).c_str(), dock_main);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kViewportId).c_str(), dock_right);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kDetailingId).c_str(), dock_right);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kForceDiagramId).c_str(), dock_right); // issue #61
    // Issue #36: tab order (Properties, Joints, Members, Loads) comes
    // from OnFrame(), not from the order these are listed below.
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kPropertiesId).c_str(), dock_right_bottom);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kResultsId).c_str(), dock_right_bottom); // issue #62
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kJointsId).c_str(), dock_right_bottom);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kMembersId).c_str(), dock_right_bottom);
    ImGui::DockBuilderDockWindow(gui::PanelWindowId(gui::kLoadsId).c_str(), dock_right_bottom);
}

} // namespace

void Application::BuildDockspace() {
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(),
                                  ImGuiDockNodeFlags_PassthruCentralNode);

    if (dockspace_initialized_ && built_layout_preset_ == current_layout_) {
        return;
    }
    dockspace_initialized_ = true;
    built_layout_preset_ = current_layout_;

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    // Issue #31: the *work* rect, not the full viewport -- the work rect
    // already excludes the main menu bar and (per OnFrame() above) the icon
    // toolbar. Seeding the builder with the full viewport size/origin made
    // the freshly-built layout one menu-bar-plus-toolbar taller than its
    // container and anchored at the viewport's top-left, so on the rebuild
    // frame every panel's tab bar was drawn up underneath the toolbar
    // before DockSpaceOverViewport() re-fitted it the frame after.
    ImGui::DockBuilderSetNodePos(dockspace_id, ImGui::GetMainViewport()->WorkPos);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    switch (current_layout_) {
        case gui::ViewLayoutPreset::Design:
            BuildDesignLayout(dockspace_id);
            break;
        case gui::ViewLayoutPreset::Optimization:
            BuildOptimizationLayout(dockspace_id);
            break;
        case gui::ViewLayoutPreset::Default:
        default:
            BuildDefaultLayout(dockspace_id);
            break;
    }

    ImGui::DockBuilderFinish(dockspace_id);
}

void Application::OnFrame() {
    // Issue #25: keep RunPanel's notion of "which dataset to run" in sync
    // with whatever's actually loaded, every frame, before anything below
    // reads RunPanel::CanRun() (the icon toolbar's Run button) or draws
    // the panel itself -- see RunPanel::SetDatasetPath()'s comment.
    run_panel_.SetDatasetPath(loaded_dataset_path_);

    bool can_save = editable_.has_value() && !loaded_dataset_path_.empty();
    bool can_export_text = editable_.has_value();
    bool can_export_pdf = has_run_results_;
    bool can_export_inf = editable_.has_value();

    // Issue #37: View > Subwindows binds directly to these fields --
    // rebuilt fresh each frame (just 8 pointer copies) rather than kept as
    // a persistent member, since the fields themselves are the persistent
    // state and this struct is only ever a view over them.
    gui::PanelVisibility panel_visibility;
    panel_visibility.viewport = &viewport_open_;
    panel_visibility.detailing = &detailing_open_;
    panel_visibility.properties = &properties_open_;
    panel_visibility.optimization = &run_open_;
    panel_visibility.joints = &joints_open_;
    panel_visibility.members = &members_open_;
    panel_visibility.loads = &loads_open_;
    panel_visibility.log = &log_open_;
    panel_visibility.force_diagram = &force_diagram_open_; // issue #61
    panel_visibility.results = &results_open_;              // issue #62

    toolbar_.Draw(undo_stack_.CanUndo(), undo_stack_.CanRedo(), can_save, can_export_text, can_export_pdf,
                  can_export_inf, current_layout_, editor_options_, icon_visibility_, panel_visibility);

    // Ctrl+S was only ever a display hint on the File > Save menu item
    // (ImGui::MenuItem's shortcut text is cosmetic, it doesn't bind the
    // key) -- actually wire it here. No text-input focus check: none of
    // this app's text fields are multi-line/expect Ctrl+S themselves, so
    // there's no conflicting consumer to route around.
    if (can_export_text && ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)) {
        OnSaveRequested();
    }

    icon_toolbar_.Draw(undo_stack_.CanUndo(), undo_stack_.CanRedo(), can_export_text, run_panel_.CanRun(),
                        editor_options_, icon_visibility_);
    // Reserve the icon toolbar's row (drawn as a plain window, not through
    // BeginMainMenuBar, so it doesn't shrink the work area on its own the
    // way the menu bar does) so DockSpaceOverViewport below doesn't place
    // panels underneath it.
    // Issue #31: IconToolbar::Height() is DPI-scaled, so this reservation
    // tracks the row's real height instead of a fixed 36px that left docked
    // panels overlapping the toolbar on a high-DPI monitor. Both sides read
    // the same function -- don't reintroduce a second literal here.
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    const float icon_toolbar_height = gui::IconToolbar::Height();
    main_viewport->WorkPos.y += icon_toolbar_height;
    main_viewport->WorkSize.y -= icon_toolbar_height;

    BuildDockspace();

    gui::EditableStructure* editable_ptr = editable_ ? &*editable_ : nullptr;
    auto on_geometry_changed = [this]() { RebuildSceneAfterEdit(); };

    // Issue #37: each Draw() call is gated on its own open flag instead of
    // being called unconditionally and trusting ImGui::Begin(name, open)
    // to no-op once *open goes false. That trust was misplaced specifically
    // for *docked* windows: clicking a tab's close button does correctly
    // flip the bool (confirmed by instrumenting it directly), but a docked
    // window whose Begin() keeps getting called every frame regardless
    // never actually leaves its dock node's tab bar -- the tab lingers
    // forever, which is exactly the reported "close button does nothing"
    // bug. Skipping the call entirely once closed is what actually lets
    // Dear ImGui's docking system drop the tab. Root-caused by logging
    // io.MousePos/GetHoveredID() per click plus a before/after snapshot of
    // every open flag around the Draw() calls, not by reading engine
    // source (not available for this vcpkg-installed, headers-only copy).
    if (viewport_open_) {
        viewport_panel_.Draw(&viewport_open_, scene_, selection_, editable_ptr, &undo_stack_, editor_options_,
                              on_geometry_changed);
    }
    if (properties_open_) {
        properties_panel_.Draw(&properties_open_, scene_, selection_, editable_ptr, &undo_stack_,
                                validation_issues_, on_geometry_changed);
    }
    // Issue #36: a docked tab bar's *displayed* order follows the order
    // each window's Begin() is called within the frame, not the order
    // BuildDefaultLayout()/etc.'s DockBuilderDockWindow() calls list them
    // in (that only sets up the persistent node/ID association; found by
    // screenshotting a mismatch between the two, not by reading the code).
    // Joints/Members/Loads are called in that order here specifically so
    // the tab row reads Joints, Members, Loads left-to-right, per #36.
    if (joints_open_) {
        joints_panel_.Draw(&joints_open_, scene_, selection_, editable_ptr, &undo_stack_, on_geometry_changed);
    }
    if (members_open_) {
        members_panel_.Draw(&members_open_, scene_, selection_, editable_ptr, &undo_stack_, on_geometry_changed);
    }
    if (loads_open_) {
        loads_panel_.Draw(&loads_open_, scene_, selection_, editable_ptr, &undo_stack_, on_geometry_changed);
    }
    if (detailing_open_) {
        detailing_panel_.Draw(&detailing_open_, scene_, selection_);
    }
    if (force_diagram_open_) {
        // issue #61: nullptr when the loaded scene has no completed
        // run's results (current_analysis_ is default-constructed/empty
        // in that state, same has_run_results_-gated lifetime as
        // current_results_ -- see LoadStructure()/RebuildSceneAfterEdit()).
        force_diagram_panel_.Draw(&force_diagram_open_, scene_, selection_,
                                   has_run_results_ ? &current_analysis_ : nullptr);
    }
    if (results_open_) {
        results_panel_.Draw(&results_open_, scene_, selection_, has_run_results_ ? &current_analysis_ : nullptr);
    }
    if (run_open_) {
        run_panel_.Draw(&run_open_);
    }
    if (log_open_) {
        log_panel_.Draw(&log_open_);
    }

    // Issue #35: every panel above has now called Begin()/End() for this
    // frame, so each one's ImGuiWindow::DockNode/tab-bar geometry is final
    // -- the precondition DrawDockTabIcons() documents for itself.
    DrawDockTabIcons();
}

} // namespace orcisf::app
