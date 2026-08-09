#pragma once

#include <string>
#include <vector>

namespace orcisf::gui {

// Docked panel showing run/status messages. This is the GUI-side sink for
// progress/log messages; the detailed per-iteration calculation log to disk
// (superset of the legacy .his summary) is tracked separately in issue #3
// (engine port) and will feed this panel via the same callback interface.
class LogPanel {
public:
    void AddLine(std::string line);
    void Clear();
    void Draw(bool* open);

private:
    std::vector<std::string> lines_;
    bool auto_scroll_ = true;
};

} // namespace orcisf::gui
