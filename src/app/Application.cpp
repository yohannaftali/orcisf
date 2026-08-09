#include "app/Application.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

namespace orcisf::app {

Application::Application() {
    log_panel_.AddLine("ORCISF GUI scaffold started (issue #2).");
    log_panel_.AddLine("Viewport/Properties/Log panels are placeholders -- "
                        "see issues #5, #6/#7, #3 for real implementations.");
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
    ImGui::DockBuilderDockWindow("Log", dock_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

void Application::OnFrame() {
    toolbar_.Draw();
    BuildDockspace();

    viewport_panel_.Draw(&viewport_open_);
    properties_panel_.Draw(&properties_open_);
    log_panel_.Draw(&log_open_);
}

} // namespace orcisf::app
