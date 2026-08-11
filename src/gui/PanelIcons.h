#pragma once

// Issue #28: a small hand-drawn icon before each docked panel's title,
// matching this project's established "small, dependency-free ImDrawList
// icons, no icon font" approach (see IconToolbar.cpp / ViewportPanel.cpp's
// UCS icon). Dear ImGui offers no public way to embed an arbitrary glyph
// inside a dock-tab's own label text without either an icon font (a new
// asset/dependency this project doesn't otherwise need) or reaching into
// imgui_internal.h to hook the shared dock-node tab bar (fragile, and only
// this project's `report/PdfExport.cpp` reaches past the public API, for
// libharu's C callback model -- not a precedent for touching ImGui's own
// internals). Instead, each panel calls DrawPanelIconHeader() as the very
// first thing after ImGui::Begin(): it draws the icon + a bold-ish label
// row inside the panel's own content area -- visible whenever that panel
// is the active tab, which is the common case a user is looking at it.
namespace orcisf::gui {

enum class PanelIcon { Viewport, Detailing, Properties, JointsMembers, Loads, Run, Log };

// Draws `icon` followed by `label` as a header row at the current cursor
// position (call immediately after ImGui::Begin(), before any other
// content) and a separator beneath it.
void DrawPanelIconHeader(PanelIcon icon, const char* label);

} // namespace orcisf::gui
