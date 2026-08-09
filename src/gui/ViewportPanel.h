#pragma once

#include <functional>

#include "gui/editor/EditableStructure.h"
#include "gui/editor/Selection.h"
#include "gui/editor/UndoStack.h"
#include "gui/viewport/Camera.h"
#include "gui/viewport/SceneModel.h"
#include "gui/viewport/SceneRenderer.h"

namespace orcisf::gui {

// Docked panel for the 3D structure/results view (issue #5) and, when an
// editable dataset is loaded, its interactive editor (issue #6). Renders
// `scene` (owned by Application) offscreen via SceneRenderer and displays
// it with ImGui::Image; handles orbit/pan/zoom camera input, joint/member
// picking (writes into `selection`, shared with PropertiesPanel), an
// ImGuizmo translate handle for the selected joint, and connect-mode
// clicking to add members.
//
// `editable`/`undo` are null when nothing is loaded (view-only or no
// dataset yet) -- picking/camera still work, but gizmo dragging and
// connect-mode are disabled. `on_geometry_changed` is called (from the UI
// thread, synchronously) after any edit so Application can rebuild `scene`
// and re-run validation; edits are applied directly to the StructureData
// `editable` wraps, so the caller must rebuild `scene` from the *same*
// StructureData before the next Draw() call for the view to reflect them.
class ViewportPanel {
public:
    void Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
              UndoStack* undo, EditorOptions& options, const std::function<void()>& on_geometry_changed);

    // Re-frames the camera on the scene's bounding sphere -- called once
    // when a new dataset is loaded so the structure starts fully in view.
    void FrameScene(const SceneModel& scene);

private:
    void HandlePicking(const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
                        EditorOptions& options, const std::function<void()>& on_geometry_changed, float ndc_x,
                        float ndc_y, float aspect);
    void DrawGizmo(const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
                   EditorOptions& options, const std::function<void()>& on_geometry_changed, float image_min_x,
                   float image_min_y, float image_w, float image_h);

    Camera camera_;
    SceneRenderer renderer_;

    // Left-drag orbit / right-drag pan state, tracked across frames so a
    // drag that starts over the image keeps controlling the camera even
    // if the mouse briefly leaves the image rect mid-drag.
    bool orbiting_ = false;
    bool panning_ = false;
    float drag_pixels_ = 0.f; // accumulated |delta| this press, to distinguish a click from a drag

    bool gizmo_was_using_ = false; // detects the "drag just started" edge, to push exactly one undo snapshot
};

} // namespace orcisf::gui
