#pragma once

namespace orcisf::gui {

// Issue #37: pointers to each panel's own open-state bool (Application
// owns the real storage -- viewport_open_ etc., the same fields each
// panel's Draw() call is already gated on, see Application::OnFrame()'s
// close-button-fix comment) -- lets the View menu's new "Subwindows"
// section both *display* each panel's current open/closed state and
// *toggle* it (reopening a closed panel included) from one place,
// without Toolbar needing to know anything else about Application.
struct PanelVisibility {
    bool* viewport = nullptr;
    bool* detailing = nullptr;
    bool* properties = nullptr;
    bool* optimization = nullptr; // RunPanel -- display renamed in #37, this pointer is still run_open_
    bool* joints = nullptr;
    bool* members = nullptr;
    bool* loads = nullptr;
    bool* log = nullptr;
    bool* force_diagram = nullptr; // issue #61
    bool* results = nullptr;       // issue #62
};

} // namespace orcisf::gui
