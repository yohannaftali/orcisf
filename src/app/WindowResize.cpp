#include "app/WindowResize.h"

#include "gui/UiScale.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <array>

namespace orcisf::app {

namespace {

using gui::Scaled;

// Issue #31: hand-drawn/raw-pixel chrome constants need Scaled(), or the
// hit region stays a few physical pixels wide on a high-DPI monitor --
// nearly unreachable by cursor -- while everything else in the window
// has visually doubled.
float EdgeThickness() { return Scaled(5.f); }
// Corners get a larger hit box than a plain edge -- matching normal OS
// window behavior, where a diagonal-resize corner is easier to land on
// than a single-axis edge.
float CornerThickness() { return Scaled(12.f); }

constexpr int kMinWindowWidth = 480;
constexpr int kMinWindowHeight = 360;

bool IsWaylandNoResizePlatform() { return glfwGetPlatform() == GLFW_PLATFORM_WAYLAND; }

// Standard cursor objects are cheap to create but GLFW never frees one
// until glfwDestroyCursor() -- creating a fresh one every frame would
// leak. Cached lazily, once each, for the process lifetime (matching how
// the GLFW/ImGui backend itself caches its own standard cursors). Index:
// 0 = NS, 1 = EW, 2 = NESW, 3 = NWSE.
GLFWcursor* CursorFor(int shape) {
    static std::array<GLFWcursor*, 4> cache{nullptr, nullptr, nullptr, nullptr};
    size_t index = shape == GLFW_RESIZE_NS_CURSOR     ? 0
                   : shape == GLFW_RESIZE_EW_CURSOR    ? 1
                   : shape == GLFW_RESIZE_NESW_CURSOR  ? 2
                                                        : 3;
    if (!cache[index]) {
        cache[index] = glfwCreateStandardCursor(shape);
    }
    return cache[index];
}

} // namespace

void BorderResizer::ApplyCursorShape(GLFWwindow* window, Edge edge) const {
    switch (edge) {
        case Edge::N:
        case Edge::S:
            glfwSetCursor(window, CursorFor(GLFW_RESIZE_NS_CURSOR));
            break;
        case Edge::E:
        case Edge::W:
            glfwSetCursor(window, CursorFor(GLFW_RESIZE_EW_CURSOR));
            break;
        case Edge::NE:
        case Edge::SW:
            glfwSetCursor(window, CursorFor(GLFW_RESIZE_NESW_CURSOR));
            break;
        case Edge::NW:
        case Edge::SE:
            glfwSetCursor(window, CursorFor(GLFW_RESIZE_NWSE_CURSOR));
            break;
        case Edge::None:
            glfwSetCursor(window, nullptr); // resets to the default arrow
            break;
    }
}

BorderResizer::Edge BorderResizer::HitTest(GLFWwindow* window, double cursor_x, double cursor_y) const {
    int w = 0, h = 0;
    glfwGetWindowSize(window, &w, &h);
    if (w <= 0 || h <= 0) return Edge::None;

    float edge_t = EdgeThickness();
    float corner_t = CornerThickness();

    bool in_bounds =
        cursor_x >= -edge_t && cursor_x <= w + edge_t && cursor_y >= -edge_t && cursor_y <= h + edge_t;
    if (!in_bounds) return Edge::None;

    bool near_left = cursor_x <= edge_t;
    bool near_right = cursor_x >= w - edge_t;
    bool near_top = cursor_y <= edge_t;
    bool near_bottom = cursor_y >= h - edge_t;

    bool corner_left = cursor_x <= corner_t;
    bool corner_right = cursor_x >= w - corner_t;
    bool corner_top = cursor_y <= corner_t;
    bool corner_bottom = cursor_y >= h - corner_t;

    if (near_top && corner_left) return Edge::NW;
    if (near_top && corner_right) return Edge::NE;
    if (near_bottom && corner_left) return Edge::SW;
    if (near_bottom && corner_right) return Edge::SE;
    if (near_left && corner_top) return Edge::NW;
    if (near_left && corner_bottom) return Edge::SW;
    if (near_right && corner_top) return Edge::NE;
    if (near_right && corner_bottom) return Edge::SE;

    if (near_left) return Edge::W;
    if (near_right) return Edge::E;
    if (near_top) return Edge::N;
    if (near_bottom) return Edge::S;
    return Edge::None;
}

void BorderResizer::Update(GLFWwindow* window) {
    // A maximized window has no visible edge to grab (it fills the work
    // area), and resizing it would fight glfwMaximizeWindow's own layout
    // -- same reasoning CustomTitleBar's drag already uses to disable
    // itself while maximized.
    if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
        if (resizing_) glfwSetCursor(window, nullptr);
        resizing_ = false;
        return;
    }
    if (IsWaylandNoResizePlatform()) {
        // Matches CustomTitleBar's Phase 0 Wayland note: glfwSetWindowPos/
        // glfwSetWindowSize are silent no-ops there (a Wayland compositor,
        // not the client, owns window placement/size), so there is
        // nothing this class could do -- left inert rather than
        // pretending to work.
        return;
    }

