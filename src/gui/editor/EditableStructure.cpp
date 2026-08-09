#include "gui/editor/EditableStructure.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace orcisf::gui {

using engine::kMak;
using math3d::Vec3;

namespace {
constexpr float kCoincidentEpsilon = 1e-3f; // meters -- joints closer than this are "the same point"
}

int EditableStructure::AddJoint(const Vec3& pos) {
    if (sd_.NJ >= kMak - 1) return -1;
    ++sd_.NJ;
    int j = sd_.NJ;
    sd_.X[j] = pos.x;
    sd_.Y[j] = pos.y;
    sd_.Z[j] = pos.z;
    for (int dof = 0; dof < 6; ++dof) {
        sd_.JRL[6 * j - 5 + dof] = 0; // free by default
        sd_.AJ[6 * j - 5 + dof] = 0.f; // no load by default
    }
    return j;
}

bool EditableStructure::MoveJoint(int joint_id, const Vec3& new_pos) {
    if (joint_id < 1 || joint_id > sd_.NJ) return false;
    sd_.X[joint_id] = new_pos.x;
    sd_.Y[joint_id] = new_pos.y;
    sd_.Z[joint_id] = new_pos.z;
    return true;
}

bool EditableStructure::SetJointRestrained(int joint_id, bool restrained) {
    if (joint_id < 1 || joint_id > sd_.NJ) return false;
    for (int dof = 0; dof < 6; ++dof) {
        sd_.JRL[6 * joint_id - 5 + dof] = restrained ? 1 : 0;
    }
    return true;
}

bool EditableStructure::DeleteMember(int member_id) {
    if (member_id < 1 || member_id > sd_.M) return false;
    for (int i = member_id; i < sd_.M; ++i) {
        sd_.JJ[i] = sd_.JJ[i + 1];
        sd_.JK[i] = sd_.JK[i + 1];
        sd_.IA[i] = sd_.IA[i + 1];
        sd_.W[i] = sd_.W[i + 1];
        for (int j = 1; j <= 12; ++j) {
            sd_.AML[j][i] = sd_.AML[j][i + 1];
        }
    }
    --sd_.M;
    return true;
}

bool EditableStructure::DeleteJoint(int joint_id) {
    if (joint_id < 1 || joint_id > sd_.NJ) return false;

    // Delete every member touching this joint first, highest index first
    // so DeleteMember's own compaction never invalidates a not-yet-visited
    // index.
    std::vector<int> touching;
    for (int i = 1; i <= sd_.M; ++i) {
        if (sd_.JJ[i] == joint_id || sd_.JK[i] == joint_id) touching.push_back(i);
    }
    std::sort(touching.rbegin(), touching.rend());
    for (int member_id : touching) {
        DeleteMember(member_id);
    }

    // Compact joints above joint_id down by one.
    for (int i = joint_id; i < sd_.NJ; ++i) {
        sd_.X[i] = sd_.X[i + 1];
        sd_.Y[i] = sd_.Y[i + 1];
        sd_.Z[i] = sd_.Z[i + 1];
        for (int dof = 0; dof < 6; ++dof) {
            sd_.JRL[6 * i - 5 + dof] = sd_.JRL[6 * (i + 1) - 5 + dof];
            sd_.AJ[6 * i - 5 + dof] = sd_.AJ[6 * (i + 1) - 5 + dof];
        }
    }
    --sd_.NJ;

    // Remap remaining members' joint references above the deleted index.
    for (int i = 1; i <= sd_.M; ++i) {
        if (sd_.JJ[i] > joint_id) --sd_.JJ[i];
        if (sd_.JK[i] > joint_id) --sd_.JK[i];
    }

    return true;
}

int EditableStructure::AddMember(int joint_a, int joint_b) {
    if (joint_a == joint_b) return -1;
    if (joint_a < 1 || joint_a > sd_.NJ || joint_b < 1 || joint_b > sd_.NJ) return -1;
    if (sd_.M >= kMak - 1) return -1;

    ++sd_.M;
    int m = sd_.M;
    sd_.JJ[m] = joint_a;
    sd_.JK[m] = joint_b;
    sd_.IA[m] = 0;
    sd_.W[m] = 0.f;
    for (int j = 1; j <= 12; ++j) {
        sd_.AML[j][m] = 0.f;
    }
    return m;
}

