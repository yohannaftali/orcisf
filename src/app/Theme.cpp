#include "app/Theme.h"

#include <imgui.h>

namespace orcisf::app {

void ApplyModernTheme() {
    ImGui::StyleColorsDark(); // baseline every color below overrides from

    ImGuiStyle& style = ImGui::GetStyle();

    // Spacious, rounded, low-friction geometry -- the "premium" part of
    // the ask isn't a specific color so much as generous padding and
    // consistently soft corners instead of ImGui's default hard edges.
    style.WindowRounding = 10.0f;
    style.ChildRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f; // borders come from subtle color contrast, not thick outlines

    style.WindowPadding = ImVec2(14.0f, 12.0f);
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.IndentSpacing = 18.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;

    // Dark-slate palette: neutral cool grays for surfaces, a single
    // desaturated blue accent for interactive/active state. Kept in one
    // place (not per-widget) so retinting later is a one-function change.
    const ImVec4 kBgDeep(0.09f, 0.10f, 0.12f, 1.00f);      // window/viewport background
    const ImVec4 kBgSurface(0.13f, 0.14f, 0.17f, 1.00f);   // panels, frames at rest
    const ImVec4 kBgRaised(0.17f, 0.18f, 0.21f, 1.00f);    // hovered surfaces
    const ImVec4 kBgPressed(0.21f, 0.22f, 0.26f, 1.00f);   // active/pressed surfaces
    const ImVec4 kBorder(0.24f, 0.25f, 0.29f, 0.60f);      // subtle, not harsh
    const ImVec4 kTextPrimary(0.92f, 0.93f, 0.95f, 1.00f);
    const ImVec4 kTextMuted(0.55f, 0.57f, 0.62f, 1.00f);
    const ImVec4 kAccent(0.35f, 0.55f, 0.95f, 1.00f);      // interactive highlight
    const ImVec4 kAccentHover(0.45f, 0.64f, 1.00f, 1.00f);
    const ImVec4 kAccentActive(0.28f, 0.46f, 0.85f, 1.00f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = kTextPrimary;
    colors[ImGuiCol_TextDisabled] = kTextMuted;
    colors[ImGuiCol_WindowBg] = kBgDeep;
    colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_PopupBg] = kBgSurface;
    colors[ImGuiCol_Border] = kBorder;
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    colors[ImGuiCol_FrameBg] = kBgSurface;
    colors[ImGuiCol_FrameBgHovered] = kBgRaised;
    colors[ImGuiCol_FrameBgActive] = kBgPressed;

    colors[ImGuiCol_TitleBg] = kBgDeep;
    colors[ImGuiCol_TitleBgActive] = kBgSurface;
    colors[ImGuiCol_TitleBgCollapsed] = kBgDeep;
    colors[ImGuiCol_MenuBarBg] = kBgSurface;

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ScrollbarGrab] = kBgRaised;
    colors[ImGuiCol_ScrollbarGrabHovered] = kBgPressed;
    colors[ImGuiCol_ScrollbarGrabActive] = kAccentActive;

    colors[ImGuiCol_CheckMark] = kAccent;
    colors[ImGuiCol_SliderGrab] = kAccent;
    colors[ImGuiCol_SliderGrabActive] = kAccentActive;

    colors[ImGuiCol_Button] = kBgSurface;
    colors[ImGuiCol_ButtonHovered] = kBgRaised;
    colors[ImGuiCol_ButtonActive] = kAccentActive;

    colors[ImGuiCol_Header] = kBgRaised;
    colors[ImGuiCol_HeaderHovered] = kBgPressed;
    colors[ImGuiCol_HeaderActive] = kAccentActive;

    colors[ImGuiCol_Separator] = kBorder;
    colors[ImGuiCol_SeparatorHovered] = kAccent;
    colors[ImGuiCol_SeparatorActive] = kAccentActive;

    colors[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ResizeGripHovered] = kAccent;
    colors[ImGuiCol_ResizeGripActive] = kAccentActive;

    colors[ImGuiCol_Tab] = kBgSurface;
    colors[ImGuiCol_TabHovered] = kAccentHover;
    colors[ImGuiCol_TabActive] = kBgRaised;
    colors[ImGuiCol_TabUnfocused] = kBgSurface;
    colors[ImGuiCol_TabUnfocusedActive] = kBgRaised;

    colors[ImGuiCol_DockingPreview] = kAccent;
    colors[ImGuiCol_DockingEmptyBg] = kBgDeep;

    colors[ImGuiCol_PlotLines] = kAccent;
    colors[ImGuiCol_PlotLinesHovered] = kAccentHover;
    colors[ImGuiCol_PlotHistogram] = kAccent;
    colors[ImGuiCol_PlotHistogramHovered] = kAccentHover;

    colors[ImGuiCol_TextSelectedBg] = ImVec4(kAccent.x, kAccent.y, kAccent.z, 0.35f);
    colors[ImGuiCol_DragDropTarget] = kAccentHover;
    colors[ImGuiCol_NavHighlight] = kAccent;
}

} // namespace orcisf::app
