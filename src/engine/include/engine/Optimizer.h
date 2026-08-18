#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <vector>

#include "engine/AnalysisResults.h"
#include "engine/MemberResults.h"
#include "engine/StructureData.h"

// Port of the "Flexible Polyhedron" direct-search optimizer: Polyhedron.hpp
// (driver), Pengacakan.hpp (initial population), Penormalan.hpp (packing),
// Telusur.hpp (search direction), Baru.hpp (reflect/shrink), Pengurutan.hpp
// (sort), Diskritisasi.hpp (rounding).
namespace orcisf::engine {

struct OptimizationOptions {
    float harga_beton = 0.f;   // Rp./m^3
    float harga_besi = 0.f;    // Rp./kg
    float selimut_kolom = 0.f; // mm
    float selimut_balok = 0.f; // mm
    float finalti = 0.f;       // penalty factor
    int j_iterasi_mak = 0;     // max generations
    int fak_plus = 0;          // JSTD = JVD*fak_kali + fak_plus
    int fak_kali = 0;

    // New in this port (issue #4): how many worker threads to use for the
    // "heavy iteration" evaluation loops -- the initial-population build
    // (generation 0) and, more importantly, cari_baru()'s per-generation
    // trial search (Baru.hpp), which is what actually dominates wall-clock
    // time across a full run. 0/1 = single-threaded (identical code path
    // to before this option existed). >1 spawns `worker_threads - 1`
    // additional threads, each with its own private StructureData clone
    // (see Optimizer.cpp's "Parallel evaluation" section for why a private
    // clone per thread is required for correctness, not just performance).
    // Numeric results for a given `rng_seed` are identical regardless of
    // worker_threads (verified: see engine/README.md's Validation section)
    // -- more threads only changes wall-clock time, not the answer.
    unsigned int worker_threads = 1;

    // 0 = seed the initial population's RNG from std::random_device (the
    // default -- matches "randomize()"'s intent, a different population
    // every run). Any other value seeds std::mt19937 directly, making a
    // run fully reproducible -- used to verify that worker_threads doesn't
    // change the result, and available to callers that want reproducible
    // runs for testing/debugging.
    unsigned int rng_seed = 0;

    // Issue #16: "re-optimize from last best result". When true and
    // seed_var_b/seed_var_k are non-empty and match this run's member
    // classification exactly in size (12*jum_balok / 5*jum_kolom -- only
    // known once PrepareOptimization() has run, so RunOptimization()
    // validates the size match itself), population slot 0 is seeded with
    // this design instead of being randomized, so the search continues
    // from an already-decent starting point rather than from scratch. A
    // size mismatch (e.g. the dataset's geometry was edited since the
    // seed was captured, changing which/how many members are beams vs.
    // columns) is treated as "no usable seed" and silently falls back to
    // a normal random slot 0 -- this is an engine-level safety net, not
    // where user-facing validation belongs (see gui/RunPanel.cpp for the
    // GUI-side check that only offers this option after a real completed
    // run against the same dataset).
    //
    // Doesn't affect determinism (see engine/README.md's "Threading
    // determinism" section): seeding replaces one RNG-consuming call
    // (Randomisasi() for slot 0) with a plain copy, entirely inside the
    // single-threaded population-generation step that runs before any
    // worker threads exist -- worker_threads still only changes
    // wall-clock time, never the numeric result, with or without seeding.
    bool seed_from_previous_best = false;
    std::vector<int> seed_var_b; // flat, 0-based, [jsum + 12*isum] layout matching StructureData::var_b's row
    std::vector<int> seed_var_k; // flat, 0-based, [jsum + 5*isum] layout matching StructureData::var_k's row

    // Issue #77: practically, a beam is almost always built with the same
    // bar diameter/count at midspan (lapangan) and support (tumpuan) --
    // the same simplification a column already gets via one N_DIA/DIA
    // shared across all four sides. When true (default), every candidate
    // design's tumpuan bar slots (var_b indices 6-9 per beam: DIA1tum/
    // NL1tum/DIA2tum/NL2tum) are forced to mirror the lapangan slots
    // (indices 2-5) right before each fitness evaluation -- see
    // Optimizer.cpp's MirrorBeamTumpuan(). This does NOT shrink JVD/the
    // 12-slot beam layout (that's load-bearing throughout the optimizer,
    // see AGENTS.md's "Discrete design variables" section) -- the tumpuan
    // slots are still generated/searched like any other variable, but
    // always overwritten before they can affect a design's evaluated
    // fitness, so they have zero effect on the optimization outcome and
    // every kept design ends up symmetric. Matches the same "mirror at
    // evaluate time" pattern issue #74 already established for Analyze
    // mode's own "same lapangan/tumpuan" checkbox.
    bool symmetric_beam_reinforcement = true;

