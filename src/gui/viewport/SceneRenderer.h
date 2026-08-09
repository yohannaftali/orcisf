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
    // -1) is drawn with a highlight outline color.
    unsigned int Render(const SceneModel& scene, const Camera& camera, int width, int height, int selected_member);

private:
    void EnsureGLObjects();
    void EnsureFramebuffer(int width, int height);
    void DrawBox(const math3d::Vec3& a, const math3d::Vec3& b, float width_m, float height_m,
                 const float color[4], const float* view_proj);
    void DrawCube(const math3d::Vec3& center, float half_size, const float color[4], const float* view_proj);

    bool gl_objects_ready_ = false;
    unsigned int shader_program_ = 0;
    int u_model_ = -1, u_view_proj_ = -1, u_color_ = -1;
    unsigned int cube_vao_ = 0, cube_vbo_ = 0;

    unsigned int fbo_ = 0, color_tex_ = 0, depth_rbo_ = 0;
    int fbo_w_ = 0, fbo_h_ = 0;
};

} // namespace orcisf::gui
