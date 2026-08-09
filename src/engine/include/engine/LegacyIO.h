#pragma once

#include <string>

#include "engine/StructureData.h"

// Port of the file I/O in Optimasi Beton/Source/InOut.hpp (baca_data),
// Pembebanan.hpp (baca_beban), and CETAK.HPP (cetak_akhir) -- reading the
// legacy .inp/.isd/.idl/.ijl/.ids/.ijs/.bbn input files and writing the
// legacy .opt/.str/.kdl/.inf output files, byte-format-compatible with the
// original program (plain whitespace-separated ASCII, read with `>>`, so
// exact column widths don't matter -- only token order/count does).
namespace orcisf::engine {

// Every filename the legacy generic-name + fixed-extension convention
// derives from one base name (see AGENTS.md "Data / File Format
// Convention"). `generic` may include a path, e.g. "Example/Apl1-1/Gedung".
struct DatasetPaths {
    std::string inp, isd, idl, ijl, ids, ijs, bbn;
    std::string opt, str, kdl, inf, his;
    // New in this port (issue #3's acceptance criteria: a detailed,
    // per-generation calculation log -- a superset of .his, which only
    // ever wrote one line per shrink/convergence event).
    std::string log_detail;
};

DatasetPaths MakeDatasetPaths(const std::string& generic);

// Reads the .inp file into sd (legacy `baca_data()`'s first half: ISN, M,
// NJ, NRJ, NR, E, G, FC, FY, FYS, ND, N, joint coordinates, restraints,
// member topology).
void ReadStructureFile(StructureData& sd, const std::string& inp_path);

// Reads the five discrete-table files into sd (legacy `baca_data()`'s
// second half).
void ReadDiscreteTables(StructureData& sd, const DatasetPaths& paths);

// Reads member/joint loads into sd and applies self-weight (legacy
// `baca_beban()`, which itself calls `berat_sendiri()` at the end -- see
// StructuralAnalysis.h's BeratSendiri(), called here to match).
void ReadLoads(StructureData& sd, const std::string& bbn_path);

// New in this port (issue #7's GUI load editor): reads .bbn without
// applying self-weight. Tolerates a missing file (leaves sd.W/AML/AJ at
// their defaults). See LegacyIO.cpp's comment on why the editor needs
// this instead of ReadLoads().
void ReadLoadsRaw(StructureData& sd, const std::string& bbn_path);

// New in this port (issue #7): writes .bbn matching load_data()'s exact
// format. `sd`'s W/AML/AJ must be the raw, self-weight-free values (e.g.
// from ReadLoadsRaw() + edits) -- see LegacyIO.cpp's comment.
void WriteLoads(const StructureData& sd, const std::string& bbn_path);

// Convenience: ReadStructureFile + ReadDiscreteTables for `paths`.
void ReadDataset(StructureData& sd, const DatasetPaths& paths);

// Writes the four legacy result files (.inf/.str/.opt/.kdl), reproducing
// `cetak_akhir()` exactly -- including its specific (and load-bearing for
// byte-for-byte output compatibility) choice of which population slot's
// geometry the .str structural-analysis section reflects vs. which slot's
// dimensions/reinforcement the .opt/.kdl per-member sections reflect. Must
// be called after an optimization run has populated sd.var_b/var_k and
// sorted sd.fitstr/kendalastr/hargastr (best at index JSTD-1).
void WriteFinalResults(StructureData& sd, const DatasetPaths& paths);

} // namespace orcisf::engine
