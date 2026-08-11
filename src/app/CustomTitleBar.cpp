#include "app/CustomTitleBar.h"

#include "gui/UiScale.h"

#include <cmath>
#include <cstring>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace orcisf::app {

namespace {

using gui::Scaled;

// Issue #31: hand-drawn chrome sizes itself in raw pixels, so every
// constant here is a *base* (100%-scale) value that must go through
// Scaled() -- otherwise the three window buttons stay 46 physical pixels
// wide next to text that has doubled, and their glyphs become specks.
float ButtonWidth() { return Scaled(46.0f); }
float IconSize() { return Scaled(10.0f); }

// Wayland compositors don't let a client position its own window (unlike
// X11/Win32/Cocoa) -- glfwSetWindowPos silently does nothing there. See
// issue #19's Phase 0 correction: detect it so the drag zone can look
// visibly inert instead of just failing to respond with no explanation.
// glfwGetPlatform() needs GLFW 3.4+ (this project pins 3.5, see vcpkg.json).
bool IsWaylandNoDragPlatform() { return glfwGetPlatform() == GLFW_PLATFORM_WAYLAND; }

// One title-bar button (minimize/maximize/close): a flat hover-highlighted
// hit area with a small hand-drawn glyph, matching gui::IconToolbar's
// dependency-free hand-drawn-icon approach (no icon font).
bool TitleBarButton(const char* id, float height, ImU32 hover_color,
                     void (*draw_icon)(ImDrawList*, ImVec2, ImVec2, ImU32)) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size(ButtonWidth(), height);
    bool clicked = ImGui::InvisibleButton(id, size);
    bool hovered = ImGui::IsItemHovered();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (hovered) {
        dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), hover_color);
    }
    // Rounded to whole pixels: at an odd button width the glyph would
    // otherwise straddle a half-pixel and render visibly softer/off-centre
    // than the neighbouring buttons.
    ImVec2 center(std::round(pos.x + size.x * 0.5f), std::round(pos.y + size.y * 0.5f));
    ImU32 icon_color = IM_COL32(230, 230, 235, 255);
    draw_icon(dl, center, ImVec2(IconSize(), IconSize()), icon_color);
    return clicked;
}

void DrawMinimizeIcon(ImDrawList* dl, ImVec2 center, ImVec2 size, ImU32 color) {
    dl->AddLine(ImVec2(center.x - size.x * 0.5f, center.y), ImVec2(center.x + size.x * 0.5f, center.y), color,
                Scaled(1.5f));
}

void DrawMaximizeIcon(ImDrawList* dl, ImVec2 center, ImVec2 size, ImU32 color) {
    ImVec2 half(size.x * 0.5f, size.y * 0.5f);
    dl->AddRect(ImVec2(center.x - half.x, center.y - half.y), ImVec2(center.x + half.x, center.y + half.y), color, 0.f,
                0, Scaled(1.3f));
}

void DrawRestoreIcon(ImDrawList* dl, ImVec2 center, ImVec2 size, ImU32 color) {
    ImVec2 half(size.x * 0.38f, size.y * 0.38f);
    ImVec2 offset(size.x * 0.18f, -size.y * 0.18f);
    dl->AddRect(ImVec2(center.x - half.x + offset.x, center.y - half.y + offset.y),
                ImVec2(center.x + half.x + offset.x, center.y + half.y + offset.y), color, 0.f, 0, Scaled(1.2f));
    dl->AddRect(ImVec2(center.x - half.x - offset.x, center.y - half.y - offset.y),
                ImVec2(center.x + half.x - offset.x, center.y + half.y - offset.y), color, 0.f, 0, Scaled(1.2f));
}

void DrawCloseIcon(ImDrawList* dl, ImVec2 center, ImVec2 size, ImU32 color) {
    ImVec2 half(size.x * 0.5f, size.y * 0.5f);
    dl->AddLine(ImVec2(center.x - half.x, center.y - half.y), ImVec2(center.x + half.x, center.y + half.y), color,
                Scaled(1.5f));
    dl->AddLine(ImVec2(center.x - half.x, center.y + half.y), ImVec2(center.x + half.x, center.y - half.y), color,
                Scaled(1.5f));
}

} // namespace

