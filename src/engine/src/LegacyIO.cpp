#include "engine/LegacyIO.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "engine/BeamDesign.h"
#include "engine/ColumnDesign.h"
#include "engine/StructuralAnalysis.h"

namespace orcisf::engine {

namespace {

std::ifstream OpenRead(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        throw std::runtime_error("Tidak dapat membuka file: " + path);
    }
    return f;
}

std::ofstream OpenWrite(const std::string& path) {
    std::ofstream f(path);
    if (!f) {
        throw std::runtime_error("Tidak dapat menulis file: " + path);
    }
    return f;
}

} // namespace

DatasetPaths MakeDatasetPaths(const std::string& generic) {
    DatasetPaths p;
    p.inp = generic + ".inp";
    p.isd = generic + ".isd";
    p.idl = generic + ".idl";
    p.ijl = generic + ".ijl";
    p.ids = generic + ".ids";
    p.ijs = generic + ".ijs";
    p.bbn = generic + ".bbn";
    p.opt = generic + ".opt";
    p.str = generic + ".str";
    p.kdl = generic + ".kdl";
    p.inf = generic + ".inf";
    p.his = generic + ".his";
    p.log_detail = generic + ".log.txt";
    return p;
}

void ReadStructureFile(StructureData& sd, const std::string& inp_path) {
    std::ifstream baca = OpenRead(inp_path);
    baca >> sd.ISN;
    baca >> sd.M;
    baca >> sd.NJ;
    baca >> sd.NRJ;
    baca >> sd.NR;
    baca >> sd.E;
    baca >> sd.G;
    baca >> sd.FC;
    baca >> sd.FY;
    baca >> sd.FYS;
    baca >> sd.ND;
    baca >> sd.N;
    baca >> sd.sub_name; // "[Koordinat]"
    for (int kinp = 1; kinp <= sd.NJ; ++kinp) {
        int idx;
        baca >> idx;
        baca >> sd.X[kinp];
        baca >> sd.Y[kinp];
        baca >> sd.Z[kinp];
    }
    baca >> sd.sub_name; // "[Pengekang]"
    for (int iinp = 1; iinp <= sd.NRJ; ++iinp) {
        baca >> sd.T_K[iinp];
        baca >> sd.JRL[6 * sd.T_K[iinp] - 5];
        baca >> sd.JRL[6 * sd.T_K[iinp] - 4];
        baca >> sd.JRL[6 * sd.T_K[iinp] - 3];
        baca >> sd.JRL[6 * sd.T_K[iinp] - 2];
        baca >> sd.JRL[6 * sd.T_K[iinp] - 1];
        baca >> sd.JRL[6 * sd.T_K[iinp]];
    }
    baca >> sd.sub_name; // "[Batang]" / "[InformasiBatang]"
    for (int iinp = 1; iinp <= sd.M; ++iinp) {
        int idx;
        baca >> idx;
        baca >> sd.JJ[iinp];
        baca >> sd.JK[iinp];
        baca >> sd.IA[iinp];
        if (sd.IA[iinp] != 0) {
            baca >> sd.XP[iinp];
            baca >> sd.YP[iinp];
            baca >> sd.ZP[iinp];
        }
    }
}

void ReadDiscreteTables(StructureData& sd, const DatasetPaths& paths) {
    {
        std::ifstream baca1 = OpenRead(paths.isd);
        baca1 >> sd.sub_name; // "[LebarBalok]"
        baca1 >> sd.nsisi_B;
        for (int i = 0; i < sd.nsisi_B; ++i) baca1 >> sd.sisi_d_B[i];
        baca1 >> sd.sub_name; // "[TinggiBalok]"
        baca1 >> sd.nsisi_H;
        for (int i = 0; i < sd.nsisi_H; ++i) baca1 >> sd.sisi_d_H[i];
        baca1 >> sd.sub_name; // "[SisiKolom]"
        baca1 >> sd.nsisi_K;
        for (int i = 0; i < sd.nsisi_K; ++i) baca1 >> sd.sisi_d_K[i];
    }
    {
        std::ifstream baca2 = OpenRead(paths.idl);
        baca2 >> sd.sub_name;
        baca2 >> sd.nDIA;
        for (int i = 0; i < sd.nDIA; ++i) baca2 >> sd.DIA_d[i];
    }
    {
        std::ifstream baca3 = OpenRead(paths.ijl);
        baca3 >> sd.sub_name;
        baca3 >> sd.nNL;
        for (int i = 0; i < sd.nNL; ++i) baca3 >> sd.NL_d[i];
    }
    {
        std::ifstream baca4 = OpenRead(paths.ids);
        baca4 >> sd.sub_name;
        baca4 >> sd.nDIAS;
        for (int i = 0; i < sd.nDIAS; ++i) baca4 >> sd.DIAS_d[i];
    }
    {
        std::ifstream baca5 = OpenRead(paths.ijs);
        baca5 >> sd.sub_name;
        baca5 >> sd.nJS;
        for (int i = 0; i < sd.nJS; ++i) baca5 >> sd.JS_d[i];
    }
}

