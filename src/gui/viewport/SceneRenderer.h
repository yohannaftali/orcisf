#pragma once

#include "gui/viewport/Camera.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::gui {

// Offscreen OpenGL renderer for the 3D viewport: draws a SceneModel from a
// Camera's point of view into a color texture (via an FBO), which
// ViewportPanel then displays with ImGui::Image. Owns all its GL objects
// (framebuffer, shader program, unit-cube mesh) and lazily creates them on
// first use so constructing a SceneRenderer before a GL context exists is
// safe.
class SceneRenderer {
public:
    SceneRenderer() = default;
    ~SceneRenderer();
    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;

    // Renders `scene` from `camera` into an internal width x height
    // framebuffer (recreated on resize) and returns its color texture's
    // OpenGL id, ready for ImGui::Image. `selected_member` (no_batang, or
    // -1) is drawn with a highlight outline color. Issue #60:
    // `show_deformed_shape` (only meaningful when `scene.has_deformation`
    // -- caller gates this, see ViewportPanel::Draw) overlays a
    // `deformation_scale`-exaggerated deformed shape alongside the
    // (always-drawn) undeformed structure. Issue #71: `show_force_diagram`
    // overlays a `force_diagram_scale`-exaggerated N/V/M/T ribbon
    // (`force_diagram_component`: 0=N,1=V,2=M,3=T) for `selected_member`
    // alone, or every member with force_diagram data when
    // `force_diagram_all_members` is true.
    unsigned int Render(const SceneModel& scene, const Camera& camera, int width, int height, int selected_member,
                         bool show_deformed_shape, float deformation_scale, bool show_force_diagram,
                         int force_diagram_component, float force_diagram_scale, bool force_diagram_all_members);

private:
    void EnsureGLObjects();
    void EnsureFramebuffer(int width, int height);
    void DrawBox(const math3d::Vec3& a, const math3d::Vec3& b, float width_m, float height_m,
                 const float color[4], const float* view_proj);
    void DrawCube(const math3d::Vec3& center, float half_size, const float color[4], const float* view_proj);
    // Thin shaft (DrawBox) + a small cube arrowhead at `tip`, from `tail`
    // to `tip` -- issue #7's load glyphs.
    void DrawArrow(const math3d::Vec3& tail, const math3d::Vec3& tip, const float color[4], const float* view_proj);
    void DrawLoads(const SceneModel& scene, const float* view_proj);
    // Issue #50: ground-plane reference grid (X-Z plane, dark cobalt),
    // drawn as thin DrawBox lines -- same "represent a line as a thin box"
    // idiom DrawArrow's shaft already uses, so this needs no new shader or
    // GL_LINES pipeline. Layout (spacing/extent/labeled lines) comes from
    // ComputeGroundGridLayout() (SceneModel.h) so the X{i}/Z{i} text labels
    // ViewportPanel draws separately agree exactly with where these lines are.
    void DrawGrid(const SceneModel& scene, const float* view_proj);
    // Issue #60: deformed-shape overlay -- thin lines (DrawBox, same
    // "line = thin box" idiom as DrawGrid) between each member's
    // scale-exaggerated displaced endpoints, plus a small cube marker at
    // each displaced joint. Drawn on top of (not replacing) the normal
    // undeformed structure already drawn in Render(), in a color used
    // nowhere else in this renderer so it always reads as overlay data.
    void DrawDeformedShape(const SceneModel& scene, float scale, const float* view_proj);
    // Issue #71: N/V/M/T ribbon overlay -- an offset polyline (DrawBox
    // segments, same idiom) tracing `component`'s value along each
    // qualifying member's span, offset perpendicular to the member's own
    // local axis (the same axis_y DrawBox() derives internally) by
    // `value * scale`, plus a short stem at each end connecting the
    // ribbon back to the member's baseline so it reads as a diagram
    // anchored to the structure rather than a floating line. `component`:
    // 0=N,1=V,2=M,3=T, matching ForceDiagramSample's field order.
    void DrawForceDiagramOverlay(const SceneModel& scene, int selected_member, bool all_members, int component,
                                  float scale, const float* view_proj);

    bool gl_objects_ready_ = false;
    unsigned int shader_program_ = 0;
    int u_model_ = -1, u_view_proj_ = -1, u_color_ = -1;
    unsigned int cube_vao_ = 0, cube_vbo_ = 0;

    unsigned int fbo_ = 0, color_tex_ = 0, depth_rbo_ = 0;
    int fbo_w_ = 0, fbo_h_ = 0;
};

} // namespace orcisf::gui
