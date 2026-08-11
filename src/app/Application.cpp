#include "app/Application.h"

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
    run_panel_.SetOnResult([this](engine::StructureData sd, std::string dataset_path) {
        OnRunResult(std::move(sd), std::move(dataset_path));
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
}

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
        report::WriteTextExport(loaded_sd_, loaded_dataset_path_, has_run_results_, loaded_dataset_path_);
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

    std::string source = loaded_dataset_path_;
    std::string err = report::WriteTextExport(loaded_sd_, *generic, has_run_results_, source);
    if (err.empty()) {
        loaded_dataset_path_ = *generic;
        log_panel_.AddLine("Saved as: " + loaded_dataset_path_);
    } else {
        log_panel_.AddLine("Save As failed: " + err);
    }
}

void Application::LoadStructure(engine::StructureData sd, const std::vector<engine::MemberResult>* results,
                                 std::string dataset_path) {
    loaded_sd_ = std::move(sd);
    loaded_dataset_path_ = std::move(dataset_path);
    editable_.emplace(loaded_sd_);
    undo_stack_.Clear();
    selection_.Clear();
    editor_options_.connect_mode = false;
    editor_options_.connect_first_joint = -1;

    has_run_results_ = (results != nullptr);
    current_results_ = results ? *results : std::vector<engine::MemberResult>{};

    scene_ = gui::BuildSceneModel(loaded_sd_, results, loaded_dataset_path_);
    validation_issues_ = editable_->Validate();
    viewport_panel_.FrameScene(scene_);
}

void Application::RebuildSceneAfterEdit() {
    if (!editable_) return;
    has_run_results_ = false;
    current_results_.clear();
    scene_ = gui::BuildSceneModel(loaded_sd_, nullptr, loaded_dataset_path_);
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

void Application::OnRunResult(engine::StructureData sd, std::string dataset_path) {
    std::vector<engine::MemberResult> results = engine::ComputeMemberResults(sd);

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

    LoadStructure(std::move(sd), &results, dataset_path);
    log_panel_.AddLine("Viewport updated with results from: " + dataset_path);
}

void Application::OnAddJointRequested() {
    if (!editable_) {
        // No dataset loaded yet -- start a blank in-memory structure the
        // user can build up from scratch (issue #6's editor isn't only for
        // editing an imported file).
        LoadStructure(engine::StructureData{}, nullptr, "");
        log_panel_.AddLine("Started a new blank structure.");
    }

    gui::math3d::Vec3 pos = scene_.Empty() ? gui::math3d::Vec3{0.f, 0.f, 0.f} : scene_.bounds_center;
    undo_stack_.PushUndo(editable_->SdForUndo());
    int joint_id = editable_->AddJoint(pos);
    if (joint_id < 0) {
        log_panel_.AddLine("Could not add joint: dataset is at its maximum size.");
        return;
    }
    selection_ = {gui::SelectionKind::Joint, joint_id};
    RebuildSceneAfterEdit();
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

    std::string err = report::WriteTextExport(loaded_sd_, dest_generic, has_run_results_, loaded_dataset_path_);
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

void Application::BuildDockspace() {
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(),
                                  ImGuiDockNodeFlags_PassthruCentralNode);

    if (dockspace_initialized_) {
        return;
    }
    dockspace_initialized_ = true;

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    ImGuiID dock_main = dockspace_id;
    ImGuiID dock_right =
        ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
    ImGuiID dock_bottom =
        ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.25f, nullptr, &dock_main);

    ImGui::DockBuilderDockWindow("Viewport", dock_main);
    ImGui::DockBuilderDockWindow("Detailing", dock_main);
    ImGui::DockBuilderDockWindow("Properties", dock_right);
    ImGui::DockBuilderDockWindow("Run Optimization", dock_right);
    ImGui::DockBuilderDockWindow("Loads", dock_bottom);
    ImGui::DockBuilderDockWindow("Log", dock_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

void Application::OnFrame() {
    bool can_save = editable_.has_value() && !loaded_dataset_path_.empty();
    bool can_export_text = editable_.has_value();
    bool can_export_pdf = has_run_results_;
    bool can_export_inf = editable_.has_value();
    toolbar_.Draw(undo_stack_.CanUndo(), undo_stack_.CanRedo(), can_save, can_export_text, can_export_pdf,
                  can_export_inf, editor_options_);

    // Ctrl+S was only ever a display hint on the File > Save menu item
    // (ImGui::MenuItem's shortcut text is cosmetic, it doesn't bind the
    // key) -- actually wire it here. No text-input focus check: none of
    // this app's text fields are multi-line/expect Ctrl+S themselves, so
    // there's no conflicting consumer to route around.
    if (can_export_text && ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)) {
        OnSaveRequested();
    }

    BuildDockspace();

    gui::EditableStructure* editable_ptr = editable_ ? &*editable_ : nullptr;
    auto on_geometry_changed = [this]() { RebuildSceneAfterEdit(); };

    viewport_panel_.Draw(&viewport_open_, scene_, selection_, editable_ptr, &undo_stack_, editor_options_,
                          on_geometry_changed);
    properties_panel_.Draw(&properties_open_, scene_, selection_, editable_ptr, &undo_stack_, validation_issues_,
                            on_geometry_changed);
    loads_panel_.Draw(&loads_open_, scene_, selection_, editable_ptr, &undo_stack_, on_geometry_changed);
    detailing_panel_.Draw(&detailing_open_, scene_, selection_);
    run_panel_.Draw(&run_open_);
    log_panel_.Draw(&log_open_);
}

} // namespace orcisf::app
