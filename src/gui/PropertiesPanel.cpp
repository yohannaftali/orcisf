#include "gui/PropertiesPanel.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <imgui.h>

#include "engine/LegacyIO.h"
#include "gui/PanelTitles.h"

namespace orcisf::gui {

namespace {

// Issue #11: title + material property inputs and read-only auto-calculated
// structural parameters, shown when nothing is selected. All fields already
// exist on engine::StructureData (ISN/E/G/FC/FY/FYS, M/NJ) -- this is
// purely a missing form, no new engine data. The read-only fields reuse
// engine::ComputeRestraintSummary() (the same derivation
// WriteStructureFile()/WriteInfPreview() use) rather than trusting sd's own
// NR/NRJ/N, which the GUI editor never maintains as joints/restraints
// change.
void DrawGeneralProperties(EditableStructure* editable) {
    if (!editable) {
        ImGui::TextDisabled("Click a joint or member in the Viewport to see its data here.");
        return;
    }
    engine::StructureData& sd = editable->Sd();

    ImGui::SeparatorText("General");
    char title_buf[80];
    std::snprintf(title_buf, sizeof(title_buf), "%s", sd.ISN.c_str());
    if (ImGui::InputText("Title", title_buf, sizeof(title_buf))) {
        sd.ISN = title_buf;
    }

    ImGui::SeparatorText("Material Properties");
    ImGui::InputFloat("Young's Modulus / E (N/m^2)", &sd.E, 0.f, 0.f, "%.6g");
    ImGui::InputFloat("Shear Modulus / G (N/m^2)", &sd.G, 0.f, 0.f, "%.6g");
    ImGui::InputFloat("f'c (MPa)", &sd.FC, 0.f, 0.f, "%.6g");
    ImGui::InputFloat("fy (MPa)", &sd.FY, 0.f, 0.f, "%.6g");
    ImGui::InputFloat("fyt (MPa)", &sd.FYS, 0.f, 0.f, "%.6g");

    engine::RestraintSummary summary = engine::ComputeRestraintSummary(sd);
    ImGui::SeparatorText("Structural Parameters (auto-calculated)");
    ImGui::BeginDisabled();
    ImGui::Text("Number of members: %d", sd.M);
    ImGui::Text("DOF: %d", summary.n);
    ImGui::Text("Number of joints: %d", sd.NJ);
    ImGui::Text("Number of support restraints: %d", summary.nr);
    ImGui::Text("Number of restrained joints: %d", summary.nrj);
    ImGui::EndDisabled();
}

void DrawValidationSection(const std::vector<std::string>& issues, bool has_dataset) {
    if (!has_dataset) return;
    ImGui::SeparatorText("Validation");
    if (issues.empty()) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.f), "No issues -- OK to run.");
        return;
    }
    ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.35f, 1.f), "%zu issue(s) -- fix before running:", issues.size());
    for (const std::string& issue : issues) {
        ImGui::BulletText("%s", issue.c_str());
    }
}

void DrawJointProperties(const SceneModel& scene, Selection& selection, EditableStructure* editable, UndoStack* undo,
                          const std::function<void()>& on_geometry_changed) {
    auto it = std::find_if(scene.joints.begin(), scene.joints.end(),
                            [&](const JointVisual& jv) { return jv.no_joint == selection.id; });
    if (it == scene.joints.end()) {
        ImGui::TextDisabled("Selected joint is no longer in the loaded dataset.");
        return;
    }
    const JointVisual& jv = *it;

    ImGui::Text("Joint %d", jv.no_joint);
    ImGui::Separator();

    float pos[3] = {jv.pos.x, jv.pos.y, jv.pos.z};
    bool can_edit = editable != nullptr;
    ImGui::BeginDisabled(!can_edit);
    bool edited = ImGui::InputFloat3("Position (m)", pos);
    if (edited && ImGui::IsItemDeactivatedAfterEdit() && can_edit) {
        if (undo) undo->PushUndo(editable->SdForUndo());
        editable->MoveJoint(jv.no_joint, math3d::Vec3{pos[0], pos[1], pos[2]});
        if (on_geometry_changed) on_geometry_changed();
    }

    bool restrained = jv.restrained;
    if (ImGui::Checkbox("Restrained (fixed support)", &restrained) && can_edit) {
        if (undo) undo->PushUndo(editable->SdForUndo());
        editable->SetJointRestrained(jv.no_joint, restrained);
        if (on_geometry_changed) on_geometry_changed();
    }

    ImGui::Spacing();
    if (ImGui::Button("Delete Joint") && can_edit) {
        if (undo) undo->PushUndo(editable->SdForUndo());
        editable->DeleteJoint(jv.no_joint);
        selection.Clear();
        if (on_geometry_changed) on_geometry_changed();
        ImGui::EndDisabled();
        return;
    }

    ImGui::SeparatorText("Load (issue #7)");
    auto load_it = std::find_if(scene.joint_loads.begin(), scene.joint_loads.end(),
                                 [&](const JointLoadVisual& jl) { return jl.no_joint == jv.no_joint; });
    float actions[6] = {0, 0, 0, 0, 0, 0};
    if (load_it != scene.joint_loads.end()) {
        for (int i = 0; i < 6; ++i) actions[i] = load_it->actions[i];
    }
    bool force_edited = ImGui::InputFloat3("Force Fx,Fy,Fz (N)", actions);
    bool force_committed = force_edited && ImGui::IsItemDeactivatedAfterEdit();
    bool moment_edited = ImGui::InputFloat3("Moment Mx,My,Mz (Nm)", actions + 3);
    bool moment_committed = moment_edited && ImGui::IsItemDeactivatedAfterEdit();
    if ((force_committed || moment_committed) && can_edit) {
        if (undo) undo->PushUndo(editable->SdForUndo());
        editable->SetJointLoad(jv.no_joint, actions);
        if (on_geometry_changed) on_geometry_changed();
    }
    if (load_it != scene.joint_loads.end() && ImGui::Button("Clear Load") && can_edit) {
        if (undo) undo->PushUndo(editable->SdForUndo());
        editable->ClearJointLoad(jv.no_joint);
        if (on_geometry_changed) on_geometry_changed();
    }

    ImGui::EndDisabled();
    if (!can_edit) {
        ImGui::TextDisabled("Editing requires a loaded dataset (not just a run-result preview).");
    }
}

