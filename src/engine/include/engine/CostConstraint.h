#pragma once

#include "engine/StructureData.h"

// Port of Optimasi Beton/Source/Kendala.hpp's Kendala_Harga(): for a given
// (already-analyzed) structure and a candidate discrete design-variable
// set, runs every beam's and column's design check and returns the
// exterior-penalty-function fitness value.
//
// Two legacy quirks are preserved deliberately (see Kendala.hpp):
//  1. It does *not* re-derive member forces from var_b_nya/var_k_nya --
//     it reuses whatever MLAP/MTUM_*/GESER_*/PK/MKX/MKY/GK are already in
//     `sd` from the most recent Struktur(sd) call. The legacy source did
//     call baca_data()/baca_beban()/inersia()/struktur() again at the top
//     of every single invocation, but always against the same frozen
//     `no_struktur`, so those calls were idempotent after the first one --
//     re-running them here would only waste time (and break thread-safety
//     for issue #4's parallel evaluation), not change the result, so
//     they're deliberately omitted. The caller is responsible for calling
//     Struktur(sd) once before evaluating any candidates against a given
//     frozen geometry.
//  2. It adaptively tightens each column's stirrup spacing (and, if the
//     slenderness constraint is violated, its side length) to the
//     smallest discrete value that still satisfies `Sref`/the slenderness
//     limit, **mutating `var_k_nya` in place** -- but does *not* do the
//     equivalent adjustment for beams (unlike the optimizer's first-
//     generation/end-of-generation evaluation path, which adjusts both).
//     This asymmetry is a real characteristic of the original algorithm,
//     not an oversight in this port -- see Optimizer.cpp's
//     EvaluateCandidateFull() for the beam+column version used elsewhere.
namespace orcisf::engine {

float KendalaHarga(StructureData& sd, const LegacyArray<int>& var_b_nya, LegacyArray<int>& var_k_nya);

} // namespace orcisf::engine
