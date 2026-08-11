#include "gui/viewport/Camera.h"

#include <algorithm>
#include <cmath>

namespace orcisf::gui {

using namespace math3d;

namespace {
constexpr float kPi = 3.14159265358979323846f;
float ToRadians(float deg) { return deg * kPi / 180.f; }
} // namespace

void Camera::Orbit(float dyaw_degrees, float dpitch_degrees) {
    yaw_degrees += dyaw_degrees;
    pitch_degrees = std::clamp(pitch_degrees + dpitch_degrees, -89.f, 89.f);
}

void Camera::SetViewPlane(ViewPlane plane, float axis_offset) {
    view_plane = plane;
    switch (plane) {
        case ViewPlane::XY: // locks Z: looking down -Z at the X-Y plane
            ortho_forward_ = {0.f, 0.f, -1.f};
            ortho_right_ = {1.f, 0.f, 0.f};
            target.z = axis_offset;
            break;
        case ViewPlane::XZ: // locks Y: top-down view of the X-Z plane
            ortho_forward_ = {0.f, -1.f, 0.f};
            ortho_right_ = {1.f, 0.f, 0.f};
            target.y = axis_offset;
            break;
        case ViewPlane::YZ: // locks X: side/elevation view of the Y-Z plane
            // right = (0,0,-1), not (0,0,1): Cross(right,forward) (the up
            // vector every other Camera method derives, see ViewMatrix()/
            // ScreenRay()) works out to world -Y with (0,0,1) -- Y is this
            // project's vertical axis (see the class comment), so that
            // would render this elevation view upside-down. Caught by
            // hand-deriving the cross product, not by running it.
            ortho_forward_ = {-1.f, 0.f, 0.f};
            ortho_right_ = {0.f, 0.f, -1.f};
            target.x = axis_offset;
            break;
        case ViewPlane::Free:
        default:
            break; // yaw/pitch already hold the last orbit orientation
    }
}

math3d::Vec3 Camera::Forward() const {
    if (view_plane != ViewPlane::Free) return ortho_forward_;
    float yaw = ToRadians(yaw_degrees);
    float pitch = ToRadians(pitch_degrees);
    return Vec3{std::cos(pitch) * std::cos(yaw), std::sin(pitch), std::cos(pitch) * std::sin(yaw)}.Normalized();
}

math3d::Vec3 Camera::Right() const {
    if (view_plane != ViewPlane::Free) return ortho_right_;
    return Cross(Forward(), WorldUp()).Normalized();
}

math3d::Vec3 Camera::EyePosition() const { return target - Forward() * distance; }

void Camera::Pan(float dx, float dy) {
    Vec3 right = Right();
    Vec3 up = Cross(right, Forward()).Normalized();
    float scale = distance * 0.0015f; // pan speed scales with zoom level
    target = target + right * (-dx * scale) + up * (dy * scale);
}

void Camera::Zoom(float scroll_delta) {
    distance *= std::pow(0.9f, scroll_delta);
    distance = std::clamp(distance, 0.5f, 2000.f);
}

void Camera::FrameBounds(const math3d::Vec3& center, float radius) {
    target = center;
    distance = std::max(radius * 2.2f, 1.f);
}

math3d::Mat4 Camera::ViewMatrix() const {
    // LookAt derives its own orthonormal basis from (forward, up_hint) via
    // s=Cross(f,up), u=Cross(s,f) -- WorldUp() as the hint would be
    // parallel to Forward() in the XZ (top-down) plane-locked case and
    // degenerate. Cross(Right(), Forward()) is always a valid hint since
    // Right()/Forward() are already a valid orthonormal pair in every mode
    // (see Right()'s ortho_right_ branch), matching the same up vector
    // Pan()/ScreenRay() already derive the same way.
    Vec3 up = (view_plane == ViewPlane::Free) ? WorldUp() : Cross(Right(), Forward()).Normalized();
    return Mat4::LookAt(EyePosition(), target, up);
}

math3d::Mat4 Camera::ProjectionMatrix(float aspect) const {
    if (view_plane != ViewPlane::Free) {
        // Reuses `distance` (the same field Zoom() already scales via
        // scroll) as the ortho half-height, so zooming in/out feels
        // consistent switching between perspective and a locked plane.
        float half_height = distance * 0.5f;
        float half_width = half_height * aspect;
        return Mat4::Orthographic(-half_width, half_width, -half_height, half_height, near_plane, far_plane);
    }
    return Mat4::Perspective(ToRadians(fov_degrees), aspect, near_plane, far_plane);
}

void Camera::ScreenRay(float ndc_x, float ndc_y, float aspect, math3d::Vec3& out_origin,
                        math3d::Vec3& out_dir) const {
    Vec3 forward = Forward();
    Vec3 right = Right();
    Vec3 up = Cross(right, forward).Normalized();

    if (view_plane != ViewPlane::Free) {
        // Orthographic: rays are parallel (all pointing along `forward`),
        // fanned out across the view plane instead of converging at the
        // eye -- matches ProjectionMatrix()'s ortho frustum above.
        float half_height = distance * 0.5f;
        float half_width = half_height * aspect;
        out_origin = EyePosition() + right * (ndc_x * half_width) + up * (ndc_y * half_height);
        out_dir = forward;
        return;
    }

    float half_height = std::tan(ToRadians(fov_degrees) * 0.5f);
    float half_width = half_height * aspect;

    out_origin = EyePosition();
    out_dir = (forward + right * (ndc_x * half_width) + up * (ndc_y * half_height)).Normalized();
}

} // namespace orcisf::gui
