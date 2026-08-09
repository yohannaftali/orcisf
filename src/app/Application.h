#pragma once

#include "gui/LogPanel.h"
#include "gui/PropertiesPanel.h"
#include "gui/Toolbar.h"
#include "gui/ViewportPanel.h"

namespace orcisf::app {

// Owns the docking layout and top-level panels. One instance per process;
// driven once per frame by main.cpp's render loop.
class Application {
public:
    Application();

    // Builds the dockspace and draws every panel for the current frame.
    void OnFrame();

private:
    void BuildDockspace();

    gui::Toolbar toolbar_;
    gui::ViewportPanel viewport_panel_;
    gui::PropertiesPanel properties_panel_;
    gui::LogPanel log_panel_;

    bool viewport_open_ = true;
    bool properties_open_ = true;
    bool log_open_ = true;
    bool dockspace_initialized_ = false;
};

} // namespace orcisf::app
