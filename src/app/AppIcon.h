#pragma once

struct GLFWwindow;

// Issue #26: applies the ORCISF app icon set (checked in at repo-root
// icons/, see icons/README.txt) to the running window, where the platform
// supports a runtime window icon at all.
namespace orcisf::app {

// Sets the window's icon (taskbar/Alt+Tab on Windows, window-manager
// decorations/task list on Linux) from the icon PNGs CMakeLists.txt
// copies next to the executable at build time. No-op on macOS -- GLFW
// does not support glfwSetWindowIcon() there; the Dock/Finder icon comes
// from the .app bundle's Info.plist (MACOSX_BUNDLE_ICON_FILE, wired in
// CMakeLists.txt) instead. Also a graceful no-op (not fatal) if the icon
// files can't be found/decoded -- a missing icon is cosmetic, never worth
// blocking startup over.
void ApplyWindowIcon(GLFWwindow* window);

} // namespace orcisf::app
