#pragma once

#include "engine/StructureData.h"

// Port of Optimasi Beton/Source/Struktur.hpp and the computational parts of
// Pembebanan.hpp (beban(), berat_sendiri()) -- 3D space-frame direct-
// stiffness analysis, developed from Weaver & Gere. Every function takes
// the shared StructureData context explicitly instead of touching globals.
namespace orcisf::engine {

// Resolves a discretized design-variable index into its real value from one
// of the discrete tables (legacy `isi(no_data, kelompok_data)`).
inline float Isi(int no_data, const LegacyArray<float>& kelompok_data) {
    return kelompok_data[no_data];
}

// Computes EL/CX/Cy/CZ/CXZ for member `perb` (legacy `periksa_batang`).
void PeriksaBatang(StructureData& sd, int perb);

// Fills sd.IM[1..12] with member ib's global displacement-index vector
// (legacy `indeks_batang`).
void IndeksBatang(StructureData& sd, int ib);

// Fills sd.SM (local 12x12 stiffness) and sd.SMRT (rotated) for member imk
// (legacy `isi_matrik_kekakuan`). Requires PeriksaBatang(imk) to have been
// called first so CX/Cy/CZ/rotation matrices are current.
void IsiMatrikKekakuan(StructureData& sd, int imk);

// Computes b/h/AX/XI/YI/ZI for every member from the current design
// variables at sd.no_struktur (legacy `inersia`).
void Inersia(StructureData& sd);

// Assembles the rotation matrices and the banded free-DOF stiffness matrix
// SFF (legacy `kekakuan_batang`).
void KekakuanBatang(StructureData& sd);

// Assembles the combined joint-load vector AC from applied (AJ) and
// member-equivalent (AE) loads (legacy `beban`).
void Beban(StructureData& sd);

// Adds self-weight to member loads/joint loads based on current design
// variables at sd.no_struktur (legacy `berat_sendiri`). Also re-derives
// b/h for every member as a side effect, matching the legacy function.
void BeratSendiri(StructureData& sd);

// Post-processes the solved displacement vector (sd.DF) into member forces
// (sd.AM), support reactions (sd.AR), and the per-member force summaries
// used by beam/column design (MLAP, MTUM_KI/KA, GESER_KI/KA, PK, MKX, MKY,
// GK) -- legacy `hasil`.
void Hasil(StructureData& sd);

// Full analysis pipeline: KekakuanBatang -> BandedFactorize -> Beban ->
// BandedSolve -> Hasil (legacy `struktur()`). Throws UnstableStructureError
// if the assembled stiffness matrix is not positive-definite.
void Struktur(StructureData& sd);

// ---- Port of Optimasi Beton/Source/ELEMEN.HPP ----
//
// NOTE: these are intentionally *not* identical to CostConstraint.cpp's
// inline per-member field-filling (which mirrors Kendala.hpp's own,
// slightly different, inline copy of the same logic -- e.g. ELEMEN.HPP's
// VU takes fabs() of *both* GESER_KI and GESER_KA before comparing, while
// Kendala.hpp's inline version only takes fabs() of GESER_KI; ELEMEN.HPP's
// PU is `-PK/teta` while Kendala.hpp's is `fabs(PK/teta)`). Both
// discrepancies are real characteristics of the original 1999 source, not
// oversights in this port -- see CostConstraint.cpp's header comment.

// Reads member no_el_balok's current design variables from
// sd.var_b[sd.no_struktur] into B/H/DIA*lap/DIA*tum/NL*lap/NL*tum/DIAS/
// Jarak_S/L/VU (legacy `isi_elemen_balok`).
void IsiElemenBalokFields(StructureData& sd, int no_el_balok);

// Selects the midspan (lapangan) reinforcement into DIA1/DIA2/NL1/NL2 and
// computes MU from MLAP (legacy `elemen_lapangan`). Call after
// IsiElemenBalokFields.
void ElemenLapangan(StructureData& sd, int no_el_balok);

// Selects the support (tumpuan) reinforcement into DIA1/DIA2/NL1/NL2 and
// computes MU from MTUM_KI/MTUM_KA (legacy `elemen_tumpuan`). Call after
// IsiElemenBalokFields.
void ElemenTumpuan(StructureData& sd, int no_el_balok);

// Reads member no_el_kolom's current design variables from
// sd.var_k[sd.no_struktur] into sisi/DIA/N_DIA/DIAS/Jarak_S/PU/MUX/MUY/VU/L
// (legacy `isi_elemen_kolom`).
void IsiElemenKolomFields(StructureData& sd, int no_el_kolom);

} // namespace orcisf::engine