RestraintSummary ComputeRestraintSummary(const StructureData& sd) {
    RestraintSummary summary;
    for (int j = 1; j <= sd.NJ; ++j) {
        bool any = false;
        for (int dof = 0; dof < 6; ++dof) {
            if (sd.JRL[6 * j - 5 + dof] == 1) {
                any = true;
                ++summary.nr;
            }
        }
        if (any) {
            ++summary.nrj;
            summary.restrained_joints.push_back(j);
        }
    }
    summary.nd = 6 * sd.NJ;
    summary.n = summary.nd - summary.nr;
    return summary;
}

void WriteStructureFile(const StructureData& sd, const std::string& inp_path) {
    std::ofstream tulis = OpenWrite(inp_path);

    // Recompute the derived counts from the current JRL rather than trust
    // sd.NRJ/NR/ND/N -- see header comment.
    RestraintSummary summary = ComputeRestraintSummary(sd);
    int nrj = summary.nrj, nr = summary.nr;

    tulis << sd.ISN << "\n";
    tulis << sd.M << "\n";
    tulis << sd.NJ << "\n";
    tulis << nrj << "\n";
    tulis << nr << "\n";
    tulis << sd.E << "\n";
    tulis << sd.G << "\n";
    tulis << sd.FC << "\n";
    tulis << sd.FY << "\n";
    tulis << sd.FYS << "\n";
    tulis << summary.nd << "\n";
    tulis << summary.n << "\n";

    tulis << "[Koordinat]\n";
    for (int j = 1; j <= sd.NJ; ++j) {
        tulis << j << "\n" << sd.X[j] << "\n" << sd.Y[j] << "\n" << sd.Z[j] << "\n";
    }

    tulis << "[Pengekang]\n";
    for (int j = 1; j <= sd.NJ; ++j) {
        bool any = false;
        for (int dof = 0; dof < 6; ++dof) {
            if (sd.JRL[6 * j - 5 + dof] == 1) any = true;
        }
        if (!any) continue;
        tulis << j << "\n";
        for (int dof = 0; dof < 6; ++dof) {
            tulis << sd.JRL[6 * j - 5 + dof] << "\n";
        }
    }

    tulis << "[InformasiBatang]\n";
    for (int i = 1; i <= sd.M; ++i) {
        tulis << i << "\n" << sd.JJ[i] << "\n" << sd.JK[i] << "\n" << sd.IA[i] << "\n";
        if (sd.IA[i] != 0) {
            tulis << sd.XP[i] << "\n" << sd.YP[i] << "\n" << sd.ZP[i] << "\n";
        }
    }
}

void WriteInfPreview(const StructureData& sd, const std::string& inf_path) {
    std::ofstream hinf = OpenWrite(inf_path);
    RestraintSummary summary = ComputeRestraintSummary(sd);

    hinf << "  Struktur Portal Ruang " << sd.ISN << "\n\n";
    hinf << "  Parameter Struktur\n";
    hinf << "  Jumlah batang : " << sd.M << "\n";
    hinf << "  DOF : " << summary.n << "\n";
    hinf << "  Jumlah joint : " << sd.NJ << "\n";
    hinf << "  Jumlah pengekang tumpuan : " << summary.nr << "\n";
    hinf << "  Jumlah titik kumpul yang dikekang : " << summary.nrj << "\n";
    hinf << "  Modulus Elastisitas aksial : " << sd.E << " N/m^2\n";
    hinf << "  Modulus Geser : " << sd.G << " N/m^2\n\n";
    hinf << "  Properti Elemen Material\n";
    hinf << "  Kuat desak beton karakteristik : " << sd.FC << " MPa\n";
    hinf << "  Kuat tarik baja tulangan : " << sd.FY << " MPa\n";
    hinf << "  Kuat tarik tulangan sengkang : " << sd.FYS << " MPa\n";

    hinf << "\n  Koordinat Titik Kumpul (m)\n";
    hinf << "  Titik     X             Y             Z \n";
    for (int kout = 1; kout <= sd.NJ; ++kout) {
        hinf << "  " << kout << "\t" << sd.X[kout] << "\t" << sd.Y[kout] << "\t" << sd.Z[kout] << "\n";
    }

    hinf << "\n  Informasi Batang \n";
    hinf << "  Batang  JJ      JK      IA\n";
    for (int iout = 1; iout <= sd.M; ++iout) {
        hinf << "  " << iout << "\t" << sd.JJ[iout] << "\t" << sd.JK[iout] << "\t" << sd.IA[iout] << "\n";
        if (sd.IA[iout] != 0) {
            hinf << "  XP = " << sd.XP[iout] << "  YP = " << sd.YP[iout] << "  ZP = " << sd.ZP[iout] << "\n\n";
        }
    }

    hinf << "\n  Pengekang Titik Kumpul \n";
    hinf << "  Titik     JR1   JR2   JR3   JR4   JR5   JR6\n";
    for (int joint : summary.restrained_joints) {
        hinf << "  " << joint;
        for (int k = 5; k >= 0; --k) {
            hinf << "\t" << sd.JRL[6 * joint - k];
        }
        hinf << "\n";
    }
}

