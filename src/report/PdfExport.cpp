#include "report/PdfExport.h"

#include <algorithm>
#include <csetjmp>
#include <cstdio>
#include <sstream>

#include <hpdf.h>

#include "gui/detailing/DetailingLayout.h"

namespace orcisf::report {

using engine::MemberResult;
using engine::StructureData;
using gui::DetailingDrawing;
using gui::DetailingSection;
using gui::MemberVisual;

namespace {

// HPDF's error handler doesn't return a value to the caller -- the
// standard libharu pattern (used by every example in the library itself)
// is to longjmp back out of whatever HPDF_* call triggered it.
jmp_buf g_jmp_env;

void HPDF_STDCALL ErrorHandler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void*) {
    std::fprintf(stderr, "HPDF error: 0x%04X (detail 0x%04X)\n", static_cast<unsigned>(error_no),
                 static_cast<unsigned>(detail_no));
    longjmp(g_jmp_env, 1);
}

constexpr float kMargin = 45.f;

void Line(HPDF_Page page, HPDF_Font font, float size, float x, float& y, const std::string& text) {
    HPDF_Page_SetFontAndSize(page, font, size);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, x, y, text.c_str());
    HPDF_Page_EndText(page);
    y -= size * 1.5f;
}

std::string Fmt2(const char* label, float value, const char* suffix = "") {
    std::ostringstream oss;
    oss << label << value << suffix;
    return oss.str();
}

// Mirrors DetailingPanel.cpp's DrawSection() exactly, with HPDF calls
// instead of ImDrawList calls -- PDF page space is already +Y-up
// (bottom-left origin), so unlike the ImGui renderer this needs no Y flip.
void DrawSectionToPdf(HPDF_Page page, HPDF_Font font, const DetailingSection& sec, float origin_x, float origin_y,
                       float box_w, float box_h) {
    float scale = 0.55f * std::min(box_w / std::max(1.f, sec.width_mm), box_h / std::max(1.f, sec.height_mm));
    scale = std::max(scale, 0.001f);
    float cx = origin_x + box_w * 0.5f;
    float cy = origin_y + box_h * 0.5f;
    auto ToPage = [&](float x_mm, float y_mm) {
        return std::pair<float, float>{cx + x_mm * scale, cy + y_mm * scale};
    };

    // Concrete outline.
    auto [cx0, cy0] = ToPage(-sec.width_mm * 0.5f, -sec.height_mm * 0.5f);
    HPDF_Page_SetRGBFill(page, 0.85f, 0.85f, 0.87f);
    HPDF_Page_SetRGBStroke(page, 0.2f, 0.2f, 0.22f);
    HPDF_Page_SetLineWidth(page, 1.f);
    HPDF_Page_Rectangle(page, cx0, cy0, sec.width_mm * scale, sec.height_mm * scale);
    HPDF_Page_FillStroke(page);

    // Stirrup outline (inset by cover + half the stirrup diameter).
    float inset = sec.stirrup_inset_mm;
    auto [sx0, sy0] = ToPage(-sec.width_mm * 0.5f + inset, -(sec.height_mm * 0.5f - inset));
    HPDF_Page_SetRGBStroke(page, 0.3f, 0.4f, 0.6f);
    HPDF_Page_Rectangle(page, sx0, sy0, (sec.width_mm - 2.f * inset) * scale, (sec.height_mm - 2.f * inset) * scale);
    HPDF_Page_Stroke(page);

    // Main bars.
    for (const gui::RebarCircle& bar : sec.bars) {
        auto [bx, by] = ToPage(bar.x_mm, bar.y_mm);
        float r = std::max(1.f, bar.diameter_mm * 0.5f * scale);
        if (bar.is_tension) {
            HPDF_Page_SetRGBFill(page, 0.75f, 0.2f, 0.15f);
        } else {
            HPDF_Page_SetRGBFill(page, 0.15f, 0.4f, 0.7f);
        }
        HPDF_Page_Circle(page, bx, by, r);
        HPDF_Page_Fill(page);
    }

    // Dimension/reinforcement labels.
    HPDF_Page_SetRGBFill(page, 0.f, 0.f, 0.f);
    HPDF_Page_SetFontAndSize(page, font, 7.f);
    for (const gui::DimensionLabel& lbl : sec.labels) {
        auto [lx, ly] = ToPage(lbl.x_mm, lbl.y_mm);
        HPDF_REAL tw = HPDF_Page_TextWidth(page, lbl.text.c_str());
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, lx - tw * 0.5f, ly - 3.f, lbl.text.c_str());
        HPDF_Page_EndText(page);
    }
}

