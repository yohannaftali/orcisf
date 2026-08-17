#include "gui/ResultsPanel.h"

#include <string>

#include <imgui.h>

#include "gui/PanelTitles.h"
#include "gui/TableView.h"

namespace orcisf::gui {

namespace {

void DrawDisplacementsTable(const engine::AnalysisResults& analysis, Selection& selection) {
    if (!BeginTableView("results_displacements", 7)) return;
    ImGui::TableSetupColumn("Joint", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableSetupColumn("UX (m)");
    ImGui::TableSetupColumn("UY (m)");
    ImGui::TableSetupColumn("UZ (m)");
    ImGui::TableSetupColumn("RX (rad)");
    ImGui::TableSetupColumn("RY (rad)");
    ImGui::TableSetupColumn("RZ (rad)");
    ImGui::TableHeadersRow();

    for (const engine::JointDisplacement& d : analysis.displacements) {
        ImGui::PushID(d.no_joint);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (TableRowSelectable(std::to_string(d.no_joint).c_str(),
                                selection.kind == SelectionKind::Joint && selection.id == d.no_joint)) {
            selection = {SelectionKind::Joint, d.no_joint};
        }
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.6g", d.ux);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.6g", d.uy);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%.6g", d.uz);
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%.6g", d.rx);
        ImGui::TableSetColumnIndex(5);
        ImGui::Text("%.6g", d.ry);
        ImGui::TableSetColumnIndex(6);
        ImGui::Text("%.6g", d.rz);
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void DrawMemberForcesTable(const engine::AnalysisResults& analysis, Selection& selection) {
    if (!BeginTableView("results_member_forces", 13)) return;
    ImGui::TableSetupColumn("Member", ImGuiTableColumnFlags_WidthFixed, 60.f);
    for (const char* label : {"N_a (N)", "Vy_a (N)", "Vz_a (N)", "T_a (Nm)", "My_a (Nm)", "Mz_a (Nm)",
                               "N_b (N)", "Vy_b (N)", "Vz_b (N)", "T_b (Nm)", "My_b (Nm)", "Mz_b (Nm)"}) {
        ImGui::TableSetupColumn(label);
    }
    ImGui::TableHeadersRow();

    for (const engine::MemberForces& f : analysis.member_forces) {
        ImGui::PushID(f.no_batang);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (TableRowSelectable(std::to_string(f.no_batang).c_str(),
                                selection.kind == SelectionKind::Member && selection.id == f.no_batang)) {
            selection = {SelectionKind::Member, f.no_batang};
        }
        const float values[12] = {f.axial_a,  f.shear_y_a, f.shear_z_a, f.torsion_a, f.moment_y_a, f.moment_z_a,
                                   f.axial_b,  f.shear_y_b, f.shear_z_b, f.torsion_b, f.moment_y_b, f.moment_z_b};
        for (int c = 0; c < 12; ++c) {
            ImGui::TableSetColumnIndex(c + 1);
            ImGui::Text("%.6g", values[c]);
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void DrawReactionsTable(const engine::AnalysisResults& analysis) {
    if (!BeginTableView("results_reactions", 7)) return;
    ImGui::TableSetupColumn("Joint", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableSetupColumn("Fx (N)");
    ImGui::TableSetupColumn("Fy (N)");
    ImGui::TableSetupColumn("Fz (N)");
    ImGui::TableSetupColumn("Mx (Nm)");
    ImGui::TableSetupColumn("My (Nm)");
    ImGui::TableSetupColumn("Mz (Nm)");
    ImGui::TableHeadersRow();

    for (const engine::JointReaction& r : analysis.reactions) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", r.no_joint);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.6g", r.fx);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.6g", r.fy);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%.6g", r.fz);
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%.6g", r.mx);
        ImGui::TableSetColumnIndex(5);
        ImGui::Text("%.6g", r.my);
        ImGui::TableSetColumnIndex(6);
        ImGui::Text("%.6g", r.mz);
    }
    ImGui::EndTable();
}

void DrawEquilibriumTable(const engine::AnalysisResults& analysis) {
    if (!analysis.has_applied_load) {
        ImGui::TextDisabled(
            "Not available -- these results were loaded from a saved .str file, which does not "
            "record applied loads. Run the optimizer to see the equilibrium check.");
        return;
    }
    static const char* kLabels[6] = {"Fx", "Fy", "Fz", "Mx", "My", "Mz"};
    if (!BeginTableView("results_equilibrium", 4)) return;
    ImGui::TableSetupColumn("DOF", ImGuiTableColumnFlags_WidthFixed, 50.f);
    ImGui::TableSetupColumn("Total applied load");
    ImGui::TableSetupColumn("Total reaction");
    ImGui::TableSetupColumn("Residual (should be ~0)");
    ImGui::TableHeadersRow();

    for (int dof = 0; dof < 6; ++dof) {
        float applied = analysis.total_applied_load[dof];
        float reaction = analysis.TotalReaction(dof);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(kLabels[dof]);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.6g", applied);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.6g", reaction);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%.6g", applied + reaction);
    }
    ImGui::EndTable();
}

} // namespace

void ResultsPanel::Draw(bool* open, const SceneModel& scene, Selection& selection,
                         const engine::AnalysisResults* analysis) {
    (void)scene;
    if (!ImGui::Begin(PanelWindowTitle(kResultsId, "Results").c_str(), open)) {
        ImGui::End();
        return;
    }
    if (!analysis) {
        ImGui::TextDisabled("Run an optimization first to compute joint displacements, member forces, and reactions.");
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Global Equilibrium Check");
    DrawEquilibriumTable(*analysis);

    ImGui::SeparatorText("Joint Displacements");
    DrawDisplacementsTable(*analysis, selection);

    ImGui::SeparatorText("Member End Forces");
    DrawMemberForcesTable(*analysis, selection);

    ImGui::SeparatorText("Support Reactions");
    DrawReactionsTable(*analysis);

    ImGui::End();
}

} // namespace orcisf::gui
