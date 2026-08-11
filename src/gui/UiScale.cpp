#include "gui/UiScale.h"

namespace orcisf::gui {

namespace {
// Read from the UI thread only (every caller is inside the render loop or
// main()'s startup), so a plain global needs no synchronization.
float g_ui_scale = 1.f;
} // namespace

float UiScale() { return g_ui_scale; }

void SetUiScale(float scale) { g_ui_scale = (scale > 0.f) ? scale : 1.f; }

} // namespace orcisf::gui
