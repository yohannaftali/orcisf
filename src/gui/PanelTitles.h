#pragma once

#include <string>

namespace orcisf::gui {

// Single source of truth for each panel's stable dock/window identity --
// referenced by both the panel's own ImGui::Begin() call (via
// PanelWindowTitle()) and Application.cpp's DockBuilderDockWindow()/
// DockTabIcons.cpp's FindWindowByName() calls (via PanelWindowId()). Keep
// these here, not duplicated as string literals at each call site, so a
// typo can't silently desync a panel from its own dock-layout/icon wiring.
inline constexpr const char* kViewportId = "Viewport";
inline constexpr const char* kDetailingId = "Detailing";
inline constexpr const char* kPropertiesId = "Properties";
inline constexpr const char* kRunOptimizationId = "RunOptimization";
inline constexpr const char* kJointsId = "Joints";
inline constexpr const char* kMembersId = "Members";
inline constexpr const char* kLoadsId = "Loads";
inline constexpr const char* kLogId = "Log";
inline constexpr const char* kForceDiagramId = "ForceDiagram"; // issue #61
inline constexpr const char* kResultsId = "Results"; // issue #62

// Issue #35: the literal string every panel now passes to `ImGui::Begin()`
// is `<N leading spaces><display label>###<stable_id>`, not a plain label.
// The leading spaces reserve exactly enough horizontal room in the tab for
// DockTabIcons.cpp's icon overlay to sit in front of the label without
// overlapping it -- recomputed every frame from the *current* font metrics,
// so it always matches whatever DPI scale (issue #31) is active right now.
//
// The "###stable_id" suffix is what keeps this safe: per Dear ImGui's ID
// rules (see `ImHashSkipUncontributingPrefix()` in imgui_internal.h), a
// "###" marker makes everything BEFORE it -- the variable-length leading
// spaces, the display label -- not contribute to the window's ID hash at
// all. Only "stable_id" does. That means the leading-space count changing
// every time DPI changes never changes the window's identity, so docking
// placement, DockBuilderDockWindow() matching, and imgui.ini persistence
// all stay stable across a live DPI rescale -- exactly the property issue
// #31's live-rescale support needs from anything drawn in this frame.
//
// DockBuilderDockWindow()/FindWindowByName() callers that only need to
// reference a panel's identity (not display anything) use PanelWindowId()
// instead -- just the "###stable_id" part, since that alone hashes to the
// same ID as the full title above.
std::string PanelWindowTitle(const char* stable_id, const char* display_label);
std::string PanelWindowId(const char* stable_id);

} // namespace orcisf::gui
