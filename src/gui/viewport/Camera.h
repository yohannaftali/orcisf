#pragma once

#include "gui/editor/Selection.h" // ViewPlane (issue #22)
#include "gui/viewport/Math3D.h"

namespace orcisf::gui {

// Orbit camera (target + distance + yaw/pitch), matching the legacy
// structural-analysis convention where Y is the vertical/up axis (see
// orcisf_cli's equilibrium check: "arah 2 = Y"). Input handling itself
// (translating ImGui mouse/keyboard state into Orbit/Pan/Zoom calls) lives
// in ViewportPanel -- this class is pure camera math, easy to reason about
// independent of ImGui.
//
// Issue #22 added an orthographic plane-locked mode (`view_plane`). Orbit
// is meaningless there (SetViewPlane fixes the view direction along the
// locked axis) -- ViewportPanel is responsible for not calling Orbit()
// while view_plane != Free, this class doesn't guard against it itself.
class Camera {
public:
    void Orbit(float dyaw_degrees, float dpitch_degrees);
    void Pan(float dx, float dy); // screen-space pixels; sign matches typical drag-to-pan feel
    void Zoom(float scroll_delta);
    void FrameBounds(const math3d::Vec3& center, float radius); // point the camera at a scene's bounding sphere

    // Issue #22: switches to (or out of, via ViewPlane::Free) a 2D
    // plane-locked orthographic view. `axis_offset` is only used for
    // plane != Free -- it becomes the locked coordinate of `target`
    // (e.g. XY at axis_offset=10 sets target.z=10), so the plane the
    // camera looks at actually sits at that offset. Yaw/pitch are left
    // untouched so returning to Free resumes the orbit orientation the
    // user had before switching.
    void SetViewPlane(ViewPlane plane, float axis_offset);
    bool IsOrthographic() const { return view_plane != ViewPlane::Free; }

    math3d::Vec3 EyePosition() const;
    math3d::Vec3 Forward() const;
    math3d::Vec3 Right() const;
    static math3d::Vec3 WorldUp() { return {0.f, 1.f, 0.f}; }

    math3d::Mat4 ViewMatrix() const;
    math3d::Mat4 ProjectionMatrix(float aspect) const;

    // Builds a world-space ray for a click at normalized device coordinates
    // (ndc_x, ndc_y each in [-1, 1], ndc_y positive = up) -- used for
    // member picking (issue #5's "clicking a member" acceptance criterion).
    // In plane-locked mode (issue #22) this returns parallel rays (all
    // sharing Forward() as direction), matching an orthographic
    // projection -- callers that want the exact plane-locked click point
    // can intersect at t = distance, see ViewportPanel::HandlePicking().
    void ScreenRay(float ndc_x, float ndc_y, float aspect, math3d::Vec3& out_origin, math3d::Vec3& out_dir) const;

    math3d::Vec3 target{0.f, 0.f, 0.f};
    float distance = 20.f;
    float yaw_degrees = -60.f;
    float pitch_degrees = 25.f;
    float fov_degrees = 45.f;
    float near_plane = 0.05f;
    float far_plane = 500.f;

    ViewPlane view_plane = ViewPlane::Free;

private:
    // Fixed orthonormal forward/right basis for the active locked plane
    // (issue #22) -- bypasses yaw/pitch entirely to avoid the gimbal-lock
    // case Forward()/Right()'s usual Cross(Forward(), WorldUp()) hits when
    // looking straight down world Y (the XZ/top-down plane): Forward and
    // WorldUp become parallel there, so Cross() would degenerate to zero.
    math3d::Vec3 ortho_forward_{0.f, 0.f, -1.f};
    math3d::Vec3 ortho_right_{1.f, 0.f, 0.f};
};

} // namespace orcisf::gui