void CustomTitleBar::Draw(GLFWwindow* window, const char* title) {
    const float height = ImGui::GetFrameHeight();
    const float window_width = ImGui::GetWindowWidth();
    const float buttons_width = ButtonWidth() * 3.0f;

    // The one screen-space anchor everything below is aligned against: the
    // menu bar window's true right edge, minus the button cluster.
    //
    // This deliberately does NOT go through ImGui::SameLine(offset_x) any
    // more. SameLine()'s offset is measured from the *content* origin, so
    // it silently adds the enclosing window's padding
    // (BeginMenuBar() opens a group whose GroupOffset.x == WindowPadding.x)
    // on top of whatever offset is passed -- the cluster was therefore
    // being pushed WindowPadding.x past the window's right edge and the
    // Close button clipped by exactly that much. At 100% scale that is
    // ApplyModernTheme()'s 14px, which reads as "the X glyph sits right of
    // centre"; at 200% it is 28px of a 92px button, i.e. visibly cut off.
    // Absolute screen coordinates have no such hidden term.
    const float bar_right_x = ImGui::GetWindowPos().x + window_width;
    const float buttons_start_screen_x = bar_right_x - buttons_width;

    const bool wayland_no_drag = IsWaylandNoDragPlatform();
    const bool maximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) != 0;

    ImGui::SameLine();
    const ImVec2 row_origin = ImGui::GetCursorScreenPos();
    const float drag_zone_width = buttons_start_screen_x - row_origin.x;

    if (drag_zone_width > Scaled(20.0f)) {
        ImVec2 drag_pos = row_origin;
        ImGui::InvisibleButton("##titlebar_drag", ImVec2(drag_zone_width, height));

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec4 text_col_v4 = wayland_no_drag ? ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]
                                              : ImGui::GetStyle().Colors[ImGuiCol_Text];
        ImU32 text_col = ImGui::ColorConvertFloat4ToU32(text_col_v4);
        const float text_x = drag_pos.x + Scaled(10.0f);

        // Issue #31: the title has to be truncated to the drag zone. It is
        // drawn straight onto the draw list, which does no layout of its
        // own, so at a large DPI scale the (unchanged) title string simply
        // grew until it ran underneath the Minimize/Maximize/Close cluster
        // -- text and glyphs overlapping, both unreadable. Trim to fit with
        // an ellipsis rather than just clipping, so it degrades to a
        // readable prefix instead of a word cut mid-glyph.
        const char* text_end = title + std::strlen(title);
        const float avail = (buttons_start_screen_x - Scaled(8.0f)) - text_x;
        std::string elided;
        if (avail > 0.f && ImGui::CalcTextSize(title, text_end).x > avail) {
            const char* ellipsis = "...";
            const float ellipsis_w = ImGui::CalcTextSize(ellipsis).x;
            std::size_t keep = std::strlen(title);
            while (keep > 0 && ImGui::CalcTextSize(title, title + keep).x + ellipsis_w > avail) {
                --keep;
            }
            elided.assign(title, keep);
            elided += ellipsis;
        }
        const char* draw_text = elided.empty() ? title : elided.c_str();
        ImVec2 text_size = ImGui::CalcTextSize(draw_text);
        if (avail > 0.f) {
            dl->AddText(ImVec2(text_x, drag_pos.y + (height - text_size.y) * 0.5f), text_col, draw_text);
        }

        if (wayland_no_drag) {
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Window dragging isn't supported on Wayland yet (see issue #19) -- "
                                   "use your window manager's move gesture instead.");
            }
        } else {
            // Anchored to an absolute screen-cursor position captured once
            // at drag start, NOT accumulated per-frame ImGui MouseDelta.
            // The delta approach jitters: glfwSetWindowPos moves the
            // window but not the physical cursor, so the cursor's
            // *window-relative* position (which is what GLFW's mouse
            // callback -- and therefore ImGui's MouseDelta -- reports)
            // silently shifts by the exact opposite of the move on the
            // very next frame, creating a feedback oscillation. Recomputing
            // glfwGetWindowPos() + glfwGetCursorPos() fresh every frame
            // (both always current, not event-cached) and referencing
            // everything back to the drag-start anchor sidesteps that
            // entirely. Caught interactively -- the delta version visibly
            // glittered/shook while dragging.
            if (!maximized && ImGui::IsItemActivated()) {
                dragging_ = true;
                double cursor_x = 0.0, cursor_y = 0.0;
                glfwGetCursorPos(window, &cursor_x, &cursor_y);
                glfwGetWindowPos(window, &drag_start_window_x_, &drag_start_window_y_);
                drag_start_cursor_screen_x_ = drag_start_window_x_ + cursor_x;
                drag_start_cursor_screen_y_ = drag_start_window_y_ + cursor_y;
            }
            if (dragging_ && ImGui::IsItemActive() && !maximized) {
                double cursor_x = 0.0, cursor_y = 0.0;
                glfwGetCursorPos(window, &cursor_x, &cursor_y);
                int win_x = 0, win_y = 0;
                glfwGetWindowPos(window, &win_x, &win_y);
                double cursor_screen_x = win_x + cursor_x;
                double cursor_screen_y = win_y + cursor_y;
                int new_x = drag_start_window_x_ + static_cast<int>(cursor_screen_x - drag_start_cursor_screen_x_);
                int new_y = drag_start_window_y_ + static_cast<int>(cursor_screen_y - drag_start_cursor_screen_y_);
                if (new_x != win_x || new_y != win_y) {
                    glfwSetWindowPos(window, new_x, new_y);
                }
            }
            if (!ImGui::IsItemActive()) {
                dragging_ = false;
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (maximized) {
                    glfwRestoreWindow(window);
                } else {
                    glfwMaximizeWindow(window);
                }
            }
        }
    }

    // Minimize / Maximize-Restore / Close, right-aligned flush to the
    // window's true right edge regardless of how wide the drag zone ended
    // up. Positioned with SetCursorScreenPos() rather than
    // SameLine(offset_x) -- see buttons_start_screen_x above for why the
    // latter silently shifted the whole cluster off the edge. Zero-spacing
    // SameLine() *between* the buttons still matters: style.ItemSpacing
    // (nonzero, see ApplyModernTheme()) would otherwise widen the cluster
    // past buttons_width and push Close out again.
    ImGui::SetCursorScreenPos(ImVec2(buttons_start_screen_x, row_origin.y));
    if (TitleBarButton("##minimize", height, IM_COL32(255, 255, 255, 25), DrawMinimizeIcon)) {
        glfwIconifyWindow(window);
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (TitleBarButton("##maximize", height, IM_COL32(255, 255, 255, 25), maximized ? DrawRestoreIcon : DrawMaximizeIcon)) {
        if (maximized) {
            glfwRestoreWindow(window);
        } else {
            glfwMaximizeWindow(window);
        }
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (TitleBarButton("##close", height, IM_COL32(232, 17, 35, 255), DrawCloseIcon)) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

} // namespace orcisf::app
