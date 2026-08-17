#pragma once

#include <string>

namespace orcisf::engine {

// Issue #63 investigation (RunPanel hangs after the engine has already
// fully finished a run). The GUI build has no console (issue #12), so
// std::cerr goes nowhere -- this appends a millisecond-timestamped line
// to "<temp dir>/orcisf_run_debug.log" instead, which is the only way to
// see exactly which step a background/main thread is on when the app
// appears hung. Deliberately not gated behind a build flag: the calls
// are a handful of cheap, mutex-guarded file appends, only on the run
// completion path (never per-generation), so the overhead is negligible
// even in a normal release build. Remove the call sites (not necessarily
// this header) once #63 is confirmed fixed and no longer needs live
// diagnosis.
void DebugLog(const std::string& tag, const std::string& msg);

} // namespace orcisf::engine
