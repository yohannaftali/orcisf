#pragma once

#include <stdexcept>

#include "engine/StructureData.h"

// Port of Optimasi Beton/Source/Solver.hpp: modified-Choleski banded-matrix
// factorization/solve for the free-joint-displacement stiffness equations.
namespace orcisf::engine {

// Thrown instead of the legacy `banfac()`'s behavior of printing "Struktur
// tak stabil" to the console and calling exit(1) when the assembled
// stiffness matrix is not positive-definite (unstable structure).
class UnstableStructureError : public std::runtime_error {
public:
    UnstableStructureError()
        : std::runtime_error("Struktur tidak stabil (stiffness matrix bukan positif tentu)") {}
};

// In-place banded Choleski factorization of SFF (N x NB banded storage,
// legacy `banfac(N, NB, SFF)`).
void BandedFactorize(int N, int NB, LegacyArray2D<float>& SFF);

// Forward/back sweep against the factorized SFF to solve for X_DF given the
// combined load vector B_AC (legacy `bansol(N, NB, SFF, AC, DF)`).
void BandedSolve(int N, int NB, const LegacyArray2D<float>& U_SFF,
                  const LegacyArray<float>& B_AC, LegacyArray<float>& X_DF);

} // namespace orcisf::engine
