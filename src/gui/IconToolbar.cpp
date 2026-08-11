#include "gui/IconToolbar.h"

#include "gui/UiScale.h"

#include <imgui.h>

namespace orcisf::gui {

namespace {

constexpr float kBasePadding = 4.f;
constexpr float kPi = 3.14159265358979323846f;

// Issue #31: every pixel constant below is a base (100%-scale) value that
// has to be multiplied by the current DPI scale -- see gui/UiScale.h.
// Glyph geometry is expressed as a fraction of ButtonSize() (via P()), so
// it follows automatically; only the absolute values -- padding, button
// size and stroke thickness -- need scaling by hand.
float Padding() { return Scaled(kBasePadding); }
float ButtonSize() { return IconToolbar::Height() - Padding() * 2.f; }
float Stroke(float base_thickness) { return Scaled(base_thickness); }

// Draws one square icon button at the ImGui cursor, calling `draw_icon` to
// paint its glyph (line/shape primitives, in button-local coordinates: (0,0)
// top-left, (kButtonSize,kButtonSize) bottom-right) into the foreground
// draw list. Returns true the frame the button is clicked.
template <typename DrawIconFn>
bool IconButton(const char* id, const char* tooltip, bool enabled, DrawIconFn draw_icon) {
    const float button_size = ButtonSize();
    ImGui::BeginDisabled(!enabled);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, ImVec2(button_size, button_size));
    bool hovered = ImGui::IsItemHovered();
    ImGui::EndDisabled();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 bg = hovered ? IM_COL32(70, 70, 78, 255) : IM_COL32(45, 45, 51, 255);
    ImU32 fg = enabled ? IM_COL32(230, 230, 235, 255) : IM_COL32(110, 110, 115, 255);
    dl->AddRectFilled(pos, ImVec2(pos.x + button_size, pos.y + button_size), bg, Scaled(3.f));
    draw_icon(dl, pos, fg);

    if (hovered && tooltip && enabled) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return clicked && enabled;
}

// Local point (x,y in [0,1] of the button) -> screen coordinates.
ImVec2 P(const ImVec2& origin, float x, float y) {
    const float button_size = ButtonSize();
    return ImVec2(origin.x + x * button_size, origin.y + y * button_size);
}

void DrawNewDataIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddRect(P(o, 0.25f, 0.15f), P(o, 0.75f, 0.85f), c, 0.f, 0, Stroke(1.5f));
    dl->AddLine(P(o, 0.5f, 0.32f), P(o, 0.5f, 0.68f), c, Stroke(1.5f));
    dl->AddLine(P(o, 0.34f, 0.5f), P(o, 0.66f, 0.5f), c, Stroke(1.5f));
}

void DrawOpenDataIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddLine(P(o, 0.15f, 0.35f), P(o, 0.4f, 0.35f), c, Stroke(1.5f));
    dl->AddLine(P(o, 0.4f, 0.35f), P(o, 0.5f, 0.25f), c, Stroke(1.5f));
    dl->AddLine(P(o, 0.5f, 0.25f), P(o, 0.85f, 0.25f), c, Stroke(1.5f));
    dl->AddLine(P(o, 0.85f, 0.25f), P(o, 0.85f, 0.75f), c, Stroke(1.5f));
    dl->AddLine(P(o, 0.85f, 0.75f), P(o, 0.15f, 0.75f), c, Stroke(1.5f));
    dl->AddLine(P(o, 0.15f, 0.75f), P(o, 0.15f, 0.35f), c, Stroke(1.5f));
}

void DrawSaveIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddRect(P(o, 0.2f, 0.15f), P(o, 0.8f, 0.85f), c, 0.f, 0, Stroke(1.5f));
    dl->AddRectFilled(P(o, 0.32f, 0.15f), P(o, 0.68f, 0.4f), c);
    dl->AddRect(P(o, 0.3f, 0.55f), P(o, 0.7f, 0.8f), c, 0.f, 0, Stroke(1.5f));
}

void DrawUndoIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->PathArcTo(P(o, 0.55f, 0.55f), ButtonSize() * 0.28f, kPi * 0.9f, kPi * 2.3f);
    dl->PathStroke(c, 0, Stroke(1.6f));
    dl->AddTriangleFilled(P(o, 0.22f, 0.30f), P(o, 0.40f, 0.22f), P(o, 0.30f, 0.44f), c);
}

void DrawRedoIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->PathArcTo(P(o, 0.45f, 0.55f), ButtonSize() * 0.28f, -kPi * 0.3f, kPi * 1.1f);
    dl->PathStroke(c, 0, Stroke(1.6f));
    dl->AddTriangleFilled(P(o, 0.78f, 0.30f), P(o, 0.60f, 0.22f), P(o, 0.70f, 0.44f), c);
}

void DrawAddJointIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddCircleFilled(P(o, 0.35f, 0.65f), ButtonSize() * 0.1f, c);
    dl->AddLine(P(o, 0.72f, 0.28f), P(o, 0.72f, 0.6f), c, Stroke(1.6f));
    dl->AddLine(P(o, 0.56f, 0.44f), P(o, 0.88f, 0.44f), c, Stroke(1.6f));
}

void DrawConnectIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddCircle(P(o, 0.25f, 0.75f), ButtonSize() * 0.09f, c, 0, Stroke(1.5f));
    dl->AddCircle(P(o, 0.75f, 0.25f), ButtonSize() * 0.09f, c, 0, Stroke(1.5f));
    dl->AddLine(P(o, 0.32f, 0.68f), P(o, 0.68f, 0.32f), c, Stroke(1.5f));
}

void DrawRunIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddTriangleFilled(P(o, 0.28f, 0.2f), P(o, 0.28f, 0.8f), P(o, 0.78f, 0.5f), c);
}

} // namespace

float IconToolbar::Height() { return Scaled(kBaseHeight); }

void IconToolbar::SetOnNewData(std::function<void()> callback) { on_new_data_ = std::move(callback); }
void IconToolbar::SetOnOpenFolder(std::function<void()> callback) { on_open_folder_ = std::move(callback); }
void IconToolbar::SetOnSave(std::function<void()> callback) { on_save_ = std::move(callback); }
void IconToolbar::SetOnUndo(std::function<void()> callback) { on_undo_ = std::move(callback); }
void IconToolbar::SetOnRedo(std::function<void()> callback) { on_redo_ = std::move(callback); }
void IconToolbar::SetOnAddJoint(std::function<void()> callback) { on_add_joint_ = std::move(callback); }
void IconToolbar::SetOnRun(std::function<void()> callback) { on_run_ = std::move(callback); }

void IconToolbar::Draw(bool can_undo, bool can_redo, bool can_save, bool can_run, EditorOptions& options,
                        const IconVisibility& visibility) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                              ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings;
    const float padding = Padding();
    // Issue #31: anchored to the *work* area's top-left, not the raw
    // viewport origin plus a re-derived menu bar height -- WorkPos already
    // has the main menu bar's real height subtracted by ImGui itself, so
    // the toolbar sits flush under it at every DPI scale instead of
    // relying on GetFrameHeight() happening to match.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, IconToolbar::Height()));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
    ImGui::Begin("##IconToolbar", nullptr, flags);

    // Issue #37: each button's slot is skipped entirely when hidden (not
    // left as a blank gap) -- `first` tracks whether anything has been
    // drawn yet so the very first *visible* button never gets a leading
    // SameLine, regardless of which button that ends up being.
    bool first = true;
    auto Sep = [&](bool double_gap) {
        if (!first) ImGui::SameLine(0.f, double_gap ? padding * 2.f : padding);
        first = false;
    };

    if (visibility.new_data) {
        Sep(false);
        if (IconButton("##new_data", "New Data", true, DrawNewDataIcon) && on_new_data_) on_new_data_();
    }
    if (visibility.open_data) {
        Sep(false);
        if (IconButton("##open_data", "Open Data...", true, DrawOpenDataIcon) && on_open_folder_) on_open_folder_();
    }
    if (visibility.save) {
        Sep(false);
        if (IconButton("##save", "Save (Ctrl+S)", can_save, DrawSaveIcon) && on_save_) on_save_();
    }

    if (visibility.undo) {
        Sep(true);
        if (IconButton("##undo", "Undo", can_undo, DrawUndoIcon) && on_undo_) on_undo_();
    }
    if (visibility.redo) {
        Sep(false);
        if (IconButton("##redo", "Redo", can_redo, DrawRedoIcon) && on_redo_) on_redo_();
    }

    if (visibility.add_joint) {
        Sep(true);
        bool add_joint_active = options.add_joint_mode;
        const char* add_joint_tooltip =
            add_joint_active ? "Add Joint (active -- click in viewport to place, click here to stop)" : "Add Joint";
        if (IconButton("##add_joint", add_joint_tooltip, true, DrawAddJointIcon)) {
            options.add_joint_mode = !options.add_joint_mode;
            if (options.add_joint_mode) {
                options.connect_mode = false;
                options.connect_first_joint = -1;
                options.load_mode = LoadPlacementMode::None;
                if (on_add_joint_) on_add_joint_();
            }
        }
        if (add_joint_active) {
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(90, 230, 110, 255), Scaled(3.f), 0, Stroke(2.f));
        }
    }
    if (visibility.connect_joints) {
        Sep(false);
        bool connect_active = options.connect_mode;
        const char* connect_tooltip = connect_active ? "Connect Joints (active -- click to stop)" : "Connect Joints";
        if (IconButton("##connect", connect_tooltip, true, DrawConnectIcon)) {
            options.connect_mode = !options.connect_mode;
            options.connect_first_joint = -1;
            options.add_joint_mode = false;
            options.load_mode = LoadPlacementMode::None;
        }
        if (connect_active) {
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(255, 209, 38, 255), Scaled(3.f), 0, Stroke(2.f));
        }
    }

    if (visibility.run) {
        Sep(true);
        if (IconButton("##run", can_run ? "Run Optimization" : "Run Optimization (load or create a dataset first)",
                        can_run, DrawRunIcon) &&
            on_run_) {
            on_run_();
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace orcisf::gui
