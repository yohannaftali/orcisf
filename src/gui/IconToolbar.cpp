#include "gui/IconToolbar.h"

#include <imgui.h>

namespace orcisf::gui {

namespace {

constexpr float kPadding = 4.f;
constexpr float kButtonSize = IconToolbar::kHeight - kPadding * 2.f;
constexpr float kPi = 3.14159265358979323846f;

// Draws one square icon button at the ImGui cursor, calling `draw_icon` to
// paint its glyph (line/shape primitives, in button-local coordinates: (0,0)
// top-left, (kButtonSize,kButtonSize) bottom-right) into the foreground
// draw list. Returns true the frame the button is clicked.
template <typename DrawIconFn>
bool IconButton(const char* id, const char* tooltip, bool enabled, DrawIconFn draw_icon) {
    ImGui::BeginDisabled(!enabled);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, ImVec2(kButtonSize, kButtonSize));
    bool hovered = ImGui::IsItemHovered();
    ImGui::EndDisabled();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 bg = hovered ? IM_COL32(70, 70, 78, 255) : IM_COL32(45, 45, 51, 255);
    ImU32 fg = enabled ? IM_COL32(230, 230, 235, 255) : IM_COL32(110, 110, 115, 255);
    dl->AddRectFilled(pos, ImVec2(pos.x + kButtonSize, pos.y + kButtonSize), bg, 3.f);
    draw_icon(dl, pos, fg);

    if (hovered && tooltip && enabled) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return clicked && enabled;
}

// Local point (x,y in [0,1] of the button) -> screen coordinates.
ImVec2 P(const ImVec2& origin, float x, float y) {
    return ImVec2(origin.x + x * kButtonSize, origin.y + y * kButtonSize);
}

void DrawNewDataIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddRect(P(o, 0.25f, 0.15f), P(o, 0.75f, 0.85f), c, 0.f, 0, 1.5f);
    dl->AddLine(P(o, 0.5f, 0.32f), P(o, 0.5f, 0.68f), c, 1.5f);
    dl->AddLine(P(o, 0.34f, 0.5f), P(o, 0.66f, 0.5f), c, 1.5f);
}

void DrawOpenDataIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddLine(P(o, 0.15f, 0.35f), P(o, 0.4f, 0.35f), c, 1.5f);
    dl->AddLine(P(o, 0.4f, 0.35f), P(o, 0.5f, 0.25f), c, 1.5f);
    dl->AddLine(P(o, 0.5f, 0.25f), P(o, 0.85f, 0.25f), c, 1.5f);
    dl->AddLine(P(o, 0.85f, 0.25f), P(o, 0.85f, 0.75f), c, 1.5f);
    dl->AddLine(P(o, 0.85f, 0.75f), P(o, 0.15f, 0.75f), c, 1.5f);
    dl->AddLine(P(o, 0.15f, 0.75f), P(o, 0.15f, 0.35f), c, 1.5f);
}

void DrawSaveIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddRect(P(o, 0.2f, 0.15f), P(o, 0.8f, 0.85f), c, 0.f, 0, 1.5f);
    dl->AddRectFilled(P(o, 0.32f, 0.15f), P(o, 0.68f, 0.4f), c);
    dl->AddRect(P(o, 0.3f, 0.55f), P(o, 0.7f, 0.8f), c, 0.f, 0, 1.5f);
}

void DrawUndoIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->PathArcTo(P(o, 0.55f, 0.55f), kButtonSize * 0.28f, kPi * 0.9f, kPi * 2.3f);
    dl->PathStroke(c, 0, 1.6f);
    dl->AddTriangleFilled(P(o, 0.22f, 0.30f), P(o, 0.40f, 0.22f), P(o, 0.30f, 0.44f), c);
}

void DrawRedoIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->PathArcTo(P(o, 0.45f, 0.55f), kButtonSize * 0.28f, -kPi * 0.3f, kPi * 1.1f);
    dl->PathStroke(c, 0, 1.6f);
    dl->AddTriangleFilled(P(o, 0.78f, 0.30f), P(o, 0.60f, 0.22f), P(o, 0.70f, 0.44f), c);
}

void DrawAddJointIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddCircleFilled(P(o, 0.35f, 0.65f), kButtonSize * 0.1f, c);
    dl->AddLine(P(o, 0.72f, 0.28f), P(o, 0.72f, 0.6f), c, 1.6f);
    dl->AddLine(P(o, 0.56f, 0.44f), P(o, 0.88f, 0.44f), c, 1.6f);
}

void DrawConnectIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddCircle(P(o, 0.25f, 0.75f), kButtonSize * 0.09f, c, 0, 1.5f);
    dl->AddCircle(P(o, 0.75f, 0.25f), kButtonSize * 0.09f, c, 0, 1.5f);
    dl->AddLine(P(o, 0.32f, 0.68f), P(o, 0.68f, 0.32f), c, 1.5f);
}

void DrawRunIcon(ImDrawList* dl, ImVec2 o, ImU32 c) {
    dl->AddTriangleFilled(P(o, 0.28f, 0.2f), P(o, 0.28f, 0.8f), P(o, 0.78f, 0.5f), c);
}

} // namespace

void IconToolbar::SetOnNewData(std::function<void()> callback) { on_new_data_ = std::move(callback); }
void IconToolbar::SetOnOpenFolder(std::function<void()> callback) { on_open_folder_ = std::move(callback); }
void IconToolbar::SetOnSave(std::function<void()> callback) { on_save_ = std::move(callback); }
void IconToolbar::SetOnUndo(std::function<void()> callback) { on_undo_ = std::move(callback); }
void IconToolbar::SetOnRedo(std::function<void()> callback) { on_redo_ = std::move(callback); }
void IconToolbar::SetOnAddJoint(std::function<void()> callback) { on_add_joint_ = std::move(callback); }
void IconToolbar::SetOnRun(std::function<void()> callback) { on_run_ = std::move(callback); }

void IconToolbar::Draw(bool can_undo, bool can_redo, bool can_save, bool can_run, EditorOptions& options) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                              ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y + ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x, IconToolbar::kHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(kPadding, kPadding));
    ImGui::Begin("##IconToolbar", nullptr, flags);

    if (IconButton("##new_data", "New Data", true, DrawNewDataIcon) && on_new_data_) on_new_data_();
    ImGui::SameLine(0.f, kPadding);
    if (IconButton("##open_data", "Open Data...", true, DrawOpenDataIcon) && on_open_folder_) on_open_folder_();
    ImGui::SameLine(0.f, kPadding);
    if (IconButton("##save", "Save (Ctrl+S)", can_save, DrawSaveIcon) && on_save_) on_save_();

    ImGui::SameLine(0.f, kPadding * 2.f);
    if (IconButton("##undo", "Undo", can_undo, DrawUndoIcon) && on_undo_) on_undo_();
    ImGui::SameLine(0.f, kPadding);
    if (IconButton("##redo", "Redo", can_redo, DrawRedoIcon) && on_redo_) on_redo_();

    ImGui::SameLine(0.f, kPadding * 2.f);
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
        ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(90, 230, 110, 255), 3.f, 0, 2.f);
    }
    ImGui::SameLine(0.f, kPadding);
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
        ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(255, 209, 38, 255), 3.f, 0, 2.f);
    }

    ImGui::SameLine(0.f, kPadding * 2.f);
    if (IconButton("##run", can_run ? "Run Optimization" : "Run Optimization (set a dataset path first)", can_run,
                    DrawRunIcon) &&
        on_run_) {
        on_run_();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace orcisf::gui
