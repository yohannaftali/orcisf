#pragma once

namespace orcisf::app {

// Issue #19 (Phase 0): a premium dark-slate ImGui style, replacing the
// default ImGui::StyleColorsDark() the app started with. Call once at
// startup, after ImGui::CreateContext(). Pure ImGui style/color state --
// no window-chrome/GLFW dependency, unlike CustomTitleBar.
void ApplyModernTheme();

} // namespace orcisf::app
