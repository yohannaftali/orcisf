#include "gui/PanelIcons.h"

#include <initializer_list>

#include <imgui.h>

namespace orcisf::gui {

namespace {

constexpr float kIconSize = 16.f;

// Local point (x,y in [0,1] of the icon box) -> screen coordinates, same
// helper pattern as IconToolbar.cpp's P().
ImVec2 P(const ImVec2& origin, float x, float y) { return ImVec2(origin.x + x * kIconSize, origin.y + y * kIconSize); }

void DrawViewportIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    // A simple cube (3D viewport).
    dl->AddQuad(P(o, 0.15f, 0.35f), P(o, 0.55f, 0.2f), P(o, 0.95f, 0.35f), P(o, 0.55f, 0.5f), c, 1.3f);
    dl->AddLine(P(o, 0.15f, 0.35f), P(o, 0.15f, 0.75f), c, 1.3f);
    dl->AddLine(P(o, 0.95f, 0.35f), P(o, 0.95f, 0.75f), c, 1.3f);
    dl->AddLine(P(o, 0.55f, 0.5f), P(o, 0.55f, 0.9f), c, 1.3f);
    dl->AddLine(P(o, 0.15f, 0.75f), P(o, 0.55f, 0.9f), c, 1.3f);
    dl->AddLine(P(o, 0.95f, 0.75f), P(o, 0.55f, 0.9f), c, 1.3f);
}

void DrawDetailingIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    // A cross-section outline with a rebar dot -- matches the reinforcement
    // detailing drawings this panel shows.
    dl->AddRect(P(o, 0.2f, 0.15f), P(o, 0.8f, 0.85f), c, 0.f, 0, 1.3f);
    dl->AddCircleFilled(P(o, 0.35f, 0.3f), kIconSize * 0.06f, c);
    dl->AddCircleFilled(P(o, 0.65f, 0.3f), kIconSize * 0.06f, c);
    dl->AddCircleFilled(P(o, 0.35f, 0.7f), kIconSize * 0.06f, c);
    dl->AddCircleFilled(P(o, 0.65f, 0.7f), kIconSize * 0.06f, c);
}

void DrawPropertiesIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    // Sliders (property list).
    for (float y : {0.28f, 0.55f, 0.82f}) {
        dl->AddLine(P(o, 0.12f, y), P(o, 0.88f, y), c, 1.3f);
    }
    dl->AddCircleFilled(P(o, 0.35f, 0.28f), kIconSize * 0.09f, c);
    dl->AddCircleFilled(P(o, 0.62f, 0.55f), kIconSize * 0.09f, c);
    dl->AddCircleFilled(P(o, 0.42f, 0.82f), kIconSize * 0.09f, c);
}

void DrawJointsMembersIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    // Two joints connected by a member -- same visual language as
    // IconToolbar.cpp's Add-Joint/Connect icons.
    dl->AddLine(P(o, 0.2f, 0.8f), P(o, 0.8f, 0.2f), c, 1.5f);
    dl->AddCircleFilled(P(o, 0.2f, 0.8f), kIconSize * 0.12f, c);
    dl->AddCircleFilled(P(o, 0.8f, 0.2f), kIconSize * 0.12f, c);
}

void DrawLoadsIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    // A downward load arrow.
    dl->AddLine(P(o, 0.5f, 0.1f), P(o, 0.5f, 0.75f), c, 1.6f);
    dl->AddTriangleFilled(P(o, 0.28f, 0.6f), P(o, 0.72f, 0.6f), P(o, 0.5f, 0.9f), c);
}

void DrawRunIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddTriangleFilled(P(o, 0.25f, 0.15f), P(o, 0.25f, 0.85f), P(o, 0.85f, 0.5f), c);
}

void DrawLogIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    // Text lines.
    for (float y : {0.22f, 0.42f, 0.62f, 0.82f}) {
        dl->AddLine(P(o, 0.15f, y), P(o, 0.85f, y), c, 1.4f);
    }
}

} // namespace

void DrawPanelIconHeader(PanelIcon icon, const char* label) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);

    switch (icon) {
        case PanelIcon::Viewport: DrawViewportIcon(dl, pos, color); break;
        case PanelIcon::Detailing: DrawDetailingIcon(dl, pos, color); break;
        case PanelIcon::Properties: DrawPropertiesIcon(dl, pos, color); break;
        case PanelIcon::JointsMembers: DrawJointsMembersIcon(dl, pos, color); break;
        case PanelIcon::Loads: DrawLoadsIcon(dl, pos, color); break;
        case PanelIcon::Run: DrawRunIcon(dl, pos, color); break;
        case PanelIcon::Log: DrawLogIcon(dl, pos, color); break;
    }

    ImGui::Dummy(ImVec2(kIconSize, kIconSize));
    ImGui::SameLine(0.f, 6.f);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::Separator();
}

} // namespace orcisf::gui