void DrawCoverPage(HPDF_Doc pdf, HPDF_Font font, HPDF_Font font_bold, const StructureData& sd) {
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    float w = HPDF_Page_GetWidth(page);
    float y = HPDF_Page_GetHeight(page) - 60.f;

    Line(page, font_bold, 18.f, kMargin, y, "Laporan Hasil Optimasi Struktur Portal Ruang");
    y -= 6.f;
    Line(page, font, 12.f, kMargin, y, sd.ISN);
    y -= 20.f;

    Line(page, font_bold, 12.f, kMargin, y, "Data Struktur");
    Line(page, font, 10.f, kMargin, y, Fmt2("Jumlah batang (M)      : ", static_cast<float>(sd.M)));
    Line(page, font, 10.f, kMargin, y, Fmt2("Jumlah titik kumpul (NJ): ", static_cast<float>(sd.NJ)));
    Line(page, font, 10.f, kMargin, y, Fmt2("Modulus elastisitas E  : ", sd.E * 1.e-6f, " MPa"));
    Line(page, font, 10.f, kMargin, y, Fmt2("Modulus geser G        : ", sd.G * 1.e-6f, " MPa"));
    Line(page, font, 10.f, kMargin, y, Fmt2("Kuat desak beton fc'   : ", sd.FC, " MPa"));
    Line(page, font, 10.f, kMargin, y, Fmt2("Kuat tarik tulangan fy : ", sd.FY, " MPa"));
    Line(page, font, 10.f, kMargin, y, Fmt2("Kuat tarik sengkang fys: ", sd.FYS, " MPa"));
    y -= 10.f;

    Line(page, font_bold, 12.f, kMargin, y, "Parameter Optimasi");
    Line(page, font, 10.f, kMargin, y, Fmt2("Harga beton  : Rp. ", sd.harga_beton, " /m3"));
    Line(page, font, 10.f, kMargin, y, Fmt2("Harga besi   : Rp. ", sd.harga_besi, " /kg"));
    Line(page, font, 10.f, kMargin, y, Fmt2("Selimut balok: ", sd.selimut_balok, " mm"));
    Line(page, font, 10.f, kMargin, y, Fmt2("Selimut kolom: ", sd.selimut_kolom, " mm"));
    Line(page, font, 10.f, kMargin, y, Fmt2("Faktor finalti: ", sd.finalti));
    Line(page, font, 10.f, kMargin, y, Fmt2("Jumlah variabel desain (JVD): ", static_cast<float>(sd.JVD)));
    Line(page, font, 10.f, kMargin, y, Fmt2("Jumlah struktur desain (JSTD): ", static_cast<float>(sd.JSTD)));
    y -= 10.f;

    if (sd.JSTD > 0) {
        Line(page, font_bold, 12.f, kMargin, y, "Hasil Optimasi (Struktur Terbaik)");
        Line(page, font, 10.f, kMargin, y, Fmt2("Fitness : ", sd.fitstr[sd.JSTD - 1]));
        Line(page, font, 10.f, kMargin, y, Fmt2("Harga   : Rp. ", sd.hargastr[sd.JSTD - 1]));
        Line(page, font, 10.f, kMargin, y, Fmt2("Kendala : ", sd.kendalastr[sd.JSTD - 1]));
    }

    (void)w;
}

void DrawSummaryTable(HPDF_Doc pdf, HPDF_Font font, HPDF_Font font_bold, const std::vector<MemberResult>& results) {
    HPDF_Page page = nullptr;
    float y = 0.f;
    float w = 0.f;

    auto NewPage = [&]() {
        page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
        w = HPDF_Page_GetWidth(page);
        y = HPDF_Page_GetHeight(page) - 60.f;
        Line(page, font_bold, 14.f, kMargin, y, "Ringkasan Dimensi dan Biaya per Elemen");
        y -= 6.f;
        HPDF_Page_SetFontAndSize(page, font_bold, 9.f);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, kMargin, y, "Batang");
        HPDF_Page_TextOut(page, kMargin + 60.f, y, "Tipe");
        HPDF_Page_TextOut(page, kMargin + 110.f, y, "Dimensi (mm)");
        HPDF_Page_TextOut(page, kMargin + 230.f, y, "Harga (Rp.)");
        HPDF_Page_TextOut(page, kMargin + 330.f, y, "Kendala");
        HPDF_Page_EndText(page);
        y -= 16.f;
        (void)w;
    };

    NewPage();
    for (const MemberResult& r : results) {
        if (y < kMargin + 30.f) NewPage();
        std::ostringstream dim;
        dim << r.width << " x " << r.height;
        std::ostringstream harga;
        harga << r.harga;
        std::ostringstream kendala;
        kendala << r.Kendala();

        HPDF_Page_SetFontAndSize(page, font, 9.f);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, kMargin, y, std::to_string(r.no_batang).c_str());
        HPDF_Page_TextOut(page, kMargin + 60.f, y, r.is_beam ? "Balok" : "Kolom");
        HPDF_Page_TextOut(page, kMargin + 110.f, y, dim.str().c_str());
        HPDF_Page_TextOut(page, kMargin + 230.f, y, harga.str().c_str());
        HPDF_Page_TextOut(page, kMargin + 330.f, y, kendala.str().c_str());
        HPDF_Page_EndText(page);
        y -= 14.f;
    }
}

