#include "gui/PropertiesPanel.h"

#include <imgui.h>

namespace orcisf::gui {

void PropertiesPanel::Draw(bool* open) {
    if (!ImGui::Begin("Properties", open)) {
        ImGui::End();
        return;
    }

    ImGui::TextDisabled(
        "Selection properties placeholder.\n"
        "Numeric joint/member/load editing lands in issue #6/#7.");

    ImGui::End();
}

} // namespace orcisf::gui
