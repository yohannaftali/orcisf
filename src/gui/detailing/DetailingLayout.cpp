#include "gui/detailing/DetailingLayout.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace orcisf::gui {

using engine::MemberResult;

namespace {

// Distance from the concrete edge to a bar's centerline: cover, then the
// full stirrup diameter (the bar sits just inside the stirrup ring), then
// half the bar's own diameter.
float BarOffset(float cover_mm, float stirrup_dia_mm, float bar_dia_mm) {
    return cover_mm + stirrup_dia_mm + bar_dia_mm * 0.5f;
}

void AddBarRow(std::vector<RebarCircle>& bars, float row_y, float half_width, int count, float dia_mm,
                bool is_tension) {
    if (count <= 0 || dia_mm <= 0.f) return;
    if (count == 1) {
        bars.push_back({0.f, row_y, dia_mm, is_tension});
        return;
    }
    for (int i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(count - 1); // 0..1
        float x = -half_width + t * (2.f * half_width);
        bars.push_back({x, row_y, dia_mm, is_tension});
    }
}

std::string Fmt(const char* prefix, float value, const char* suffix = "") {
    std::ostringstream oss;
    oss << prefix << value << suffix;
    return oss.str();
}

DetailingSection BuildBeamSection(const std::string& title, float B, float H, float cover, float stirrup_dia,
                                   float stirrup_spacing, float dia_tarik, float n_tarik, float dia_tekan,
                                   float n_tekan, bool tension_on_top) {
    DetailingSection s;
    s.title = title;
    s.width_mm = B;
    s.height_mm = H;
    s.stirrup_dia_mm = stirrup_dia;
    s.stirrup_inset_mm = cover + stirrup_dia * 0.5f;

    int n_tarik_i = static_cast<int>(std::lround(n_tarik));
    int n_tekan_i = static_cast<int>(std::lround(n_tekan));

    float tarik_offset = BarOffset(cover, stirrup_dia, dia_tarik);
    float tekan_offset = BarOffset(cover, stirrup_dia, dia_tekan);
    float half_width_tarik = std::max(0.f, B * 0.5f - tarik_offset);
    float half_width_tekan = std::max(0.f, B * 0.5f - tekan_offset);

    float tarik_y = tension_on_top ? (H * 0.5f - tarik_offset) : (-H * 0.5f + tarik_offset);
    float tekan_y = tension_on_top ? (-H * 0.5f + tekan_offset) : (H * 0.5f - tekan_offset);

    AddBarRow(s.bars, tarik_y, half_width_tarik, n_tarik_i, dia_tarik, true);
    AddBarRow(s.bars, tekan_y, half_width_tekan, n_tekan_i, dia_tekan, false);

    s.labels.push_back({0.f, H * 0.5f + 40.f, Fmt("B=", B, " mm")});
    s.labels.push_back({-B * 0.5f - 70.f, 0.f, Fmt("H=", H, " mm")});
    s.labels.push_back({0.f, -H * 0.5f - 40.f,
                         "Sengkang D" + Fmt("", stirrup_dia) + "-" + Fmt("", stirrup_spacing)});
    s.labels.push_back({0.f, tarik_y + (tension_on_top ? -25.f : 25.f),
                         Fmt("", n_tarik) + "D" + Fmt("", dia_tarik) + " (tarik)"});
    s.labels.push_back({0.f, tekan_y + (tension_on_top ? 25.f : -25.f),
                         Fmt("", n_tekan) + "D" + Fmt("", dia_tekan) + " (tekan)"});
    s.labels.push_back({0.f, 0.f, Fmt("Selimut=", cover, " mm")});

    return s;
}

DetailingSection BuildColumnSection(const MemberResult& r) {
    DetailingSection s;
    s.title = "Column";
    s.width_mm = r.width;
    s.height_mm = r.height;
    s.stirrup_dia_mm = r.stirrup_dia;
    s.stirrup_inset_mm = r.cover_mm + r.stirrup_dia * 0.5f;

    int n_per_side = static_cast<int>(std::lround(r.col_n_dia));
    float offset = BarOffset(r.cover_mm, r.stirrup_dia, r.col_dia);
    float half = std::max(0.f, r.width * 0.5f - offset);

    if (n_per_side >= 1) {
        // Four corners.
        float corners_x[4] = {-half, half, half, -half};
        float corners_y[4] = {-half, -half, half, half};
        for (int i = 0; i < 4; ++i) {
            s.bars.push_back({corners_x[i], corners_y[i], r.col_dia, false});
        }
        // Interior points along each side (excludes the two corners already added).
        int interior = n_per_side - 2;
        if (interior > 0) {
            // Side 0: bottom (-half,-half)->(half,-half); 1: right; 2: top (reversed); 3: left.
            struct Side {
                float x0, y0, x1, y1;
            };
            Side sides[4] = {
                {-half, -half, half, -half}, {half, -half, half, half}, {half, half, -half, half},
                {-half, half, -half, -half}};
            for (const Side& side : sides) {
                for (int k = 1; k <= interior; ++k) {
                    float t = static_cast<float>(k) / static_cast<float>(interior + 1);
                    float x = side.x0 + t * (side.x1 - side.x0);
                    float y = side.y0 + t * (side.y1 - side.y0);
                    s.bars.push_back({x, y, r.col_dia, false});
                }
            }
        }
    }

    s.labels.push_back({0.f, r.height * 0.5f + 40.f, Fmt("B=", r.width, " mm")});
    s.labels.push_back({-r.width * 0.5f - 70.f, 0.f, Fmt("H=", r.height, " mm")});
    s.labels.push_back(
        {0.f, -r.height * 0.5f - 40.f, "Sengkang D" + Fmt("", r.stirrup_dia) + "-" + Fmt("", r.stirrup_spacing)});
    std::ostringstream main_bar;
    main_bar << (4 * n_per_side - 4) << "D" << r.col_dia << " (spasi " << r.col_bar_spacing << " mm)";
    s.labels.push_back({0.f, 0.f, main_bar.str()});
    s.labels.push_back({0.f, r.height * 0.5f + 70.f, Fmt("Selimut=", r.cover_mm, " mm")});

    return s;
}

} // namespace

DetailingDrawing BuildDetailingDrawing(const MemberVisual& mv) {
    DetailingDrawing d;
    d.no_batang = mv.no_batang;
    d.is_beam = mv.is_beam;
    d.valid = mv.has_results;
    if (!d.valid) return d;

    const MemberResult& r = mv.result;
    if (mv.is_beam) {
        // Tumpuan first, matching WriteFinalResults' beam text-output order.
        d.sections.push_back(BuildBeamSection("Tumpuan (Support)", r.width, r.height, r.cover_mm, r.stirrup_dia,
                                               r.stirrup_spacing, r.tum_dia_tarik, r.tum_n_tarik, r.tum_dia_tekan,
                                               r.tum_n_tekan,
                                               /*tension_on_top=*/true));
        d.sections.push_back(BuildBeamSection("Lapangan (Midspan)", r.width, r.height, r.cover_mm, r.stirrup_dia,
                                               r.stirrup_spacing, r.lap_dia_tarik, r.lap_n_tarik, r.lap_dia_tekan,
                                               r.lap_n_tekan,
                                               /*tension_on_top=*/false));
    } else {
        d.sections.push_back(BuildColumnSection(r));
    }
    return d;
}

} // namespace orcisf::gui