namespace {
// Fixed-end forces for a uniform transverse distributed load w (N/m) over
// a member of length L (m), matching Pembebanan.hpp's load_data() exactly
// (AML[1..12]: axial/shear/moment at each end, no torsion/axial component
// for this load type).
void ComputeUdlFixedEndForces(float w, float length, engine::StructureData& sd, int member_id) {
    sd.AML[1][member_id] = 0.f;
    sd.AML[2][member_id] = w * length / 2.f;
    sd.AML[3][member_id] = 0.f;
    sd.AML[4][member_id] = 0.f;
    sd.AML[5][member_id] = 0.f;
    sd.AML[6][member_id] = w * length * length / 12.f;
    sd.AML[7][member_id] = 0.f;
    sd.AML[8][member_id] = w * length / 2.f;
    sd.AML[9][member_id] = 0.f;
    sd.AML[10][member_id] = 0.f;
    sd.AML[11][member_id] = 0.f;
    sd.AML[12][member_id] = -w * length * length / 12.f;
}
} // namespace

bool EditableStructure::SetMemberLoad(int member_id, float w_n_per_m) {
    if (member_id < 1 || member_id > sd_.M) return false;
    int ja = sd_.JJ[member_id];
    int jb = sd_.JK[member_id];
    float dx = sd_.X[jb] - sd_.X[ja];
    float dy = sd_.Y[jb] - sd_.Y[ja];
    float dz = sd_.Z[jb] - sd_.Z[ja];
    float length = std::sqrt(dx * dx + dy * dy + dz * dz);

    sd_.W[member_id] = w_n_per_m;
    ComputeUdlFixedEndForces(w_n_per_m, length, sd_, member_id);
    return true;
}

bool EditableStructure::ClearMemberLoad(int member_id) { return SetMemberLoad(member_id, 0.f); }

bool EditableStructure::SetJointLoad(int joint_id, const float actions[6]) {
    if (joint_id < 1 || joint_id > sd_.NJ) return false;
    for (int dof = 0; dof < 6; ++dof) {
        sd_.AJ[6 * joint_id - 5 + dof] = actions[dof];
    }
    return true;
}

bool EditableStructure::ClearJointLoad(int joint_id) {
    float zero[6] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
    return SetJointLoad(joint_id, zero);
}

std::vector<std::string> EditableStructure::Validate() const {
    std::vector<std::string> issues;

    for (int i = 1; i <= sd_.NJ; ++i) {
        for (int j = i + 1; j <= sd_.NJ; ++j) {
            float dx = sd_.X[i] - sd_.X[j];
            float dy = sd_.Y[i] - sd_.Y[j];
            float dz = sd_.Z[i] - sd_.Z[j];
            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (dist < kCoincidentEpsilon) {
                std::ostringstream oss;
                oss << "Joints " << i << " and " << j << " are coincident (distance " << dist << " m)";
                issues.push_back(oss.str());
            }
        }
    }

    for (int i = 1; i <= sd_.M; ++i) {
        int ja = sd_.JJ[i];
        int jb = sd_.JK[i];
        float dx = sd_.X[ja] - sd_.X[jb];
        float dy = sd_.Y[ja] - sd_.Y[jb];
        float dz = sd_.Z[ja] - sd_.Z[jb];
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (dist < kCoincidentEpsilon) {
            std::ostringstream oss;
            oss << "Member " << i << " has zero length (joints " << ja << ", " << jb << ")";
            issues.push_back(oss.str());
        }
        for (int j = i + 1; j <= sd_.M; ++j) {
            bool same_pair = (sd_.JJ[j] == ja && sd_.JK[j] == jb) || (sd_.JJ[j] == jb && sd_.JK[j] == ja);
            if (same_pair) {
                std::ostringstream oss;
                oss << "Members " << i << " and " << j << " both connect joints " << ja << " and " << jb;
                issues.push_back(oss.str());
            }
        }
    }

    return issues;
}

} // namespace orcisf::gui
