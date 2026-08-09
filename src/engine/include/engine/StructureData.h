#pragma once

#include <algorithm>
#include <string>
#include <vector>

// This header is a faithful, line-for-line port of the global state declared
// in Optimasi Beton/Source/Variabel.hpp into a single owned (RAII) context
// object instead of bare globals. Field names, types, and the 1-based
// indexing convention (vectors sized kMak, index 0 unused) are kept
// identical to the legacy source on purpose: the numerical formulas that
// consume this state (StructuralAnalysis, BeamDesign, ColumnDesign,
// Optimizer) are transcribed the same way, and a 1:1 name mapping against
// Optimasi Beton/Source/*.hpp is what makes that transcription checkable.
// See AGENTS.md ("Optimasi Beton/ is a historical archive") -- this is a
// new file, not an edit to the legacy source, but keeps its identifiers for
// traceability. Comments are in English (new code); the ported formulas'
// intent is documented against the Indonesian legacy names.

namespace orcisf::engine {

inline constexpr int kMak = 825; // mirrors ORCISF.cpp's `const int mak=825;`
inline constexpr int kMD = 12;   // mirrors Variabel.hpp's `#define MD 12`

inline constexpr float kPi = 3.14f;
inline constexpr float kTeta = 0.8f;       // strength reduction factor used throughout Kendala_Harga-equivalent code
inline constexpr float kLimitNol = 1.E-3f; // "limit_nol"
inline constexpr float kBjBesi = 7850.f;   // steel unit weight (kg/m^3)

// A fixed-size, 1-based vector: index 0 is unused, matching every legacy
// `float X[mak]` / `int JRL[mak]` access pattern (`X[1]..X[NJ]`).
template <typename T>
class LegacyArray {
public:
    LegacyArray() : data_(kMak, T{}) {}
    T& operator[](int i) { return data_[static_cast<size_t>(i)]; }
    const T& operator[](int i) const { return data_[static_cast<size_t>(i)]; }
    void assign_all(T value) { std::fill(data_.begin(), data_.end(), value); }

private:
    std::vector<T> data_;
};

// 2D legacy array, e.g. `float AML[13][mak]` or `int var_b[mak][mak]`.
template <typename T>
class LegacyArray2D {
public:
    LegacyArray2D(int rows, int cols) : rows_(rows), cols_(cols), data_(static_cast<size_t>(rows) * cols, T{}) {}
    T* operator[](int row) { return &data_[static_cast<size_t>(row) * cols_]; }
    const T* operator[](int row) const { return &data_[static_cast<size_t>(row) * cols_]; }

private:
    int rows_, cols_;
    std::vector<T> data_;
};

// Faithful port of every field in Variabel.hpp. Grouped the same way the
// original file groups them, for side-by-side comparison.
struct StructureData {
    // ---- Variabel umum (file names / structure name) ----
    std::string ISN;    // nama struktur
    std::string sub_name;

    // ---- Mekanika rekayasa portal ruang ----
    int NB = 0, M = 0, NJ = 0, NR = 0, NRJ = 0;
    LegacyArray<int> JRL, T_K;
    int ND = 0, N = 0;
    LegacyArray<int> IA, JJ, JK, ID;
    int IR = 0, IC = 0;
    LegacyArray<int> IM, LML;

    float E = 0.f, G = 0.f;
    LegacyArray<float> X, Y, Z;
    LegacyArray<float> b, h, AX, XI, YI, ZI, EL;

    float CX = 0.f, Cy = 0.f, CZ = 0.f, CXZ = 0.f;
    LegacyArray<float> XP, YP, ZP;
    float XPS = 0.f, YPS = 0.f, ZPS = 0.f, YPG = 0.f, ZPG = 0.f;
    float COSA = 0.f, SINA = 0.f;

    LegacyArray<float> R11, R12, R13, R21, R22, R23, R31, R32, R33;

    LegacyArray2D<float> SM{13, 13};
    LegacyArray2D<float> SMRT{13, 13};
    LegacyArray2D<float> SMS{kMak, kMak};
    LegacyArray2D<float> SFF{kMak, kMak};
    LegacyArray<float> DF, AJ;
    LegacyArray2D<float> AML{13, kMak};
    LegacyArray<float> AE, AC, DJ, AMD;
    LegacyArray2D<float> AM{kMak, kMak};
    LegacyArray<float> AR, W, W_Balok, P_Kolom;

    LegacyArray<float> MTUM_KI, MTUM_KA, MLAP, GESER_KI, GESER_KA;
    LegacyArray<float> MKX, MKY, PK, GK;

