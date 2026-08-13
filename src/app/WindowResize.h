#pragma once

struct GLFWwindow;

namespace orcisf::app {

// Issue #53: manual edge/corner resize for the borderless window (issue
// #19 removed GLFW_DECORATED, which also removes the OS's own resize
// border/grips along with its title bar). Platform-agnostic via GLFW's
// own cross-platform API only (glfwGetCursorPos/glfwSetWindowSize/
// glfwSetWindowPos/glfwSetCursor with GLFW's standard resize-cursor
// shapes) -- no native window handles, matching CustomTitleBar's existing
// constraint. Same Wayland limitation as CustomTitleBar's drag
// (glfwSetWindowPos/glfwSetWindowSize are no-ops there): detected and
// left inert the same way, rather than fighting a platform that won't
// let a client reposition/resize itself.
class BorderResizer {
public:
    // Call once per frame, right after glfwPollEvents() and before
    // ImGui::NewFrame() -- so a resize started this frame is visible to
    // ImGui's own viewport-size read the same frame, not one frame late.
    // Reads raw GLFW cursor/window state only; never touches ImGui, so
    // it can't conflict with CustomTitleBar's drag zone or button
    // hit-testing (those run later, inside the ImGui frame).
    void Update(GLFWwindow* window);

private:
    enum class Edge { None, N, S, E, W, NE, NW, SE, SW };
    Edge HitTest(GLFWwindow* window, double cursor_x, double cursor_y) const;
    void ApplyCursorShape(GLFWwindow* window, Edge edge) const;

    bool resizing_ = false;
    Edge active_edge_ = Edge::None;
    double start_cursor_screen_x_ = 0.0;
    double start_cursor_screen_y_ = 0.0;
    int start_window_x_ = 0;
    int start_window_y_ = 0;
    int start_window_w_ = 0;
    int start_window_h_ = 0;
};

} // namespace orcisf::app
