// ORCISF GUI entry point: GLFW + OpenGL3 + Dear ImGui (docking) bootstrap.
// Cross-platform (Windows 11 / macOS / Linux) per issue #2. The actual
// engine/viewport/editor/export features are separate issues (#3-#9);
// this file only wires up the window, render loop, and docking layout.

#include "app/Application.h"
#include "app/Theme.h"

// gl3w must be included before any other header that pulls in GL/gl.h
// (GLFW included with GLFW_INCLUDE_NONE below, so this is the only GL
// header in the whole program) -- it provides every core-profile GL
// function ViewportPanel's renderer needs (framebuffers, VAOs/VBOs,
// shaders) as loaded function pointers, avoiding a hand-rolled loader.
#include <GL/gl3w.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <nfd.h>

#include <cstdarg>
#include <cstdio>

namespace {

// orcisf_gui has no console (see issue #12: WIN32 subsystem on Windows),
// so a plain fprintf(stderr, ...) is invisible there -- also append fatal
// startup errors to a log file next to the executable so they're not lost.
void LogStartupError(const char* format, ...) {
    std::va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);

    if (std::FILE* f = std::fopen("orcisf_gui_startup.log", "a")) {
        va_start(args, format);
        std::vfprintf(f, format, args);
        va_end(args);
        std::fclose(f);
    }
}

void GlfwErrorCallback(int error, const char* description) {
    LogStartupError("GLFW error %d: %s\n", error, description);
}

} // namespace

int main(int, char**) {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) {
        return 1;
    }

#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Issue #19 (Phase 0): borderless window, platform-agnostic via GLFW's
    // own cross-platform hint -- no native handles needed. The custom
    // title bar (drawn inside Application's menu bar every frame, see
    // app::CustomTitleBar) replaces the removed OS chrome; its header
    // documents the Wayland-drag limitation this implies.
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(
        1600, 900, "ORCISF - Optimasi Beton Bertulang Pada Struktur Portal Ruang", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    if (gl3wInit() != 0) {
        LogStartupError("Failed to initialize gl3w (OpenGL function loader)\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    orcisf::app::ApplyModernTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    NFD_Init();

    orcisf::app::Application app;
    app.SetWindow(window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        app.OnFrame();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.10f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    NFD_Quit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
