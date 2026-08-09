#pragma once

#include <cmath>

// Minimal column-major (OpenGL convention) float3/float4x4 math -- just
// enough for an orbit camera and per-member model matrices. Not a
// general-purpose math library on purpose: adding a dependency like GLM
// for this handful of operations would be more machinery than the
// viewport actually needs.
namespace orcisf::gui::math3d {

struct Vec3 {
    float x = 0.f, y = 0.f, z = 0.f;

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }

    float Length() const { return std::sqrt(x * x + y * y + z * z); }

    Vec3 Normalized() const {
        float len = Length();
        return (len > 1e-8f) ? Vec3{x / len, y / len, z / len} : Vec3{0.f, 0.f, 0.f};
    }
};

inline float Dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 Cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// 16 floats, column-major: element [col*4 + row], matching what OpenGL's
// glUniformMatrix4fv(transpose=GL_FALSE) expects directly.
struct Mat4 {
    float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    static Mat4 Identity() { return Mat4{}; }

    static Mat4 Perspective(float fovy_radians, float aspect, float znear, float zfar) {
        Mat4 r;
        for (float& f : r.m) f = 0.f;
        float f = 1.f / std::tan(fovy_radians * 0.5f);
        r.m[0] = f / aspect;
        r.m[5] = f;
        r.m[10] = (zfar + znear) / (znear - zfar);
        r.m[11] = -1.f;
        r.m[14] = (2.f * zfar * znear) / (znear - zfar);
        return r;
    }

    static Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = (center - eye).Normalized();
        Vec3 s = Cross(f, up).Normalized();
        Vec3 u = Cross(s, f);

        Mat4 r = Identity();
        r.m[0] = s.x;
        r.m[4] = s.y;
        r.m[8] = s.z;
        r.m[1] = u.x;
        r.m[5] = u.y;
        r.m[9] = u.z;
        r.m[2] = -f.x;
        r.m[6] = -f.y;
        r.m[10] = -f.z;
        r.m[12] = -Dot(s, eye);
        r.m[13] = -Dot(u, eye);
        r.m[14] = Dot(f, eye);
        return r;
    }

    // Builds a model matrix placing a unit cube (centered on origin, [-0.5,0.5]
    // in each local axis) at `origin`, with its local X/Y/Z axes mapped to the
    // given (assumed orthonormal) world-space basis vectors, scaled per-axis
    // by `scale`. Used to orient a member's box mesh along its own length.
    static Mat4 FromBasis(const Vec3& origin, const Vec3& axis_x, const Vec3& axis_y, const Vec3& axis_z,
                           const Vec3& scale) {
        Mat4 r;
        r.m[0] = axis_x.x * scale.x;
        r.m[1] = axis_x.y * scale.x;
        r.m[2] = axis_x.z * scale.x;
        r.m[3] = 0.f;
        r.m[4] = axis_y.x * scale.y;
        r.m[5] = axis_y.y * scale.y;
        r.m[6] = axis_y.z * scale.y;
        r.m[7] = 0.f;
        r.m[8] = axis_z.x * scale.z;
        r.m[9] = axis_z.y * scale.z;
        r.m[10] = axis_z.z * scale.z;
        r.m[11] = 0.f;
        r.m[12] = origin.x;
        r.m[13] = origin.y;
        r.m[14] = origin.z;
        r.m[15] = 1.f;
        return r;
    }

    static Mat4 Multiply(const Mat4& a, const Mat4& b) {
        Mat4 r;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.f;
                for (int k = 0; k < 4; ++k) sum += a.m[k * 4 + row] * b.m[col * 4 + k];
                r.m[col * 4 + row] = sum;
            }
        }
        return r;
    }
};

} // namespace orcisf::gui::math3d
