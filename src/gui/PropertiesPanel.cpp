#include "gui/PropertiesPanel.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

namespace orcisf::gui {

void PropertiesPanel::Draw(bool* open, const SceneModel& scene, int selected_member) {
    if (!ImGui::Begin("Properties", open)) {
        ImGui::End();
        return;
    }

    if (selected_member < 0) {
        ImGui::TextDisabled("Click a member in the Viewport to see its data here.");
        ImGui::End();
        return;
    }

    auto it = std::find_if(scene.members.begin(), scene.members.end(),
                            [selected_member](const MemberVisual& mv) { return mv.no_batang == selected_member; });
    if (it == scene.members.end()) {
        ImGui::TextDisabled("Selected member is no longer in the loaded dataset.");
        ImGui::End();
        return;
    }
    const MemberVisual& mv = *it;

    ImGui::Text("Batang %d (%s)", mv.no_batang, mv.is_beam ? "Balok" : "Kolom");
    ImGui::Separator();
    ImGui::Text("Panjang: %.3f m", (mv.b - mv.a).Length());
    ImGui::Text("Dimensi: %.0f x %.0f mm", mv.width_m * 1000.f, mv.height_m * 1000.f);

    if (!mv.has_results) {
        ImGui::Spacing();
        ImGui::TextDisabled("No design results yet -- run an optimization to see forces/capacity/constraints.");
        ImGui::End();
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

    ImGui::End();
}

} // namespace orcisf::gui
