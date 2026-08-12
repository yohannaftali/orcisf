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
        // Issue #41: AllowOverlap + explicit height, see JointsPanel.cpp's
        // comment on this exact pattern (AllowOverlap is the fix that
        // actually matters -- without it SpanAllColumns unconditionally
        // blocks every later-column widget's clicks, confirmed by direct
        // ActiveID instrumentation, not assumed). This table has no
        // InputFloat cells yet (Type/Joint A/B are read-only Text today),
        // so today this is a preemptive, currently-inert fix; issue #39
        // adds editable fields to this same row and must not reintroduce
        // the click-blocking bug.
        if (ImGui::Selectable(std::to_string(mv.no_batang).c_str(), selection.kind == SelectionKind::Member &&
                                                                          selection.id == mv.no_batang,
                               ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
                               ImVec2(0.f, ImGui::GetFrameHeight()))) {
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
