#include "gui/viewport/SceneModel.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace orcisf::gui {

using engine::MemberResult;
using engine::StructureData;
using math3d::Vec3;

SceneModel BuildSceneModel(const StructureData& sd, const std::vector<MemberResult>* results,
                            std::string dataset_path) {
    SceneModel scene;
    scene.dataset_path = std::move(dataset_path);

    std::unordered_map<int, const MemberResult*> result_by_batang;
    if (results) {
        for (const MemberResult& r : *results) {
            result_by_batang[r.no_batang] = &r;
        }
    }

    Vec3 min_pt{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()};
    Vec3 max_pt{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                -std::numeric_limits<float>::max()};

    scene.joints.reserve(static_cast<size_t>(sd.NJ));
    for (int j = 1; j <= sd.NJ; ++j) {
        JointVisual jv;
        jv.no_joint = j;
        jv.pos = Vec3{sd.X[j], sd.Y[j], sd.Z[j]};
        for (int dof = 0; dof < 6; ++dof) {
            if (sd.JRL[6 * j - 5 + dof] == 1) {
                jv.restrained = true;
                break;
            }
        }
        min_pt = Vec3{std::min(min_pt.x, jv.pos.x), std::min(min_pt.y, jv.pos.y), std::min(min_pt.z, jv.pos.z)};
        max_pt = Vec3{std::max(max_pt.x, jv.pos.x), std::max(max_pt.y, jv.pos.y), std::max(max_pt.z, jv.pos.z)};
        scene.joints.push_back(jv);
    }

    scene.members.reserve(static_cast<size_t>(sd.M));
    for (int i = 1; i <= sd.M; ++i) {
        MemberVisual mv;
        mv.no_batang = i;
        mv.a = Vec3{sd.X[sd.JJ[i]], sd.Y[sd.JJ[i]], sd.Z[sd.JJ[i]]};
        mv.b = Vec3{sd.X[sd.JK[i]], sd.Y[sd.JK[i]], sd.Z[sd.JK[i]]};

        // Beam-vs-column classification, mirroring PeriksaBatang's CXZ
        // check (Optimizer.cpp: CXZ > 0.001f = beam) so geometry-only
        // views (no MemberResult yet) still render the right member type
        // without needing a mutable StructureData& to call PeriksaBatang.
        float len = (mv.b - mv.a).Length();
        if (len > 1e-6f) {
            float cx = (mv.b.x - mv.a.x) / len;
            float cz = (mv.b.z - mv.a.z) / len;
            mv.is_beam = std::fabs(std::sqrt(cx * cx + cz * cz)) > 0.001f;
        }

        auto it = result_by_batang.find(i);
        if (it != result_by_batang.end()) {
            mv.has_results = true;
            mv.result = *it->second;
            mv.width_m = std::max(0.05f, it->second->width / 1000.f);
            mv.height_m = std::max(0.05f, it->second->height / 1000.f);
            mv.kendala = it->second->Kendala();
        }
        scene.members.push_back(mv);

        if (sd.W[i] != 0.f) {
            MemberLoadVisual ml;
            ml.no_batang = i;
            ml.w_n_per_m = sd.W[i];
            ml.midpoint = (mv.a + mv.b) * 0.5f;
            ml.axis = (len > 1e-6f) ? (mv.b - mv.a) * (1.f / len) : Vec3{1.f, 0.f, 0.f};
            scene.member_loads.push_back(ml);
        }
    }

    scene.joint_loads.reserve(static_cast<size_t>(sd.NJ));
    for (int j = 1; j <= sd.NJ; ++j) {
        bool any_nonzero = false;
        JointLoadVisual jl;
        jl.no_joint = j;
        jl.pos = Vec3{sd.X[j], sd.Y[j], sd.Z[j]};
        for (int dof = 0; dof < 6; ++dof) {
            jl.actions[dof] = sd.AJ[6 * j - 5 + dof];
            if (jl.actions[dof] != 0.f) any_nonzero = true;
        }
        if (any_nonzero) scene.joint_loads.push_back(jl);
    }

    if (sd.NJ > 0) {
        scene.bounds_center = (min_pt + max_pt) * 0.5f;
        scene.bounds_radius = std::max(1.f, (max_pt - min_pt).Length() * 0.5f);
    }

    return scene;
}

namespace {

// Closest distance between a ray (origin + s*dir, s >= 0, dir normalized)
// and a finite segment [segA, segB] -- the standard closest-point-between-
// two-lines derivation (Ericson, "Real-Time Collision Detection" 5.1.9),
// with the ray side's parameter clamped to [0, inf) instead of [0, 1].
float ClosestDistanceRaySegment(const Vec3& ray_origin, const Vec3& ray_dir, const Vec3& seg_a, const Vec3& seg_b) {
    Vec3 d2 = seg_b - seg_a;
    Vec3 r = ray_origin - seg_a;
    float e = Dot(d2, d2);

    if (e < 1e-10f) { // segment degenerates to a point
        float s = std::max(0.f, Dot(ray_dir, seg_a - ray_origin));
        return ((ray_origin + ray_dir * s) - seg_a).Length();
    }

    float c = Dot(ray_dir, r);
    float b = Dot(ray_dir, d2);
    float denom = e - b * b; // a (=dot(ray_dir,ray_dir)) is 1 since ray_dir is normalized
    float s = (denom > 1e-10f) ? std::max(0.f, (b * Dot(d2, r) - c * e) / denom) : 0.f;
    float t = (b * s + Dot(d2, r)) / e;

    if (t < 0.f) {
        t = 0.f;
        s = std::max(0.f, -c);
    } else if (t > 1.f) {
        t = 1.f;
        s = std::max(0.f, b - c);
    }

    Vec3 point_on_ray = ray_origin + ray_dir * s;
    Vec3 point_on_seg = seg_a + d2 * t;
    return (point_on_ray - point_on_seg).Length();
}

} // namespace

int PickMember(const SceneModel& scene, const Vec3& ray_origin, const Vec3& ray_dir) {
    int best_no_batang = -1;
    float best_distance = std::numeric_limits<float>::max();

    for (const MemberVisual& mv : scene.members) {
        float pick_radius = std::max(0.15f, std::max(mv.width_m, mv.height_m) * 0.75f);
        float dist = ClosestDistanceRaySegment(ray_origin, ray_dir, mv.a, mv.b);
        if (dist <= pick_radius && dist < best_distance) {
            best_distance = dist;
            best_no_batang = mv.no_batang;
        }
    }
    return best_no_batang;
}

namespace {
constexpr float kJointPickRadius = 0.28f; // meters; a bit larger than the rendered marker for easy clicking
}

int PickJoint(const SceneModel& scene, const Vec3& ray_origin, const Vec3& ray_dir) {
    int best_no_joint = -1;
    float best_distance = std::numeric_limits<float>::max();

    for (const JointVisual& jv : scene.joints) {
        Vec3 to_point = jv.pos - ray_origin;
        float t = Dot(to_point, ray_dir);
        if (t < 0.f) continue; // behind the camera
        Vec3 closest_on_ray = ray_origin + ray_dir * t;
        float dist = (closest_on_ray - jv.pos).Length();
        if (dist <= kJointPickRadius && dist < best_distance) {
            best_distance = dist;
            best_no_joint = jv.no_joint;
        }
    }
    return best_no_joint;
}

} // namespace orcisf::gui
