#include "report/TextExport.h"

#include <filesystem>

#include "engine/LegacyIO.h"

namespace orcisf::report {

using engine::DatasetPaths;
using engine::MakeDatasetPaths;

std::string WriteTextExport(engine::StructureData& sd, const std::string& dest_generic_path, bool has_run_results,
                             const std::string& source_generic_path) {
    try {
        DatasetPaths dest = MakeDatasetPaths(dest_generic_path);

        engine::WriteStructureFile(sd, dest.inp);
        engine::WriteDiscreteTables(sd, dest);
        engine::WriteLoads(sd, dest.bbn);

        if (has_run_results) {
            engine::WriteFinalResults(sd, dest); // .opt/.str/.kdl/.inf

            if (!source_generic_path.empty() && source_generic_path != dest_generic_path) {
                DatasetPaths src = MakeDatasetPaths(source_generic_path);
                std::error_code ec;
                std::filesystem::copy_file(src.his, dest.his, std::filesystem::copy_options::overwrite_existing, ec);
                std::filesystem::copy_file(src.log_detail, dest.log_detail,
                                            std::filesystem::copy_options::overwrite_existing, ec);
                // Best-effort: a missing .his/.log.txt (e.g. a run that
                // never produced them) shouldn't fail the whole export.
            }
        }

        return "";
    } catch (const std::exception& e) {
        return e.what();
    }
}

} // namespace orcisf::report