void WriteDiscreteTables(const StructureData& sd, const DatasetPaths& paths) {
    {
        std::ofstream t = OpenWrite(paths.isd);
        t << "[LebarBalok]\n" << sd.nsisi_B << "\n";
        for (int i = 0; i < sd.nsisi_B; ++i) t << sd.sisi_d_B[i] << "\n";
        t << "[TinggiBalok]\n" << sd.nsisi_H << "\n";
        for (int i = 0; i < sd.nsisi_H; ++i) t << sd.sisi_d_H[i] << "\n";
        t << "[SisiKolom]\n" << sd.nsisi_K << "\n";
        for (int i = 0; i < sd.nsisi_K; ++i) t << sd.sisi_d_K[i] << "\n";
    }
    {
        std::ofstream t = OpenWrite(paths.idl);
        t << "[DiameterTulanganUtama]\n" << sd.nDIA << "\n";
        for (int i = 0; i < sd.nDIA; ++i) t << sd.DIA_d[i] << "\n";
    }
    {
        std::ofstream t = OpenWrite(paths.ijl);
        t << "[JumlahTulanganUtama]\n" << sd.nNL << "\n";
        for (int i = 0; i < sd.nNL; ++i) t << sd.NL_d[i] << "\n";
    }
    {
        std::ofstream t = OpenWrite(paths.ids);
        t << "[DiameterTulanganSengkang]\n" << sd.nDIAS << "\n";
        for (int i = 0; i < sd.nDIAS; ++i) t << sd.DIAS_d[i] << "\n";
    }
    {
        std::ofstream t = OpenWrite(paths.ijs);
        t << "[JarakAntarSengkang]\n" << sd.nJS << "\n";
        for (int i = 0; i < sd.nJS; ++i) t << sd.JS_d[i] << "\n";
    }
}

void ReadDataset(StructureData& sd, const DatasetPaths& paths) {
    ReadStructureFile(sd, paths.inp);
    ReadDiscreteTables(sd, paths);
}

namespace {

// Shared by ReadLoads and ReadLoadsRaw: parses the .bbn file's two
// sections into sd.W/AML/AJ verbatim (no self-weight applied).
void ParseLoadsFile(StructureData& sd, std::ifstream& baca) {
    baca >> sd.sub_name; // "[BebanBatang]"
    baca >> sd.M;
    for (int kinp = 1; kinp <= sd.M; ++kinp) {
        int idx;
        baca >> idx;
        baca >> sd.W[kinp];
        for (int j = 1; j <= 12; ++j) {
            baca >> sd.AML[j][kinp];
        }
    }

    baca >> sd.sub_name; // "[BebanTitik]"
    baca >> sd.NJ;
    for (int kinp = 1; kinp <= sd.NJ; ++kinp) {
        int idx;
        baca >> idx;
        baca >> sd.AJ[6 * kinp - 5];
        baca >> sd.AJ[6 * kinp - 4];
        baca >> sd.AJ[6 * kinp - 3];
        baca >> sd.AJ[6 * kinp - 2];
        baca >> sd.AJ[6 * kinp - 1];
        baca >> sd.AJ[6 * kinp];
    }
}

} // namespace

void ReadLoads(StructureData& sd, const std::string& bbn_path) {
    std::ifstream baca = OpenRead(bbn_path);
    ParseLoadsFile(sd, baca);
    BeratSendiri(sd);
}

