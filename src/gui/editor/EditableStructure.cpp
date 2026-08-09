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
    return m;
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
