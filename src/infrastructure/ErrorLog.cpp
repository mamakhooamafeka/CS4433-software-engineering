#include "infrastructure/ErrorLog.hpp"

#include "domain/DateTime.hpp"

#include <chrono>
#include <fstream>
#include <mutex>

namespace lms {
namespace {

std::string& logPath() {
    static std::string path = "lms-error.log";
    return path;
}

std::mutex& logMutex() {
    static std::mutex m;
    return m;
}

}  // namespace

void ErrorLog::setPath(const std::string& path) {
    std::lock_guard<std::mutex> lock(logMutex());
    logPath() = path;
}

void ErrorLog::write(const std::string& detail) {
    std::lock_guard<std::mutex> lock(logMutex());
    std::ofstream out(logPath(), std::ios::app);
    if (out) {
        out << toIso8601(std::chrono::system_clock::now()) << "  " << detail << '\n';
    }
}

}  // namespace lms
