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
    log_panel_.AddLine("3D viewport (issue #5) and Run panel (issue #4) are wired to the engine; "
                        "interactive editing lands in issue #6/#7.");

    run_panel_.SetLogSink([this](std::string line) { log_panel_.AddLine(std::move(line)); });
    run_panel_.SetOnResult([this](engine::StructureData sd, std::string dataset_path) {
        OnRunResult(std::move(sd), std::move(dataset_path));
    });
    toolbar_.SetOnOpenFolder([this]() { OnOpenFolderRequested(); });
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
            scene_ = gui::BuildSceneModel(sd, nullptr, *generic);
            selected_member_ = -1;
            viewport_panel_.FrameScene(scene_);
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
    scene_ = gui::BuildSceneModel(sd, &results, dataset_path);
    selected_member_ = -1;
    viewport_panel_.FrameScene(scene_);
    log_panel_.AddLine("Viewport updated with results from: " + dataset_path);
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
    toolbar_.Draw();
    BuildDockspace();

    viewport_panel_.Draw(&viewport_open_, scene_, selected_member_);
    properties_panel_.Draw(&properties_open_, scene_, selected_member_);
    run_panel_.Draw(&run_open_);
    log_panel_.Draw(&log_open_);
}

} // namespace orcisf::app