    int win_x = 0, win_y = 0, win_w = 0, win_h = 0;
    glfwGetWindowPos(window, &win_x, &win_y);
    glfwGetWindowSize(window, &win_w, &win_h);

    double cursor_x = 0.0, cursor_y = 0.0;
    glfwGetCursorPos(window, &cursor_x, &cursor_y);

    bool mouse_down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (!resizing_) {
        Edge edge = HitTest(window, cursor_x, cursor_y);
        ApplyCursorShape(window, edge);
        if (edge == Edge::None || !mouse_down) return;

        resizing_ = true;
        active_edge_ = edge;
        start_cursor_screen_x_ = win_x + cursor_x;
        start_cursor_screen_y_ = win_y + cursor_y;
        start_window_x_ = win_x;
        start_window_y_ = win_y;
        start_window_w_ = win_w;
        start_window_h_ = win_h;
        return;
    }

    if (!mouse_down) {
        resizing_ = false;
        return;
    }

    // Same "anchor to an absolute screen-cursor position captured once at
    // drag start" technique as CustomTitleBar's own drag -- see that
    // file's comment for why per-frame MouseDelta accumulation jitters
    // (glfwSetWindowPos/Size moves the window but not the physical
    // cursor, so the cursor's window-relative position silently shifts
    // by the exact opposite of the move on the very next frame).
    double cursor_screen_x = win_x + cursor_x;
    double cursor_screen_y = win_y + cursor_y;
    int dx = static_cast<int>(cursor_screen_x - start_cursor_screen_x_);
    int dy = static_cast<int>(cursor_screen_y - start_cursor_screen_y_);

    int new_x = start_window_x_;
    int new_y = start_window_y_;
    int new_w = start_window_w_;
    int new_h = start_window_h_;

    bool touches_east = active_edge_ == Edge::E || active_edge_ == Edge::NE || active_edge_ == Edge::SE;
    bool touches_west = active_edge_ == Edge::W || active_edge_ == Edge::NW || active_edge_ == Edge::SW;
    bool touches_north = active_edge_ == Edge::N || active_edge_ == Edge::NE || active_edge_ == Edge::NW;
    bool touches_south = active_edge_ == Edge::S || active_edge_ == Edge::SE || active_edge_ == Edge::SW;

    if (touches_east) new_w = start_window_w_ + dx;
    if (touches_west) new_w = start_window_w_ - dx;
    if (touches_south) new_h = start_window_h_ + dy;
    if (touches_north) new_h = start_window_h_ - dy;

    // Clamp size first, then re-derive the position for whichever edge
    // moves the window's own origin (W/N) -- computing them independently
    // (clamp size, but keep the unclamped position) would let the window
    // visually drift away from the cursor once the minimum is hit, since
    // position and size would then disagree about where the fixed
    // (opposite) edge actually is.
    if (new_w < kMinWindowWidth) new_w = kMinWindowWidth;
    if (new_h < kMinWindowHeight) new_h = kMinWindowHeight;
    if (touches_west) new_x = start_window_x_ + (start_window_w_ - new_w);
    if (touches_north) new_y = start_window_y_ + (start_window_h_ - new_h);

    if (new_x != win_x || new_y != win_y) glfwSetWindowPos(window, new_x, new_y);
    if (new_w != win_w || new_h != win_h) glfwSetWindowSize(window, new_w, new_h);
}

} // namespace orcisf::app
