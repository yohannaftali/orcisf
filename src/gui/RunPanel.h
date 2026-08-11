#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "engine/Optimizer.h"
#include "engine/StructureData.h"

namespace orcisf::gui {

// Docked panel satisfying issue #4's "settings control lets the user choose
// how many CPU cores to use" + "cancellable from the GUI" + "progress
// callback ... reports accurate live state" acceptance criteria. Owns a
// background std::thread that runs engine::RunFullOptimization so the GUI
// frame loop never blocks; the engine's progress/detail callbacks fire on
// that worker thread, so their results are only ever written into
// `progress_` under `progress_mutex_` and read back out the same way from
// Draw() (main thread) -- everything else engine-side (the StructureData
// passed to on_detail) is NOT safe to touch off-thread and is intentionally
// not retained here.
class RunPanel {
public:
    RunPanel();
    ~RunPanel();

    // Called (from the UI thread, inside Draw()) whenever a message should
    // be surfaced to the user, e.g. wired to LogPanel::AddLine by Application.
    void SetLogSink(std::function<void(std::string)> sink);

    // Called (from the UI thread, inside Draw()) once, right after a run
    // finishes successfully (not cancelled, no error) -- with a *copy* of
    // the finished StructureData (so RunPanel's own result_sd_ stays valid
    // for the next run) and the dataset path that produced it. Wired by
    // Application to refresh the 3D viewport with the new results (#5).
    void SetOnResult(std::function<void(engine::StructureData, std::string)> callback);

    void Draw(bool* open);

    // Issue #14 (icon toolbar): lets an external "Run" button trigger the
    // exact same start path the panel's own button uses, gated the same
    // way (not already running, dataset_path_ set via this panel's own
    // "Dataset generic path" field -- there's no dataset-path parameter
    // here, this only starts a run the user has already configured).
    bool CanRun() const;
    void TriggerRun();

private:
    void StartRun();
    void RequestCancel();
    bool IsRunning() const;

    std::function<void(std::string)> log_sink_;
    std::function<void(engine::StructureData, std::string)> on_result_;

    // ---- Run configuration (edited on the UI thread only) ----
    char dataset_path_[512] = "";
    float harga_beton_ = 250000.f;
    float harga_besi_ = 5000.f;
    float selimut_kolom_ = 50.f;
    float selimut_balok_ = 50.f;
    float finalti_ = 1e10f;
    int j_iterasi_mak_ = 2000;
    int fak_plus_ = 3;
    int fak_kali_ = 1;
    int worker_threads_ = 1;
    unsigned int rng_seed_ = 0;

    // ---- Background run state ----
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> cancel_{false};

    mutable std::mutex progress_mutex_;
    engine::ProgressInfo progress_;
    bool has_progress_ = false;

    std::string last_error_;
    bool has_error_ = false;
    bool was_running_ = false; // detects the running:true -> false edge for a one-shot completion log line
    engine::StructureData result_sd_; // only touched while !running_

    // Issue #16: true only while result_sd_ holds a *completed* run's
    // final population (not a cancelled/errored run's partial state, and
    // not a run against a different dataset than dataset_path_ currently
    // names) -- gates the "Re-optimize from last best" checkbox below.
    // Cleared the moment a new run starts (StartRun() reads result_sd_'s
    // best slot into the new run's seed *before* clearing this, then
    // result_sd_ itself gets overwritten by the new run in progress).
    bool has_result_ = false;
    std::string result_dataset_path_; // dataset_path_ at the time result_sd_ was captured
    bool reoptimize_from_last_ = false;
};

} // namespace orcisf::gui
