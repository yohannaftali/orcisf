#pragma once

// Issue #35 (correcting #28 Part 2): draws each registered panel's small
// hand-drawn icon directly onto its own dock tab button, immediately before
// the tab's title text -- see PanelIcons.h for why this needed
// imgui_internal.h rather than a baked font-atlas glyph, and PanelIcons.cpp
// for the actual icon shapes drawn.
//
// Lives under app/, not gui/, because it operates on dock-node/tab-bar
// internals that are conceptually an Application-level (not per-panel)
// concern -- the same boundary Application.cpp's own BuildDockspace()
// (issue #15) already draws by being the only other file in this port that
// includes imgui_internal.h, for the DockBuilder* functions.
namespace orcisf::app {

// Call once per frame, after every panel's Begin()/End() has run for that
// frame (so each panel's ImGuiWindow::DockNode/tab-bar geometry is final)
// and before ImGui::Render() -- Application::OnFrame() calls this last.
void DrawDockTabIcons();

} // namespace orcisf::app
