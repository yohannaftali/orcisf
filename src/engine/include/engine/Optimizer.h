#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <vector>

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
    // per-generation population evaluation. 0/1 = single-threaded (matches
    // the legacy program's behavior exactly, including RNG call order).
    // >1 parallelizes the embarrassingly-parallel per-candidate evaluation
    // loop (each candidate's analysis+design is independent once the
    // initial random population/search-direction bookkeeping -- which is
    // inherently sequential -- has been decided).
    unsigned int worker_threads = 1;
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

} // namespace orcisf::engine
