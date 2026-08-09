#pragma once

#include <string>
#include <vector>

#include "engine/MemberResults.h"
#include "gui/viewport/SceneModel.h"

// Issue #8's "draw the actual reinforced-concrete detailing" acceptance
// criterion: pure geometry (no ImGui/OpenGL dependency) computed once from
// a MemberVisual/MemberResult, in local cross-section coordinates (mm,
// origin at the section's center, +X right, +Y up) -- so issue #9's PDF
// export can consume the exact same DetailingDrawing with a different
// renderer (libharu instead of ImDrawList) instead of recomputing bar
// positions from scratch.
namespace orcisf::gui {

struct RebarCircle {
    float x_mm = 0.f, y_mm = 0.f;
    float diameter_mm = 0.f;
    bool is_tension = false; // for color-coding: tension vs compression face
};

// A single labeled dimension, drawn as an extension line with a text label
// -- deliberately simple (not a full parametric CAD dimension-line-with-
// arrowheads system) but sufficient to read cover/diameter/spacing off the
// drawing, per issue #8's "dimensioned ... well enough to be usable as a
// reference drawing" criterion.
struct DimensionLabel {
    float x_mm = 0.f, y_mm = 0.f; // anchor position, local section coordinates
    std::string text;
};

struct DetailingSection {
    std::string title; // "Lapangan (Midspan)", "Tumpuan (Support)", or "Column"
    float width_mm = 0.f, height_mm = 0.f;   // concrete outline, centered at origin
    float stirrup_inset_mm = 0.f;            // cover + stirrup_dia/2, same on all 4 sides
    float stirrup_dia_mm = 0.f;
    std::vector<RebarCircle> bars;
    std::vector<DimensionLabel> labels;
};

struct DetailingDrawing {
    int no_batang = 0;
    bool is_beam = true;
    bool valid = false; // false if the member has no design results yet
    // Beam: [Tumpuan, Lapangan] (support drawn first -- matches WriteFinalResults'
    // beam text-output order). Column: a single "Column" section.
    std::vector<DetailingSection> sections;
};

// Builds the drawing from a MemberVisual with results (mv.has_results must
// be true; returns a `valid=false` drawing otherwise). Bar placement: beam
// tension bars go on the face the region's moment sign puts them on
// (lapangan/midspan = bottom, tumpuan/support = top -- standard sagging/
// hogging convention, matching why the legacy format tracks lapangan and
// tumpuan reinforcement independently in the first place); compression
// bars on the opposite face. Column bars are spread evenly around all
// four sides (4*N_DIA-4 total, matching WriteFinalResults' "(4*N_DIA-4) D
// DIA" display).
DetailingDrawing BuildDetailingDrawing(const MemberVisual& mv);

} // namespace orcisf::gui
