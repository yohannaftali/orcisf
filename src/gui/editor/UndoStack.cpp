#include "gui/editor/UndoStack.h"

namespace orcisf::gui {

GeometrySnapshot CaptureSnapshot(const engine::StructureData& sd) {
    GeometrySnapshot snap;
    snap.NJ = sd.NJ;
    snap.M = sd.M;

    snap.X.assign(static_cast<size_t>(sd.NJ) + 1, 0.f);
    snap.Y.assign(static_cast<size_t>(sd.NJ) + 1, 0.f);
    snap.Z.assign(static_cast<size_t>(sd.NJ) + 1, 0.f);
    snap.JRL.assign(static_cast<size_t>(6 * sd.NJ) + 1, 0);
    for (int j = 1; j <= sd.NJ; ++j) {
        snap.X[j] = sd.X[j];
        snap.Y[j] = sd.Y[j];
        snap.Z[j] = sd.Z[j];
        for (int dof = 0; dof < 6; ++dof) {
            snap.JRL[6 * j - 5 + dof] = sd.JRL[6 * j - 5 + dof];
        }
    }

    snap.JJ.assign(static_cast<size_t>(sd.M) + 1, 0);
    snap.JK.assign(static_cast<size_t>(sd.M) + 1, 0);
    snap.IA.assign(static_cast<size_t>(sd.M) + 1, 0);
    for (int i = 1; i <= sd.M; ++i) {
        snap.JJ[i] = sd.JJ[i];
        snap.JK[i] = sd.JK[i];
        snap.IA[i] = sd.IA[i];
    }

    return snap;
}

void RestoreSnapshot(engine::StructureData& sd, const GeometrySnapshot& snapshot) {
    sd.NJ = snapshot.NJ;
    sd.M = snapshot.M;

    for (int j = 1; j <= snapshot.NJ; ++j) {
        sd.X[j] = snapshot.X[j];
        sd.Y[j] = snapshot.Y[j];
        sd.Z[j] = snapshot.Z[j];
        for (int dof = 0; dof < 6; ++dof) {
            sd.JRL[6 * j - 5 + dof] = snapshot.JRL[6 * j - 5 + dof];
        }
    }
    for (int i = 1; i <= snapshot.M; ++i) {
        sd.JJ[i] = snapshot.JJ[i];
        sd.JK[i] = snapshot.JK[i];
        sd.IA[i] = snapshot.IA[i];
    }
}

void UndoStack::PushUndo(const engine::StructureData& sd) {
    undo_stack_.push_back(CaptureSnapshot(sd));
    redo_stack_.clear(); // a fresh edit invalidates the redo history
}

void UndoStack::Undo(engine::StructureData& sd) {
    if (!CanUndo()) return;
    redo_stack_.push_back(CaptureSnapshot(sd));
    RestoreSnapshot(sd, undo_stack_.back());
    undo_stack_.pop_back();
}

void UndoStack::Redo(engine::StructureData& sd) {
    if (!CanRedo()) return;
    undo_stack_.push_back(CaptureSnapshot(sd));
    RestoreSnapshot(sd, redo_stack_.back());
    redo_stack_.pop_back();
}

void UndoStack::Clear() {
    undo_stack_.clear();
    redo_stack_.clear();
}

} // namespace orcisf::gui