// New in this port (issue #7): reads .bbn the same way ReadLoads() does,
// but *without* BeratSendiri()'s self-weight side effect -- the file on
// disk (and load_data()'s original console-input flow that wrote it)
// never includes self-weight; ReadLoads() re-derives it fresh on every
// read for real analysis runs. The GUI load editor needs the raw,
// self-weight-free values (so re-saving doesn't bake self-weight into the
// file, which would then get double-counted on the next ReadLoads()).
// Tolerates a missing file (leaves sd.W/AML/AJ at their defaults) so a
// brand-new dataset with no .bbn yet can still be edited.
void ReadLoadsRaw(StructureData& sd, const std::string& bbn_path) {
    std::ifstream baca(bbn_path);
    if (!baca) return;
    ParseLoadsFile(sd, baca);
}

// New in this port (issue #7): writes .bbn matching load_data()'s exact
// format (CETAK.HPP has no equivalent -- the legacy program only ever
// wrote loads from its own interactive console prompts, never from a
// re-editable in-memory model). Callers must pass raw (self-weight-free)
// values -- see ReadLoadsRaw()'s comment; writing sd.W/AJ after a real
// ReadLoads()+BeratSendiri() call would bake self-weight into the file.
void WriteLoads(const StructureData& sd, const std::string& bbn_path) {
    std::ofstream tulis = OpenWrite(bbn_path);

    tulis << "[BebanBatang]\n";
    tulis << sd.M << "\n";
    for (int kbn = 1; kbn <= sd.M; ++kbn) {
        tulis << kbn << "\n";
        tulis << sd.W[kbn] << "\n";
        for (int j = 1; j <= 12; ++j) {
            tulis << sd.AML[j][kbn] << "\n";
        }
    }

    tulis << "[BebanTitik]\n";
    tulis << sd.NJ << "\n";
    for (int kbn = 1; kbn <= sd.NJ; ++kbn) {
        tulis << kbn << "\n";
        tulis << sd.AJ[6 * kbn - 5] << "\n";
        tulis << sd.AJ[6 * kbn - 4] << "\n";
        tulis << sd.AJ[6 * kbn - 3] << "\n";
        tulis << sd.AJ[6 * kbn - 2] << "\n";
        tulis << sd.AJ[6 * kbn - 1] << "\n";
        tulis << sd.AJ[6 * kbn] << "\n";
    }
}

