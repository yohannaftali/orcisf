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

math3d::Vec3 Camera::Forward() const {
    float yaw = ToRadians(yaw_degrees);
    float pitch = ToRadians(pitch_degrees);
    return Vec3{std::cos(pitch) * std::cos(yaw), std::sin(pitch), std::cos(pitch) * std::sin(yaw)}.Normalized();
}

math3d::Vec3 Camera::Right() const { return Cross(Forward(), WorldUp()).Normalized(); }

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

math3d::Mat4 Camera::ViewMatrix() const { return Mat4::LookAt(EyePosition(), target, WorldUp()); }

math3d::Mat4 Camera::ProjectionMatrix(float aspect) const {
    return Mat4::Perspective(ToRadians(fov_degrees), aspect, near_plane, far_plane);
}

void Camera::ScreenRay(float ndc_x, float ndc_y, float aspect, math3d::Vec3& out_origin,
                        math3d::Vec3& out_dir) const {
    Vec3 forward = Forward();
    Vec3 right = Right();
    Vec3 up = Cross(right, forward).Normalized();

    float half_height = std::tan(ToRadians(fov_degrees) * 0.5f);
    float half_width = half_height * aspect;

    out_origin = EyePosition();
    out_dir = (forward + right * (ndc_x * half_width) + up * (ndc_y * half_height)).Normalized();
}

} // namespace orcisf::gui
