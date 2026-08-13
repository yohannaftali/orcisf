#pragma once

#include <imgui.h>

// Issue #35 (correcting #28 Part 2): each docked panel's small hand-drawn
// icon renders directly on that panel's own dock tab button, immediately
// before its title text -- see app/DockTabIcons.{h,cpp} for where it's
// actually drawn. #28 Part 2 originally drew the icon as a content-row
// header inside each panel's body instead (DrawPanelIconHeader(), now
// removed); that duplicated the title already shown on the tab and wasted
// a line of vertical space. This header now only exposes the low-level
// per-icon draw primitive (DrawPanelIcon()) so both DockTabIcons.cpp and
// (should a future issue want it) any other caller can render one of these
// icons at an arbitrary position/size, matching this project's established
// "small, dependency-free ImDrawList icons, no icon font" approach (see
// IconToolbar.cpp / ViewportPanel.cpp's UCS icon).
namespace orcisf::gui {

enum class PanelIcon { Viewport, Detailing, Properties, Joints, Members, Loads, Run, Log };

// Draws `icon` into `dl`, `size`x`size` pixels, top-left corner at `origin`.
// `size` is the caller's responsibility to DPI-scale (via gui::Scaled()) --
// this function does no scaling of its own, so it can be reused at any
// resolution the caller needs (e.g. DockTabIcons.cpp sizes it off the tab
// bar's own font metrics, not a fixed constant).
void DrawPanelIcon(PanelIcon icon, ImDrawList* dl, ImVec2 origin, ImU32 color, float size);

// Issue #54: a small trash-can glyph for destructive Delete/Clear table
// actions, standalone (not part of the PanelIcon enum/dispatch above --
// it isn't a panel) but sharing this file's icon-drawing home and the
// same normalized-[0,1]-box convention as every DrawXxxIcon() above.
void DrawTrashIcon(ImDrawList* dl, ImVec2 origin, ImU32 color, float size);

} // namespace orcisf::gui
