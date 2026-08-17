#include "gui/ForceDiagramPanel.h"

#include <algorithm>
#include <string>
#include <vector>

#include <imgui.h>
#include <implot.h>

#include "gui/PanelTitles.h"
#include "gui/diagrams/ForceDiagram.h"

namespace orcisf::gui {

namespace {

// One small ImPlot line plot for a single diagram (N, V, M, or T),
// stacked vertically -- simpler and more robust than ImPlot::
// BeginSubplots for just 4 fixed rows, and each keeps its own
// independent Y autoscale (a tiny torsion value next to a large moment
// value would otherwise squash the torsion line to a flat-looking line).
void DrawOnePlot(const char* title, const char* y_label, const std::vector<double>& xs,
                  const std::vector<double>& ys, float height, double* drag_x) {
    if (ImPlot::BeginPlot(title, ImVec2(-1, height), ImPlotFlags_NoLegend)) {
        ImPlot::SetupAxes("x (m)", y_label, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::PlotShaded("##fill", xs.data(), ys.data(), static_cast<int>(xs.size()), 0.0);
        ImPlot::PlotLine("##line", xs.data(), ys.data(), static_cast<int>(xs.size()));
        ImPlot::DragLineX(0, drag_x, ImVec4(1.f, 0.82f, 0.15f, 1.f), 1.5f);
        ImPlot::EndPlot();
    }
}

} // namespace

void ForceDiagramPanel::Draw(bool* open, const SceneModel& scene, const Selection& selection,
                              const engine::AnalysisResults* analysis) {
    if (!ImGui::Begin(PanelWindowTitle(kForceDiagramId, "Force Diagrams").c_str(), open)) {
        ImGui::End();
        return;
    }

    if (!analysis) {
        ImGui::TextDisabled("Run an optimization first to compute member forces.");
        ImGui::End();
        return;
    }
    if (selection.kind != SelectionKind::Member) {
        ImGui::TextDisabled("Select a member in the Viewport to see its N/V/M/T force diagrams.");
        ImGui::End();
        return;
    }

    auto mv_it = std::find_if(scene.members.begin(), scene.members.end(),
                               [&](const MemberVisual& mv) { return mv.no_batang == selection.id; });
    if (mv_it == scene.members.end()) {
        ImGui::TextDisabled("Selected member is no longer in the loaded dataset.");
        ImGui::End();
        return;
    }
    auto mf_it = std::find_if(analysis->member_forces.begin(), analysis->member_forces.end(),
                               [&](const engine::MemberForces& f) { return f.no_batang == selection.id; });
    if (mf_it == analysis->member_forces.end()) {
        ImGui::TextDisabled("No analysis results for this member.");
        ImGui::End();
        return;
    }

    float length_m = (mv_it->b - mv_it->a).Length();
    // Only pass a real section to the stress calculation when this member
    // actually has a completed design -- MemberVisual::width_m/height_m
    // otherwise default to a placeholder 0.3m regardless of has_results,
    // which would silently produce a meaningless stress number.
    float width_mm = mv_it->has_results ? mv_it->width_m * 1000.f : 0.f;
    float height_mm = mv_it->has_results ? mv_it->height_m * 1000.f : 0.f;

    ForceDiagram diagram = ComputeForceDiagram(*mf_it, length_m, width_mm, height_mm, 61);

    ImGui::Text("Batang %d (%s) -- length %.2f m", selection.id, mv_it->is_beam ? "Balok" : "Kolom", length_m);
    ImGui::Separator();

    std::vector<double> xs, n_ys, v_ys, m_ys, t_ys;
    xs.reserve(diagram.samples.size());
    n_ys.reserve(diagram.samples.size());
    v_ys.reserve(diagram.samples.size());
    m_ys.reserve(diagram.samples.size());
    t_ys.reserve(diagram.samples.size());
    for (const ForceDiagramSample& s : diagram.samples) {
        xs.push_back(s.x_m);
        n_ys.push_back(s.n_n);
        v_ys.push_back(s.v_n);
        m_ys.push_back(s.m_nm);
        t_ys.push_back(s.t_nm);
    }

    double drag_x = static_cast<double>(std::clamp(station_t_, 0.f, 1.f) * length_m);
    float plot_h = std::max(90.f, (ImGui::GetContentRegionAvail().y - 160.f) * 0.25f);

    DrawOnePlot("Axial (N)", "N (N)", xs, n_ys, plot_h, &drag_x);
    DrawOnePlot("Shear (V, local y)", "V (N)", xs, v_ys, plot_h, &drag_x);
    DrawOnePlot("Moment (M, local z)", "M (Nm)", xs, m_ys, plot_h, &drag_x);
    DrawOnePlot("Torsion (T)", "T (Nm)", xs, t_ys, plot_h, &drag_x);

    station_t_ = length_m > 1e-6f ? std::clamp(static_cast<float>(drag_x) / length_m, 0.f, 1.f) : 0.f;

    ImGui::Separator();
    ImGui::SliderFloat("Station", &station_t_, 0.f, 1.f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp);

    // Nearest-sample readout (61 stations is dense enough that this is
    // visually indistinguishable from an exact analytic re-evaluation for
    // an inspection aid, without duplicating ComputeForceDiagram's formula
    // here).
    size_t idx = diagram.samples.empty()
                     ? 0
                     : std::min(diagram.samples.size() - 1,
                                static_cast<size_t>(std::lround(station_t_ * static_cast<float>(diagram.samples.size() - 1))));
    if (!diagram.samples.empty()) {
        const ForceDiagramSample& s = diagram.samples[idx];
        ImGui::Text("x = %.3f m", s.x_m);
        ImGui::Text("N = %.4g N   V = %.4g N   M = %.4g Nm   T = %.4g Nm", s.n_n, s.v_n, s.m_nm, s.t_nm);
        if (diagram.has_section) {
            ImGui::Text("Extreme-fiber stress: top %.4g MPa, bottom %.4g MPa", s.sigma_top_mpa, s.sigma_bottom_mpa);
        } else {
            ImGui::TextDisabled("No completed design for this member -- extreme-fiber stress unavailable.");
        }
    }

    ImGui::End();
}

} // namespace orcisf::gui
