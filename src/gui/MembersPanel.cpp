#include "gui/MembersPanel.h"

#include <string>

#include <imgui.h>

#include "gui/PanelTitles.h"

namespace orcisf::gui {

namespace {

void DrawMembersTable(const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
                       const std::function<void()>& on_geometry_changed) {
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

} // namespace

void MembersPanel::Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
                         UndoStack* undo, const std::function<void()>& on_geometry_changed) {
    if (!ImGui::Begin(PanelWindowTitle(kMembersId, "Members").c_str(), open)) {
        ImGui::End();
        return;
    }
    if (scene.members.empty()) {
        ImGui::TextDisabled("No dataset loaded, or no members yet.");
        ImGui::End();
        return;
    }

    DrawMembersTable(scene, selection, editable, undo, on_geometry_changed);

    ImGui::End();
}

} // namespace orcisf::gui
