#include "gui/AnalyzePanel.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <string>
#include <utility>

#include <imgui.h>

#include "engine/StructuralAnalysis.h"
#include "gui/PanelTitles.h"
#include "gui/TableView.h"

namespace orcisf::gui {

namespace {

// Issue #50/#60's documented viewport palette, reused here so a member's
// Safe/Unsafe text color matches its own viewport cube color exactly.
constexpr ImVec4 kSatisfiedColor(0.30f, 0.75f, 0.38f, 1.f);
constexpr ImVec4 kViolatedColor(0.85f, 0.28f, 0.24f, 1.f);

// One dropdown for one design-variable slot: `table`/`count` is the
// discrete table this slot indexes into (matches Optimizer.cpp's
// anonymous-namespace LoadBatasAtas() mapping exactly -- see
// AnalyzePanel.h's header comment). Returns true if the user picked a
// different index this frame.
bool DesignVarCombo(const char* str_id, int& index, const engine::LegacyArray<float>& table, int count,
                     const char* fmt) {
    if (count <= 0) {
        ImGui::TextDisabled("--");
        return false;
    }
    if (index < 0 || index >= count) index = 0; // guard against a stale index from a shrunk table

    char preview[32];
    std::snprintf(preview, sizeof(preview), fmt, engine::Isi(index, table));

    bool changed = false;
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(str_id, preview)) {
        for (int i = 0; i < count; ++i) {
            bool selected = (i == index);
            char item[32];
            std::snprintf(item, sizeof(item), fmt, engine::Isi(i, table));
            ImGui::PushID(i);
            if (ImGui::Selectable(item, selected)) {
                index = i;
                changed = true;
            }
            if (selected) ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return changed;
}

void DrawInputTable(const engine::StructureData& sd, const SceneModel& scene,
                     std::unordered_map<int, std::array<int, 12>>& beam_choices,
                     std::unordered_map<int, std::array<int, 5>>& column_choices, bool same_lap_tum) {
    // Slot table indexed by the beam's own 12-slot layout (see
    // AGENTS.md's "Discrete design variables"). `is_tumpuan` marks the 4
    // slots [74]'s "same" toggle hides/mirrors -- kept as a plain lookup
    // (not a lambda capturing `sd`) so it can stay a compile-time table.
    struct SlotSpec {
        int table_id; // 0=sisi_d_B,1=sisi_d_H,2=DIA_d,3=NL_d,4=DIAS_d,5=JS_d
        const char* fmt;
        bool is_tumpuan;
    };
    static constexpr SlotSpec kBeamSlots[12] = {
        {0, "%.0f", false},  // B
        {1, "%.0f", false},  // H
        {2, "D%.0f", false}, // DIA1lap (tarik)
        {3, "%.0f", false},  // NL1lap
        {2, "D%.0f", false}, // DIA2lap (tekan)
        {3, "%.0f", false},  // NL2lap
        {2, "D%.0f", true},  // DIA1tum (tarik)
        {3, "%.0f", true},   // NL1tum
        {2, "D%.0f", true},  // DIA2tum (tekan)
        {3, "%.0f", true},   // NL2tum
        {4, "D%.0f", false}, // DIAS
        {5, "%.0f", false},  // Jarak_S
    };
    auto slot_table = [&](int table_id) -> std::pair<const engine::LegacyArray<float>*, int> {
        switch (table_id) {
            case 0: return {&sd.sisi_d_B, sd.nsisi_B};
            case 1: return {&sd.sisi_d_H, sd.nsisi_H};
            case 2: return {&sd.DIA_d, sd.nDIA};
            case 3: return {&sd.NL_d, sd.nNL};
            case 4: return {&sd.DIAS_d, sd.nDIAS};
            default: return {&sd.JS_d, sd.nJS};
        }
    };

    ImGui::TextUnformatted("Beams");
    // Issue #74: hiding the 4 tumpuan columns is the whole point of the
    // "same" toggle ("number of column become shorter") -- an ImGui table's
    // column count is fixed for the whole table, so this has to be a
    // table-wide column count, not a per-row thing.
    const int visible_slots = same_lap_tum ? 8 : 12; // B,H,Dia+,N+,Dia-,N-,DiaS,S vs. all 12
    if (BeginTableView("analyze_beams", visible_slots + 1)) {
        ImGui::TableSetupColumn("Batang", ImGuiTableColumnFlags_WidthFixed, 55.f);
        if (same_lap_tum) {
            for (const char* h : {"B", "H", "Dia+", "N+", "Dia-", "N-", "DiaS", "S"}) ImGui::TableSetupColumn(h);
        } else {
            for (const char* h : {"B", "H", "DiaLap+", "NLap+", "DiaLap-", "NLap-", "DiaTum+", "NTum+", "DiaTum-",
                                   "NTum-", "DiaS", "S"}) {
                ImGui::TableSetupColumn(h);
            }
        }
        ImGui::TableHeadersRow();

        for (const MemberVisual& mv : scene.members) {
            if (!mv.is_beam) continue;
            std::array<int, 12>& c = beam_choices[mv.no_batang]; // inserts a zeroed default if new
            ImGui::PushID(mv.no_batang);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", mv.no_batang);

            int col = 1;
            for (int j = 0; j < 12; ++j) {
                if (same_lap_tum && kBeamSlots[j].is_tumpuan) continue; // hidden -- Run Analyze mirrors these
                ImGui::TableSetColumnIndex(col++);
                ImGui::PushID(j);
                auto [table, count] = slot_table(kBeamSlots[j].table_id);
                DesignVarCombo("##v", c[static_cast<size_t>(j)], *table, count, kBeamSlots[j].fmt);
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("Columns");
    if (BeginTableView("analyze_columns", 6)) {
        ImGui::TableSetupColumn("Batang", ImGuiTableColumnFlags_WidthFixed, 55.f);
        for (const char* h : {"Sisi", "Dia", "N_Dia", "DiaS", "S"}) ImGui::TableSetupColumn(h);
        ImGui::TableHeadersRow();

        for (const MemberVisual& mv : scene.members) {
            if (mv.is_beam) continue;
            std::array<int, 5>& c = column_choices[mv.no_batang];
            ImGui::PushID(mv.no_batang);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", mv.no_batang);

            static const struct {
                const engine::LegacyArray<float>* table;
                int count;
                const char* fmt;
            } kSlots[5] = {
                {&sd.sisi_d_K, sd.nsisi_K, "%.0f"}, {&sd.DIA_d, sd.nDIA, "D%.0f"}, {&sd.NL_d, sd.nNL, "%.0f"},
                {&sd.DIAS_d, sd.nDIAS, "D%.0f"},    {&sd.JS_d, sd.nJS, "%.0f"},
            };
            for (int j = 0; j < 5; ++j) {
                ImGui::TableSetColumnIndex(j + 1);
                ImGui::PushID(j);
                DesignVarCombo("##v", c[static_cast<size_t>(j)], *kSlots[j].table, kSlots[j].count, kSlots[j].fmt);
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void DrawResultsTable(const SceneModel& scene, Selection& selection) {
    bool any_results = false;
    for (const MemberVisual& mv : scene.members) {
        if (mv.has_results) {
            any_results = true;
            break;
        }
    }
    if (!any_results) {
        ImGui::TextDisabled("Click \"Run Analyze\" to check the design above.");
        return;
    }

    int unsafe_count = 0;
    for (const MemberVisual& mv : scene.members) {
        if (mv.has_results && mv.kendala > 0.f) ++unsafe_count;
    }
    if (unsafe_count == 0) {
        ImGui::TextColored(kSatisfiedColor, "Safe: all members satisfy every design check.");
    } else {
        ImGui::TextColored(kViolatedColor, "Unsafe: %d of %d members fail a design check.", unsafe_count,
                            static_cast<int>(scene.members.size()));
    }

    if (!BeginTableView("analyze_results", 7)) return;
    ImGui::TableSetupColumn("Batang", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 60.f);
    ImGui::TableSetupColumn("Section (mm)");
    ImGui::TableSetupColumn("Demand");
    ImGui::TableSetupColumn("Capacity");
    ImGui::TableSetupColumn("Kendala");
    ImGui::TableSetupColumn("Verdict", ImGuiTableColumnFlags_WidthFixed, 70.f);
    ImGui::TableHeadersRow();

    for (const MemberVisual& mv : scene.members) {
        if (!mv.has_results) continue;
        const engine::MemberResult& r = mv.result;
        ImGui::PushID(mv.no_batang);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (TableRowSelectable(std::to_string(mv.no_batang).c_str(),
                                selection.kind == SelectionKind::Member && selection.id == mv.no_batang)) {
            selection = {SelectionKind::Member, mv.no_batang};
        }
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(r.is_beam ? "Beam" : "Column");
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.0f x %.0f", r.width, r.height);
        ImGui::TableSetColumnIndex(3);
        if (r.is_beam) {
            ImGui::Text("%.4g Nm", std::max({r.lapangan_MU, r.tumpuan_MU}));
        } else {
            ImGui::Text("%.4g N", r.axial_demand);
        }
        ImGui::TableSetColumnIndex(4);
        if (r.is_beam) {
            ImGui::Text("%.4g Nm", std::max(r.lapangan_FMU, r.tumpuan_FMU));
        } else {
            ImGui::Text("%.4g N", r.axial_capacity);
        }
        ImGui::TableSetColumnIndex(5);
        ImGui::Text("%.6g", r.Kendala());
        ImGui::TableSetColumnIndex(6);
        if (r.Kendala() <= 0.f) {
            ImGui::TextColored(kSatisfiedColor, "Safe");
        } else {
            ImGui::TextColored(kViolatedColor, "Unsafe");
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

} // namespace

void AnalyzePanel::SetOnRunRequested(
    std::function<void(const engine::OptimizationOptions&, const std::vector<int>&, const std::vector<int>&)> cb) {
    on_run_ = std::move(cb);
}

void AnalyzePanel::SetError(std::string message) {
    has_error_ = !message.empty();
    last_error_ = std::move(message);
}

void AnalyzePanel::Draw(bool* open, const engine::StructureData* sd, const SceneModel& scene, Selection& selection) {
    if (!ImGui::Begin(PanelWindowTitle(kAnalyzeId, "Analyze").c_str(), open)) {
        ImGui::End();
        return;
    }
    if (!sd) {
        ImGui::TextDisabled("Load or create a dataset first.");
        ImGui::End();
        return;
    }

    ImGui::TextWrapped(
        "Pick each member's cross-section/reinforcement by hand, then run a design check against exactly "
        "that design -- no cost-optimization search happens here.");

    ImGui::SeparatorText("Design Parameters");
    ImGui::InputFloat("Concrete price (Rp/m^3)", &harga_beton_);
    ImGui::InputFloat("Steel price (Rp/kg)", &harga_besi_);
    ImGui::InputFloat("Column cover (mm)", &selimut_kolom_);
    ImGui::InputFloat("Beam cover (mm)", &selimut_balok_);

    ImGui::SeparatorText("Design Input");
    ImGui::Checkbox("Same bars at lapangan (midspan) and tumpuan (support)", &same_lap_tum_);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Practical default: real construction almost always runs the same bar diameter/count through "
            "both regions. Uncheck only for the rare/theoretical case of independently-chosen lapangan and "
            "tumpuan reinforcement.");
    }
    DrawInputTable(*sd, scene, beam_choices_, column_choices_, same_lap_tum_);

    ImGui::Spacing();
    bool has_members = !scene.members.empty();
    ImGui::BeginDisabled(!has_members);
    if (ImGui::Button("Run Analyze")) {
        // Member order must match sd.no_balok/no_kolom's construction
        // (PrepareOptimization()'s single ascending pass over 1..M,
        // splitting on the same is_beam/CXZ check) -- scene.members is
        // already built in that same ascending member-id order (see
        // SceneModel.cpp), so filtering it by is_beam here reproduces it
        // exactly without needing a mutable StructureData& to re-derive it.
        std::vector<int> var_b, var_k;
        for (const MemberVisual& mv : scene.members) {
            if (!mv.is_beam) continue;
            std::array<int, 12> c = beam_choices_[mv.no_batang]; // copy -- may be mirrored below
            // Issue #74: when "same" is on, tumpuan reinforcement always
            // mirrors lapangan at Run time, regardless of whatever the
            // (hidden) tumpuan slots currently hold -- this is what makes
            // toggling back to "same" a clean re-sync rather than reviving
            // stale independent values.
            if (same_lap_tum_) {
                c[6] = c[2]; // DIA1tum = DIA1lap
                c[7] = c[3]; // NL1tum  = NL1lap
                c[8] = c[4]; // DIA2tum = DIA2lap
                c[9] = c[5]; // NL2tum  = NL2lap
            }
            for (int v : c) var_b.push_back(v);
        }
        for (const MemberVisual& mv : scene.members) {
            if (mv.is_beam) continue;
            const std::array<int, 5>& c = column_choices_[mv.no_batang];
            for (int v : c) var_k.push_back(v);
        }

        engine::OptimizationOptions options;
        options.harga_beton = harga_beton_;
        options.harga_besi = harga_besi_;
        options.selimut_kolom = selimut_kolom_;
        options.selimut_balok = selimut_balok_;
        options.finalti = 1.f;
        options.j_iterasi_mak = 0;
        options.fak_plus = 0;
        options.fak_kali = 1;
        options.worker_threads = 1;

        if (on_run_) on_run_(options, var_b, var_k);
    }
    ImGui::EndDisabled();
    if (!has_members) {
        ImGui::SameLine();
        ImGui::TextDisabled("(no members in the loaded dataset)");
    }

    if (has_error_) {
        ImGui::TextColored(kViolatedColor, "%s", last_error_.c_str());
    }

    ImGui::SeparatorText("Design Check Results");
    DrawResultsTable(scene, selection);

    ImGui::End();
}

} // namespace orcisf::gui
