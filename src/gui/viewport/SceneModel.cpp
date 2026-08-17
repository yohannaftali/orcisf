#include "gui/viewport/SceneModel.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace orcisf::gui {

using engine::AnalysisResults;
using engine::MemberResult;
using engine::StructureData;
using math3d::Vec3;

SceneModel BuildSceneModel(const StructureData& sd, const std::vector<MemberResult>* results,
                            std::string dataset_path, const std::array<int, engine::kMak>* type_overrides,
                            const AnalysisResults* analysis) {
    SceneModel scene;
    scene.dataset_path = std::move(dataset_path);
    scene.has_deformation = (analysis != nullptr);

    std::unordered_map<int, const MemberResult*> result_by_batang;
    if (results) {
        for (const MemberResult& r : *results) {
            result_by_batang[r.no_batang] = &r;
        }
    }

    // Issue #60: joint no_joint -> translational displacement (meters),
    // looked up by id rather than assumed-index (0-based vector position)
    // to stay correct even if a future caller ever supplies a
    // differently-ordered/partial AnalysisResults.
    std::unordered_map<int, Vec3> disp_by_joint;
    if (analysis) {
        for (const engine::JointDisplacement& d : analysis->displacements) {
            disp_by_joint[d.no_joint] = Vec3{d.ux, d.uy, d.uz};
        }
    }
    auto joint_disp = [&](int no_joint) -> Vec3 {
        auto it = disp_by_joint.find(no_joint);
        return it != disp_by_joint.end() ? it->second : Vec3{0.f, 0.f, 0.f};
    };

    // Issue #71: no_batang -> MemberForces, for the viewport force-diagram
    // ribbon overlay -- same lookup-by-id convention as disp_by_joint above
    // (a .str-loaded AnalysisResults, #66, may not cover every member).
    std::unordered_map<int, const engine::MemberForces*> forces_by_batang;
    if (analysis) {
        for (const engine::MemberForces& f : analysis->member_forces) {
            forces_by_batang[f.no_batang] = &f;
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
        jv.displacement_m = joint_disp(j);
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
        mv.disp_a = joint_disp(sd.JJ[i]);
        mv.disp_b = joint_disp(sd.JK[i]);

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

        // Issue #39: Members panel type-dropdown override, display-only
        // (see SceneModel.h/EditableStructure.h) -- 0 = no override.
        if (type_overrides && i < static_cast<int>(type_overrides->size()) && (*type_overrides)[i] != 0) {
            mv.is_beam = (*type_overrides)[i] == 1;
        }

        auto it = result_by_batang.find(i);
        if (it != result_by_batang.end()) {
            mv.has_results = true;
            mv.result = *it->second;
            mv.width_m = std::max(0.05f, it->second->width / 1000.f);
            mv.height_m = std::max(0.05f, it->second->height / 1000.f);
            mv.kendala = it->second->Kendala();
        }

        // Issue #71: width_mm/height_mm must be 0 (not mv.width_m/height_m's
        // 0.3m *placeholder* default) when this member has no real
        // MemberResult yet -- passing the placeholder would fabricate a
        // has_section=true stress reading for a section that was never
        // actually designed. N/V/M/T themselves don't need a section at
        // all (has_section only gates the stress columns).
        auto force_it = forces_by_batang.find(i);
        if (force_it != forces_by_batang.end()) {
            float width_mm = mv.has_results ? mv.width_m * 1000.f : 0.f;
            float height_mm = mv.has_results ? mv.height_m * 1000.f : 0.f;
            mv.force_diagram = ComputeForceDiagram(*force_it->second, len, width_mm, height_mm);
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

GroundGridLayout ComputeGroundGridLayout(const SceneModel& scene) {
    GroundGridLayout layout;

    constexpr float kMinHalfExtent = 5.f; // meters -- keeps a small/empty scene's grid usable
    constexpr float kMarginM = 3.f;       // meters of margin beyond the structure's own extent

    float min_x = -kMinHalfExtent, max_x = kMinHalfExtent;
    float min_z = -kMinHalfExtent, max_z = kMinHalfExtent;
    float min_y = 0.f;
    bool have = false;
    for (const JointVisual& j : scene.joints) {
        if (!have) {
            min_x = max_x = j.pos.x;
            min_z = max_z = j.pos.z;
            min_y = j.pos.y;
            have = true;
        } else {
            min_x = std::min(min_x, j.pos.x);
            max_x = std::max(max_x, j.pos.x);
            min_z = std::min(min_z, j.pos.z);
            max_z = std::max(max_z, j.pos.z);
            min_y = std::min(min_y, j.pos.y);
        }
    }
    layout.y = min_y;

    if (have) {
        min_x -= kMarginM;
        max_x += kMarginM;
        min_z -= kMarginM;
        max_z += kMarginM;
        // A single joint (or a tightly clustered few) would otherwise
        // produce a near-zero-size grid -- guarantee a usable minimum span.
        if (max_x - min_x < 2.f * kMinHalfExtent) {
            float cx = (min_x + max_x) * 0.5f;
            min_x = cx - kMinHalfExtent;
            max_x = cx + kMinHalfExtent;
        }
        if (max_z - min_z < 2.f * kMinHalfExtent) {
            float cz = (min_z + max_z) * 0.5f;
            min_z = cz - kMinHalfExtent;
            max_z = cz + kMinHalfExtent;
        }
    }

    // Doubling the spacing until the line count fits kMaxLinesPerAxis keeps
    // the grid legible (and the per-frame draw-call count bounded)
    // regardless of how large the loaded structure is, rather than a fixed
    // spacing that would either vanish into a wall of lines on a big
    // building or barely cover a small one.
    constexpr int kMaxLinesPerAxis = 48;
    float span = std::max(max_x - min_x, max_z - min_z);
    float spacing = 1.f;
    while (span / spacing > static_cast<float>(kMaxLinesPerAxis)) {
        spacing *= 2.f;
    }
    layout.spacing_m = spacing;

    layout.x_index_min = static_cast<int>(std::floor(min_x / spacing));
    layout.x_index_max = static_cast<int>(std::ceil(max_x / spacing));
    layout.z_index_min = static_cast<int>(std::floor(min_z / spacing));
    layout.z_index_max = static_cast<int>(std::ceil(max_z / spacing));

    int x_lines = layout.x_index_max - layout.x_index_min;
    int z_lines = layout.z_index_max - layout.z_index_min;
    constexpr int kMaxLabelsPerAxis = 8;
    layout.label_stride = std::max(1, (std::max(x_lines, z_lines) + kMaxLabelsPerAxis - 1) / kMaxLabelsPerAxis);

    return layout;
}

ForceDiagramPlacement ComputeForceDiagramPlacement(const MemberVisual& mv, ForceComponent component) {
    ForceDiagramPlacement p;
    Vec3 delta = mv.b - mv.a;
    float length = delta.Length();
    if (length < 1e-5f) return p;

    p.axis_x = delta * (1.f / length);
    // Same local-basis derivation SceneRenderer::DrawBox() uses internally
    // for a member's own box mesh, so a diagram's plane lines up with the
    // rendered member rather than an independently-chosen frame.
    Vec3 reference = (std::fabs(Dot(p.axis_x, Vec3{0, 1, 0})) > 0.99f) ? Vec3{0, 0, 1} : Vec3{0, 1, 0};
    Vec3 axis_z = Cross(p.axis_x, reference).Normalized();
    Vec3 axis_y = Cross(axis_z, p.axis_x).Normalized();

    // See the header's table: shear/moment get the physically-correct local
    // 1-2 plane; axial/torsion get the perpendicular 1-3 plane purely so
    // all four stay separately readable when shown together.
    switch (component) {
        case ForceComponent::Shear:
        case ForceComponent::Moment: p.offset_dir = axis_y; break;
        case ForceComponent::Axial:
        case ForceComponent::Torsion: p.offset_dir = axis_z; break;
    }
    p.plane_normal = Cross(p.axis_x, p.offset_dir).Normalized();
    p.valid = true;
    return p;
}

float ForceComponentValue(const ForceDiagramSample& sample, ForceComponent component) {
    switch (component) {
        case ForceComponent::Axial: return sample.n_n;
        case ForceComponent::Shear: return sample.v_n;
        case ForceComponent::Moment: return sample.m_nm;
        case ForceComponent::Torsion: return sample.t_nm;
    }
    return 0.f;
}

const char* ForceComponentLabel(ForceComponent component) {
    switch (component) {
        case ForceComponent::Axial: return "N";
        case ForceComponent::Shear: return "V";
        case ForceComponent::Moment: return "M";
        case ForceComponent::Torsion: return "T";
    }
    return "?";
}

const char* ForceComponentUnit(ForceComponent component) {
    // N/V are forces (N); M/T are moments (N*m) -- see ForceDiagram.h's
    // unit-correction note, these are N*m and NOT Nmm.
    switch (component) {
        case ForceComponent::Axial:
        case ForceComponent::Shear: return "N";
        case ForceComponent::Moment:
        case ForceComponent::Torsion: return "Nm";
    }
    return "";
}

const float* ForceComponentColor(ForceComponent component) {
    // Four mutually very distinct hues (yellow / pink / lime / lavender).
    // Each is near *one* existing palette entry (yellow~selection gold,
    // lime~satisfied green, lavender~joint-moment purple, pink~violated
    // red), but every one of those is drawn on different geometry (member
    // bodies or joint markers) while these are offset ribbons, so they stay
    // tellable apart in practice. Documented in AGENTS.md's palette list.
    // Alpha must be 1.f, not omitted: the offscreen FBO is RGBA8 and
    // ImGui::Image() blends that texture using its alpha channel, so a
    // zero-alpha color renders as fully transparent (invisible), not opaque.
    static constexpr float kAxial[4] = {1.00f, 0.90f, 0.30f, 1.f};
    static constexpr float kShear[4] = {1.00f, 0.45f, 0.75f, 1.f};
    static constexpr float kMoment[4] = {0.55f, 1.00f, 0.45f, 1.f};
    static constexpr float kTorsion[4] = {0.70f, 0.55f, 1.00f, 1.f};
    switch (component) {
        case ForceComponent::Axial: return kAxial;
        case ForceComponent::Shear: return kShear;
        case ForceComponent::Moment: return kMoment;
        case ForceComponent::Torsion: return kTorsion;
    }
    return kAxial;
}

} // namespace orcisf::gui
