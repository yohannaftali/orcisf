#pragma once

#include <array>
#include <string>
#include <vector>

#include "engine/AnalysisResults.h"
#include "engine/MemberResults.h"
#include "engine/StructureData.h"
#include "gui/diagrams/ForceDiagram.h"
#include "gui/editor/Selection.h"
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

    // Issue #60: this member's two endpoints' translational displacement
    // (meters, unscaled -- SceneRenderer applies the user's live scale
    // factor at draw time, not here), from engine::AnalysisResults.
    // {0,0,0} when no analysis result was supplied to BuildSceneModel().
    math3d::Vec3 disp_a{0.f, 0.f, 0.f}, disp_b{0.f, 0.f, 0.f};

    // Issue #71: this member's N/V/M/T force diagram (gui::ComputeForceDiagram(),
    // #61 -- no new math here), precomputed once per BuildSceneModel() call
    // (not per frame) the same way disp_a/disp_b already are. Empty
    // (samples.empty()) when no AnalysisResults was supplied, or when this
    // particular member has no matching entry in it (e.g. a .str-loaded
    // AnalysisResults, #66, whose member_forces list may not cover a
    // member added by an edit since). Drives the new 3D viewport ribbon
    // overlay (#71) -- ForceDiagramPanel (#61) keeps its own separate
    // on-demand computation for just the selected member, unchanged; both
    // call the same ComputeForceDiagram() so they always agree on
    // shape/magnitude for any member both happen to show.
    ForceDiagram force_diagram;
};

struct JointVisual {
    int no_joint = 0;
    math3d::Vec3 pos;
    bool restrained = false;

    // Issue #60: this joint's translational displacement (meters,
    // unscaled), from engine::AnalysisResults. Rotational DOFs aren't
    // represented -- linear joint-to-joint interpolation only, matching
    // this renderer's existing schematic (non-rotated) member geometry.
    math3d::Vec3 displacement_m{0.f, 0.f, 0.f};
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

    // Issue #60: true iff BuildSceneModel() was given a non-null
    // AnalysisResults -- gates the deformed-shape overlay (and any other
    // future analysis-result-dependent view) the same way `has_results`
    // gates each MemberVisual's design-result display.
    bool has_deformation = false;

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
// `analysis`, if non-null (issue #60, from engine::ComputeAnalysisResults),
// fills each MemberVisual/JointVisual's displacement_m for the deformed-
// shape overlay -- same "same run, same sd" precondition as `results`.
SceneModel BuildSceneModel(const engine::StructureData& sd, const std::vector<engine::MemberResult>* results,
                            std::string dataset_path,
                            const std::array<int, engine::kMak>* type_overrides = nullptr,
                            const engine::AnalysisResults* analysis = nullptr);

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

// Issue #71: where a given member's force diagram for a given component
// sits in 3D. Shared by `SceneRenderer::DrawForceDiagramOverlay()` (the GL
// ribbon/fill geometry) and `ViewportPanel`'s text-value labels -- exactly
// the ComputeGroundGridLayout() split above, and for the same reason: two
// renderers must never disagree about where a diagram actually is.
struct ForceDiagramPlacement {
    bool valid = false;         // false for a degenerate (zero-length) member
    math3d::Vec3 axis_x;        // along the member, a -> b, normalized
    math3d::Vec3 offset_dir;    // unit direction this component's diagram sticks out in
    math3d::Vec3 plane_normal;  // normal of the diagram's own plane (for lighting the filled surface)
};

// Component -> which plane its diagram is drawn in:
//   Shear/Moment  -> the member's local 1-2 plane. This is the physically
//                    correct plane for both: `ForceDiagramSample::v_n` is
//                    shear along local 2, and `m_nm` is bending about local
//                    3 (which curves the member within the 1-2 plane).
//   Axial/Torsion -> the local 1-3 plane. This one is a *display choice*,
//                    not physics: an axial force acts along the member's own
//                    axis and torsion twists about it, so neither has a
//                    natural bending plane to be drawn in. Putting them in
//                    the perpendicular plane keeps them from overlapping the
//                    shear/moment diagrams when several are shown at once.
// Sign is preserved in every case (a positive value offsets along
// +offset_dir), so a diagram's side still reads as its sign.
ForceDiagramPlacement ComputeForceDiagramPlacement(const MemberVisual& mv, ForceComponent component);

// The value this component contributes at one sampled station -- N/V in
// newtons, M/T in newton-metres (see ForceDiagram.h's unit note).
float ForceComponentValue(const ForceDiagramSample& sample, ForceComponent component);

// "N"/"V"/"M"/"T" and the matching unit string, for labels/legends.
const char* ForceComponentLabel(ForceComponent component);
const char* ForceComponentUnit(ForceComponent component);

// Per-component RGBA, shared so a ribbon and its own text label always
// match. Cross-checked against every other documented viewport color (see
// AGENTS.md's viewport-palette list) -- this palette is now close to full,
// so a future issue adding more overlay colors should probably add a
// legend rather than keep hunting for unused hues.
const float* ForceComponentColor(ForceComponent component); // -> float[4]

} // namespace orcisf::gui
