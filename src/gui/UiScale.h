#pragma once

namespace orcisf::gui {

// Issue #31: the DPI content scale of the monitor the main window is
// currently on (1.0 = 96 DPI/100%, 1.5 = 150%, 2.0 = 200%, ...), published
// by app/main.cpp once at startup and again every time the window is
// dragged onto a monitor with a different scale.
//
// Dear ImGui scales its own font (style.FontScaleDpi) and style metrics
// (ImGuiStyle::ScaleAllSizes) from this same number, but every piece of
// hand-drawn chrome in this port -- IconToolbar's icon glyphs,
// CustomTitleBar's window buttons, PanelIcons' panel headers,
// ViewportPanel's UCS icon and plane-offset overlay -- sizes itself from
// raw pixel constants ImGui knows nothing about. Those must be written as
// Scaled(constant), or they stay physically tiny (and, where they feed a
// layout calculation, misaligned) on a high-DPI monitor while the text
// around them grows correctly.
//
// Lives in gui/ rather than app/ so app/ -- which already depends on gui/,
// not the other way round -- can read it without inverting this port's
// layering.
float UiScale();

// Called from app/main.cpp only, outside of an ImGui frame.
void SetUiScale(float scale);

inline float Scaled(float value) { return value * UiScale(); }

} // namespace orcisf::gui
