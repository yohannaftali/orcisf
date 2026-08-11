#include "app/AppIcon.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <png.h>

#include <cstddef>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <climits>
#else
#include <climits>
#include <unistd.h>
#endif

namespace orcisf::app {

namespace {

// The executable's own directory -- icon PNGs are copied next to it at
// build time (see CMakeLists.txt's POST_BUILD copy, same reasoning as
// the existing MinGW-runtime-DLL copy in that file: the process's
// current working directory isn't guaranteed to be the executable's own
// directory, e.g. launched from a shortcut or a different shell cwd).
std::string ExecutableDir() {
#if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) return {};
    std::wstring wpath(buf, len);
    // Build output paths on this project's supported platforms are
    // ASCII-only (no need for a real UTF-16 -> UTF-8 conversion here).
    std::string path(wpath.begin(), wpath.end());
    size_t slash = path.find_last_of("\\/");
    return slash == std::string::npos ? std::string(".") : path.substr(0, slash);
#elif defined(__APPLE__)
    char buf[PATH_MAX];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0) return {};
    std::string path(buf);
    size_t slash = path.find_last_of('/');
    return slash == std::string::npos ? std::string(".") : path.substr(0, slash);
#else
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0) return {};
    buf[len] = '\0';
    std::string path(buf);
    size_t slash = path.find_last_of('/');
    return slash == std::string::npos ? std::string(".") : path.substr(0, slash);
#endif
}

// Decodes a PNG file into raw RGBA8 pixels via libpng's simplified API
// (png_image) -- far less boilerplate than the full streaming API for a
// one-shot "load this whole small icon" use case. `out_pixels` owns the
// decoded bytes; `out_image.pixels` points into it (matches GLFWimage's
// expected layout, top-to-bottom row-major RGBA8).
bool LoadPngRgba(const std::string& path, GLFWimage& out_image, std::vector<unsigned char>& out_pixels) {
    png_image image{};
    image.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_file(&image, path.c_str())) {
        return false;
    }
    image.format = PNG_FORMAT_RGBA;
    out_pixels.resize(PNG_IMAGE_SIZE(image));
    if (!png_image_finish_read(&image, nullptr, out_pixels.data(), 0, nullptr)) {
        png_image_free(&image);
        return false;
    }
    out_image.width = static_cast<int>(image.width);
    out_image.height = static_cast<int>(image.height);
    out_image.pixels = out_pixels.data();
    png_image_free(&image);
    return true;
}

} // namespace

void ApplyWindowIcon(GLFWwindow* window) {
#if defined(__APPLE__)
    (void)window;
    return; // see AppIcon.h's comment -- glfwSetWindowIcon is a no-op on macOS anyway
#else
    std::string dir = ExecutableDir();
    if (dir.empty()) return;

    // Multiple sizes so the OS/window manager can pick the closest match
    // (GLFW's own recommendation for glfwSetWindowIcon) -- matches the
    // sizes CMakeLists.txt copies next to the executable.
    static const int kSizes[] = {16, 32, 48, 256};
    constexpr size_t kCount = sizeof(kSizes) / sizeof(kSizes[0]);

    std::vector<GLFWimage> images;
    std::vector<std::vector<unsigned char>> storage(kCount); // must outlive the glfwSetWindowIcon() call below
    images.reserve(kCount);

    for (size_t i = 0; i < kCount; ++i) {
        std::string path =
            dir + "/icons/icon_" + std::to_string(kSizes[i]) + "x" + std::to_string(kSizes[i]) + ".png";
        GLFWimage img{};
        if (LoadPngRgba(path, img, storage[i])) {
            images.push_back(img);
        }
    }
    if (!images.empty()) {
        glfwSetWindowIcon(window, static_cast<int>(images.size()), images.data());
    }
#endif
}

} // namespace orcisf::app
