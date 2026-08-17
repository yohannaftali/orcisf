#pragma once

#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/Optimizer.h"
#include "engine/StructureData.h"
#include "gui/editor/Selection.h"
#include "gui/viewport/SceneModel.h"

// Issues #69/#70 (epic #67 -- manual-dimension "Analyze" mode): lets the
// user pick each beam/column's discrete design-variable indices by hand
// (the same .isd/.idl/.ijl/.ids/.ijs tables a real dataset already loads --
// see AGENTS.md's "Discrete design variables" section, 12 slots per beam, 5
// per column) instead of letting the Flexible Polyhedron search choose
// them, then runs engine::AnalyzeFixedDesign() (issue #68) against that
// fixed design and shows a demand-vs-capacity / Safe-Unsafe verdict per
// member. One panel covers both sub-issues' UI (input table + results
// table), matching this project's existing "one panel, multiple stacked
// sections" precedent (ResultsPanel, issue #62) rather than splitting into
// two separately-dockable windows for what is really one workflow.
namespace orcisf::gui {

class AnalyzePanel {
public:
    // Called (UI thread, synchronously -- AnalyzeFixedDesign() is a single
    // direct analysis pass, not a search, so it never needs RunPanel's
    // background-thread machinery) when the user clicks "Run Analyze".
    // var_b/var_k are flat, 0-based, in the exact sd.no_balok/no_kolom
    // order engine::AnalyzeFixedDesign() expects; `options` carries this
    // panel's own unit-price/cover fields (fak_kali/fak_plus/finalti/
    // j_iterasi_mak are set to harmless defaults -- AnalyzeFixedDesign()
    // never runs optimasi()'s search loop, so they're unused). Application
    // performs the actual engine call (it owns loaded_sd_) and reports
    // back via SetError() plus its own scene_ update -- this panel doesn't
    // hold a StructureData itself.
    void SetOnRunRequested(std::function<void(const engine::OptimizationOptions& options,
                                               const std::vector<int>& var_b, const std::vector<int>& var_k)>
                                cb);

    void SetError(std::string message); // empty = success, clears any previous error

    // `sd`, if non-null, is the currently loaded dataset -- used to read
    // discrete-table values (sisi_d_B, DIA_d, ...) for the dropdown
    // labels. `scene` supplies the beam/column member list + is_beam
    // classification (same geometry-only CXZ check PrepareOptimization()
    // uses, see SceneModel.cpp) and, once a Run Analyze has updated it,
    // the resulting MemberResult/kendala per member for the results
    // table + viewport-coloring cross-check.
    void Draw(bool* open, const engine::StructureData* sd, const SceneModel& scene, Selection& selection);

private:
    // Per-member chosen discrete-table indices, keyed by no_batang so
    // choices survive geometry edits/undo naturally (a member that no
    // longer exists just stops being read; a new member gets an inserted
    // default the next Draw() call) without any explicit staleness
    // tracking. `idx` slots [2..5] are lapangan tension/compression
    // dia+count, [6..9] the tumpuan equivalents -- the legacy 12-slot
    // layout (see AGENTS.md's "Discrete design variables" section).
    std::unordered_map<int, std::array<int, 12>> beam_choices_;
    std::unordered_map<int, std::array<int, 5>> column_choices_;

    // Issue #74: one table-wide toggle (not per-beam -- an ImGui table's
    // column count is fixed for the whole table, so a genuinely per-row
    // "shorter table" as first specified isn't representable; a single
    // switch for the whole Design Input table achieves the same practical
    // goal -- "the same bars run continuously, don't make me enter them
    // twice" -- without that constraint). Defaults true (matching real
    // construction practice: lapangan/tumpuan almost always share the same
    // bar diameter/count). When true, the table hides the 4 tumpuan
    // reinforcement columns and Run Analyze mirrors idx[2..5] into
    // idx[6..9] for every beam regardless of whatever idx[6..9] currently
    // holds -- so re-checking after unchecking always cleanly re-syncs
    // rather than reviving stale values. When false, all 12 slots are
    // shown/edited independently per beam (the rare/theoretical case).
    bool same_lap_tum_ = true;

    // Run configuration -- mirrors RunPanel's own fields/defaults (unit
    // prices, cover thickness) since ComputeMemberResults()'s cost/design
    // calculations need them even with no cost-minimization search
    // happening. finalti/iteration/population-factor fields are
    // deliberately absent here -- Analyze mode never runs optimasi()'s
    // search loop, so they'd be meaningless.
    float harga_beton_ = 250000.f;
    float harga_besi_ = 5000.f;
    float selimut_kolom_ = 50.f;
    float selimut_balok_ = 50.f;

    std::function<void(const engine::OptimizationOptions&, const std::vector<int>&, const std::vector<int>&)>
        on_run_;
    std::string last_error_;
    bool has_error_ = false;
};

} // namespace orcisf::gui
