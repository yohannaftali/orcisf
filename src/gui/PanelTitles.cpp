#include "gui/PanelTitles.h"

#include "gui/UiScale.h"

#include <imgui.h>

#include <algorithm>
#include <cmath>

namespace orcisf::gui {

std::string PanelWindowTitle(const char* stable_id, const char* display_label) {
    // Must match DockTabIcons.cpp's icon_size + gap exactly, or the icon
    // either overlaps the label or leaves an oversized gap in front of it.
    const float reserve = Scaled(13.f) + Scaled(6.f);
    const float space_w = ImGui::CalcTextSize(" ").x;
    const int n = (space_w > 0.f) ? static_cast<int>(std::ceil(reserve / space_w)) : 0;

    std::string title(static_cast<size_t>(std::max(0, n)), ' ');
    title += display_label;
    title += "###";
    title += stable_id;
    return title;
}

std::string PanelWindowId(const char* stable_id) { return std::string("###") + stable_id; }

} // namespace orcisf::gui
