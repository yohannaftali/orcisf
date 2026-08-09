#include "gui/Toolbar.h"

#include <imgui.h>

namespace orcisf::gui {

void Toolbar::SetOnOpenFolder(std::function<void()> callback) { on_open_folder_ = std::move(callback); }

void Toolbar::Draw() {
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
        ImGui::EndMainMenuBar();
    }
}

} // namespace orcisf::gui
