#pragma once

#include <optional>
#include <vector>

#include "engine/MemberResults.h"
#include "gui/DetailingPanel.h"
#include "gui/IconToolbar.h"
#include "gui/LoadsPanel.h"
#include "gui/LogPanel.h"
#include "gui/PropertiesPanel.h"
#include "gui/RunPanel.h"
#include "gui/Toolbar.h"
#include "gui/ViewportPanel.h"
#include "gui/editor/EditableStructure.h"
#include "gui/editor/Selection.h"
#include "gui/editor/UndoStack.h"
#include "gui/viewport/SceneModel.h"

namespace orcisf::app {

// Owns the docking layout and top-level panels. One instance per process;
// driven once per frame by main.cpp's render loop. Also owns the shared
// scene/selection/editing state (issues #5/#6): the currently loaded
// StructureData (`loaded_sd_`, the single source of truth an
// EditableStructure mutates in place), the derived SceneModel read by
// ViewportPanel/PropertiesPanel, which member/joint is selected, the
// undo/redo stack, and the current validation-issue list.
class Application {
public:
    Application();

    // Builds the dockspace and draws every panel for the current frame.
    void OnFrame();

private:
    void BuildDockspace();
    void OnNewDataRequested();
    void OnSaveRequested();
    void OnSaveAsRequested();
    void OnOpenFolderRequested();
    void OnRunResult(engine::StructureData sd, std::string dataset_path);
    void OnAddJointRequested();
    void OnUndo();
    void OnRedo();
    void OnSaveLoadsRequested();
    void OnExportTextRequested();
    void OnExportPdfRequested();
    void OnExportInfRequested();

    // Common load path for both OnOpenFolderRequested (view-only, no
    // results yet) and OnRunResult (results available): replaces
    // loaded_sd_, resets selection/undo/editor state, and rebuilds
    // scene_/validation_issues_.
    void LoadStructure(engine::StructureData sd, const std::vector<engine::MemberResult>* results,
                        std::string dataset_path);

    // Rebuilds scene_ + validation_issues_ from the current loaded_sd_ --
    // called after every edit. Any edit invalidates previously-computed
    // MemberResult data (member ids can shift on delete), so this always
    // rebuilds geometry-only (no results) once editing has started.
    void RebuildSceneAfterEdit();

    gui::Toolbar toolbar_;
    gui::IconToolbar icon_toolbar_;
    gui::ViewportPanel viewport_panel_;
    gui::PropertiesPanel properties_panel_;
    gui::LoadsPanel loads_panel_;
    gui::DetailingPanel detailing_panel_;
    gui::LogPanel log_panel_;
    gui::RunPanel run_panel_;

    engine::StructureData loaded_sd_;
    std::optional<gui::EditableStructure> editable_; // wraps loaded_sd_ once something is loaded
    gui::UndoStack undo_stack_;
    gui::Selection selection_;
    gui::EditorOptions editor_options_;
    std::string loaded_dataset_path_;

    gui::SceneModel scene_;
    std::vector<std::string> validation_issues_;

    // Issue #9: true iff loaded_sd_ is exactly what a completed
    // optimization run produced (var_b/var_k/fitstr etc. all valid,
    // matching engine::WriteFinalResults()'s precondition) with no edit
    // since -- gates "Export PDF..." and .opt/.str/.kdl/.inf in "Export
    // Text...". current_results_ is the same MemberResult list the scene
    // was built from, kept around so PDF export doesn't need to recompute
    // it (which would also mutate loaded_sd_'s scratch fields again).
    bool has_run_results_ = false;
    std::vector<engine::MemberResult> current_results_;

    bool viewport_open_ = true;
    bool properties_open_ = true;
    bool loads_open_ = true;
    bool detailing_open_ = true;
    bool log_open_ = true;
    bool run_open_ = true;
    bool dockspace_initialized_ = false;

    // Issue #15: which View menu preset is active. BuildDockspace() rebuilds
    // the layout whenever this differs from built_layout_preset_ (the
    // preset the current dock arrangement was last built for).
    gui::ViewLayoutPreset current_layout_ = gui::ViewLayoutPreset::Default;
    gui::ViewLayoutPreset built_layout_preset_ = gui::ViewLayoutPreset::Default;
};

} // namespace orcisf::app
