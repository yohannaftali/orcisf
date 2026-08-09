#pragma once

#include "engine/StructureData.h"

// Port of Optimasi Beton/Source/Kolom.hpp (biaxial RC column design,
// symmetric reinforcement, per SK SNI T-15-1991-03 + the Hulse & Mosley
// (1986) biaxial contour-of-load method).
//
// Inputs consumed from `sd`: sisi, DIA, N_DIA, DIAS, Jarak_S, FC, FY, FYS,
// PU, MUX, MUY, VU, L, selimut_kolom.
// Outputs written to `sd`: kendala, harga, FPU, FMU, plus the intermediate
// kendala_r[_min|_mak]/kendala_tul/kendala_kelangsingan/kendala_gaya/
// kendala_po/kendala_pn/kendala_mn/Sref/jarak_antar_tulangan fields.
namespace orcisf::engine {

void DesignColumn(StructureData& sd);

} // namespace orcisf::engine
