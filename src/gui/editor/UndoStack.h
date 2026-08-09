#pragma once

#include <vector>

#include "engine/StructureData.h"

// Undo/redo for issue #6's geometry edits. Snapshots are sized to the
// *current* NJ/M (not the full kMak=825 legacy array), the same lesson
// learned the hard way in issue #4: a full StructureData copy is ~14MB
// and would make every edit (and every undo/redo step) pay that cost for
// no reason -- only geometry (X/Y/Z/JRL per joint, JJ/JK/IA per member)
// needs to round-trip here, everything else (analysis/design scratch
// state) is recomputed from scratch on the next run regardless.
namespace orcisf::gui {

struct GeometrySnapshot {
    int NJ = 0, M = 0;
    std::vector<float> X, Y, Z; // index 0 unused, size NJ+1
    std::vector<int> JRL;       // index 0 unused, size 6*NJ+1
    std::vector<int> JJ, JK, IA; // index 0 unused, size M+1
};

GeometrySnapshot CaptureSnapshot(const engine::StructureData& sd);
void RestoreSnapshot(engine::StructureData& sd, const GeometrySnapshot& snapshot);

class UndoStack {
public:
    // Call *before* applying a mutating EditableStructure operation.
    void PushUndo(const engine::StructureData& sd);

    bool CanUndo() const { return !undo_stack_.empty(); }
    bool CanRedo() const { return !redo_stack_.empty(); }

    // No-ops if CanUndo()/CanRedo() is false.
    void Undo(engine::StructureData& sd);
    void Redo(engine::StructureData& sd);

    void Clear();

private:
    std::vector<GeometrySnapshot> undo_stack_;
    std::vector<GeometrySnapshot> redo_stack_;
};

} // namespace orcisf::gui