namespace {

// Shared by WriteFinalResults: the .inf content (legacy: the same block
// input_data_umum()'s output_* helpers print interactively, also mirrored
// into cetak_akhir()'s `hinf` stream).
void WriteInformasi(StructureData& sd, std::ofstream& hinf) {
    hinf << "  Struktur Portal Ruang " << sd.ISN << "\n\n";
    hinf << "  Parameter Struktur\n";
    hinf << "  Jumlah batang : " << sd.M << "\n";
    hinf << "  DOF : " << sd.N << "\n";
    hinf << "  Jumlah joint : " << sd.NJ << "\n";
    hinf << "  Jumlah pengekang tumpuan : " << sd.NR << "\n";
    hinf << "  Jumlah titik kumpul yang dikekang : " << sd.NRJ << "\n";
    hinf << "  Modulus Elastisitas aksial : " << sd.E << " N/m^2\n";
    hinf << "  Modulus Geser : " << sd.G << " N/m^2\n\n";
    hinf << "  Properti Elemen Material\n";
    hinf << "  Kuat desak beton karakteristik : " << sd.FC << " MPa\n";
    hinf << "  Kuat tarik baja tulangan : " << sd.FY << " MPa\n";
    hinf << "  Kuat tarik tulangan sengkang : " << sd.FYS << " MPa\n";

    hinf << "\n  Koordinat Titik Kumpul (m)\n";
    hinf << "  Titik     X             Y             Z \n";
    for (int kout = 1; kout <= sd.NJ; ++kout) {
        hinf << "  " << kout << "\t" << sd.X[kout] << "\t" << sd.Y[kout] << "\t" << sd.Z[kout] << "\n";
    }

    hinf << "\n  Informasi Batang \n";
    hinf << "  Batang  JJ      JK      IA\n";
    for (int iout = 1; iout <= sd.M; ++iout) {
        hinf << "  " << iout << "\t" << sd.JJ[iout] << "\t" << sd.JK[iout] << "\t" << sd.IA[iout] << "\n";
        if (sd.IA[iout] != 0) {
            hinf << "  XP = " << sd.XP[iout] << "  YP = " << sd.YP[iout] << "  ZP = " << sd.ZP[iout] << "\n\n";
        }
    }

    hinf << "\n  Pengekang Titik Kumpul \n";
    hinf << "  Titik     JR1   JR2   JR3   JR4   JR5   JR6\n";
    for (int iout = 1; iout <= sd.NRJ; ++iout) {
        hinf << "  " << sd.T_K[iout];
        for (int k = 5; k >= 0; --k) {
            hinf << "\t" << sd.JRL[6 * sd.T_K[iout] - k];
        }
        hinf << "\n";
    }

    hinf << "\n  Gaya di Ujung Batang Terkekang Akibat Beban (Nm)\n";
    hinf << "  Batang  AML1..AML12\n";
    for (int kout = 1; kout <= sd.M; ++kout) {
        hinf << "  " << kout;
        for (int j = 1; j <= 12; ++j) {
            hinf << "\t" << sd.AML[j][kout];
        }
        hinf << "\n";
    }

    hinf << "\n  Beban Titik (N)\n";
    hinf << "  Titik   Arah 1..Arah 6\n";
    for (int kout = 1; kout <= sd.NJ; ++kout) {
        hinf << "  " << kout;
        for (int k = 5; k >= 0; --k) {
            hinf << "\t" << sd.AJ[6 * kout - k];
        }
        hinf << "\n";
    }
}

void WriteStrukturSection(StructureData& sd, std::ofstream& hstr) {
    hstr << "Hasil Analisa Struktur Dengan Metoda Kekakuan\n";
    hstr << "Dikembangkan dari Weaver & Gere\n";
    hstr << "Oleh Yohan Naftali 1999 (diporting ke C++ modern, lihat issue #3)\n\n";
    hstr << "Jumlah batang : " << sd.M << "\n";
    hstr << "Jumlah titik kumpul : " << sd.NJ << "\n";
    hstr << "Modulus elastisitas tarik/tekan : " << (sd.E * 1.E-6f) << " MPa\n";
    hstr << "Modulus elastisitas geser G : " << (sd.G * 1.E-6f) << " MPa\n";

    hstr << "\nPerpindahan Titik Kumpul\n";
    hstr << "Titik  DJ1  DJ2  DJ3  DJ4  DJ5  DJ6\n";
    hstr << std::showpos << std::fixed;
    for (int cst = 1; cst <= sd.NJ; ++cst) {
        hstr << cst;
        for (int k = 5; k >= 0; --k) {
            hstr << "\t" << sd.DJ[6 * cst - k];
        }
        hstr << "\n";
    }
    hstr << std::noshowpos;
    hstr.unsetf(std::ios::fixed);

    hstr << "\nGaya Ujung Batang\n";
    hstr << "Batang  AM1..AM12\n";
    for (int ih = 1; ih <= sd.M; ++ih) {
        PeriksaBatang(sd, ih);
        IsiMatrikKekakuan(sd, ih);
        IndeksBatang(sd, ih);
        for (int jh = 1; jh <= kMD; ++jh) {
            sd.AMD[jh] = 0.f;
            for (int kh = 1; kh <= kMD; ++kh) {
                sd.AMD[jh] = sd.AMD[jh] + sd.SMRT[jh][kh] * sd.DJ[sd.IM[kh]];
            }
            sd.AM[ih][jh] = sd.AML[jh][ih] + sd.AMD[jh];
        }
        hstr << ih;
        for (int j = 1; j <= 12; ++j) {
            hstr << "\t" << sd.AM[ih][j];
        }
        hstr << "\n";
    }

    hstr << "\nReaksi Tumpuan\n";
    hstr << "Titik   AR1..AR6\n";
    for (int ih = 1; ih <= sd.NJ; ++ih) {
        int J[6] = {6 * ih - 5, 6 * ih - 4, 6 * ih - 3, 6 * ih - 2, 6 * ih - 1, 6 * ih};
        int N1 = sd.JRL[J[0]] + sd.JRL[J[1]] + sd.JRL[J[2]] + sd.JRL[J[3]] + sd.JRL[J[4]] + sd.JRL[J[5]];
        if (N1 != 0) {
            hstr << ih;
            for (int k = 0; k < 6; ++k) {
                hstr << "\t" << sd.AR[J[k]];
            }
            hstr << "\n";
        }
    }
}

} // namespace

