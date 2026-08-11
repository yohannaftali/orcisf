#pragma once

struct GLFWwindow;

namespace orcisf::app {

// Issue #19 (Phase 0): a borderless-window replacement for the OS title
// bar -- app title, click-and-drag-to-move, and Minimize/Maximize/Close
// buttons, all drawn *inside* the app's existing main menu bar row
// (matching e.g. VS Code's own custom-title-bar layout) rather than as a
// separate stacked ImGui window above it. That's not a style choice: Dear
// ImGui's BeginMainMenuBar() always claims the true top of the viewport
// for itself regardless of any prior window's manual placement there, so
// a second top-of-viewport window submitted before it gets silently
// painted over (verified interactively -- a debug-colored standalone
// title bar window rendered zero visible pixels). Living inside the same
// window sidesteps that entirely.
//
// Platform-agnostic: uses only GLFW's own cross-platform API
// (glfwSetWindowPos for dragging, glfwIconifyWindow/glfwMaximizeWindow/
// glfwRestoreWindow/glfwSetWindowShouldClose for the buttons), no native
// window handles.
//
// Known limitation (see issue #19's Phase 0 correction): dragging is a
// no-op on Wayland, since Wayland compositors don't allow a client to
// position its own window at all (unlike X11/Win32/Cocoa). Detected at
// runtime and surfaced via a disabled-looking drag zone + tooltip rather
// than failing silently. Native per-platform drag/resize/snap (including
// a real fix for Wayland via xdg_toplevel_move) is Phase 1+, tracked as
// follow-up work in the same issue.
class CustomTitleBar {
public:
    // Call from *inside* an already-open ImGui menu bar (i.e. between
    // Toolbar's last ImGui::BeginMenu block and ImGui::EndMainMenuBar()),
    // wired via Toolbar::SetTitleBarDrawer(). Right-aligns the drag zone
    // and buttons to the current window's width; draws the title text
    // over the drag zone.
    void Draw(GLFWwindow* window, const char* title);

private:
    // Drag anchor, captured once when the drag zone item activates (see
    // Draw()'s comment on why this must be an absolute reference rather
    // than accumulated per-frame deltas -- the latter jitters).
    bool dragging_ = false;
    double drag_start_cursor_screen_x_ = 0.0;
    double drag_start_cursor_screen_y_ = 0.0;
    int drag_start_window_x_ = 0;
    int drag_start_window_y_ = 0;
};

} // namespace orcisf::app
