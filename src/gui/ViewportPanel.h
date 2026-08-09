#pragma once

#include "gui/viewport/Camera.h"
#include "gui/viewport/SceneModel.h"
#include "gui/viewport/SceneRenderer.h"

namespace orcisf::gui {

// Docked panel for the 3D structure/results view (issue #5). Renders
// `scene` (owned by Application, built from a loaded dataset and/or the
// latest optimization run) offscreen via SceneRenderer and displays it
// with ImGui::Image; handles orbit/pan/zoom camera input and member
// picking (writes into `selected_member`, shared with PropertiesPanel so
// it can show the clicked member's data).
class ViewportPanel {
public:
    void Draw(bool* open, const SceneModel& scene, int& selected_member);

    // Re-frames the camera on the scene's bounding sphere -- called once
    // when a new dataset is loaded so the structure starts fully in view.
    void FrameScene(const SceneModel& scene);

private:
    Camera camera_;
    SceneRenderer renderer_;

    // Left-drag orbit / right-drag pan state, tracked across frames so a
    // drag that starts over the image keeps controlling the camera even
    // if the mouse briefly leaves the image rect mid-drag.
    bool orbiting_ = false;
    bool panning_ = false;
    float drag_pixels_ = 0.f; // accumulated |delta| this press, to distinguish a click from a drag
};

} // namespace orcisf::gui
