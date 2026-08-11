#include "gui/LoadsPanel.h"

#include <string>

#include <imgui.h>

#include "gui/PanelTitles.h"

namespace orcisf::gui {

namespace {

void DrawMemberLoadsTable(const SceneModel& scene, Selection& selection, EditableStructure* editable,
                           UndoStack* undo, const std::function<void()>& on_geometry_changed) {
    ImGui::SeparatorText("Member Loads (distributed, N/m)");
    if (scene.member_loads.empty()) {
        ImGui::TextDisabled("None. Use Loads > Add Member Load, then click a member in the Viewport.");
        return;
    }
    if (!ImGui::BeginTable("member_loads", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) return;
    ImGui::TableSetupColumn("Batang", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableSetupColumn("W (N/m)");
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableHeadersRow();

    for (const MemberLoadVisual& ml : scene.member_loads) {
        ImGui::PushID(ml.no_batang);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (ImGui::Selectable(std::to_string(ml.no_batang).c_str(), selection.kind == SelectionKind::Member &&
                                                                          selection.id == ml.no_batang,
                               ImGuiSelectableFlags_SpanAllColumns)) {
            selection = {SelectionKind::Member, ml.no_batang};
        }
        ImGui::TableSetColumnIndex(1);
        float w = ml.w_n_per_m;
        ImGui::SetNextItemWidth(-1.f);
        bool edited = ImGui::InputFloat("##w", &w);
        if (edited && ImGui::IsItemDeactivatedAfterEdit() && editable) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->SetMemberLoad(ml.no_batang, w);
            if (on_geometry_changed) on_geometry_changed();
        }
        ImGui::TableSetColumnIndex(2);
        if (editable && ImGui::SmallButton("Clear")) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->ClearMemberLoad(ml.no_batang);
            if (on_geometry_changed) on_geometry_changed();
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void DrawJointLoadsTable(const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
                          const std::function<void()>& on_geometry_changed) {
    ImGui::SeparatorText("Joint Loads (Fx,Fy,Fz [N] / Mx,My,Mz [Nm])");
    if (scene.joint_loads.empty()) {
        ImGui::TextDisabled("None. Use Loads > Add Joint Load, then click a joint in the Viewport.");
        return;
    }
    if (!ImGui::BeginTable("joint_loads", 8, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) return;
    ImGui::TableSetupColumn("Joint", ImGuiTableColumnFlags_WidthFixed, 50.f);
    ImGui::TableSetupColumn("Fx");
    ImGui::TableSetupColumn("Fy");
    ImGui::TableSetupColumn("Fz");
    ImGui::TableSetupColumn("Mx");
    ImGui::TableSetupColumn("My");
    ImGui::TableSetupColumn("Mz");
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableHeadersRow();

    static const char* kFieldIds[6] = {"##fx", "##fy", "##fz", "##mx", "##my", "##mz"};

    for (const JointLoadVisual& jl : scene.joint_loads) {
        ImGui::PushID(jl.no_joint);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (ImGui::Selectable(std::to_string(jl.no_joint).c_str(),
                               selection.kind == SelectionKind::Joint && selection.id == jl.no_joint,
                               ImGuiSelectableFlags_SpanAllColumns)) {
            selection = {SelectionKind::Joint, jl.no_joint};
        }

        float actions[6];
        for (int i = 0; i < 6; ++i) actions[i] = jl.actions[i];
        bool any_committed = false;
        for (int i = 0; i < 6; ++i) {
            ImGui::TableSetColumnIndex(1 + i);
            ImGui::SetNextItemWidth(-1.f);
            ImGui::InputFloat(kFieldIds[i], &actions[i]);
            if (ImGui::IsItemDeactivatedAfterEdit()) any_committed = true;
        }
        if (any_committed && editable) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->SetJointLoad(jl.no_joint, actions);
            if (on_geometry_changed) on_geometry_changed();
        }

        ImGui::TableSetColumnIndex(7);
        if (editable && ImGui::SmallButton("Clear")) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->ClearJointLoad(jl.no_joint);
            if (on_geometry_changed) on_geometry_changed();
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

} // namespace

void LoadsPanel::Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
                       UndoStack* undo, const std::function<void()>& on_geometry_changed) {
    if (!ImGui::Begin(PanelWindowTitle(kLoadsId, "Loads").c_str(), open)) {
        ImGui::End();
        return;
    }
    if (scene.Empty()) {
        ImGui::TextDisabled("No dataset loaded.");
        ImGui::End();
        return;
    }

    DrawMemberLoadsTable(scene, selection, editable, undo, on_geometry_changed);
    ImGui::Spacing();
    DrawJointLoadsTable(scene, selection, editable, undo, on_geometry_changed);

    ImGui::End();
}

} // namespace orcisf::gui
