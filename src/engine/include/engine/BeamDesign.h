#pragma once

#include "engine/StructureData.h"

// Port of Optimasi Beton/Source/Balok.hpp (RC beam design: doubly-reinforced
// rectangular section, ultimate strength design per SK SNI T-15-1991-03)
// and its `lendutan()` deflection check.
//
// The legacy `class balok` did all its work as constructor side effects
// against shared globals (`balok lapangan;` instantiated as a bare local
// purely to trigger the calculation). DesignBeam() reproduces that same
// side-effecting-calculation shape against an explicit StructureData&
// instead, since that pattern (not the class-as-trigger idiom) is what
// AGENTS.md's "no bare globals where avoidable" constraint is about.
//
// Inputs consumed from `sd` (must be set by the caller first): B, H, DIA1,
// DIA2, NL1, NL2, FC, FY, FYS, DIAS, Jarak_S, L, VU, MU, selimut_balok.
// Outputs written to `sd`: kendala, harga, volume_beton, berat_besi,
// berat_sengkang, plus the intermediate kendala_rho[_b|_m]/kendala_M/
// kendala_sb/Sref/BT1/AS/AS1/D/DS fields (all consumed by CETAK/Kendala
// output).
namespace orcisf::engine {

void DesignBeam(StructureData& sd);

// Deflection check for member `no_batang_l` per SK SNI T-15-1991-03 3.2.5.
// Reads sd.B/H/DIA1/DIA2/NL1/NL2/selimut_balok/FC/E and sd.MLAP[no_batang_l];
// writes sd.LENDUTAN, sd.LENDUTAN_IJIN, sd.kendala_lendutan (and the
// intermediate n/fr/LGN/Icr/Ig/Ie/Mcr/Lambda/AS/AS1 fields).
void Lendutan(StructureData& sd, int no_batang_l);

} // namespace orcisf::engine
