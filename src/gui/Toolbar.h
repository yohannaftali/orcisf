#pragma once

#include <functional>

namespace orcisf::gui {

// Top toolbar. "File > Open Folder..." is wired (issue #5, native folder
// dialog + dataset loading); everything else is still a placeholder --
// load-type selection lands in issue #7, run/cancel + core-count control
// live in the RunPanel (issue #4).
class Toolbar {
public:
    void Draw();

    // Called when the user picks "File > Open Folder..." -- wired by
    // Application to a native folder dialog + engine::LoadDatasetForViewing.
    void SetOnOpenFolder(std::function<void()> callback);

private:
    std::function<void()> on_open_folder_;
};

} // namespace orcisf::gui
