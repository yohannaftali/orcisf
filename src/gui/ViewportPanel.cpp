#include "gui/ViewportPanel.h"

#include <imgui.h>

namespace orcisf::gui {

void ViewportPanel::Draw(bool* open) {
    if (!ImGui::Begin("Viewport", open)) {
        ImGui::End();
        return;
    }

    ImGui::TextDisabled(
        "3D structure/results viewport placeholder.\n"
        "Scene rendering, camera controls, and Example/ dataset loading\n"
        "land in issue #5.");

    ImGui::End();
}

} // namespace orcisf::gui
