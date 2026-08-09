#pragma once

namespace orcisf::gui {

// Docked panel for the 3D structure/results view. Placeholder for this
// scaffold (issue #2) -- actual OpenGL scene rendering, camera controls,
// and dataset loading land in issue #5.
class ViewportPanel {
public:
    void Draw(bool* open);
};

} // namespace orcisf::gui
