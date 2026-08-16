#include "gui/PanelIcons.h"

#include <initializer_list>

namespace orcisf::gui {

namespace {

// Local point (x,y in [0,1] of the icon box) -> screen coordinates, same
// helper pattern as IconToolbar.cpp's P() -- parameterized on `size` (issue
// #35) instead of a file-local constant, since these icons are now drawn at
// whatever size the caller's context needs (tab-bar-sized, not a fixed 16px).
ImVec2 P(const ImVec2& origin, float x, float y, float size) { return ImVec2(origin.x + x * size, origin.y + y * size); }

void DrawViewportIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    // A simple cube (3D viewport).
    dl->AddQuad(P(o, 0.15f, 0.35f, size), P(o, 0.55f, 0.2f, size), P(o, 0.95f, 0.35f, size), P(o, 0.55f, 0.5f, size), c, 1.1f);
    dl->AddLine(P(o, 0.15f, 0.35f, size), P(o, 0.15f, 0.75f, size), c, 1.1f);
    dl->AddLine(P(o, 0.95f, 0.35f, size), P(o, 0.95f, 0.75f, size), c, 1.1f);
    dl->AddLine(P(o, 0.55f, 0.5f, size), P(o, 0.55f, 0.9f, size), c, 1.1f);
    dl->AddLine(P(o, 0.15f, 0.75f, size), P(o, 0.55f, 0.9f, size), c, 1.1f);
    dl->AddLine(P(o, 0.95f, 0.75f, size), P(o, 0.55f, 0.9f, size), c, 1.1f);
}

void DrawDetailingIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    // A cross-section outline with a rebar dot -- matches the reinforcement
    // detailing drawings this panel shows.
    dl->AddRect(P(o, 0.2f, 0.15f, size), P(o, 0.8f, 0.85f, size), c, 0.f, 0, 1.1f);
    dl->AddCircleFilled(P(o, 0.35f, 0.3f, size), size * 0.06f, c);
    dl->AddCircleFilled(P(o, 0.65f, 0.3f, size), size * 0.06f, c);
    dl->AddCircleFilled(P(o, 0.35f, 0.7f, size), size * 0.06f, c);
    dl->AddCircleFilled(P(o, 0.65f, 0.7f, size), size * 0.06f, c);
}

void DrawPropertiesIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    // Sliders (property list).
    for (float y : {0.28f, 0.55f, 0.82f}) {
        dl->AddLine(P(o, 0.12f, y, size), P(o, 0.88f, y, size), c, 1.1f);
    }
    dl->AddCircleFilled(P(o, 0.35f, 0.28f, size), size * 0.09f, c);
    dl->AddCircleFilled(P(o, 0.62f, 0.55f, size), size * 0.09f, c);
    dl->AddCircleFilled(P(o, 0.42f, 0.82f, size), size * 0.09f, c);
}

void DrawJointsIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    // Three standalone joint dots -- issue #36 split the combined
    // Joints/Members glyph (issue #21) into two panels; this is the
    // joints-only subset of it.
    dl->AddCircleFilled(P(o, 0.25f, 0.75f, size), size * 0.14f, c);
    dl->AddCircleFilled(P(o, 0.75f, 0.75f, size), size * 0.14f, c);
    dl->AddCircleFilled(P(o, 0.5f, 0.25f, size), size * 0.14f, c);
}

void DrawMembersIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    // A single member (line) between two small unfilled joint end-caps --
    // the members-only subset of the original combined glyph (issue #36),
    // deliberately unfilled circles (vs. DrawJointsIcon's filled dots) so
    // the two icons read as distinct at a glance.
    dl->AddLine(P(o, 0.18f, 0.82f, size), P(o, 0.82f, 0.18f, size), c, 1.3f);
    dl->AddCircle(P(o, 0.18f, 0.82f, size), size * 0.11f, c, 0, 1.1f);
    dl->AddCircle(P(o, 0.82f, 0.18f, size), size * 0.11f, c, 0, 1.1f);
}

void DrawLoadsIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    // A downward load arrow.
    dl->AddLine(P(o, 0.5f, 0.1f, size), P(o, 0.5f, 0.75f, size), c, 1.3f);
    dl->AddTriangleFilled(P(o, 0.28f, 0.6f, size), P(o, 0.72f, 0.6f, size), P(o, 0.5f, 0.9f, size), c);
}

void DrawRunIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    dl->AddTriangleFilled(P(o, 0.25f, 0.15f, size), P(o, 0.25f, 0.85f, size), P(o, 0.85f, 0.5f, size), c);
}

void DrawLogIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    // Text lines.
    for (float y : {0.22f, 0.42f, 0.62f, 0.82f}) {
        dl->AddLine(P(o, 0.15f, y, size), P(o, 0.85f, y, size), c, 1.1f);
    }
}

void DrawResultsIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    // Issue #62: a small spreadsheet/table grid -- distinct from
    // DrawLogIcon's plain text lines (no vertical dividers there) and
    // DrawPropertiesIcon's sliders (no dots here).
    dl->AddRect(P(o, 0.15f, 0.18f, size), P(o, 0.85f, 0.82f, size), c, 0.f, 0, 1.1f);
    dl->AddLine(P(o, 0.15f, 0.42f, size), P(o, 0.85f, 0.42f, size), c, 1.f);
    dl->AddLine(P(o, 0.15f, 0.62f, size), P(o, 0.85f, 0.62f, size), c, 1.f);
    dl->AddLine(P(o, 0.5f, 0.18f, size), P(o, 0.5f, 0.82f, size), c, 1.f);
}

void DrawForceDiagramIcon(ImDrawList* dl, ImVec2 o, ImU32 c, float size) {
    // Issue #61: a schematic bending-moment diagram -- baseline + a
    // triangular bump above/below it, distinct at a glance from
    // DrawPropertiesIcon's flat horizontal sliders.
    dl->AddLine(P(o, 0.1f, 0.5f, size), P(o, 0.9f, 0.5f, size), c, 1.f);
    ImVec2 pts[4] = {P(o, 0.1f, 0.5f, size), P(o, 0.35f, 0.15f, size), P(o, 0.65f, 0.82f, size),
                      P(o, 0.9f, 0.5f, size)};
    dl->AddPolyline(pts, 4, c, 0, 1.5f);
}

} // namespace

// Issue #54: lid + handle + can body with two rib lines -- a standard
// trash-can silhouette, kept to the same normalized-[0,1]-box + P()
// convention as every icon above, but not routed through DrawPanelIcon()
// since it isn't a panel icon.
void DrawTrashIcon(ImDrawList* dl, ImVec2 origin, ImU32 color, float size) {
    dl->AddLine(P(origin, 0.4f, 0.1f, size), P(origin, 0.4f, 0.2f, size), color, 1.1f);
    dl->AddLine(P(origin, 0.6f, 0.1f, size), P(origin, 0.6f, 0.2f, size), color, 1.1f);
    dl->AddLine(P(origin, 0.4f, 0.1f, size), P(origin, 0.6f, 0.1f, size), color, 1.1f);
    dl->AddLine(P(origin, 0.15f, 0.22f, size), P(origin, 0.85f, 0.22f, size), color, 1.3f);
    dl->AddRect(P(origin, 0.25f, 0.25f, size), P(origin, 0.75f, 0.88f, size), color, 0.f, 0, 1.2f);
    dl->AddLine(P(origin, 0.4f, 0.35f, size), P(origin, 0.4f, 0.78f, size), color, 1.f);
    dl->AddLine(P(origin, 0.6f, 0.35f, size), P(origin, 0.6f, 0.78f, size), color, 1.f);
}

void DrawPanelIcon(PanelIcon icon, ImDrawList* dl, ImVec2 origin, ImU32 color, float size) {
    switch (icon) {
        case PanelIcon::Viewport: DrawViewportIcon(dl, origin, color, size); break;
        case PanelIcon::Detailing: DrawDetailingIcon(dl, origin, color, size); break;
        case PanelIcon::Properties: DrawPropertiesIcon(dl, origin, color, size); break;
        case PanelIcon::Joints: DrawJointsIcon(dl, origin, color, size); break;
        case PanelIcon::Members: DrawMembersIcon(dl, origin, color, size); break;
        case PanelIcon::Loads: DrawLoadsIcon(dl, origin, color, size); break;
        case PanelIcon::Run: DrawRunIcon(dl, origin, color, size); break;
        case PanelIcon::Log: DrawLogIcon(dl, origin, color, size); break;
        case PanelIcon::ForceDiagram: DrawForceDiagramIcon(dl, origin, color, size); break;
        case PanelIcon::Results: DrawResultsIcon(dl, origin, color, size); break;
    }
}

} // namespace orcisf::gui