void WriteFinalResults(StructureData& sd, const DatasetPaths& paths) {
    std::ofstream hopt = OpenWrite(paths.opt);
    std::ofstream hstr = OpenWrite(paths.str);
    std::ofstream hkdl = OpenWrite(paths.kdl);
    std::ofstream hinf = OpenWrite(paths.inf);

    WriteInformasi(sd, hinf);

    // Legacy quirk, preserved deliberately: this re-analysis uses whatever
    // `sd.no_struktur` is left set by the caller (the optimizer leaves it
    // at the "frozen" slot -- see Optimizer.cpp) -- *not* necessarily the
    // best (JSTD-1) structure. The .str file below reflects that slot's
    // geometry, while the .opt/.kdl per-member loops further down
    // explicitly switch to no_struktur=(JSTD-1) for their own dimensions.
    // This mirrors cetak_akhir() in CETAK.HPP exactly.
    Inersia(sd);
    Struktur(sd);

    WriteStrukturSection(sd, hstr);
    hstr.close();

    hopt << "Hasil Optimasi Beton Bertulang Pada Struktur Portal Ruang\n";
    hopt << "Metoda Optimasi : Flexible Polyhedron\n";
    hopt << "Jumlah Variabel Desain : " << sd.JVD << "\n";
    hopt << "Jumlah Struktur Desain : " << sd.JSTD << "\n";
    hopt << "Oleh Yohan Naftali 1999 (diporting ke C++ modern, lihat issue #3)\n\n";
    hopt << "Nama Struktur : " << sd.ISN << "\n";
    hopt << "Jumlah batang : " << sd.M << "\n";
    hopt << "Jumlah titik kumpul : " << sd.NJ << "\n";
    hopt << "Modulus elastisitas tarik/tekan : " << (sd.E * 1.E-6f) << " MPa\n";
    hopt << "Modulus elastisitas geser G : " << (sd.G * 1.E-6f) << " MPa\n";
    hopt << "Kuat desak Karakteristik Beton : " << sd.FC << " MPa\n";
    hopt << "Kuat tarik baja tulangan lentur : " << sd.FY << " MPa\n";
    hopt << "Kuat tarik baja tulangan sengkang : " << sd.FYS << " MPa\n";

    hkdl << "Kendala Pada Struktur\n";
    hkdl << "Oleh Yohan Naftali 1999 (diporting ke C++ modern, lihat issue #3)\n\n";
    hkdl << "Nama Struktur : " << sd.ISN << "\n";

    sd.no_struktur = sd.JSTD - 1; // best structure after sort()
    for (int oio = 0; oio < sd.jum_balok; ++oio) {
        PeriksaBatang(sd, sd.no_balok[oio]);
        IsiElemenBalokFields(sd, oio); // sets B,H,DIA*,NL*,DIAS,Jarak_S,L,VU

        hopt << "-----------------------------------------------------\n";
        hopt << "Balok " << (oio + 1) << " " << sd.B << " x " << sd.H << "\n";
        hopt << "Nomor Batang : " << sd.no_balok[oio] << "\n";
        hopt << "Panjang Bentang : " << sd.L << " m\n";
        hopt << "Beban Total : " << sd.W[sd.no_balok[oio]] << " N/m\n";
        hopt << "Berat Sendiri : " << sd.W_Balok[sd.no_balok[oio]] << " N/m\n";
        hopt << "Momen kiri : " << (sd.MTUM_KI[sd.no_balok[oio]] / 0.8f) << " Nm\n";
        hopt << "Momen tengah : " << (sd.MLAP[sd.no_balok[oio]] / 0.8f) << " Nm\n";
        hopt << "Momen kanan : " << (sd.MTUM_KA[sd.no_balok[oio]] / 0.8f) << " Nm\n";
        hopt << "Geser kiri : " << (sd.GESER_KI[sd.no_balok[oio]] / 0.8f) << " N\n";
        hopt << "Geser kanan : " << (sd.GESER_KA[sd.no_balok[oio]] / 0.8f) << " N\n";

        // Daerah lapangan
        ElemenLapangan(sd, oio);
        DesignBeam(sd);
        Lendutan(sd, sd.no_balok[oio]);

        hopt << "Lendutan Tengah Bentang : " << sd.LENDUTAN << " mm\n";
        hopt << "Lendutan ijin " << sd.LENDUTAN_IJIN << " mm\n";
        hopt << "Sengkang " << sd.DIAS << " - " << sd.Jarak_S << "\n";
        hopt << "Jarak sengkang maksimum : " << sd.Sref << " mm\n";
        hopt << "\nDaerah Lapangan\n";
        hopt << "Tulangan tarik " << sd.NL1 << " D " << sd.DIA1 << "\n";
        hopt << "Tulangan desak " << sd.NL2 << " D " << sd.DIA2 << "\n";
        hopt << "Volume beton : " << sd.volume_beton << " m^3\n";
        hopt << "Berat Tulangan utama : " << sd.berat_besi << " kg\n";
        hopt << "Berat Tulangan geser : " << sd.berat_sengkang << " kg\n";
        hopt << "Harga balok daerah lapangan : " << sd.harga << "\n";
        hopt << "Momen yang membebani : " << sd.MU << " Nm\n";
        hopt << "Momen yang dapat ditahan : " << sd.FMU << " Nm\n\n";

        hkdl << "\nBalok " << (oio + 1) << "\n";
        hkdl << "Nomor Batang : " << sd.no_balok[oio] << "\n";
        hkdl << "Kendala akibat lendutan : " << sd.kendala_lendutan << "\n";
        hkdl << "\nDaerah Lapangan\n";
        hkdl << "Kendala rasio penulangan " << sd.kendala_rho << "\n";
        hkdl << "Kendala sengkang : " << sd.kendala_sb << "\n";
        hkdl << "Kendala momen lentur : " << sd.kendala_M << "\n";

        // Daerah tumpuan
        ElemenTumpuan(sd, oio);
        DesignBeam(sd);

        hopt << "Daerah Tumpuan\n";
        hopt << "Tulangan desak " << sd.NL2 << " D " << sd.DIA2 << "\n";
        hopt << "Tulangan tarik " << sd.NL1 << " D " << sd.DIA1 << "\n";
        hopt << "Volume beton : " << sd.volume_beton << " m^3\n";
        hopt << "Berat Tulangan utama : " << sd.berat_besi << " kg\n";
        hopt << "Berat Tulangan geser : " << sd.berat_sengkang << " kg\n";
        hopt << "Harga balok pada tumpuan kiri + kanan : " << sd.harga << "\n";
        hopt << "Momen yang membebani : " << sd.MU << " Nm\n";
        hopt << "Momen yang dapat ditahan : " << sd.FMU << " Nm\n\n";

        hkdl << "\nDaerah Tumpuan\n";
        hkdl << "Kendala rasio penulangan " << sd.kendala_rho << "\n";
        hkdl << "Kendala sengkang : " << sd.kendala_sb << "\n";
        hkdl << "Kendala momen lentur : " << sd.kendala_M << "\n";
    }

    for (int oio = 0; oio < sd.jum_kolom; ++oio) {
        PeriksaBatang(sd, sd.no_kolom[oio]);
        IsiElemenKolomFields(sd, oio);

        DesignColumn(sd);

        hopt << "-----------------------------------------------------\n";
        hopt << "Kolom " << (oio + 1) << " " << sd.sisi << " x " << sd.sisi << "\n";
        hopt << "Nomor Batang : " << sd.no_kolom[oio] << "\n";
        hopt << "Tulangan utama " << (4 * sd.N_DIA - 4) << " D " << sd.DIA << "\n";
        hopt << "Jarak antar tulangan : " << sd.jarak_antar_tulangan << " mm\n";
        hopt << "Sengkang " << sd.DIAS << " - " << sd.Jarak_S << "\n";
        hopt << "Jarak sengkang maksimum : " << sd.Sref << " mm\n";
        hopt << "Volume beton : " << sd.volume_beton << " m^3\n";
        hopt << "Berat Tulangan utama : " << sd.berat_besi << " kg\n";
        hopt << "Berat Tulangan geser : " << sd.berat_sengkang << " kg\n";
        hopt << "Harga kolom : " << sd.harga << "\n";
        hopt << "Berat Kolom : " << (-sd.P_Kolom[sd.no_kolom[oio]]) << " N\n";
        hopt << "Gaya aksial : " << (sd.PK[sd.no_kolom[oio]] / 0.8f) << " N\n";
        hopt << "Gaya aksial yang dapat ditahan = " << sd.FPU << " N\n";
        hopt << "Momen arah X : " << (sd.MKX[sd.no_kolom[oio]] / 0.8f) << " Nm\n";
        hopt << "Momen arah Y : " << (sd.MKY[sd.no_kolom[oio]] / 0.8f) << " Nm\n";
        hopt << "Momen yang dapat ditahan : " << sd.FMU << " Nm\n";
        hopt << "Geser pada kolom : " << (sd.GK[sd.no_kolom[oio]] / 0.8f) << " N\n\n";

        hkdl << "\nKolom " << (oio + 1) << "\n";
        hkdl << "Nomor Batang : " << sd.no_kolom[oio] << "\n";
        hkdl << "\nKendala gaya : " << sd.kendala_gaya << "\n";
        hkdl << "Kendala rasio penulangan : " << sd.kendala_r << "\n";
        hkdl << "Kendala jarak tulangan : " << sd.kendala_tul << "\n";
        hkdl << "Kendala sengkang : " << sd.kendala_sengkang << "\n";
        hkdl << "Kendala kelangsingan kolom : " << sd.kendala_kelangsingan << "\n";
    }

    hopt << "\nHarga Beton Rp." << sd.harga_beton << " /m^3\n";
    hopt << "Harga Besi Rp." << sd.harga_besi << " /kg\n";
    hopt << "Tebal selimut kolom " << sd.selimut_kolom << " mm\n";
    hopt << "Tebal selimut balok " << sd.selimut_balok << " mm\n";
    hopt << "Faktor finalti : " << sd.finalti << "\n";
    hopt << "JSTD = (JVD*" << sd.fak_kali << ")+" << sd.fak_plus << "\n";
}