    // Issue #78: real construction practice runs an even number of
    // longitudinal bars (bars come in symmetric pairs), minimum 4 -- odd
    // counts are unusual/impractical, and 1-3 bars is not a valid
    // beam/column reinforcement choice at all. When true (default), every
    // candidate design's bar-count slots (beam var_b indices 3/5/7/9,
    // reading the shared NL_d discrete table; column var_k index 2) are
    // snapped to the nearest table index whose resolved value is even and
    // >= 4, right before each fitness evaluation (same two call sites as
    // symmetric_beam_reinforcement above -- see Optimizer.cpp's
    // SnapBarCountIndex()). When false, odd counts become reachable too,
    // but the >= 4 floor still applies either way.
    bool even_bar_count_only = true;
};

struct ProgressInfo {
    int generation = 0;
    int max_generation = 0;
    float best_fitness = 0.f;
    float best_harga = 0.f;
    float best_kendala = 0.f;
    double elapsed_seconds = 0.0;
    bool converged = false;
};

// Called once per generation with a progress snapshot. Returning false
// requests cancellation (checked between generations, same granularity as
// the cancellation token below).
using ProgressCallback = std::function<bool(const ProgressInfo&)>;

// Called once per generation, right after that generation's population has
// been sorted, with full read access to `sd` -- satisfying issue #3's
// "detailed calculation log" requirement (design variables via
// sd.var_b/var_k[0..JSTD-1], fitness/kendala/harga via
// sd.fitstr/kendalastr/hargastr[0..JSTD-1], index JSTD-1 = current best).
// `sd` must not be mutated by the callback; it's only valid for the
// duration of the call (the optimizer mutates it again on the next
// generation).
using DetailLogCallback = std::function<void(int generation, const StructureData& sd)>;

// Runs the Flexible Polyhedron optimization to convergence or
// options.j_iterasi_mak generations, whichever comes first.
//
// Preconditions: sd must already have a dataset loaded (LegacyIO::
// ReadDataset + ReadLoads) and sd.jum_balok/jum_kolom/no_balok/no_kolom/
// JVD/JSTD/nvb/nvk/nvm populated the same way Polyhedron.hpp's optimasi()
// does at its start (member classification, design-variable count,
// population size, discrete-table upper bounds).
//
// `cancel`, if non-null, is polled between generations; when it becomes
// true the run stops early with whatever the current best is (matching
// the "cancellable" requirement in issue #3/#4 -- the legacy program had
// no such mechanism).
void RunOptimization(StructureData& sd, const OptimizationOptions& options, const ProgressCallback& on_progress,
                      const DetailLogCallback& on_detail, const std::atomic<bool>* cancel = nullptr);

// Port of Polyhedron.hpp's member-classification + design-variable-count
// bookkeeping that optimasi() does before generating the first population
// (periksa_batang-based beam/column split, JVD, JSTD). Call once after
// loading the dataset and before RunOptimization().
void PrepareOptimization(StructureData& sd, const OptimizationOptions& options);

// Issue #68 (epic #67 -- manual-dimension "Analyze" mode): runs analysis +
// the existing RC design-check logic against a single, caller-supplied
// design-variable set -- no Flexible Polyhedron search, no population, no
// cost minimization. Formalizes the ad hoc pattern orcisf_cli's
// CmdEquilibrium() already used (write to population slot 0, call
// Inersia()+Struktur() directly) into a real, documented, reusable engine
// entry point.
struct AnalyzeResult {
    std::vector<MemberResult> member_results; // one per beam/column, via ComputeMemberResults()
    AnalysisResults analysis;                 // via ComputeAnalysisResults()
};

// `sd` must already have a dataset loaded (LegacyIO::ReadDataset +
// ReadLoads/ReadLoadsRaw), the same precondition RunOptimization() has.
// Calls PrepareOptimization(sd, options) itself -- the caller must NOT
// call it first (harmless if already called, since PrepareOptimization
// only re-derives sd.jum_balok/jum_kolom/JVD/JSTD from sd.M's live
// PeriksaBatang() classification, but redundant).
//
// var_b/var_k: flat, 0-based design-variable index arrays, the exact
// [jsum + 12*isum] / [jsum + 5*isum] layout OptimizationOptions::
// seed_var_b/seed_var_k already use (12 slots per beam, 5 per column, in
// sd.no_balok/no_kolom order -- see AGENTS.md's "Discrete design
// variables" section). Sizes must be exactly 12*jum_balok / 5*jum_kolom
// (only known once this function's own PrepareOptimization() call has
// run) and every index must be within its discrete table's own upper
// bound -- both throw std::invalid_argument rather than silently
// clamping or writing out of bounds, since a manually-chosen Analyze-mode
// design should never be altered by the engine without the caller
// knowing.
AnalyzeResult AnalyzeFixedDesign(StructureData& sd, const OptimizationOptions& options, const std::vector<int>& var_b,
                                  const std::vector<int>& var_k);

} // namespace orcisf::engine
