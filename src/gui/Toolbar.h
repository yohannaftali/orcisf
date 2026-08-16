#pragma once

#include <functional>

#include "gui/IconVisibility.h"
#include "gui/PanelVisibility.h"
#include "gui/editor/Selection.h"

namespace orcisf::gui {

// Issue #15: switchable docking-layout presets, selected from the View
// menu. Default matches the layout this app has always had; Design and
// Optimization re-weight which panels get the large/primary area for
// their respective workflow stage -- no panel is ever fully hidden by a
// preset, just resized/re-tabbed, so nothing becomes unreachable.
enum class ViewLayoutPreset { Default, Design, Optimization };

// Top toolbar. "File > Open Folder..." (#5), the "Edit" menu (#6:
// Undo/Redo, Add Joint, Connect Mode, Snap to Grid), the "Loads" menu +
// "File > Save Loads (.bbn)" (#7), "File > Export PDF.../Export
// Text..." (#9), and the "View" menu's layout presets (#15) are wired;
// Run menu is still a placeholder (Run is covered by the separate
// RunPanel/IconToolbar's Run button, #14).
class Toolbar {
public:
    // Issue #37: icon_visibility/panel_visibility back the View menu's new
    // "Menubar"/"Subwindows" sections -- see IconVisibility.h/PanelVisibility.h.
    void Draw(bool can_undo, bool can_redo, bool can_save, bool can_export_text, bool can_export_pdf,
              bool can_export_inf, ViewLayoutPreset current_layout, EditorOptions& options,
              IconVisibility& icon_visibility, const PanelVisibility& panel_visibility);

    // Issue #11: starts a brand-new, empty dataset in the editor.
    void SetOnNewData(std::function<void()> callback);
    // Writes the full legacy input file set (.inp/.isd/.idl/.ijl/.ids/.ijs/
    // .bbn) back to the dataset's current path; falls back to Save As if
    // there isn't one yet.
    void SetOnSave(std::function<void()> callback);
    // Same, but always asks for a new folder + base filename first.
    void SetOnSaveAs(std::function<void()> callback);
    void SetOnOpenFolder(std::function<void()> callback);
    void SetOnUndo(std::function<void()> callback);
    void SetOnRedo(std::function<void()> callback);
    // Adds a new joint at a sensible default position (the current scene's
    // bounding-sphere center, or the origin if nothing is loaded) -- the
    // user then drags it into place via the viewport's gizmo or types
    // exact coordinates in the Properties panel.
    void SetOnAddJoint(std::function<void()> callback);
    // Writes the current raw loads back to the loaded dataset's .bbn file.
    void SetOnSaveLoads(std::function<void()> callback);
    // Issue #9: full legacy text file set / PDF report, via an NFD save dialog.
    void SetOnExportText(std::function<void()> callback);
    void SetOnExportPdf(std::function<void()> callback);
    // Issue #11: .inf preview generated directly from the in-GUI dataset,
    // independent of a completed optimization run.
    void SetOnExportInf(std::function<void()> callback);
    // Issue #62: joint displacement/member force/reaction tables as CSV,
    // via an NFD save dialog -- gated the same as can_export_pdf (a
    // completed run's results, not since edited).
    void SetOnExportResultsCsv(std::function<void()> callback);
    // Issue #15: fires when the user picks a View menu layout preset.
    void SetOnViewLayout(std::function<void(ViewLayoutPreset)> callback);
    // Issue #19: called right-aligned inside the still-open main menu bar,
    // after the last menu/status text and before EndMainMenuBar() --
    // lets Application draw the custom title bar (app title, drag zone,
    // Minimize/Maximize/Close) in the same row instead of a separate
    // window (see CustomTitleBar.h for why it has to be this way). Kept
    // as a generic hook rather than a GLFW-typed parameter so this file
    // stays free of any window-toolkit dependency, matching the rest of
    // gui/.
    void SetTitleBarDrawer(std::function<void()> drawer);

private:
    std::function<void()> on_new_data_;
    std::function<void()> on_save_;
    std::function<void()> on_save_as_;
    std::function<void()> on_open_folder_;
    std::function<void()> on_undo_;
    std::function<void()> on_redo_;
    std::function<void()> on_add_joint_;
    std::function<void()> on_save_loads_;
    std::function<void()> on_export_text_;
    std::function<void()> on_export_pdf_;
    std::function<void()> on_export_inf_;
    std::function<void()> on_export_results_csv_;
    std::function<void(ViewLayoutPreset)> on_view_layout_;
    std::function<void()> on_title_bar_drawer_;
};

} // namespace orcisf::gui
