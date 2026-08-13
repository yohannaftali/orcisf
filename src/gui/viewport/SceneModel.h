#pragma once

#include <array>
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

// Issue #7: a member's uniform distributed load, present only for members
// with a nonzero raw W (see engine::ReadLoadsRaw()/EditableStructure --
// self-weight is never shown here, only user-applied loads).
struct MemberLoadVisual {
    int no_batang = 0;
    float w_n_per_m = 0.f; // legacy convention: positive = downward (gravity sense)
    math3d::Vec3 midpoint;
    math3d::Vec3 axis; // normalized member direction, for placing the arrow along its length
};

// Issue #7: a joint's 6 generalized actions (Fx,Fy,Fz,Mx,My,Mz -- the
// legacy "arah 1..6" convention), present only for joints with at least
// one nonzero component.
struct JointLoadVisual {
    int no_joint = 0;
    float actions[6] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
    math3d::Vec3 pos;
};

struct SceneModel {
    std::string dataset_path; // generic path (no extension) this scene was loaded from
    std::vector<MemberVisual> members;
    std::vector<JointVisual> joints;
    std::vector<MemberLoadVisual> member_loads;
    std::vector<JointLoadVisual> joint_loads;
    math3d::Vec3 bounds_center{0.f, 0.f, 0.f};
    float bounds_radius = 1.f;

    bool Empty() const { return members.empty() && joints.empty(); }
};

// Builds a SceneModel from a loaded StructureData (geometry via
// engine::LoadDatasetForViewing is enough for joints/members/restraints;
// `results`, if non-null, must have one entry per member in `sd`'s
// no_balok/no_kolom order -- e.g. from engine::ComputeMemberResults --
// and is used for dimension-scaled thickness + constraint-status color).
// Also reads `sd`'s raw W/AJ directly for member_loads/joint_loads (issue
// #7) -- callers must have populated those via engine::ReadLoadsRaw() (or
// left them at zero), never via engine::ReadLoads(), or self-weight would
// show up as a phantom "user load".
// `type_overrides`, if non-null (issue #39, see EditableStructure::
// MemberTypeOverrides()), folds each member's GUI-only type override into
// MemberVisual::is_beam so the viewport/Properties/Detailing panels agree
// with what the Members panel's dropdown shows -- purely a display choice,
// never fed into engine::StructuralAnalysis/Optimizer (see EditableStructure.h).
SceneModel BuildSceneModel(const engine::StructureData& sd, const std::vector<engine::MemberResult>* results,
                            std::string dataset_path,
                            const std::array<int, engine::kMak>* type_overrides = nullptr);

// Returns the no_batang of the member whose centerline is closest to the
// ray (world-space origin/direction, direction must be normalized), or -1
// if none is within a reasonable pick radius of the nearest point on any
// member. Used for issue #5's "clicking a member in 3D" acceptance
// criterion -- pure geometry, no OpenGL/rendering involved.
int PickMember(const SceneModel& scene, const math3d::Vec3& ray_origin, const math3d::Vec3& ray_dir);

// Same idea as PickMember but for joint markers (ray-vs-sphere against
// each joint's position) -- used by issue #6's editor so a click can
// select a joint (to drag/delete/retype its coordinates) in preference to
// the member underneath it. Returns -1 if none is within pick range.
int PickJoint(const SceneModel& scene, const math3d::Vec3& ray_origin, const math3d::Vec3& ray_dir);

// Issue #50: pure-geometry layout for the ground-plane reference grid
// (X-Z plane, per ORCISF's Y-up convention -- see AGENTS.md's
// gui/viewport/ section) -- shared by SceneRenderer::DrawGrid() (the GL
// line geometry) and ViewportPanel's label overlay so both agree on
// exactly where every line/label sits, the same reasoning
// DetailingLayout/DetailingPanel's split (#8) uses. Grid lines are
// anchored to true world-space multiples of `spacing_m` (not centered on
// the structure's own bounding box), so a label like "X5" always means
// world X=5 -- matching the coordinates already shown in the
// Joints/Properties panels.
struct GroundGridLayout {
    float spacing_m = 1.f;          // meters between adjacent grid lines
    int x_index_min = -5, x_index_max = 5;  // grid covers world X in [x_index_min*spacing_m, x_index_max*spacing_m]
    int z_index_min = -5, z_index_max = 5;  // same, for world Z
    float y = 0.f;                  // world Y the grid plane sits at (lowest joint's Y, or 0 if none)
    int label_stride = 1;           // label every Nth line (1 = every line)
};
GroundGridLayout ComputeGroundGridLayout(const SceneModel& scene);

} // namespace orcisf::gui
