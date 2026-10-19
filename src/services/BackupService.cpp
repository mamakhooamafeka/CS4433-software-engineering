#include "services/BackupService.hpp"

#include "services/Validation.hpp"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace lms {
namespace {

// "2026-11-07T08:30:00Z" -> "2026-11-07_083000" (no ':' so it is a valid Windows folder name).
std::string folderNameFor(DateTime t) {
    std::string s = toIso8601(t);
    s.erase(std::remove(s.begin(), s.end(), ':'), s.end());
    s.erase(std::remove(s.begin(), s.end(), 'Z'), s.end());
    std::replace(s.begin(), s.end(), 'T', '_');
    return s;
}

} // namespace

bool BackupService::backupIfFirstStartToday() {
    const std::string today = toIso8601(clock_.now()).substr(0, 10);
    std::error_code ec;
    if (fs::is_directory(backupRoot_, ec)) {
        for (const auto& entry : fs::directory_iterator(backupRoot_, ec)) {
            if (entry.path().filename().string().rfind(today, 0) == 0) return false;
        }
    }
    backupNow();
    return true;
}

std::string BackupService::backupNow() {
    const std::string name = folderNameFor(clock_.now());
    const fs::path folder = fs::path(backupRoot_) / name;
    std::error_code ec;
    fs::create_directories(folder, ec);
    if (ec) throw DataAccessError("The backup folder could not be created.");
    db_.backupTo((folder / "lms.db").string());
    files_.copyAll((folder / "submissions").string());
    return name;
}

std::vector<std::string> BackupService::listBackups(const User& admin) {
    requireRole(admin, Role::ADMIN);
    std::vector<std::string> names;
    std::error_code ec;
    if (fs::is_directory(backupRoot_, ec)) {
        for (const auto& entry : fs::directory_iterator(backupRoot_, ec)) {
            if (entry.is_directory()) names.push_back(entry.path().filename().string());
        }
    }
    std::sort(names.rbegin(), names.rend());   // newest first
    return names;
}

void BackupService::restore(const User& admin, const std::string& backupName) {
    requireRole(admin, Role::ADMIN);
    const auto names = listBackups(admin);
    if (std::find(names.begin(), names.end(), backupName) == names.end()) {
        throw ValidationError("There is no backup with that name.");
    }
    const fs::path folder = fs::path(backupRoot_) / backupName;
    db_.restoreFrom((folder / "lms.db").string());
    files_.restoreFrom((folder / "submissions").string());
}

} // namespace lms
