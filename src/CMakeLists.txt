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
# Issue #26: decodes the app icon PNGs for the runtime glfwSetWindowIcon()
# call (app/AppIcon.cpp) -- already an installed transitive dependency of
# libharu (PNG image embedding in PDFs), now also a direct one so this
# doesn't depend on that staying true.
find_package(PNG CONFIG REQUIRED)

# Issue #26: the app icon set (Windows .ico, macOS .icns, PNGs for the
# Linux desktop file and every platform's runtime window icon) -- lives
# at the repo root, not under src/, since it's shared reference art, not
# build-generated. See ../icons/README.txt.
set(ORCISF_ICONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../icons")

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
    app/AppIcon.cpp
    app/CustomTitleBar.cpp
    app/WindowResize.cpp
    app/Theme.cpp
    app/DockTabIcons.cpp
    gui/UiScale.cpp
    gui/Toolbar.cpp
    gui/IconToolbar.cpp
    gui/PanelIcons.cpp
    gui/PanelTitles.cpp
    gui/ViewportPanel.cpp
    gui/PropertiesPanel.cpp
    gui/RunPanel.cpp
    gui/LogPanel.cpp
    gui/LoadsPanel.cpp
    gui/JointsPanel.cpp
    gui/MembersPanel.cpp
    gui/DetailingPanel.cpp
    gui/ForceDiagramPanel.cpp
    gui/diagrams/ForceDiagram.cpp
    gui/viewport/Camera.cpp
    gui/viewport/SceneModel.cpp
    gui/viewport/SceneRenderer.cpp
    gui/editor/EditableStructure.cpp
    gui/editor/UndoStack.cpp
    gui/detailing/DetailingLayout.cpp
    report/TextExport.cpp
    report/PdfExport.cpp
)

# Issue #26: per-platform app icon wiring.
if(WIN32)
    # Embeds icons/windows/orcisf.ico as the .exe's own icon (Explorer,
    # taskbar, Alt+Tab) -- CMake auto-detects the .rc extension and
    # invokes rc.exe (MSVC) / windres (MinGW) for it, no extra language
    # setup needed.
    target_sources(orcisf_gui PRIVATE app/orcisf.rc)
elseif(APPLE)
    # Packages orcisf_gui as a real .app bundle with icons/macos/orcisf.icns
    # as its Dock/Finder icon -- MACOSX_BUNDLE_ICON_FILE must name a file
    # that's both copied into the bundle's Resources (via
    # MACOSX_PACKAGE_LOCATION below) and added to the target's sources.
    set(ORCISF_ICNS "${ORCISF_ICONS_DIR}/macos/orcisf.icns")
    set_source_files_properties(${ORCISF_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(orcisf_gui PRIVATE ${ORCISF_ICNS})
    set_target_properties(orcisf_gui PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE orcisf.icns
        MACOSX_BUNDLE_BUNDLE_NAME "ORCISF"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yohannaftali.orcisf"
    )
endif()

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
    PNG::PNG
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

# Issue #26: runtime window icon PNGs (app/AppIcon.cpp's ApplyWindowIcon())
# -- copied next to the executable, same "don't trust the process's cwd"
# reasoning as the MinGW DLL copy above. Not needed on macOS: the bundle
# icon (MACOSX_BUNDLE_ICON_FILE above) is the only icon macOS uses, and
# ApplyWindowIcon() is a no-op there.
if(NOT APPLE)
    add_custom_command(TARGET orcisf_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:orcisf_gui>/icons"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ORCISF_ICONS_DIR}/png/icon_16x16.png"
            "${ORCISF_ICONS_DIR}/png/icon_32x32.png"
            "${ORCISF_ICONS_DIR}/png/icon_48x48.png"
            "${ORCISF_ICONS_DIR}/png/icon_256x256.png"
            "$<TARGET_FILE_DIR:orcisf_gui>/icons"
    )
endif()

# Issue #26: Linux desktop-environment integration (application-menu/
# launcher entry + hicolor icon theme) -- only takes effect via
# `cmake --install`, a packaging step this project doesn't otherwise
# script; not exercised in this environment (no Linux machine available,
# see AGENTS.md's Validation section).
if(UNIX AND NOT APPLE)
    install(TARGETS orcisf_gui RUNTIME DESTINATION bin)
    install(FILES "${ORCISF_ICONS_DIR}/linux/icon_256x256.png"
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME orcisf.png)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/orcisf.desktop"
        DESTINATION share/applications)
endif()
