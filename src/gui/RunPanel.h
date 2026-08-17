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
    // for the next run), the input dataset path that produced it, and the
    // generic *output* path (issue #25) the run's .opt/.str/.kdl/.inf/.his/
    // .log.txt actually landed in (a fresh timestamped subfolder, see
    // engine::RunFullOptimization's comment -- never the same as the input
    // path). Wired by Application to refresh the 3D viewport (#5) and to
    // know where to find those files for later export (#9).
    void SetOnResult(std::function<void(engine::StructureData, std::string dataset_path, std::string output_path)>
                          callback);

    // Issue #25: which dataset this panel will run against -- always
    // whatever Application currently has loaded (File > Open Data/New
    // Data/Save As), not a separately user-typed path. Call once per
    // frame, before Draw()/CanRun() (Application::OnFrame() does this
    // right at the top, ahead of the icon toolbar's CanRun() check).
    void SetDatasetPath(std::string path);

    void Draw(bool* open);

    // Issue #14 (icon toolbar): lets an external "Run" button trigger the
    // exact same start path the panel's own button uses, gated the same
    // way (not already running, a dataset loaded via SetDatasetPath()).
    bool CanRun() const;
    void TriggerRun();

private:
    void StartRun();
    void RequestCancel();
    bool IsRunning() const;

    std::function<void(std::string)> log_sink_;
    std::function<void(engine::StructureData, std::string, std::string)> on_result_;

    // ---- Run configuration (edited on the UI thread only) ----
    // Issue #25: no longer a separately user-typed field (was `char
    // dataset_path_[512]`, editable via its own InputText) -- always kept
    // in sync with Application's loaded dataset via SetDatasetPath().
    std::string dataset_path_;
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
    std::string result_output_path_;  // issue #25: the generic output path that run actually wrote to
    bool reoptimize_from_last_ = false;

    // Issue #63: true only for the one-shot completion transition (set in
    // the was_running_->!running edge, cleared the moment a new run starts)
    // -- see Draw()'s Progress section for why this exists. `progress_`
    // itself is only ever written by the engine's progress callback *during*
    // a run (see engine::RunFullOptimization's progress_wrapper), which can
    // legitimately stop issuing callbacks one generation before
    // max_generation on early convergence (the loop detects convergence
    // using the *previous* generation's state and breaks before emitting
    // that generation's own callback) -- without this flag, a finished run
    // left the Progress section frozen on that last mid-run snapshot
    // (e.g. "98% / Generation 39 of 40") forever, with no visual signal
    // that the run had actually completed. This was the real, complete
    // root cause of issue #63's "hang": the engine and every completion
    // step (LoadStructure, on_result_, ...) had already finished --
    // confirmed via engine::DebugLog() timestamps -- the GUI was never
    // actually stuck, it was just displaying stale progress text
    // indefinitely, which read exactly like a hang to a human watching it
    // (Cancel appearing unresponsive was likewise just it being correctly
    // disabled once running_ was already false, not evidence of a freeze).
    bool has_finished_ = false;
};

} // namespace orcisf::gui
