#include "gui/viewport/SceneRenderer.h"

#include <GL/gl3w.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace orcisf::gui {

using namespace math3d;

namespace {

#if defined(__APPLE__)
constexpr const char* kShaderVersion = "#version 150\n";
#else
constexpr const char* kShaderVersion = "#version 130\n";
#endif

// Cube has axis-aligned face normals, and every model matrix used here is
// a pure rotation (orthonormal basis) composed with a positive diagonal
// scale -- for that specific combination, transforming a normal by the
// model matrix's upper-left 3x3 and re-normalizing gives the same
// direction as the correct inverse-transpose normal matrix would (the
// per-axis scale only rescales the single nonzero component of an
// axis-aligned normal, which normalization removes). See mat3(uModel) in
// the vertex shader below -- no separate normal matrix needed.
const char* kVertexShaderBody = R"(
in vec3 aPos;
in vec3 aNormal;
uniform mat4 uModel;
uniform mat4 uViewProj;
out vec3 vNormalWorld;
void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    gl_Position = uViewProj * worldPos;
    vNormalWorld = normalize(mat3(uModel) * aNormal);
}
)";

const char* kFragmentShaderBody = R"(
in vec3 vNormalWorld;
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    vec3 lightDir = normalize(vec3(0.4, 0.8, 0.5));
    float diff = max(dot(normalize(vNormalWorld), lightDir), 0.0);
    float ambient = 0.45;
    float lighting = ambient + (1.0 - ambient) * diff;
    FragColor = vec4(uColor.rgb * lighting, uColor.a);
}
)";

unsigned int CompileShader(unsigned int type, const char* body) {
    std::string src = std::string(kShaderVersion) + body;
    const char* src_ptr = src.c_str();
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src_ptr, nullptr);
    glCompileShader(shader);
    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::fprintf(stderr, "ViewportPanel shader compile error: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// 36 vertices (12 triangles), position + normal, unit cube spanning
// [-0.5, 0.5] on each local axis.
const float kCubeVertices[] = {
    // +X
    0.5f, -0.5f, -0.5f, 1, 0, 0, 0.5f, 0.5f, -0.5f, 1, 0, 0, 0.5f, 0.5f, 0.5f, 1, 0, 0, 0.5f, -0.5f, -0.5f, 1, 0, 0,
    0.5f, 0.5f, 0.5f, 1, 0, 0, 0.5f, -0.5f, 0.5f, 1, 0, 0,
    // -X
    -0.5f, -0.5f, 0.5f, -1, 0, 0, -0.5f, 0.5f, 0.5f, -1, 0, 0, -0.5f, 0.5f, -0.5f, -1, 0, 0, -0.5f, -0.5f, 0.5f, -1,
    0, 0, -0.5f, 0.5f, -0.5f, -1, 0, 0, -0.5f, -0.5f, -0.5f, -1, 0, 0,
    // +Y
    -0.5f, 0.5f, -0.5f, 0, 1, 0, -0.5f, 0.5f, 0.5f, 0, 1, 0, 0.5f, 0.5f, 0.5f, 0, 1, 0, -0.5f, 0.5f, -0.5f, 0, 1, 0,
    0.5f, 0.5f, 0.5f, 0, 1, 0, 0.5f, 0.5f, -0.5f, 0, 1, 0,
    // -Y
    -0.5f, -0.5f, 0.5f, 0, -1, 0, -0.5f, -0.5f, -0.5f, 0, -1, 0, 0.5f, -0.5f, -0.5f, 0, -1, 0, -0.5f, -0.5f, 0.5f, 0,
    -1, 0, 0.5f, -0.5f, -0.5f, 0, -1, 0, 0.5f, -0.5f, 0.5f, 0, -1, 0,
    // +Z
    0.5f, -0.5f, 0.5f, 0, 0, 1, 0.5f, 0.5f, 0.5f, 0, 0, 1, -0.5f, 0.5f, 0.5f, 0, 0, 1, 0.5f, -0.5f, 0.5f, 0, 0, 1,
    -0.5f, 0.5f, 0.5f, 0, 0, 1, -0.5f, -0.5f, 0.5f, 0, 0, 1,
    // -Z
    -0.5f, -0.5f, -0.5f, 0, 0, -1, -0.5f, 0.5f, -0.5f, 0, 0, -1, 0.5f, 0.5f, -0.5f, 0, 0, -1, -0.5f, -0.5f, -0.5f, 0,
    0, -1, 0.5f, 0.5f, -0.5f, 0, 0, -1, 0.5f, -0.5f, -0.5f, 0, 0, -1,
};

} // namespace

SceneRenderer::~SceneRenderer() {
    if (fbo_) glDeleteFramebuffers(1, &fbo_);
    if (color_tex_) glDeleteTextures(1, &color_tex_);
    if (depth_rbo_) glDeleteRenderbuffers(1, &depth_rbo_);
    if (cube_vbo_) glDeleteBuffers(1, &cube_vbo_);
    if (cube_vao_) glDeleteVertexArrays(1, &cube_vao_);
    if (shader_program_) glDeleteProgram(shader_program_);
}

