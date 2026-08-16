#pragma once

#include <optional>
#include <vector>

#include "app/CustomTitleBar.h"
#include "engine/AnalysisResults.h"
#include "engine/MemberResults.h"
#include "gui/DetailingPanel.h"
#include "gui/ForceDiagramPanel.h"
#include "gui/IconToolbar.h"
#include "gui/IconVisibility.h"
#include "gui/JointsPanel.h"
#include "gui/LoadsPanel.h"
#include "gui/LogPanel.h"
#include "gui/MembersPanel.h"
#include "gui/PanelTitles.h"
#include "gui/PanelVisibility.h"
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

    // Issue #19: the GLFWwindow* CustomTitleBar needs for its drag/
    // minimize/maximize/close commands. Must be called once, before the
    // first OnFrame(), since main.cpp is the only file that owns a
    // GLFWwindow* (kept that way deliberately -- see CustomTitleBar.h).
    void SetWindow(GLFWwindow* window);

    // Builds the dockspace and draws every panel for the current frame.
    void OnFrame();

private:
    void BuildDockspace();
    void OnNewDataRequested();
    void OnSaveRequested();
    void OnSaveAsRequested();
    void OnOpenFolderRequested();
    void OnRunResult(engine::StructureData sd, std::string dataset_path, std::string output_path);
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
    // scene_/validation_issues_. `analysis` (issue #60), if non-null,
    // must be the AnalysisResults computed from the *same* sd `results`
    // came from -- feeds SceneModel's deformed-shape data.
    void LoadStructure(engine::StructureData sd, const std::vector<engine::MemberResult>* results,
                        std::string dataset_path, const engine::AnalysisResults* analysis = nullptr);

    // Rebuilds scene_ + validation_issues_ from the current loaded_sd_ --
    // called after every edit. Any edit invalidates previously-computed
    // MemberResult data (member ids can shift on delete), so this always
    // rebuilds geometry-only (no results) once editing has started.
    void RebuildSceneAfterEdit();

    gui::Toolbar toolbar_;
    gui::IconToolbar icon_toolbar_;
    CustomTitleBar title_bar_;
    GLFWwindow* window_ = nullptr;
    gui::ViewportPanel viewport_panel_;
    gui::PropertiesPanel properties_panel_;
    gui::LoadsPanel loads_panel_;
    gui::JointsPanel joints_panel_;
    gui::MembersPanel members_panel_;
    gui::DetailingPanel detailing_panel_;
    gui::ForceDiagramPanel force_diagram_panel_; // issue #61
    gui::LogPanel log_panel_;
    gui::RunPanel run_panel_;

    engine::StructureData loaded_sd_;
    std::optional<gui::EditableStructure> editable_; // wraps loaded_sd_ once something is loaded
    gui::UndoStack undo_stack_;
    gui::Selection selection_;
    gui::EditorOptions editor_options_;
    std::string loaded_dataset_path_;

    // Issue #37: backing storage for the View menu's Menubar section --
    // Toolbar writes to this via checkable MenuItems, IconToolbar reads it
    // to decide which buttons to draw.
    gui::IconVisibility icon_visibility_;

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

    // Issue #60: raw analysis results (member end forces, joint
    // displacements/reactions) for the same run current_results_ was
    // computed from -- the deformed-shape overlay's source data. Cleared
    // (default-constructed, empty) whenever has_run_results_ goes false,
    // same lifetime as current_results_.
    engine::AnalysisResults current_analysis_;

    // Issue #25: the generic path the *most recent completed run* actually
    // wrote its .opt/.str/.kdl/.inf/.his/.log.txt to (a fresh timestamped
    // subfolder per run, see engine::RunFullOptimization) -- no longer the
    // same as loaded_dataset_path_ (which stays the *input* dataset's
    // path, used for Save/Save As). report::WriteTextExport()'s
    // source_generic_path argument must use this, not
    // loaded_dataset_path_, whenever has_run_results_ is true, or it would
    // look for .his/.log.txt in the wrong place. Cleared implicitly by
    // has_run_results_ going false (RebuildSceneAfterEdit()); left stale
    // otherwise, which is harmless since it's never read while
    // has_run_results_ is false.
    std::string last_run_output_path_;

    bool viewport_open_ = true;
    bool properties_open_ = true;
    bool loads_open_ = true;
    bool joints_open_ = true;
    bool members_open_ = true;
    bool detailing_open_ = true;
    bool force_diagram_open_ = true; // issue #61
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
