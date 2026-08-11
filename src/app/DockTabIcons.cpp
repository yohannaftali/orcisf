#include "app/DockTabIcons.h"

// Must come before any other header that might transitively include
// imgui.h first (PanelIcons.h/PanelTitles.h/UiScale.h below all do) --
// imgui_internal.h hard-errors if imgui.h was already included without
// this defined, since the macro can only take effect on imgui.h's first
// inclusion in the translation unit.
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "gui/PanelIcons.h"
#include "gui/PanelTitles.h"
#include "gui/UiScale.h"

#include <array>

namespace orcisf::app {

namespace {

struct Entry {
    const char* stable_id; // one of gui::kXxxId, see PanelTitles.h
    gui::PanelIcon icon;
};

// Issue #37 will edit this table's display: "Run Optimization" ->
// "Optimization" is a display-only change, gui::kRunOptimizationId's
// *identity* string doesn't need to change for that. Keep this table in
// sync with whichever gui::kXxxId constants Application.cpp's panels
// currently pass to PanelWindowTitle().
constexpr std::array<Entry, 8> kPanels = {{
    {gui::kViewportId, gui::PanelIcon::Viewport},
    {gui::kDetailingId, gui::PanelIcon::Detailing},
    {gui::kPropertiesId, gui::PanelIcon::Properties},
    {gui::kRunOptimizationId, gui::PanelIcon::Run},
    {gui::kJointsId, gui::PanelIcon::Joints},
    {gui::kMembersId, gui::PanelIcon::Members},
    {gui::kLoadsId, gui::PanelIcon::Loads},
    {gui::kLogId, gui::PanelIcon::Log},
}};

} // namespace

void DrawDockTabIcons() {
    ImDrawList* fg = ImGui::GetForegroundDrawList();
    const float icon_size = gui::Scaled(13.f);
    const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);

    for (const Entry& entry : kPanels) {
        // FindWindowByName() hashes its argument the same way Begin() does
        // (ImHashStr(), which -- per ImHashSkipUncontributingPrefix() --
        // only hashes the part after "###") -- so the bare "###stable_id"
        // string here resolves to the exact same window that
        // PanelWindowTitle() built with a *different* (DPI-dependent)
        // leading-space count in front. See PanelTitles.h for the full
        // reasoning.
        ImGuiWindow* window = ImGui::FindWindowByName(gui::PanelWindowId(entry.stable_id).c_str());
        // DockIsActive is false for a single-window ("floating") dock node
        // even though DockNode may still be non-null there (see the field's
        // own comment in imgui_internal.h) -- gate on it explicitly rather
        // than just `!window->DockNode`, since a lone panel with an
        // auto-hidden tab bar (ImGuiDockNodeFlags_AutoHideTabBar) has no
        // tab button to draw an icon onto in the first place.
        if (!window || !window->DockIsActive || !window->DockNode) continue;
        ImGuiTabBar* tab_bar = window->DockNode->TabBar;
        if (!tab_bar) continue;

        for (const ImGuiTabItem& tab : tab_bar->Tabs) {
            if (tab.Window != window) continue;
            // Matches Dear ImGui's own tab-item layout (imgui_widgets.cpp's
            // TabItemLabelAndCloseButton()): each tab's label starts
            // FramePadding.x in from the tab's own left edge, which is
            // BarRect.Min.x + tab.Offset. Vertically centered in the bar.
            float label_x = tab_bar->BarRect.Min.x + tab.Offset + tab_bar->FramePadding.x;
            float center_y = tab_bar->BarRect.Min.y + tab_bar->BarRect.GetHeight() * 0.5f;
            ImVec2 origin(label_x, center_y - icon_size * 0.5f);
            gui::DrawPanelIcon(entry.icon, fg, origin, color, icon_size);
            break;
        }
    }
}

} // namespace orcisf::app