void SceneRenderer::EnsureGLObjects() {
    if (gl_objects_ready_) return;
    gl_objects_ready_ = true;

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, kVertexShaderBody);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, kFragmentShaderBody);
    if (vs && fs) {
        shader_program_ = glCreateProgram();
        glAttachShader(shader_program_, vs);
        glAttachShader(shader_program_, fs);
        glBindAttribLocation(shader_program_, 0, "aPos");
        glBindAttribLocation(shader_program_, 1, "aNormal");
        glLinkProgram(shader_program_);
        int ok = 0;
        glGetProgramiv(shader_program_, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[1024];
            glGetProgramInfoLog(shader_program_, sizeof(log), nullptr, log);
            std::fprintf(stderr, "ViewportPanel shader link error: %s\n", log);
            glDeleteProgram(shader_program_);
            shader_program_ = 0;
        } else {
            u_model_ = glGetUniformLocation(shader_program_, "uModel");
            u_view_proj_ = glGetUniformLocation(shader_program_, "uViewProj");
            u_color_ = glGetUniformLocation(shader_program_, "uColor");
        }
    }
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);

    glGenVertexArrays(1, &cube_vao_);
    glGenBuffers(1, &cube_vbo_);
    glBindVertexArray(cube_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVertices), kCubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glBindVertexArray(0);
}

void SceneRenderer::EnsureFramebuffer(int width, int height) {
    width = width < 1 ? 1 : width;
    height = height < 1 ? 1 : height;
    if (fbo_ != 0 && width == fbo_w_ && height == fbo_h_) return;

    if (fbo_) glDeleteFramebuffers(1, &fbo_);
    if (color_tex_) glDeleteTextures(1, &color_tex_);
    if (depth_rbo_) glDeleteRenderbuffers(1, &depth_rbo_);

    fbo_w_ = width;
    fbo_h_ = height;

    glGenTextures(1, &color_tex_);
    glBindTexture(GL_TEXTURE_2D, color_tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenRenderbuffers(1, &depth_rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo_);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "ViewportPanel: framebuffer incomplete\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneRenderer::DrawCube(const Vec3& center, float half_size, const float color[4], const float* view_proj) {
    Mat4 model = Mat4::FromBasis(center, Vec3{1, 0, 0}, Vec3{0, 1, 0}, Vec3{0, 0, 1},
                                  Vec3{half_size * 2.f, half_size * 2.f, half_size * 2.f});
    glUniformMatrix4fv(u_model_, 1, GL_FALSE, model.m);
    glUniformMatrix4fv(u_view_proj_, 1, GL_FALSE, view_proj);
    glUniform4fv(u_color_, 1, color);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void SceneRenderer::DrawBox(const Vec3& a, const Vec3& b, float width_m, float height_m, const float color[4],
                             const float* view_proj) {
    Vec3 delta = b - a;
    float length = delta.Length();
    if (length < 1e-5f) return;
    Vec3 axis_x = delta * (1.f / length);

    Vec3 reference = (std::fabs(Dot(axis_x, Vec3{0, 1, 0})) > 0.99f) ? Vec3{0, 0, 1} : Vec3{0, 1, 0};
    Vec3 axis_z = Cross(axis_x, reference).Normalized();
    Vec3 axis_y = Cross(axis_z, axis_x).Normalized();

    Vec3 mid = (a + b) * 0.5f;
    Mat4 model = Mat4::FromBasis(mid, axis_x, axis_y, axis_z, Vec3{length, height_m, width_m});
    glUniformMatrix4fv(u_model_, 1, GL_FALSE, model.m);
    glUniformMatrix4fv(u_view_proj_, 1, GL_FALSE, view_proj);
    glUniform4fv(u_color_, 1, color);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

unsigned int SceneRenderer::Render(const SceneModel& scene, const Camera& camera, int width, int height,
                                    int selected_member) {
    EnsureGLObjects();
    EnsureFramebuffer(width, height);

    GLint previous_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, fbo_w_, fbo_h_);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.14f, 0.15f, 0.17f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (shader_program_ == 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<unsigned int>(previous_fbo));
        return color_tex_;
    }

    glUseProgram(shader_program_);
    glBindVertexArray(cube_vao_);

    float aspect = static_cast<float>(fbo_w_) / static_cast<float>(fbo_h_);
    Mat4 view_proj = Mat4::Multiply(camera.ProjectionMatrix(aspect), camera.ViewMatrix());

    static constexpr float kBeamColor[4] = {0.62f, 0.62f, 0.68f, 1.f};
    static constexpr float kColumnColor[4] = {0.52f, 0.56f, 0.68f, 1.f};
    static constexpr float kSatisfiedColor[4] = {0.30f, 0.75f, 0.38f, 1.f};
    static constexpr float kViolatedColor[4] = {0.85f, 0.28f, 0.24f, 1.f};
    static constexpr float kHighlightColor[4] = {1.f, 0.82f, 0.15f, 1.f};
    static constexpr float kJointColor[4] = {0.75f, 0.78f, 0.82f, 1.f};
    static constexpr float kRestraintColor[4] = {0.95f, 0.55f, 0.15f, 1.f};

    for (const MemberVisual& mv : scene.members) {
        const float* color = mv.is_beam ? kBeamColor : kColumnColor;
        if (mv.has_results) {
            color = (mv.kendala <= 0.f) ? kSatisfiedColor : kViolatedColor;
        }
        if (mv.no_batang == selected_member) {
            color = kHighlightColor;
        }
        DrawBox(mv.a, mv.b, mv.width_m, mv.height_m, color, view_proj.m);
    }

    for (const JointVisual& jv : scene.joints) {
        float size = jv.restrained ? 0.12f : 0.08f;
        DrawCube(jv.pos, size, jv.restrained ? kRestraintColor : kJointColor, view_proj.m);
    }

    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<unsigned int>(previous_fbo));

    return color_tex_;
}

} // namespace orcisf::gui
