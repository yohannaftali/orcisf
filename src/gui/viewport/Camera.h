#pragma once

#include "gui/viewport/Math3D.h"

namespace orcisf::gui {

// Orbit camera (target + distance + yaw/pitch), matching the legacy
// structural-analysis convention where Y is the vertical/up axis (see
// orcisf_cli's equilibrium check: "arah 2 = Y"). Input handling itself
// (translating ImGui mouse/keyboard state into Orbit/Pan/Zoom calls) lives
// in ViewportPanel -- this class is pure camera math, easy to reason about
// independent of ImGui.
class Camera {
public:
    void Orbit(float dyaw_degrees, float dpitch_degrees);
    void Pan(float dx, float dy); // screen-space pixels; sign matches typical drag-to-pan feel
    void Zoom(float scroll_delta);
    void FrameBounds(const math3d::Vec3& center, float radius); // point the camera at a scene's bounding sphere

    math3d::Vec3 EyePosition() const;
    math3d::Vec3 Forward() const;
    math3d::Vec3 Right() const;
    static math3d::Vec3 WorldUp() { return {0.f, 1.f, 0.f}; }

    math3d::Mat4 ViewMatrix() const;
    math3d::Mat4 ProjectionMatrix(float aspect) const;

    // Builds a world-space ray for a click at normalized device coordinates
    // (ndc_x, ndc_y each in [-1, 1], ndc_y positive = up) -- used for
    // member picking (issue #5's "clicking a member" acceptance criterion).
    void ScreenRay(float ndc_x, float ndc_y, float aspect, math3d::Vec3& out_origin, math3d::Vec3& out_dir) const;

    math3d::Vec3 target{0.f, 0.f, 0.f};
    float distance = 20.f;
    float yaw_degrees = -60.f;
    float pitch_degrees = 25.f;
    float fov_degrees = 45.f;
    float near_plane = 0.05f;
    float far_plane = 500.f;
};

} // namespace orcisf::gui
