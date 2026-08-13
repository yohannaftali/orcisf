#pragma once

#include <imgui.h>

#include "gui/PanelIcons.h"

// Issue #52: a shared, reusable base for every editable list table in this
// project (Joints/Members/Loads today; any future panel needing a table
// should use this instead of re-implementing ImGui::BeginTable from
// scratch). Deliberately tiny -- two thin wrappers, not a full framework --
// since the actual per-table row content (which columns, which editable
// widgets) still varies enough per panel that abstracting further would
// cost more than it saves.
namespace orcisf::gui {

// The table-wide flags every shared table uses. Resizable lets the user
// drag a column border to widen/narrow it; SizingStretchProp is the fix
// for the original bug report (columns getting cut off on a high-
// resolution display instead of reflowing) -- without an explicit sizing
// policy, ImGui's table sizing defaults to fixed/content-based widths, so
// columns never grow to fill the available panel width as the window is
// resized. RowBg/Borders are purely visual, matching what these tables
// already looked like before this issue.
constexpr ImGuiTableFlags kTableViewFlags =
    ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;

inline bool BeginTableView(const char* str_id, int column_count) {
    return ImGui::BeginTable(str_id, column_count, kTableViewFlags);
}

// Issue #41's fix, factored out so every row-selecting cell in this
// project's tables shares the exact same (hard-won) flags: plain
// ImGuiSelectableFlags_SpanAllColumns unconditionally consumes every
// click across the whole row, making any later-column widget (InputFloat,
// Combo, ...) unreachable by click -- AllowOverlap is what actually lets
// a later-submitted widget win hit-testing at its own rect instead (see
// AGENTS.md's issue #41 note for the direct-instrumentation story behind
// this). The explicit GetFrameHeight() height matches an InputFloat
// cell's real height so the row highlight covers the whole row, not a
// short band above it.
inline bool TableRowSelectable(const char* label, bool selected) {
    return ImGui::Selectable(label, selected,
                              ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
                              ImVec2(0.f, ImGui::GetFrameHeight()));
}

// Issue #54/#55: horizontally centers a narrow, fixed-size widget (a
// checkbox, the trash button, ...) about to be drawn within the
// *current table column's* available width, rather than leaving it at
// ImGui's default left edge of the cell -- a `WidthFixed` DOF/action
// column is deliberately wider than the widget itself (room for the
// short header text above it), so without this the widget reads as
// visibly left-aligned under a centered-looking header instead of
// lining up with it. Must be called after `TableSetColumnIndex()` and
// before the widget, in the same cell.
inline void CenterNextTableItem(float item_width) {
    float avail = ImGui::GetContentRegionAvail().x;
    if (avail > item_width) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - item_width) * 0.5f);
    }
}

// Issue #54: the action column's width for TableTrashButton() below --
// just enough for a square icon button + a little breathing room, tied
// to ImGui::GetFrameHeight() (already DPI-correct via issue #31's
// FontScaleDpi) rather than a hardcoded literal, so it scales with the
// current font/frame size the same way every other cell in these tables
// already does.
inline float TableTrashColumnWidth() { return ImGui::GetFrameHeight() * 1.6f; }

// Issue #54: replaces every plain-text "Delete"/"Clear" `SmallButton` in
// these tables with a light-red trash-can icon button, so a destructive
// row action reads as visually distinct at a glance. Deliberately always
// tinted (not just on hover) -- the whole point is to stand out
// immediately, not only once the user happens to hover it. `{0.82f,
// 0.22f, 0.24f, ...}` (~#D13A3D) is documented alongside this project's
// other UI/viewport colors in AGENTS.md's issue #54 note; pick a
// different one there first if this ever needs to change, don't just
// edit the literal here.
inline bool TableTrashButton(const char* str_id) {
    const float h = ImGui::GetFrameHeight();
    CenterNextTableItem(h);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 size(h, h);
    bool clicked = ImGui::InvisibleButton(str_id, size);
    bool hovered = ImGui::IsItemHovered();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 bg = hovered ? IM_COL32(209, 58, 61, 255) : IM_COL32(209, 58, 61, 200);
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bg, ImGui::GetStyle().FrameRounding);

    float icon_size = h * 0.5f;
    ImVec2 icon_origin(pos.x + (size.x - icon_size) * 0.5f, pos.y + (size.y - icon_size) * 0.5f);
    DrawTrashIcon(dl, icon_origin, IM_COL32(255, 255, 255, 255), icon_size);
    return clicked;
}

} // namespace orcisf::gui
