#include "gui/JointsPanel.h"

#include <string>

#include <imgui.h>

#include "gui/PanelTitles.h"
#include "gui/TableView.h"

namespace orcisf::gui {

namespace {

// Issue #55: same order/convention as PropertiesPanel.cpp's kDofLabels
// (issue #40) -- kept in sync deliberately, since both surfaces edit the
// exact same EditableStructure::SetJointDof()/GetJointDof() state and
// must never disagree about which column is which DOF.
constexpr const char* kDofLabels[6] = {"UX", "UY", "UZ", "RX", "RY", "RZ"};
constexpr const char* kDofIds[6] = {"##ux", "##uy", "##uz", "##rx", "##ry", "##rz"};

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
    // Issue #55: the single "Restrained" summary checkbox is replaced by
    // 6 individual DOF columns, matching PropertiesPanel's per-DOF
    // editing (issue #40) exactly. Each DOF column is a narrow WidthFixed
    // column (just enough for the short "UX"-style header + a checkbox)
    // rather than participating in #52's default proportional-stretch
    // sizing -- letting 6 short columns stretch would waste most of the
    // table's width on empty space around tiny checkboxes, at the direct
    // expense of the X/Y/Z columns that actually benefit from stretching.
    if (!BeginTableView("joints_list", 11)) return;
    ImGui::TableSetupColumn("Joint", ImGuiTableColumnFlags_WidthFixed, 50.f);
    ImGui::TableSetupColumn("X (m)");
    ImGui::TableSetupColumn("Y (m)");
    ImGui::TableSetupColumn("Z (m)");
    const float dof_col_width = ImGui::GetFrameHeight() * 1.4f;
    for (const char* label : kDofLabels) {
        ImGui::TableSetupColumn(label, ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize,
                                 dof_col_width);
    }
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize,
                             TableTrashColumnWidth());
    ImGui::TableHeadersRow();

    static const char* kFieldIds[3] = {"##x", "##y", "##z"};

    for (const JointVisual& jv : scene.joints) {
        ImGui::PushID(jv.no_joint);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        // Issue #41/#52: TableRowSelectable() carries forward the
        // AllowOverlap + explicit-height fix (see gui/TableView.h) so
        // later-column widgets (InputFloat here) stay clickable.
        if (TableRowSelectable(std::to_string(jv.no_joint).c_str(),
                                selection.kind == SelectionKind::Joint && selection.id == jv.no_joint)) {
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

        // Issue #55: reads/writes the exact same EditableStructure::
        // GetJointDof()/SetJointDof() (issue #40) PropertiesPanel's own 6
        // checkboxes use -- checked fresh from JRL every frame here too,
        // so this table and the Properties panel can never drift out of
        // sync with each other for the currently-selected joint.
        for (int i = 0; i < 6; ++i) {
            ImGui::TableSetColumnIndex(4 + i);
            CenterNextTableItem(ImGui::GetFrameHeight());
            bool v = editable && editable->GetJointDof(jv.no_joint, i);
            if (editable && ImGui::Checkbox(kDofIds[i], &v)) {
                if (undo) undo->PushUndo(editable->SdForUndo());
                editable->SetJointDof(jv.no_joint, i, v);
                if (on_geometry_changed) on_geometry_changed();
            }
        }

        ImGui::TableSetColumnIndex(10);
        if (editable && TableTrashButton("##delete")) {
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

void JointsPanel::Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
                        UndoStack* undo, const std::function<void()>& on_geometry_changed) {
    if (!ImGui::Begin(PanelWindowTitle(kJointsId, "Joints").c_str(), open)) {
        ImGui::End();
        return;
    }

    // Issue #39: "+ Add Joint", top-right, above the table header --
    // mirrors MembersPanel's button. A second, table-driven entry point
    // alongside the existing toolbar/viewport-click Add Joint flow
    // (#14/#18); doesn't replace it. Defaults to the current joints'
    // centroid (or the origin for an empty structure) rather than a fixed
    // world position, so it lands somewhere relevant to what's already
    // there -- same "should produce something visible/editable, not a
    // silent no-op" reasoning issue #7 used for load placement defaults.
    {
        const char* add_label = "+ Add Joint";
        float add_w = ImGui::CalcTextSize(add_label).x + ImGui::GetStyle().FramePadding.x * 2.f;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - add_w - ImGui::GetStyle().WindowPadding.x);
        ImGui::BeginDisabled(!editable);
        if (ImGui::Button(add_label)) {
            math3d::Vec3 centroid{0.f, 0.f, 0.f};
            if (!scene.joints.empty()) {
                for (const JointVisual& jv : scene.joints) centroid = centroid + jv.pos;
                centroid = centroid * (1.f / static_cast<float>(scene.joints.size()));
            }
            if (undo) undo->PushUndo(editable->SdForUndo());
            int new_id = editable->AddJoint(centroid);
            if (new_id > 0) {
                selection = {SelectionKind::Joint, new_id};
                if (on_geometry_changed) on_geometry_changed();
            }
        }
        ImGui::EndDisabled();
    }

    if (scene.joints.empty()) {
        ImGui::TextDisabled("No dataset loaded, or no joints yet.");
        ImGui::End();
        return;
    }

    bool open_delete_popup = false;
    DrawJointsTable(scene, selection, editable, undo, on_geometry_changed, pending_delete_joint_,
                    pending_delete_joint_members_, open_delete_popup);

    if (open_delete_popup) ImGui::OpenPopup("Delete Joint##confirm");
    DrawPendingDeleteModal(selection, editable, undo, on_geometry_changed, pending_delete_joint_,
                            pending_delete_joint_members_);

    ImGui::End();
}

} // namespace orcisf::gui
