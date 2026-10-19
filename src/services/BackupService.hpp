// UC08 Back up and restore (FR-21, FR-22, R12). A backup is a folder holding a copy
// of the database and of every submitted file.
#pragma once

#include "data/SqliteDatabase.hpp"
#include "domain/Infrastructure.hpp"
#include "domain/User.hpp"

#include <string>
#include <vector>

namespace lms {

class BackupService {
public:
    BackupService(SqliteDatabase& db, IFileStore& files, IClock& clock, std::string backupRoot)
        : db_(db), files_(files), clock_(clock), backupRoot_(std::move(backupRoot)) {}

    // Called at start-up: makes one backup per calendar day (UTC). Returns true if it did.
    bool backupIfFirstStartToday();
    // Returns the name of the new backup folder.
    std::string backupNow();
    std::vector<std::string> listBackups(const User& admin);
    // Administrator only; the menu asks for confirmation first (FR-22).
    void restore(const User& admin, const std::string& backupName);

private:
    SqliteDatabase& db_;
    IFileStore& files_;
    IClock& clock_;
    std::string backupRoot_;
};

} // namespace lms
