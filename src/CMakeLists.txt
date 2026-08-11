cmake_minimum_required(VERSION 3.21)
project(orcisf_gui CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)

# Headless analysis/design/optimization engine (issue #3) -- no GUI
# dependencies of its own; see engine/README.md.
add_subdirectory(engine)

# WIN32 makes the linker use /SUBSYSTEM:WINDOWS instead of the default
# /SUBSYSTEM:CONSOLE on MSVC/MinGW, so launching orcisf_gui.exe doesn't
# also open a console window (see issue #12) -- CMake ignores this
# keyword entirely on non-Windows generators, so it's safe unconditionally.
add_executable(orcisf_gui WIN32
    app/main.cpp
    app/Application.cpp
    app/CustomTitleBar.cpp
    app/Theme.cpp
    gui/Toolbar.cpp
    gui/IconToolbar.cpp
    gui/ViewportPanel.cpp
    gui/PropertiesPanel.cpp
    gui/RunPanel.cpp
    gui/LogPanel.cpp
    gui/LoadsPanel.cpp
    gui/DetailingPanel.cpp
    gui/viewport/Camera.cpp
    gui/viewport/SceneModel.cpp
    gui/viewport/SceneRenderer.cpp
    gui/editor/EditableStructure.cpp
    gui/editor/UndoStack.cpp
    gui/detailing/DetailingLayout.cpp
    report/TextExport.cpp
    report/PdfExport.cpp
)

target_include_directories(orcisf_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(orcisf_gui PRIVATE
    orcisf_engine
    imgui::imgui
    glfw
    OpenGL::GL
    unofficial::gl3w::gl3w
    imguizmo::imguizmo
    implot::implot
    nfd::nfd
    unofficial::libharu::hpdf
)

if(MSVC)
    target_compile_options(orcisf_gui PRIVATE /W4)
    # WIN32 above switches MSVC to /SUBSYSTEM:WINDOWS, whose CRT startup
    # expects a WinMain() by default; main.cpp intentionally keeps a plain
    # main() (portable across all three platforms), so tell the linker to
    # keep using the C entry point instead -- MinGW's CRT does this
    # automatically with WIN32 + a plain main(), no equivalent flag needed.
    target_link_options(orcisf_gui PRIVATE /ENTRY:mainCRTStartup)
else()
    target_compile_options(orcisf_gui PRIVATE -Wall -Wextra)
endif()

# MinGW's runtime DLLs (libstdc++/libgcc_s_seh/libwinpthread) aren't on
# every Windows machine's PATH, and even when they are, another program's
# bundled copy earlier in PATH can shadow the compiler's own (observed
# during #5 development: PostgreSQL's bin/ ships an ABI-incompatible
# libwinpthread-1.dll, causing an "Entry Point Not Found: clock_gettime64"
# error at launch). Copying the compiler's own copies next to the exe
# makes the app directory's DLLs win regardless of PATH order -- standard
# practice for distributing a MinGW-built Windows executable. No-op on the
# MSVC/x64-windows triplet CI actually ships (see AGENTS.md/CMakePresets.json).
if(MINGW)
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(_runtime_dll libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
        if(EXISTS "${MINGW_BIN_DIR}/${_runtime_dll}")
            add_custom_command(TARGET orcisf_gui POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/${_runtime_dll}" "$<TARGET_FILE_DIR:orcisf_gui>"
            )
        endif()
    endforeach()
endif()
