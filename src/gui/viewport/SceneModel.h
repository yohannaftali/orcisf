#pragma once

#include <string>
#include <vector>

#include "engine/MemberResults.h"
#include "engine/StructureData.h"
#include "gui/viewport/Math3D.h"

// Render-ready snapshot of a loaded dataset, built once when a folder is
// loaded / an optimization run finishes (not rebuilt every frame). Decouples
// SceneRenderer (pure OpenGL, no engine knowledge beyond these plain
// structs) from engine::StructureData's legacy 1-based LegacyArray layout.
namespace orcisf::gui {

struct MemberVisual {
    int no_batang = 0;
    bool is_beam = true;
    math3d::Vec3 a, b;         // joint coordinates (meters, world space)
    float width_m = 0.3f;      // cross-section width, meters
    float height_m = 0.3f;     // cross-section height, meters
    bool has_results = false;  // false = geometry-only (no analysis/design run yet)
    float kendala = 0.f;       // 0 = every check satisfied; >0 = violated (see MemberResult::Kendala)
    engine::MemberResult result; // valid iff has_results
};

struct JointVisual {
    int no_joint = 0;
    math3d::Vec3 pos;
    bool restrained = false;
};

struct SceneModel {
    std::string dataset_path; // generic path (no extension) this scene was loaded from
    std::vector<MemberVisual> members;
    std::vector<JointVisual> joints;
    math3d::Vec3 bounds_center{0.f, 0.f, 0.f};
    float bounds_radius = 1.f;

    bool Empty() const { return members.empty() && joints.empty(); }
};

// Builds a SceneModel from a loaded StructureData (geometry via
// engine::LoadDatasetForViewing is enough for joints/members/restraints;
// `results`, if non-null, must have one entry per member in `sd`'s
// no_balok/no_kolom order -- e.g. from engine::ComputeMemberResults --
// and is used for dimension-scaled thickness + constraint-status color).
SceneModel BuildSceneModel(const engine::StructureData& sd, const std::vector<engine::MemberResult>* results,
                            std::string dataset_path);

// Returns the no_batang of the member whose centerline is closest to the
// ray (world-space origin/direction, direction must be normalized), or -1
// if none is within a reasonable pick radius of the nearest point on any
// member. Used for issue #5's "clicking a member in 3D" acceptance
// criterion -- pure geometry, no OpenGL/rendering involved.
int PickMember(const SceneModel& scene, const math3d::Vec3& ray_origin, const math3d::Vec3& ray_dir);

} // namespace orcisf::gui