void DrawMemberDetailPage(HPDF_Doc pdf, HPDF_Font font, HPDF_Font font_bold, const MemberResult& r) {
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_LANDSCAPE);
    float w = HPDF_Page_GetWidth(page);
    float h = HPDF_Page_GetHeight(page);

    float y = h - 40.f;
    std::ostringstream title;
    title << "Batang " << r.no_batang << " (" << (r.is_beam ? "Balok" : "Kolom") << ")";
    Line(page, font_bold, 14.f, kMargin, y, title.str());

    MemberVisual mv;
    mv.no_batang = r.no_batang;
    mv.is_beam = r.is_beam;
    mv.has_results = true;
    mv.result = r;
    DetailingDrawing drawing = gui::BuildDetailingDrawing(mv);

    float drawing_top = y - 10.f;
    float drawing_bottom = 130.f;
    float drawing_height = drawing_top - drawing_bottom;
    float section_width = (w - 2.f * kMargin) / static_cast<float>(std::max<size_t>(1, drawing.sections.size()));

    for (size_t i = 0; i < drawing.sections.size(); ++i) {
        float box_x = kMargin + static_cast<float>(i) * section_width;
        HPDF_Page_SetFontAndSize(page, font_bold, 10.f);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, box_x, drawing_top, drawing.sections[i].title.c_str());
        HPDF_Page_EndText(page);
        DrawSectionToPdf(page, font, drawing.sections[i], box_x, drawing_bottom, section_width - 10.f,
                          drawing_top - 20.f - drawing_bottom);
    }
    (void)drawing_height;

    // Forces/capacity text along the bottom.
    float ty = 100.f;
    Line(page, font, 9.f, kMargin, ty, Fmt2("Harga: Rp. ", r.harga));
    if (r.is_beam) {
        std::ostringstream lap, tum, lend;
        lap << "Lapangan  -  Momen: " << r.lapangan_MU << " / " << r.lapangan_FMU
            << " Nmm   Kendala: " << r.lapangan_kendala;
        tum << "Tumpuan   -  Momen: " << r.tumpuan_MU << " / " << r.tumpuan_FMU
            << " Nmm   Kendala: " << r.tumpuan_kendala;
        lend << "Lendutan  -  Kendala: " << r.kendala_lendutan;
        Line(page, font, 9.f, kMargin, ty, lap.str());
        Line(page, font, 9.f, kMargin, ty, tum.str());
        Line(page, font, 9.f, kMargin, ty, lend.str());
    } else {
        std::ostringstream ax, mo, kd;
        ax << "Gaya aksial: " << r.axial_demand << " / " << r.axial_capacity << " N";
        mo << "Momen biaksial - Mx: " << r.moment_x_demand << " Nmm  My: " << r.moment_y_demand
           << " Nmm  Kapasitas: " << r.moment_capacity << " Nmm";
        kd << "Kendala: " << r.kendala_kolom;
        Line(page, font, 9.f, kMargin, ty, ax.str());
        Line(page, font, 9.f, kMargin, ty, mo.str());
        Line(page, font, 9.f, kMargin, ty, kd.str());
    }
}

} // namespace

std::string WritePdfReport(const StructureData& sd, const std::vector<MemberResult>& results,
                            const std::string& pdf_path) {
    HPDF_Doc pdf = HPDF_New(ErrorHandler, nullptr);
    if (!pdf) return "Failed to initialize PDF document";

    // C4611 (setjmp/longjmp vs. C++ object destruction) is expected and
    // accepted here -- see AGENTS.md's report/ section: this is the
    // standard libharu error-handling pattern used throughout its own
    // examples, not something to rewrite HPDF's API surface to avoid.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4611)
#endif
    if (setjmp(g_jmp_env)) {
        HPDF_Free(pdf);
        return "PDF generation failed (see stderr for HPDF error code)";
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", nullptr);
    HPDF_Font font_bold = HPDF_GetFont(pdf, "Helvetica-Bold", nullptr);

    DrawCoverPage(pdf, font, font_bold, sd);
    if (!results.empty()) {
        DrawSummaryTable(pdf, font, font_bold, results);
        for (const MemberResult& r : results) {
            DrawMemberDetailPage(pdf, font, font_bold, r);
        }
    }

    HPDF_SaveToFile(pdf, pdf_path.c_str());
    HPDF_Free(pdf);
    return "";
}

} // namespace orcisf::report