void DrawMemberProperties(const SceneModel& scene, Selection& selection, EditableStructure* editable,
                           UndoStack* undo, const std::function<void()>& on_geometry_changed) {
    auto it = std::find_if(scene.members.begin(), scene.members.end(),
                            [&](const MemberVisual& mv) { return mv.no_batang == selection.id; });
    if (it == scene.members.end()) {
        ImGui::TextDisabled("Selected member is no longer in the loaded dataset.");
        return;
    }
    const MemberVisual& mv = *it;

    ImGui::Text("Batang %d (%s)", mv.no_batang, mv.is_beam ? "Balok" : "Kolom");
    ImGui::Separator();
    ImGui::Text("Panjang: %.3f m", (mv.b - mv.a).Length());
    ImGui::Text("Dimensi: %.0f x %.0f mm", mv.width_m * 1000.f, mv.height_m * 1000.f);

    if (editable) {
        ImGui::Spacing();
        if (ImGui::Button("Delete Member")) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->DeleteMember(mv.no_batang);
            selection.Clear();
            if (on_geometry_changed) on_geometry_changed();
            return;
        }

        ImGui::SeparatorText("Load (issue #7)");
        auto load_it = std::find_if(scene.member_loads.begin(), scene.member_loads.end(),
                                     [&](const MemberLoadVisual& ml) { return ml.no_batang == mv.no_batang; });
        float w = (load_it != scene.member_loads.end()) ? load_it->w_n_per_m : 0.f;
        bool edited = ImGui::InputFloat("Distributed load (N/m)", &w);
        if (edited && ImGui::IsItemDeactivatedAfterEdit()) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->SetMemberLoad(mv.no_batang, w);
            if (on_geometry_changed) on_geometry_changed();
        }
        if (load_it != scene.member_loads.end() && ImGui::Button("Clear Load")) {
            if (undo) undo->PushUndo(editable->SdForUndo());
            editable->ClearMemberLoad(mv.no_batang);
            if (on_geometry_changed) on_geometry_changed();
        }
    }

    if (!mv.has_results) {
        ImGui::Spacing();
        ImGui::TextDisabled("No design results yet -- run an optimization to see forces/capacity/constraints.");
        return;
    }

    ImGui::Spacing();
    ImGui::Text("Harga: Rp. %.0f", mv.result.harga);
    if (mv.kendala <= 0.f) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.f), "Kendala terpenuhi");
    } else {
        ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.35f, 1.f), "Kendala terlanggar (%.4g)", mv.kendala);
    }

    if (mv.is_beam) {
        ImGui::SeparatorText("Daerah Lapangan");
        ImGui::Text("Momen: %.4g / %.4g Nmm%s", mv.result.lapangan_MU, mv.result.lapangan_FMU,
                    mv.result.lapangan_MU > mv.result.lapangan_FMU ? "  (!)" : "");
        ImGui::Text("Kendala: %.4g", mv.result.lapangan_kendala);
        ImGui::SeparatorText("Daerah Tumpuan");
        ImGui::Text("Momen: %.4g / %.4g Nmm%s", mv.result.tumpuan_MU, mv.result.tumpuan_FMU,
                    mv.result.tumpuan_MU > mv.result.tumpuan_FMU ? "  (!)" : "");
        ImGui::Text("Kendala: %.4g", mv.result.tumpuan_kendala);
        ImGui::SeparatorText("Lendutan");
        ImGui::Text("Kendala: %.4g", mv.result.kendala_lendutan);
    } else {
        ImGui::SeparatorText("Gaya Aksial");
        ImGui::Text("%.4g / %.4g N%s", mv.result.axial_demand, mv.result.axial_capacity,
                    std::fabs(mv.result.axial_demand) > mv.result.axial_capacity ? "  (!)" : "");
        ImGui::SeparatorText("Momen Biaksial");
        ImGui::Text("Mx: %.4g Nmm   My: %.4g Nmm", mv.result.moment_x_demand, mv.result.moment_y_demand);
        ImGui::Text("Kapasitas momen: %.4g Nmm", mv.result.moment_capacity);
        ImGui::SeparatorText("Kendala");
        ImGui::Text("%.4g", mv.result.kendala_kolom);
    }
}

} // namespace

void PropertiesPanel::Draw(bool* open, const SceneModel& scene, Selection& selection, EditableStructure* editable,
                            UndoStack* undo, const std::vector<std::string>& validation_issues,
                            const std::function<void()>& on_geometry_changed) {
    if (!ImGui::Begin(PanelWindowTitle(kPropertiesId, "Properties").c_str(), open)) {
        ImGui::End();
        return;
    }
    switch (selection.kind) {
        case SelectionKind::Joint:
            DrawJointProperties(scene, selection, editable, undo, on_geometry_changed);
            break;
        case SelectionKind::Member:
            DrawMemberProperties(scene, selection, editable, undo, on_geometry_changed);
            break;
        case SelectionKind::None:
            DrawGeneralProperties(editable);
            break;
    }

    DrawValidationSection(validation_issues, !scene.Empty());

    ImGui::End();
}

} // namespace orcisf::gui
