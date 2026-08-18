#include "gui/RunPanel.h"

#include <algorithm>
#include <exception>
#include <limits>
#include <random>

#include <imgui.h>

#include "engine/DebugLog.h" // issue #63 investigation, see DebugLog.h
#include "engine/Engine.h"
#include "gui/PanelTitles.h"

namespace orcisf::gui {

RunPanel::RunPanel() {
    unsigned int hw = std::thread::hardware_concurrency();
    worker_threads_ = static_cast<int>(hw > 1 ? hw - 1 : 1);
}

RunPanel::~RunPanel() {
    if (worker_.joinable()) {
        cancel_.store(true);
        worker_.join();
    }
}

void RunPanel::SetLogSink(std::function<void(std::string)> sink) { log_sink_ = std::move(sink); }

void RunPanel::SetOnResult(
    std::function<void(engine::StructureData, std::string, std::string)> callback) {
    on_result_ = std::move(callback);
}

void RunPanel::SetDatasetPath(std::string path) { dataset_path_ = std::move(path); }

bool RunPanel::IsRunning() const { return running_.load(); }

void RunPanel::StartRun() {
    if (IsRunning()) {
        return;
    }
    if (worker_.joinable()) {
        worker_.join(); // previous run already finished; reap it
    }

    has_error_ = false;
    has_progress_ = false;
    has_finished_ = false;
    cancel_.store(false);
    running_.store(true);

    std::string dataset_path = dataset_path_;
    engine::OptimizationOptions options;
    options.harga_beton = harga_beton_;
    options.harga_besi = harga_besi_;
    options.selimut_kolom = selimut_kolom_;
    options.selimut_balok = selimut_balok_;
    options.finalti = finalti_;
    options.j_iterasi_mak = j_iterasi_mak_;
    options.fak_plus = fak_plus_;
    options.fak_kali = fak_kali_;
    options.worker_threads = static_cast<unsigned int>(std::max(1, worker_threads_));
    options.rng_seed = rng_seed_;
    options.symmetric_beam_reinforcement = symmetric_beam_reinforcement_;
    options.even_bar_count_only = even_bar_count_only_;

    // Issue #16: extract the previous run's best design (population slot
    // JSTD-1, always the best after the last Sort() -- see Optimizer.h's
    // DetailLogCallback comment) from result_sd_ *before* it gets
    // overwritten by this new run. Only offered by the checkbox (below)
    // when has_result_ is true and the dataset path hasn't changed since
    // that result was captured; the engine independently re-validates the
    // seed's size against this run's actual jum_balok/jum_kolom and
    // silently ignores it on a mismatch (see Optimizer.h's comment), so
    // this is a defense-in-depth check, not the only one.
    if (reoptimize_from_last_ && has_result_ && result_dataset_path_ == dataset_path) {
        int jb = result_sd_.jum_balok;
        int jk = result_sd_.jum_kolom;
        int best_slot = result_sd_.JSTD - 1;
        options.seed_from_previous_best = true;
        options.seed_var_b.assign(result_sd_.var_b[best_slot], result_sd_.var_b[best_slot] + 12 * jb);
        options.seed_var_k.assign(result_sd_.var_k[best_slot], result_sd_.var_k[best_slot] + 5 * jk);
    }
    has_result_ = false;

    if (log_sink_) {
        std::string msg = "Starting optimization: " + dataset_path +
                           " (worker_threads=" + std::to_string(options.worker_threads) + ")";
        if (options.seed_from_previous_best) {
            msg += " [re-optimizing from last best result]";
        }
        log_sink_(msg);
    }

    worker_ = std::thread([this, dataset_path, options]() {
        engine::ProgressCallback on_progress = [this](const engine::ProgressInfo& info) -> bool {
            {
                std::lock_guard<std::mutex> lock(progress_mutex_);
                progress_ = info;
                has_progress_ = true;
            }
            return !cancel_.load();
        };

        engine::DebugLog("RunPanel::worker", "entering RunFullOptimization(), dataset=" + dataset_path);
        try {
            result_output_path_ = engine::RunFullOptimization(result_sd_, dataset_path, options, on_progress, &cancel_);
            engine::DebugLog("RunPanel::worker", "RunFullOptimization() returned normally, output=" + result_output_path_);
        } catch (const std::exception& e) {
            last_error_ = e.what();
            has_error_ = true;
            engine::DebugLog("RunPanel::worker", std::string("RunFullOptimization() threw: ") + e.what());
        }

        engine::DebugLog("RunPanel::worker", "about to call running_.store(false)");
        running_.store(false);
        engine::DebugLog("RunPanel::worker", "running_.store(false) done, worker lambda returning");
    });
    engine::DebugLog("RunPanel::StartRun", "worker thread spawned");
}

void RunPanel::RequestCancel() { cancel_.store(true); }

bool RunPanel::CanRun() const { return !IsRunning() && !dataset_path_.empty(); }
void RunPanel::TriggerRun() {
    if (CanRun()) StartRun();
}

void RunPanel::Draw(bool* open) {
    // Issue #37: display renamed "Run Optimization" -> "Optimization" for
    // brevity -- kRunOptimizationId (the dock/window *identity*) is
    // unchanged on purpose, so this rename doesn't touch DockBuilderDockWindow()
    // associations or require re-docking anywhere.
    if (!ImGui::Begin(PanelWindowTitle(kRunOptimizationId, "Optimization").c_str(), open)) {
        ImGui::End();
        return;
    }
    bool running = IsRunning();
    if (was_running_ && !running) {
        engine::DebugLog("RunPanel::Draw", "UI thread observed running_==false edge");
        if (has_error_) {
            if (log_sink_) log_sink_("Optimization run stopped with an error: " + last_error_);
        } else if (cancel_.load()) {
            if (log_sink_) log_sink_("Optimization run cancelled.");
        } else {
            if (log_sink_) log_sink_("Optimization run finished. Output: " + result_output_path_);
            has_result_ = true;
            has_finished_ = true; // issue #63: see RunPanel.h's comment on this field
            result_dataset_path_ = dataset_path_;
            engine::DebugLog("RunPanel::Draw", "about to call on_result_ (Application::OnRunResult)");
            if (on_result_) on_result_(result_sd_, dataset_path_, result_output_path_);
            engine::DebugLog("RunPanel::Draw", "on_result_ returned");
        }
    }
    was_running_ = running;

    // Issue #25: no longer an editable field -- always whatever dataset
    // Application currently has loaded (SetDatasetPath(), called once per
    // frame from Application::OnFrame()), so it can't drift out of sync
    // with what File > Open Data/New Data/Save As already point at.
    ImGui::SeparatorText("Dataset");
    if (dataset_path_.empty()) {
        ImGui::TextDisabled("No dataset loaded -- use File > Open Data... or New Data.");
    } else {
        ImGui::TextWrapped("%s", dataset_path_.c_str());
    }

    ImGui::BeginDisabled(running);
    ImGui::SeparatorText("Cost / design parameters");
    ImGui::InputFloat("Harga beton (Rp/m^3)", &harga_beton_);
    ImGui::InputFloat("Harga besi (Rp/kg)", &harga_besi_);
    ImGui::InputFloat("Selimut kolom (mm)", &selimut_kolom_);
    ImGui::InputFloat("Selimut balok (mm)", &selimut_balok_);
    ImGui::InputFloat("Finalti (penalty)", &finalti_);
    ImGui::InputInt("Max generations", &j_iterasi_mak_);
    ImGui::InputInt("fak_plus", &fak_plus_);
    ImGui::InputInt("fak_kali", &fak_kali_);
    // Issue #65: JSTD = JVD*fak_kali + fak_plus must stay > JVD, or
    // RunOptimization()'s convergence check reads fitstr[JSTD-JVD-1] out of
    // bounds (fak_kali=1, fak_plus=0 -> index -1). fak_kali=1/fak_plus=0
    // is the one combination that violates this; clamp it here so it can
    // never reach the engine via the GUI (engine::RunOptimization() also
    // guards this itself for non-GUI callers, e.g. orcisf_cli).
    fak_kali_ = std::max(1, fak_kali_);
    fak_plus_ = std::max(0, fak_plus_);
    if (fak_kali_ == 1 && fak_plus_ == 0) fak_plus_ = 1;
    // Issue #75 (found instrumenting #63): JSTD = JVD*fak_kali + fak_plus
    // must not exceed kMak (825) -- the fixed size of var_b/var_k/fitstr/
    // kendalastr/hargastr (see Optimizer.cpp's RunOptimization() guard,
    // the authoritative check since this field doesn't know the loaded
    // dataset's JVD). This panel has no dataset loaded here to compute the
    // real JVD from, so it clamps to a generous-but-bounded ceiling instead
    // -- keeps an accidental huge value (e.g. a mistyped extra digit) from
    // reaching the engine at all for any realistic dataset size, while the
    // engine's own guard remains the one that's always correct.
    fak_kali_ = std::min(fak_kali_, 100);
    fak_plus_ = std::min(fak_plus_, engine::kMak);

    ImGui::SeparatorText("Reinforcement practice");
    ImGui::Checkbox("Same bars at lapangan (midspan) and tumpuan (support)", &symmetric_beam_reinforcement_);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Practical default: real construction almost always runs the same bar diameter/count through "
            "both regions. Uncheck only for the rare/theoretical case of independently-optimized lapangan and "
            "tumpuan reinforcement.");
    }
    ImGui::Checkbox("Even bar count only (min 4)", &even_bar_count_only_);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Practical default: bars come in symmetric pairs, so real construction uses an even bar count "
            "(4, 6, 8, ...). Uncheck to allow odd counts too -- the minimum of 4 still applies either way.");
    }

    ImGui::SeparatorText("Performance");
    int max_threads = static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));
    ImGui::SliderInt("Worker threads", &worker_threads_, 1, max_threads);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "How many CPU cores to use for the heavy per-generation candidate\n"
            "evaluation loop. 1 = single-threaded. Results are numerically\n"
            "identical regardless of this value for a given RNG seed -- more\n"
            "threads only changes wall-clock time.\n"
            "This machine reports %u hardware threads.",
            std::thread::hardware_concurrency());
    }
    int rng_seed_i = static_cast<int>(rng_seed_);
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.6f);
    if (ImGui::InputInt("RNG seed (0 = random)", &rng_seed_i)) {
        rng_seed_ = static_cast<unsigned int>(std::max(0, rng_seed_i));
    }
    ImGui::SameLine();
    if (ImGui::Button("Regenerate Seed")) {
        // std::random_device rather than time-seeding: two regenerates in
        // the same frame/millisecond must still produce different values.
        // Only affects the *next* run started (StartRun() copies rng_seed_
        // into its own local options at call time) -- a run already in
        // progress is unaffected either way, but the button is disabled
        // along with the rest of this section while one is running anyway.
        // Bounded to [1, INT_MAX]: rng_seed_ round-trips through this
        // field as a plain `int` (ImGui::InputInt), so a raw
        // std::random_device value with the sign bit set would display as
        // a negative number and, worse, get silently clamped back to 0
        // (the "random" sentinel) the next time this field's edit path
        // runs its `std::max(0, rng_seed_i)`.
        std::random_device rd;
        std::uniform_int_distribution<int> dist(1, std::numeric_limits<int>::max());
        rng_seed_ = static_cast<unsigned int>(dist(rd));
    }
    ImGui::EndDisabled();

    // Issue #16: only offered once a completed run's best design exists
    // for the *currently entered* dataset path -- switching the path
    // after a run (or a cancelled/errored run, which never sets
    // has_result_) correctly disables it rather than silently seeding
    // from an unrelated dataset.
    bool seed_available = has_result_ && result_dataset_path_ == dataset_path_;
    ImGui::BeginDisabled(running || !seed_available);
    if (!seed_available) reoptimize_from_last_ = false;
    ImGui::Checkbox("Re-optimize from last best result", &reoptimize_from_last_);
    ImGui::EndDisabled();
    if (!seed_available) {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "Available after a completed run against this exact dataset path.\n"
                "Seeds one population slot with the previous best design instead\n"
                "of starting fully from scratch.");
        }
    }

    ImGui::Spacing();
    bool can_run = !running && !dataset_path_.empty();
    ImGui::BeginDisabled(!can_run);
    if (ImGui::Button("Run")) {
        StartRun();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!running || cancel_.load());
    if (ImGui::Button("Cancel")) {
        RequestCancel();
        if (log_sink_) {
            log_sink_("Cancellation requested -- stopping after the current generation.");
        }
    }
    ImGui::EndDisabled();

    engine::ProgressInfo progress_snapshot;
    bool has_progress_snapshot;
    {
        std::lock_guard<std::mutex> lock(progress_mutex_);
        progress_snapshot = progress_;
        has_progress_snapshot = has_progress_;
    }

    if (running || has_progress_snapshot) {
        ImGui::SeparatorText("Progress");
        // Issue #63: once has_finished_ is set, always show 100%/max_generation
        // rather than whatever mid-run snapshot progress_ last held -- the
        // engine's progress callback can legitimately stop one generation
        // short of max_generation on early convergence (see RunPanel.h's
        // comment on has_finished_), and without this override the bar and
        // generation count would freeze on that stale snapshot forever,
        // looking exactly like a hang even though the run is genuinely done.
        float frac = has_finished_
                         ? 1.f
                         : (progress_snapshot.max_generation > 0
                                ? static_cast<float>(progress_snapshot.generation) /
                                      static_cast<float>(progress_snapshot.max_generation)
                                : 0.f);
        ImGui::ProgressBar(std::clamp(frac, 0.f, 1.f));
        if (has_finished_) {
            ImGui::Text("Generation: %d / %d", progress_snapshot.max_generation, progress_snapshot.max_generation);
        } else {
            ImGui::Text("Generation: %d / %d", progress_snapshot.generation, progress_snapshot.max_generation);
        }
        ImGui::Text("Best fitness: %.6g  harga: %.6g  kendala: %.6g", progress_snapshot.best_fitness,
                    progress_snapshot.best_harga, progress_snapshot.best_kendala);
        ImGui::Text("Elapsed: %.1fs", progress_snapshot.elapsed_seconds);
        if (has_finished_) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.f), "Run complete.");
        } else if (progress_snapshot.converged) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.f), "Converged.");
        }
    }

    if (has_error_) {
        ImGui::SeparatorText("Error");
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.f), "%s", last_error_.c_str());
    }

    if (!running && dataset_path_.empty()) {
        ImGui::TextDisabled("Load or create a dataset (File > Open Data.../New Data) to enable Run.");
    }

    ImGui::End();
}

} // namespace orcisf::gui
