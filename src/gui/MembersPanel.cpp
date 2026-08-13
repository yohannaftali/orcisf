#include "gui/MembersPanel.h"

#include <string>

#include <imgui.h>

#include "gui/PanelTitles.h"
#include "gui/TableView.h"

namespace orcisf::gui {

namespace {

// Issue #39: 0/1/2 = Auto/Beam/Column, matching EditableStructure's
// SetMemberTypeOverride() convention exactly -- kept as one fixed 3-item
// list (not a dynamic "Auto (Beam)"/"Auto (Column)" label) so the combo's
// item count/order never changes frame-to-frame regardless of geometry.
constexpr const char* kTypeModeItems[3] = {"Auto", "Beam", "Column"};

void DrawMembersTable(const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
                       const std::function<void()>& on_geometry_changed) {
    if (!BeginTableView("members_list", 5)) return;
    ImGui::TableSetupColumn("Batang", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 110.f);
    ImGui::TableSetupColumn("Joint A");
    ImGui::TableSetupColumn("Joint B");
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableHeadersRow();

    for (const MemberVisual& mv : scene.members) {
        ImGui::PushID(mv.no_batang);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        // Issue #41/#52: TableRowSelectable() carries forward the
        // AllowOverlap + explicit-height fix (see gui/TableView.h) so
        // this table's editable Type/Joint A/B cells stay clickable.
        if (TableRowSelectable(std::to_string(mv.no_batang).c_str(),
                                selection.kind == SelectionKind::Member && selection.id == mv.no_batang)) {
            selection = {SelectionKind::Member, mv.no_batang};
        }

        ImGui::TableSetColumnIndex(1);
        if (editable) {
            int mode = editable->GetMemberTypeOverride(mv.no_batang);
            ImGui::SetNextItemWidth(-1.f);
            if (ImGui::Combo("##type", &mode, kTypeModeItems, 3)) {
                if (undo) undo->PushUndo(editable->SdForUndo());
                editable->SetMemberTypeOverride(mv.no_batang, mode);
                if (on_geometry_changed) on_geometry_changed();
            }
        } else {
            ImGui::TextUnformatted(mv.is_beam ? "Beam" : "Column");
        }

        // Issue #39: Joint A / Joint B are now editable -- typing a joint
        // number reconnects the member via SetMemberEndpoints(), which
        // validates and no-ops (leaving JJ/JK untouched) on an
        // out-of-range/duplicate value rather than corrupting the arrays;
        // an invalid in-progress edit just visually snaps back to the
        // last-valid value next frame, since this cell's InputInt is
        // always re-seeded from the actual sd_.JJ/JK, never a local buffer.
        ImGui::TableSetColumnIndex(2);
        {
            int ja = editable ? editable->SdForUndo().JJ[mv.no_batang] : 0;
            ImGui::SetNextItemWidth(-1.f);
            ImGui::InputInt("##ja", &ja, 0, 0);
            if (editable && ImGui::IsItemDeactivatedAfterEdit()) {
                int jb = editable->SdForUndo().JK[mv.no_batang];
                if (undo) undo->PushUndo(editable->SdForUndo());
                if (editable->SetMemberEndpoints(mv.no_batang, ja, jb) && on_geometry_changed) on_geometry_changed();
            }
        }
        ImGui::TableSetColumnIndex(3);
        {
            int jb = editable ? editable->SdForUndo().JK[mv.no_batang] : 0;
            ImGui::SetNextItemWidth(-1.f);
            ImGui::InputInt("##jb", &jb, 0, 0);
            if (editable && ImGui::IsItemDeactivatedAfterEdit()) {
                int ja = editable->SdForUndo().JJ[mv.no_batang];
                if (undo) undo->PushUndo(editable->SdForUndo());
                if (editable->SetMemberEndpoints(mv.no_batang, ja, jb) && on_geometry_changed) on_geometry_changed();
            }
        }

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

} // namespace

void MembersPanel::Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
                         UndoStack* undo, const std::function<void()>& on_geometry_changed) {
    if (!ImGui::Begin(PanelWindowTitle(kMembersId, "Members").c_str(), open)) {
        ImGui::End();
        return;
    }

    // Issue #39: "+ Add Member", top-right, above the table header --
    // inserts a real member (via AddMember(), never a "pending" row the
    // fixed-size legacy JJ/JK arrays have no concept of) connecting the
    // last two existing joints, which the user then retargets/retypes in
    // the new row below. Needs >=2 joints to have two distinct endpoints
    // to default to -- AddMember() itself already rejects joint_a==joint_b.
    bool can_add = editable && editable->SdForUndo().NJ >= 2;
    const char* add_label = "+ Add Member";
    float add_w = ImGui::CalcTextSize(add_label).x + ImGui::GetStyle().FramePadding.x * 2.f;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - add_w - ImGui::GetStyle().WindowPadding.x);
    ImGui::BeginDisabled(!can_add);
    if (ImGui::Button(add_label)) {
        const engine::StructureData& sd = editable->SdForUndo();
        int ja = sd.NJ - 1;
        int jb = sd.NJ;
        if (undo) undo->PushUndo(sd);
        int new_id = editable->AddMember(ja, jb);
        if (new_id > 0) {
            selection = {SelectionKind::Member, new_id};
            if (on_geometry_changed) on_geometry_changed();
        }
    }
    ImGui::EndDisabled();
    if (!can_add && ImGui::IsItemHovered()) ImGui::SetTooltip("Add at least 2 joints first");

    if (scene.members.empty()) {
        ImGui::TextDisabled("No dataset loaded, or no members yet.");
        ImGui::End();
        return;
    }

    DrawMembersTable(scene, selection, editable, undo, on_geometry_changed);

    ImGui::End();
}

} // namespace orcisf::gui
