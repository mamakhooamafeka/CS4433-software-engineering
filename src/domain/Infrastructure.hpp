// Infrastructure interfaces (class diagram 2b). The clock is injected so that
// deadline tests can use a fixed time instead of the real one.
#pragma once

#include "domain/DateTime.hpp"

#include <cstdint>
#include <string>

namespace lms {

class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;
    virtual std::string hash(const std::string& password) = 0;
    virtual bool verify(const std::string& password, const std::string& storedHash) = 0;
};

struct StoredFile {
    std::string storedPath;
    std::string originalName;
    std::int64_t sizeBytes = 0;
    std::string sha256;
};

class IFileStore {
public:
    static constexpr std::int64_t kMaxFileBytes = 10LL * 1024 * 1024;   // FR-11: 10 MB

    virtual ~IFileStore() = default;
    // Throws ValidationError if the file is missing, unreadable, empty or too large.
    virtual void validate(const std::string& path) = 0;
    virtual StoredFile store(const std::string& path, int assignmentId, int studentId,
                             int versionNo) = 0;
    virtual void remove(const std::string& storedPath) = 0;
    virtual void copyAll(const std::string& destinationDir) = 0;
    // Replaces the stored files with those in a backup folder (FR-21 restore).
    virtual void restoreFrom(const std::string& sourceDir) = 0;
};

class IClock {
public:
    virtual ~IClock() = default;
    virtual DateTime now() const = 0;
};

} // namespace lms
