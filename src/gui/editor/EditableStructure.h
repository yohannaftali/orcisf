#pragma once

#include <string>
#include <vector>

#include "engine/StructureData.h"
#include "gui/viewport/Math3D.h"

// Issue #6's "direct 3D edit" + "numeric entry" operations: add/move/delete
// joints and members directly on a loaded engine::StructureData, kept
// consistent with the legacy 1-based/fixed-size array layout (NJ/M counts,
// X/Y/Z/JJ/JK/JRL/IA arrays) so the *same* StructureData a dataset was
// loaded into can be edited in place and re-rendered (BuildSceneModel)
// after every change. Issue #7 extends this to the load fields (W/AML per
// member, AJ per joint) -- kept compacted alongside geometry for the same
// reason, and always the *raw*, self-weight-free values (see
// engine::ReadLoadsRaw()'s comment) since that's what round-trips to
// .bbn via engine::WriteLoads().
//
// Deliberately does *not* touch any other analysis/design scratch field
// (SFF, SM, AM, MTUM_*, var_b/var_k, ...) -- those are recomputed from
// scratch by Inersia()/Struktur()/the optimizer on the next run and don't
// need to stay consistent between edits (see AGENTS.md's gui/editor
// section for why this is a safe simplification, not a shortcut that
// silently breaks a later run).
namespace orcisf::gui {

class EditableStructure {
public:
    explicit EditableStructure(engine::StructureData& sd) : sd_(sd) {}

    // Read-only access to the wrapped StructureData -- e.g. for
    // UndoStack::PushUndo(), which needs to snapshot the current geometry
    // before a mutating call below.
    const engine::StructureData& SdForUndo() const { return sd_; }

    // Mutable access for structure-wide scalar fields (title, material
    // properties -- issue #11) that don't need a dedicated setter/undo path
    // the way geometry does: they don't affect array sizes or indices, so
    // direct assignment from the GUI can't corrupt the joint/member
    // compaction invariants the setters above protect.
    engine::StructureData& Sd() { return sd_; }

    // Returns the new joint's 1-based index, or -1 if the fixed-size
    // legacy arrays (kMak) are already full.
    int AddJoint(const math3d::Vec3& pos);

    // Returns false if joint_id is out of [1, NJ].
    bool MoveJoint(int joint_id, const math3d::Vec3& new_pos);

    // Sets whether every one of the joint's 6 DOF is restrained (a single
    // pin/fixed toggle -- per-DOF restraint editing is out of scope here,
    // see AGENTS.md). Returns false if joint_id is out of [1, NJ].
    bool SetJointRestrained(int joint_id, bool restrained);

    // Deletes the joint and every member touching it, then compacts joint
    // indices (and remaps JJ/JK on the remaining members) so NJ/indices
    // stay contiguous, matching every other legacy 1-based array's
    // invariant. Returns false if joint_id is out of [1, NJ].
    bool DeleteJoint(int joint_id);

    // Returns the new member's 1-based index, or -1 if joint_a == joint_b,
    // either joint id is out of range, or the arrays are full.
    int AddMember(int joint_a, int joint_b);

    // Returns false if member_id is out of [1, M].
    bool DeleteMember(int member_id);

    // Sets a uniform distributed load (N/m, legacy convention: transverse
    // to the member, gravity-direction sense -- see Pembebanan.hpp's
    // load_data()) and recomputes its 12 fixed-end-force components (AML)
    // from the member's *current* length -- so this stays correct even if
    // the member's endpoints moved since the load was first set. Returns
    // false if member_id is out of [1, M].
    bool SetMemberLoad(int member_id, float w_n_per_m);
    bool ClearMemberLoad(int member_id); // sets w=0 and all 12 AML to 0

    // Sets the joint's 6 generalized actions (order: Fx, Fy, Fz, Mx, My,
    // Mz in the legacy "arah 1..6" global-DOF convention -- the same
    // mechanism the legacy format uses for point loads, moments, and
    // lateral/wind loads alike, see Pembebanan.hpp). Returns false if
    // joint_id is out of [1, NJ].
    bool SetJointLoad(int joint_id, const float actions[6]);
    bool ClearJointLoad(int joint_id); // sets all 6 actions to 0

    // Human-readable problems that should block running an analysis/
    // optimization (issue #6's "surfaced to the user before an analysis
    // run is allowed" criterion): coincident joints, zero-length members,
    // and duplicate members (two members connecting the same joint pair).
    std::vector<std::string> Validate() const;

private:
    engine::StructureData& sd_;
};

} // namespace orcisf::gui
