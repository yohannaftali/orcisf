#include "engine/DebugLog.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>

namespace orcisf::engine {

namespace {
std::mutex g_debug_log_mutex;
}

void DebugLog(const std::string& tag, const std::string& msg) {
    std::lock_guard<std::mutex> lock(g_debug_log_mutex);
    std::filesystem::path path = std::filesystem::temp_directory_path() / "orcisf_run_debug.log";
    std::ofstream f(path, std::ios::app);
    if (!f) return;

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm tm_local{};
#if defined(_WIN32)
    localtime_s(&tm_local, &t);
#else
    localtime_r(&t, &tm_local);
#endif
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm_local);
    f << buf << "." << std::setw(3) << std::setfill('0') << ms.count() << " [" << tag << "] " << msg << "\n";
}

} // namespace orcisf::engine
