#pragma once

#include <functional>

#include "gui/editor/Selection.h"

namespace orcisf::gui {

// Top toolbar. "File > Open Folder..." (#5), the "Edit" menu (#6:
// Undo/Redo, Add Joint, Connect Mode, Snap to Grid), the "Loads" menu +
// "File > Save Loads (.bbn)" (#7), and "File > Export PDF.../Export
// Text..." (#9) are wired; Run/View menu items are still placeholders
// (Run is covered by the separate RunPanel).
class Toolbar {
public:
    void Draw(bool can_undo, bool can_redo, bool can_save, bool can_export_text, bool can_export_pdf,
              EditorOptions& options);

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

private:
    std::function<void()> on_open_folder_;
    std::function<void()> on_undo_;
    std::function<void()> on_redo_;
    std::function<void()> on_add_joint_;
    std::function<void()> on_save_loads_;
    std::function<void()> on_export_text_;
    std::function<void()> on_export_pdf_;
};

} // namespace orcisf::gui
