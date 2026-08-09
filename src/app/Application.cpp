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
#include "engine/MemberResults.h"

namespace orcisf::app {

namespace {

// Resolves a user-picked folder (e.g. "Optimasi Beton/Example/Apl1-1") to
// the generic dataset base path LegacyIO expects (folder + basename, no
// extension) by finding whichever file in it has a case-insensitive
// ".inp" extension -- the legacy naming convention uses inconsistent
// casing across datasets (GEDUNG.INP vs aplikasi.inp), see AGENTS.md's
// "Data / File Format Convention".
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
    log_panel_.AddLine("3D viewport (#5), Run panel (#4), and the interactive editor (#6) are wired to the "
                        "engine; load-input GUI lands in issue #7.");

    run_panel_.SetLogSink([this](std::string line) { log_panel_.AddLine(std::move(line)); });
    run_panel_.SetOnResult([this](engine::StructureData sd, std::string dataset_path) {
        OnRunResult(std::move(sd), std::move(dataset_path));
    });
    toolbar_.SetOnOpenFolder([this]() { OnOpenFolderRequested(); });
    toolbar_.SetOnUndo([this]() { OnUndo(); });
    toolbar_.SetOnRedo([this]() { OnRedo(); });
    toolbar_.SetOnAddJoint([this]() { OnAddJointRequested(); });
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

    scene_ = gui::BuildSceneModel(loaded_sd_, results, loaded_dataset_path_);
    validation_issues_ = editable_->Validate();
    viewport_panel_.FrameScene(scene_);
}

void Application::RebuildSceneAfterEdit() {
    if (!editable_) return;
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
            engine::LoadDatasetForViewing(sd, *generic);
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
    ImGui::DockBuilderDockWindow("Properties", dock_right);
    ImGui::DockBuilderDockWindow("Run Optimization", dock_right);
    ImGui::DockBuilderDockWindow("Log", dock_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

void Application::OnFrame() {
    toolbar_.Draw(undo_stack_.CanUndo(), undo_stack_.CanRedo(), editor_options_);
    BuildDockspace();

    gui::EditableStructure* editable_ptr = editable_ ? &*editable_ : nullptr;
    auto on_geometry_changed = [this]() { RebuildSceneAfterEdit(); };

    viewport_panel_.Draw(&viewport_open_, scene_, selection_, editable_ptr, &undo_stack_, editor_options_,
                          on_geometry_changed);
    properties_panel_.Draw(&properties_open_, scene_, selection_, editable_ptr, &undo_stack_, validation_issues_,
                            on_geometry_changed);
    run_panel_.Draw(&run_open_);
    log_panel_.Draw(&log_open_);
}

} // namespace orcisf::app
