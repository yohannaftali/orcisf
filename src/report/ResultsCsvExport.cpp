#include "report/ResultsCsvExport.h"

#include <fstream>

namespace orcisf::report {

std::string WriteResultsCsv(const engine::AnalysisResults& analysis, const std::string& csv_path) {
    std::ofstream out(csv_path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return "Could not open " + csv_path + " for writing.";
    }

    out << "Global Equilibrium Check\n";
    out << "DOF,Total applied load,Total reaction,Residual\n";
    static const char* kDofLabels[6] = {"Fx", "Fy", "Fz", "Mx", "My", "Mz"};
    for (int dof = 0; dof < 6; ++dof) {
        float applied = analysis.total_applied_load[dof];
        float reaction = analysis.TotalReaction(dof);
        out << kDofLabels[dof] << "," << applied << "," << reaction << "," << (applied + reaction) << "\n";
    }

    out << "\nJoint Displacements\n";
    out << "Joint,UX (m),UY (m),UZ (m),RX (rad),RY (rad),RZ (rad)\n";
    for (const engine::JointDisplacement& d : analysis.displacements) {
        out << d.no_joint << "," << d.ux << "," << d.uy << "," << d.uz << "," << d.rx << "," << d.ry << "," << d.rz
            << "\n";
    }

    out << "\nMember End Forces\n";
    out << "Member,N_a (N),Vy_a (N),Vz_a (N),T_a (Nm),My_a (Nm),Mz_a (Nm),"
           "N_b (N),Vy_b (N),Vz_b (N),T_b (Nm),My_b (Nm),Mz_b (Nm)\n";
    for (const engine::MemberForces& f : analysis.member_forces) {
        out << f.no_batang << "," << f.axial_a << "," << f.shear_y_a << "," << f.shear_z_a << "," << f.torsion_a
            << "," << f.moment_y_a << "," << f.moment_z_a << "," << f.axial_b << "," << f.shear_y_b << ","
            << f.shear_z_b << "," << f.torsion_b << "," << f.moment_y_b << "," << f.moment_z_b << "\n";
    }

    out << "\nSupport Reactions\n";
    out << "Joint,Fx (N),Fy (N),Fz (N),Mx (Nm),My (Nm),Mz (Nm)\n";
    for (const engine::JointReaction& r : analysis.reactions) {
        out << r.no_joint << "," << r.fx << "," << r.fy << "," << r.fz << "," << r.mx << "," << r.my << "," << r.mz
            << "\n";
    }

    if (!out.good()) {
        return "Write error while writing " + csv_path + ".";
    }
    return "";
}

} // namespace orcisf::report
