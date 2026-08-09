cmake_minimum_required(VERSION 3.21)
project(orcisf_gui CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(unofficial-libharu CONFIG REQUIRED)

add_executable(orcisf_gui
    app/main.cpp
    app/Application.cpp
    gui/Toolbar.cpp
    gui/ViewportPanel.cpp
    gui/PropertiesPanel.cpp
    gui/LogPanel.cpp
)

target_include_directories(orcisf_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(orcisf_gui PRIVATE
    imgui::imgui
    glfw
    OpenGL::GL
    imguizmo::imguizmo
    implot::implot
    nfd::nfd
    unofficial::libharu::hpdf
)

if(MSVC)
    target_compile_options(orcisf_gui PRIVATE /W4)
else()
    target_compile_options(orcisf_gui PRIVATE -Wall -Wextra)
endif()
