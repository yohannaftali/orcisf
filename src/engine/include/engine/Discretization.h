#pragma once

#include <cmath>

// Port of Optimasi Beton/Source/Diskritisasi.hpp's konversi().
namespace orcisf::engine {

// Rounds a float design-variable value to the nearest discrete table index.
//
// The legacy source computes `fabs(bil_asli) - abs(bil_asli)`. Under
// Borland C++ 5.02's <stdlib.h>/<math.h> (no C++ `abs(double)` overload
// available), `abs(float)` resolves to `int abs(int)`, silently truncating
// bil_asli toward zero *before* taking its absolute value -- not the same
// as fabs(). `std::floor(std::fabs(bil_asli))` reproduces that exact
// quantity (floor of an already-non-negative value equals its truncation,
// which equals abs() of the original value's truncation), so this is not
// std::round -- it's a deliberate reproduction of that legacy behavior.
inline int Konversi(float bil_asli) {
    if ((std::fabs(bil_asli) - std::floor(std::fabs(bil_asli))) <= 0.5f) {
        bil_asli = std::floor(bil_asli);
    } else {
        bil_asli = std::ceil(bil_asli);
    }
    return static_cast<int>(bil_asli);
}

} // namespace orcisf::engine