    // ---- Beton bertulang: data masukan umum ----
    float FC = 0.f, FY = 0.f, FYS = 0.f, BT1 = 0.f;

    // ---- Variabel desain sharing ----
    int nsisi_B = 0, nsisi_H = 0, nsisi_K = 0;
    int nDIA = 0, nNL = 0, nDIAS = 0, nJS = 0;
    float DIAS = 0.f, Jarak_S = 0.f, Sref = 0.f, jarak_antar_tulangan = 0.f;
    LegacyArray<float> sisi_d_B, sisi_d_H, sisi_d_K;
    LegacyArray<float> DIA_d, NL_d, DIAS_d, JS_d;

    // ---- Variabel desain balok ----
    float B = 0.f, H = 0.f, DIA1 = 0.f, DIA2 = 0.f, NL1 = 0.f, NL2 = 0.f;

    // ---- Variabel desain kolom ----
    float sisi = 0.f, DIA = 0.f, N_DIA = 0.f;

    // ---- Sharing variabel gaya ----
    float PU = 0.f, MU = 0.f, VU = 0.f, VC = 0.f, VS = 0.f, FMU = 0.f, FPU = 0.f;

    // ---- Sharing variabel biaya ----
    float harga_beton = 0.f, harga_besi = 0.f;
    float volume_beton = 0.f, berat_besi = 0.f, berat_sengkang = 0.f;
    float L = 0.f;

    // ---- Sharing variabel elemen ----
    float DS = 0.f, D = 0.f, AV = 0.f, AVmin = 0.f;

    // ---- Variabel khusus balok ----
    float selimut_balok = 0.f;
    float DIA1lap = 0.f, NL1lap = 0.f, DIA2lap = 0.f, NL2lap = 0.f;
    float DIA1tum = 0.f, NL1tum = 0.f, DIA2tum = 0.f, NL2tum = 0.f;
    float AS = 0.f, AS1 = 0.f, HMIN = 0.f, n = 0.f, fr = 0.f;
    float LGN = 0.f, Icr = 0.f, Ig = 0.f, Ie = 0.f, Mcr = 0.f;
    float LENDUTAN = 0.f, LENDUTAN_IJIN = 0.f, Lambda = 0.f;

    // ---- Variabel khusus kolom ----
    float MUX = 0.f, MUY = 0.f, selimut_kolom = 0.f, _K = 0.f;

    // ---- Optimasi: flexible polyhedron ----
    int jum_susut = 0, fak_plus = 0, fak_kali = 0, no_struktur = 0;
    int JVD = 0, JSTD = 0, j_iterasi_mak = 0;
    LegacyArray<int> nvm, nvk, nvb;
    int iterasi_var = 0;
    LegacyArray2D<int> var_b{kMak, kMak};
    LegacyArray2D<int> var_k{kMak, kMak};
    LegacyArray<int> tanda_arah_b, tanda_arah_k;
    LegacyArray<int> var_b_jelek, var_k_jelek, arah;
    int no_TS_terjauh = 0;
    LegacyArray<int> varnew_asli, varnew;
    LegacyArray<int> var_b_cb, var_k_cb, var_b_cb_best, var_k_cb_best;
    LegacyArray<int> no_balok, no_kolom;
    int jum_balok = 0, jum_kolom = 0, js_balok = 0, js_kolom = 0, lompat = 0;

    float patok_fit = 0.f, finalti = 0.f;
    LegacyArray<float> fitstr;
    float fitcb = 0.f, fitcb_best = 0.f;
    LegacyArray<float> kendalastr, hargastr;
    float kendala = 0.f, kendalasa = 0.f, harga = 0.f;
    LegacyArray<float> XM_b, XM_k, XS_b, XS_k, TM, TS, varplus, varnew_f;

    // ---- Kendala kolom biaksial ----
    float kendala_gaya = 0.f, kendala_po = 0.f, kendala_pn = 0.f, kendala_mn = 0.f;
    float kendala_r = 0.f, kendala_r_min = 0.f, kendala_r_mak = 0.f;
    float kendala_sengkang = 0.f, kendala_tul = 0.f, kendala_kelangsingan = 0.f;

    // ---- Kendala balok ----
    float kendala_rho = 0.f, kendala_rho_b = 0.f, kendala_rho_m = 0.f;
    float kendala_sb = 0.f, kendala_M = 0.f, kendala_lendutan = 0.f;
};

} // namespace orcisf::engine
