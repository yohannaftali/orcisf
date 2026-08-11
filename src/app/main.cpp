// ORCISF GUI entry point: GLFW + OpenGL3 + Dear ImGui (docking) bootstrap.
// Cross-platform (Windows 11 / macOS / Linux) per issue #2. The actual
// engine/viewport/editor/export features are separate issues (#3-#9);
// this file only wires up the window, render loop, and docking layout.

#include "app/AppIcon.h"
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

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {

#if defined(_WIN32)
// Issue #31: without this, Windows treats the process as DPI-unaware and
// either bitmap-stretches the whole window (blurry) or renders it at a
// fixed logical size that doesn't match a high-DPI monitor's actual pixel
// density -- on a multi-monitor setup with mixed DPI, that shows up as
// "fonts too small on the high-resolution monitor, fine on the
// mid-resolution one" (the reported symptom). Must be called before any
// window/monitor is created or queried, so this runs as the very first
// thing in main() -- before even glfwInit().
//
// A runtime call (not an embedded application manifest) was chosen --
// simpler than wiring manifest embedding into the CMake/resource build,
// see AGENTS.md's issue #31 notes. Declares its own opaque handle type
// and resolves SetProcessDpiAwarenessContext dynamically via
// GetProcAddress rather than depending on <windows.h>'s
// DPI_AWARENESS_CONTEXT/DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 --
// both are only declared when the SDK's WINVER target is Windows 10
// 1703+, which this project doesn't otherwise require -- and rather than
// link against the function directly, so this is a graceful no-op on
// older Windows 10 builds that don't export it, instead of a hard
// dependency on a specific Windows 10 update.
using OrcisfDpiAwarenessContext = void*;

void EnableWindowsDpiAwareness() {
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (!user32) return;
    using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(OrcisfDpiAwarenessContext);
    auto set_ctx = reinterpret_cast<SetProcessDpiAwarenessContextFn>(
        GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
    if (set_ctx) {
        OrcisfDpiAwarenessContext per_monitor_aware_v2 =
            reinterpret_cast<OrcisfDpiAwarenessContext>(static_cast<intptr_t>(-4));
        set_ctx(per_monitor_aware_v2);
    }
}
#endif

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
#if defined(_WIN32)
    // Issue #31: must happen before glfwInit()/any window or monitor
    // query -- Windows decides the process's DPI-awareness mode once,
    // the first time it's queried, and locks it in.
    EnableWindowsDpiAwareness();
#endif

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

    // Issue #31: lets GLFW size/scale the window to match the monitor it's
    // actually created on (relevant on a multi-monitor setup with mixed
    // DPI) -- combined with the content-scale read below to also scale
    // ImGui's font/style metrics to match.
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(
        1600, 900, "ORCISF - Optimasi Beton Bertulang Pada Struktur Portal Ruang", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }
    // Issue #26: taskbar/Alt+Tab window icon on Windows/Linux (no-op on
    // macOS, see AppIcon.h's comment). Best-effort -- a missing/unreadable
    // icon file never blocks startup.
    orcisf::app::ApplyWindowIcon(window);

    // Issue #31: the actual content scale of the monitor this window was
    // created on (1.0 = 96 DPI/100%, 1.5 = 150%, 2.0 = 200%, ...) -- read
    // once at startup and used below to scale both the ImGui font and
    // style metrics so text/controls are a consistent *physical* size
    // across monitors of different DPI, not a fixed *pixel* size that
    // looks tiny on a high-DPI display. x/y are equal in practice for
    // every desktop monitor; only x is used. Deliberately not re-read if
    // the window is later dragged to a different-DPI monitor -- live
    // rescaling is a separate, more involved feature (would need
    // rebuilding the font atlas and un-doing/redoing the style scale) and
    // is out of scope here, see AGENTS.md's issue #31 notes.
    float dpi_scale = 1.f, dpi_scale_y_unused = 1.f;
    glfwGetWindowContentScale(window, &dpi_scale, &dpi_scale_y_unused);

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

    // Issue #31: scale the theme's style metrics (padding/spacing/rounding/
    // border sizes) by the monitor's content scale -- must run *after*
    // ApplyModernTheme(), since it scales whatever values are currently
    // set, not ImGui's hardcoded defaults. Rebuilding the font at a
    // scaled pixel size (rather than the cheaper io.FontGlobalScale, which
    // just stretches the existing 13px bitmap and looks blurry at
    // non-integer scales) keeps text crisp.
    ImGui::GetStyle().ScaleAllSizes(dpi_scale);
    ImFontConfig font_cfg;
    font_cfg.SizePixels = 13.0f * dpi_scale; // ImGui's default font is authored at 13px
    io.Fonts->AddFontDefault(&font_cfg);

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
