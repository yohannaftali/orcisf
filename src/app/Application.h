#pragma once

#include "gui/LogPanel.h"
#include "gui/PropertiesPanel.h"
#include "gui/RunPanel.h"
#include "gui/Toolbar.h"
#include "gui/ViewportPanel.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::app {

// Owns the docking layout and top-level panels. One instance per process;
// driven once per frame by main.cpp's render loop. Also owns the shared
// scene/selection state (issue #5): the currently loaded SceneModel and
// which member (if any) is selected, both read by ViewportPanel and
// PropertiesPanel and written by ViewportPanel (picking) and the
// load-folder/run-completion flows below.
class Application {
public:
    Application();

    // Builds the dockspace and draws every panel for the current frame.
    void OnFrame();

private:
    void BuildDockspace();
    void OnOpenFolderRequested();
    void OnRunResult(engine::StructureData sd, std::string dataset_path);

    gui::Toolbar toolbar_;
    gui::ViewportPanel viewport_panel_;
    gui::PropertiesPanel properties_panel_;
    gui::LogPanel log_panel_;
    gui::RunPanel run_panel_;

    gui::SceneModel scene_;
    int selected_member_ = -1;

    bool viewport_open_ = true;
    bool properties_open_ = true;
    bool log_open_ = true;
    bool run_open_ = true;
    bool dockspace_initialized_ = false;
};

} // namespace orcisf::app
