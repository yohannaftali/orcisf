#include "gui/LogPanel.h"

#include <imgui.h>

namespace orcisf::gui {

void LogPanel::AddLine(std::string line) {
    lines_.push_back(std::move(line));
}

void LogPanel::Clear() {
    lines_.clear();
}

void LogPanel::Draw(bool* open) {
    if (!ImGui::Begin("Log", open)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear")) {
        Clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_);

    ImGui::Separator();
    ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& line : lines_) {
        ImGui::TextUnformatted(line.c_str());
    }
    if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();

    ImGui::End();
}

} // namespace orcisf::gui
