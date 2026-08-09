#include "gui/Toolbar.h"

#include <imgui.h>

namespace orcisf::gui {

void Toolbar::Draw() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::MenuItem("Open Folder...", nullptr, false, false);
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