namespace {

// Reads one "Jumlah <label> : <n>" header line's integer value, or returns
// -1 if `line` isn't that header at all (caller keeps scanning).
int ParseCountLine(const std::string& line, const char* label) {
    if (line.rfind(label, 0) != 0) return -1;
    size_t colon = line.find(':');
    if (colon == std::string::npos) return -1;
    try {
        return std::stoi(line.substr(colon + 1));
    } catch (const std::exception&) {
        return -1;
    }
}

} // namespace

std::optional<AnalysisResults> ReadAnalysisResultsFromStr(const std::string& str_path) {
    std::ifstream in(str_path);
    if (!in.is_open()) return std::nullopt;

    int m = 0, nj = 0;
    std::string line;
    bool found_displacements_header = false;
    while (std::getline(in, line)) {
        int mv = ParseCountLine(line, "Jumlah batang");
        if (mv >= 0) m = mv;
        int njv = ParseCountLine(line, "Jumlah titik kumpul");
        if (njv >= 0) nj = njv;
        if (line.find("Perpindahan Titik Kumpul") != std::string::npos) {
            found_displacements_header = true;
            break;
        }
    }
    if (!found_displacements_header || m <= 0 || nj <= 0) return std::nullopt;

    AnalysisResults result;
    result.has_applied_load = false; // .str has no applied-load section -- see LegacyIO.h's comment

    if (!std::getline(in, line)) return std::nullopt; // "Titik  DJ1  DJ2 ..." column header

    result.displacements.reserve(static_cast<size_t>(nj));
    for (int i = 0; i < nj; ++i) {
        if (!std::getline(in, line)) return std::nullopt;
        std::istringstream iss(line);
        JointDisplacement d;
        iss >> d.no_joint >> d.ux >> d.uy >> d.uz >> d.rx >> d.ry >> d.rz;
        if (iss.fail()) return std::nullopt;
        result.displacements.push_back(d);
    }

    bool found_forces_header = false;
    while (std::getline(in, line)) {
        if (line.find("Gaya Ujung Batang") != std::string::npos) {
            found_forces_header = true;
            break;
        }
    }
    if (!found_forces_header) return std::nullopt;
    if (!std::getline(in, line)) return std::nullopt; // "Batang  AM1..AM12" column header

    result.member_forces.reserve(static_cast<size_t>(m));
    for (int i = 0; i < m; ++i) {
        if (!std::getline(in, line)) return std::nullopt;
        std::istringstream iss(line);
        MemberForces f;
        float am[12];
        iss >> f.no_batang;
        for (float& v : am) iss >> v;
        if (iss.fail()) return std::nullopt;
        f.axial_a = am[0];
        f.shear_y_a = am[1];
        f.shear_z_a = am[2];
        f.torsion_a = am[3];
        f.moment_y_a = am[4];
        f.moment_z_a = am[5];
        f.axial_b = am[6];
        f.shear_y_b = am[7];
        f.shear_z_b = am[8];
        f.torsion_b = am[9];
        f.moment_y_b = am[10];
        f.moment_z_b = am[11];
        // w_total_n_per_m: not present in .str -- see LegacyIO.h's comment.
        result.member_forces.push_back(f);
    }

    bool found_reactions_header = false;
    while (std::getline(in, line)) {
        if (line.find("Reaksi Tumpuan") != std::string::npos) {
            found_reactions_header = true;
            break;
        }
    }
    if (!found_reactions_header) return std::nullopt;
    if (!std::getline(in, line)) return std::nullopt; // "Titik   AR1..AR6" column header

    while (std::getline(in, line)) {
        if (line.empty()) break;
        std::istringstream iss(line);
        JointReaction r;
        iss >> r.no_joint >> r.fx >> r.fy >> r.fz >> r.mx >> r.my >> r.mz;
        if (iss.fail()) break; // trailing blank/EOF line, not a real row
        result.reactions.push_back(r);
    }

    return result;
}

} // namespace orcisf::engine
