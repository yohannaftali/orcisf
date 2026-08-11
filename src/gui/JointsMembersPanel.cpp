#include "gui/JointsMembersPanel.h"

#include <string>

#include <imgui.h>

#include "gui/PanelTitles.h"

namespace orcisf::gui {

namespace {

// Members whose JJ or JK reference joint_id -- used both to size the
// confirmation modal's warning text and (indirectly, since
// EditableStructure::DeleteJoint() recomputes this itself) to decide
// whether a modal is needed at all.
std::vector<int> TouchingMembers(const engine::StructureData& sd, int joint_id) {
    std::vector<int> touching;
    for (int i = 1; i <= sd.M; ++i) {
        if (sd.JJ[i] == joint_id || sd.JK[i] == joint_id) touching.push_back(i);
    }
    return touching;
}

void DrawJointsTable(const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
                      const std::function<void()>& on_geometry_changed, int& pending_delete_joint,
                      std::vector<int>& pending_delete_joint_members, bool& open_delete_popup) {
    ImGui::SeparatorText("Joints");
    if (scene.joints.empty()) {
        ImGui::TextDisabled("None.");
        return;
    }
    if (!ImGui::BeginTable("joints_list", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) return;
    ImGui::TableSetupColumn("Joint", ImGuiTableColumnFlags_WidthFixed, 50.f);
    ImGui::TableSetupColumn("X (m)");
    ImGui::TableSetupColumn("Y (m)");
    ImGui::TableSetupColumn("Z (m)");
    ImGui::TableSetupColumn("Restrained", ImGuiTableColumnFlags_WidthFixed, 80.f);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableHeadersRow();

    static const char* kFieldIds[3] = {"##x", "##y", "##z"};

    for (const JointVisual& jv : scene.joints) {
        ImGui::PushID(jv.no_joint);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (ImGui::Selectable(std::to_string(jv.no_joint).c_str(),
                               selection.kind == SelectionKind::Joint && selection.id == jv.no_joint,
                               ImGuiSelectableFlags_SpanAllColumns)) {
            selection = {SelectionKind::Joint, jv.no_joint};
        }

        float pos[3] = {jv.pos.x, jv.pos.y, jv.pos.z};
        bool any_committed = false;
        for (int i = 0; i < 3; ++i) {
            ImGui::TableSetColumnIndex(1 + i);
            ImGui::SetNextItemWidth(-1.f);
            ImGui::InputFloat(kFieldIds[i], &pos[i]);
            if (ImGui::IsItemDeactivatedAfterEdit()) any_committed = true;
        }
        if (any_committed && editable) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->MoveJoint(jv.no_joint, math3d::Vec3{pos[0], pos[1], pos[2]});
            if (on_geometry_changed) on_geometry_changed();
        }

        ImGui::TableSetColumnIndex(4);
        bool restrained = jv.restrained;
        if (editable && ImGui::Checkbox("##restrained", &restrained)) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->SetJointRestrained(jv.no_joint, restrained);
            if (on_geometry_changed) on_geometry_changed();
        }

        ImGui::TableSetColumnIndex(5);
        if (editable && ImGui::SmallButton("Delete")) {
            std::vector<int> touching = TouchingMembers(editable->SdForUndo(), jv.no_joint);
            if (touching.empty()) {
                if (undo) undo->PushUndo(editable->SdForUndo());
                editable->DeleteJoint(jv.no_joint);
                if (selection.kind == SelectionKind::Joint && selection.id == jv.no_joint) selection.Clear();
                if (on_geometry_changed) on_geometry_changed();
            } else {
                pending_delete_joint = jv.no_joint;
                pending_delete_joint_members = std::move(touching);
                // Deferred to outside this row's PushID scope below --
                // OpenPopup here would hash the popup ID together with
                // PushID(jv.no_joint), which BeginPopupModal (called from
                // an unrelated ID scope) would never match.
                open_delete_popup = true;
            }
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void DrawMembersTable(const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
                       const std::function<void()>& on_geometry_changed) {
    ImGui::SeparatorText("Members");
    if (scene.members.empty()) {
        ImGui::TextDisabled("None.");
        return;
    }
    if (!ImGui::BeginTable("members_list", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) return;
    ImGui::TableSetupColumn("Batang", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 70.f);
    ImGui::TableSetupColumn("Joint A", ImGuiTableColumnFlags_WidthFixed, 70.f);
    ImGui::TableSetupColumn("Joint B", ImGuiTableColumnFlags_WidthFixed, 70.f);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableHeadersRow();

    const engine::StructureData* sd = editable ? &editable->SdForUndo() : nullptr;

    for (const MemberVisual& mv : scene.members) {
        ImGui::PushID(mv.no_batang);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (ImGui::Selectable(std::to_string(mv.no_batang).c_str(), selection.kind == SelectionKind::Member &&
                                                                          selection.id == mv.no_batang,
                               ImGuiSelectableFlags_SpanAllColumns)) {
            selection = {SelectionKind::Member, mv.no_batang};
        }
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(mv.is_beam ? "Beam" : "Column");
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%d", sd ? sd->JJ[mv.no_batang] : 0);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%d", sd ? sd->JK[mv.no_batang] : 0);
        ImGui::TableSetColumnIndex(4);
        if (editable && ImGui::SmallButton("Delete")) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->DeleteMember(mv.no_batang);
            if (selection.kind == SelectionKind::Member && selection.id == mv.no_batang) selection.Clear();
            if (on_geometry_changed) on_geometry_changed();
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

// Cascade-delete confirmation modal -- shown instead of deleting a joint
// immediately whenever EditableStructure::DeleteJoint() would also take
// members down with it (it always cascades; this is purely the warning
// step in front of that pre-existing behavior, see the header comment).
void DrawPendingDeleteModal(Selection& selection, EditableStructure* editable, UndoStack* undo,
                             const std::function<void()>& on_geometry_changed, int& pending_delete_joint,
                             std::vector<int>& pending_delete_joint_members) {
    if (!ImGui::BeginPopupModal("Delete Joint##confirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

    ImGui::Text("Joint %d is connected to %zu member(s):", pending_delete_joint,
                pending_delete_joint_members.size());
    std::string member_list;
    for (size_t i = 0; i < pending_delete_joint_members.size(); ++i) {
        if (i > 0) member_list += ", ";
        member_list += std::to_string(pending_delete_joint_members[i]);
    }
    ImGui::TextWrapped("%s", member_list.c_str());
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.f, 0.6f, 0.3f, 1.f), "Deleting this joint will also delete those members.");
    ImGui::Spacing();

    if (ImGui::Button("Delete Joint + Members", ImVec2(180.f, 0.f))) {
        if (editable) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->DeleteJoint(pending_delete_joint);
            if (selection.kind == SelectionKind::Joint && selection.id == pending_delete_joint) selection.Clear();
            if (on_geometry_changed) on_geometry_changed();
        }
        pending_delete_joint = -1;
        pending_delete_joint_members.clear();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(100.f, 0.f))) {
        pending_delete_joint = -1;
        pending_delete_joint_members.clear();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

} // namespace

void JointsMembersPanel::Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
                               UndoStack* undo, const std::function<void()>& on_geometry_changed) {
    if (!ImGui::Begin(PanelWindowTitle(kJointsMembersId, "Joints/Members").c_str(), open)) {
        ImGui::End();
        return;
    }
    if (scene.Empty()) {
        ImGui::TextDisabled("No dataset loaded.");
        ImGui::End();
        return;
    }

    bool open_delete_popup = false;
    DrawJointsTable(scene, selection, editable, undo, on_geometry_changed, pending_delete_joint_,
                    pending_delete_joint_members_, open_delete_popup);
    ImGui::Spacing();
    DrawMembersTable(scene, selection, editable, undo, on_geometry_changed);

    if (open_delete_popup) ImGui::OpenPopup("Delete Joint##confirm");
    DrawPendingDeleteModal(selection, editable, undo, on_geometry_changed, pending_delete_joint_,
                            pending_delete_joint_members_);

    ImGui::End();
}

} // namespace orcisf::gui
