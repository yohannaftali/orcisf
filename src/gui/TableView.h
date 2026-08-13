#pragma once

#include <imgui.h>

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

} // namespace orcisf::gui
