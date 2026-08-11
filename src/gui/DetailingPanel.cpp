#include "gui/DetailingPanel.h"

#include <algorithm>
#include <string>

#include <imgui.h>

#include "gui/PanelIcons.h"
#include "gui/detailing/DetailingLayout.h"

namespace orcisf::gui {

namespace {

void DrawSection(ImDrawList* dl, const DetailingSection& sec, ImVec2 region_size, ImVec2 origin_screen) {
    float scale = 0.55f * std::min(region_size.x / std::max(1.f, sec.width_mm),
                                    region_size.y / std::max(1.f, sec.height_mm));
    scale = std::max(scale, 0.01f);
    ImVec2 center{origin_screen.x + region_size.x * 0.5f, origin_screen.y + region_size.y * 0.5f};

    auto ToScreen = [&](float x_mm, float y_mm) {
        return ImVec2{center.x + x_mm * scale, center.y - y_mm * scale}; // local +Y up -> screen -Y
    };

    // Concrete outline.
    ImVec2 c0 = ToScreen(-sec.width_mm * 0.5f, sec.height_mm * 0.5f);
    ImVec2 c1 = ToScreen(sec.width_mm * 0.5f, -sec.height_mm * 0.5f);
    dl->AddRectFilled(c0, c1, IM_COL32(58, 58, 64, 255));
    dl->AddRect(c0, c1, IM_COL32(190, 190, 198, 255), 0.f, 0, 2.f);

    // Stirrup outline (inset from the concrete face by cover + half the
    // stirrup diameter, i.e. the stirrup's own centerline).
    float inset = sec.stirrup_inset_mm;
    ImVec2 s0 = ToScreen(-sec.width_mm * 0.5f + inset, sec.height_mm * 0.5f - inset);
    ImVec2 s1 = ToScreen(sec.width_mm * 0.5f - inset, -(sec.height_mm * 0.5f - inset));
    dl->AddRect(s0, s1, IM_COL32(130, 165, 215, 255), 2.f, 0, 1.5f);

    // Main bars.
    for (const RebarCircle& bar : sec.bars) {
        ImVec2 p = ToScreen(bar.x_mm, bar.y_mm);
        float r = std::max(2.5f, bar.diameter_mm * 0.5f * scale);
        ImU32 col = bar.is_tension ? IM_COL32(225, 95, 75, 255) : IM_COL32(95, 175, 230, 255);
        dl->AddCircleFilled(p, r, col);
        dl->AddCircle(p, r, IM_COL32(20, 20, 22, 255), 0, 1.f);
    }

    // Dimension/reinforcement labels.
    for (const DimensionLabel& lbl : sec.labels) {
        ImVec2 p = ToScreen(lbl.x_mm, lbl.y_mm);
        ImVec2 text_size = ImGui::CalcTextSize(lbl.text.c_str());
        ImVec2 text_pos{p.x - text_size.x * 0.5f, p.y - text_size.y * 0.5f};
        dl->AddRectFilled(ImVec2{text_pos.x - 2.f, text_pos.y - 1.f},
                           ImVec2{text_pos.x + text_size.x + 2.f, text_pos.y + text_size.y + 1.f},
                           IM_COL32(20, 20, 22, 180));
        dl->AddText(text_pos, IM_COL32(225, 225, 230, 255), lbl.text.c_str());
    }
}

} // namespace

void DetailingPanel::Draw(bool* open, const SceneModel& scene, const Selection& selection) {
    if (!ImGui::Begin("Detailing", open)) {
        ImGui::End();
        return;
    }
    DrawPanelIconHeader(PanelIcon::Detailing, "Detailing");

    if (selection.kind != SelectionKind::Member) {
        ImGui::TextDisabled("Select a member (beam or column) in the Viewport to see its reinforcement detailing.");
        ImGui::End();
        return;
    }

    auto it = std::find_if(scene.members.begin(), scene.members.end(),
                            [&](const MemberVisual& mv) { return mv.no_batang == selection.id; });
    if (it == scene.members.end()) {
        ImGui::TextDisabled("Selected member is no longer in the loaded dataset.");
        ImGui::End();
        return;
    }

    DetailingDrawing drawing = BuildDetailingDrawing(*it);
    if (!drawing.valid) {
        ImGui::TextDisabled("No design results yet for this member -- run an optimization first.");
        ImGui::End();
        return;
    }

    ImGui::Text("Batang %d (%s)", drawing.no_batang, drawing.is_beam ? "Balok" : "Kolom");
    auto ColorSwatch = [](ImU32 col) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float size = ImGui::GetTextLineHeight() * 0.7f;
        ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2{p.x + size, p.y + size}, col, 2.f);
        ImGui::Dummy(ImVec2{size, size});
    };
    ColorSwatch(IM_COL32(225, 95, 75, 255));
    ImGui::SameLine();
    ImGui::Text("Tarik (tension)");
    ImGui::SameLine();
    ColorSwatch(IM_COL32(95, 175, 230, 255));
    ImGui::SameLine();
    ImGui::Text("Tekan (compression)");
    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float section_width = avail.x / static_cast<float>(std::max<size_t>(1, drawing.sections.size()));
    float section_height = std::max(220.f, avail.y - 10.f);

    for (size_t i = 0; i < drawing.sections.size(); ++i) {
        const DetailingSection& sec = drawing.sections[i];
        ImGui::BeginChild(("section" + std::to_string(i)).c_str(), ImVec2{section_width - 4.f, section_height}, true);
        ImGui::TextUnformatted(sec.title.c_str());
        ImVec2 region = ImGui::GetContentRegionAvail();
        ImVec2 origin = ImGui::GetCursorScreenPos();
        DrawSection(ImGui::GetWindowDrawList(), sec, region, origin);
        ImGui::Dummy(region); // reserve the space so the child's scroll extents are correct
        ImGui::EndChild();
        if (i + 1 < drawing.sections.size()) ImGui::SameLine();
    }

    ImGui::End();
}

} // namespace orcisf::gui
