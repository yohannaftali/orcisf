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

// Issue #37 renamed the "Run Optimization" panel's *display* text to
// "Optimization" (in RunPanel.cpp's PanelWindowTitle() call) without
// touching gui::kRunOptimizationId here -- this table only needs a
// panel's stable identity, not its current display label, so that
// rename needed no change on this side. Keep this table in sync with
// whichever gui::kXxxId constants Application.cpp's panels currently
// pass to PanelWindowTitle().
constexpr std::array<Entry, 11> kPanels = {{
    {gui::kViewportId, gui::PanelIcon::Viewport},
    {gui::kDetailingId, gui::PanelIcon::Detailing},
    {gui::kForceDiagramId, gui::PanelIcon::ForceDiagram}, // issue #61
    {gui::kResultsId, gui::PanelIcon::Results},           // issue #62
    {gui::kPropertiesId, gui::PanelIcon::Properties},
    {gui::kRunOptimizationId, gui::PanelIcon::Run},
    {gui::kAnalyzeId, gui::PanelIcon::Analyze}, // issue #69, epic #67
    {gui::kJointsId, gui::PanelIcon::Joints},
    {gui::kMembersId, gui::PanelIcon::Members},
    {gui::kLoadsId, gui::PanelIcon::Loads},
    {gui::kLogId, gui::PanelIcon::Log},
}};

} // namespace

void DrawDockTabIcons() {
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

            // Issue #51 (third pass): the first two passes (clip to
            // tab_bar->BarRect, then also clamp to WorkPos.y) both
            // treated this as a *position* bug and couldn't have worked --
            // the user's actual report (opening File and seeing the
            // Viewport/Detailing tab icons rendered on top of the
            // dropdown) is the exact same root cause already diagnosed
            // and fixed for ViewportPanel's label overlays: drawing onto
            // ImGui::GetForegroundDrawList() composites *after every
            // window*, popups included, by Dear ImGui's own layering --
            // no clip rect or position clamp changes which layer a draw
            // call belongs to. The fix (at the time) was to draw onto the
            // *owning panel's own* window draw list (`window->DrawList`)
            // instead.
            //
            // Issue #73 (found later): that #51 fix had its own gap --
            // `window->DrawList` only ever gets composited by
            // ImGui::Render() while `window` is the *currently selected*
            // tab in its tab bar (a docked-but-inactive tab is marked
            // `Hidden` for the frame, and Hidden windows are skipped
            // entirely when Render() assembles draw data -- see
            // `ImGuiWindow::Hidden`'s own doc comment in
            // imgui_internal.h: "Do not display"). So a panel's icon
            // vanished the moment its tab stopped being the active one,
            // even though the tab *button* itself (owned by the dock
            // node's host window, not by `window`) stayed fully visible.
            // Fix: draw onto `window->DockNode->HostWindow->DrawList`
            // instead -- the host window is the actual container that
            // draws the tab bar chrome, and it's rendered every frame the
            // dock group itself is visible, independent of which child
            // tab is currently selected. Confirmed via imgui_internal.h's
            // `ImGuiDockNode::HostWindow` field (present on any node with
            // a tab bar, not just the [Root node only]-qualified fields).
            // Falls back to `window->DrawList` if `HostWindow` is ever
            // null (shouldn't happen for a node with a live TabBar, but
            // cheaper than risking a null dereference for a purely
            // cosmetic overlay).
            ImDrawList* dl = window->DockNode->HostWindow ? window->DockNode->HostWindow->DrawList : window->DrawList;
            dl->PushClipRect(tab_bar->BarRect.Min, tab_bar->BarRect.Max, true);
            gui::DrawPanelIcon(entry.icon, dl, origin, color, icon_size);
            dl->PopClipRect();
            break;
        }
    }
}

} // namespace orcisf::app
