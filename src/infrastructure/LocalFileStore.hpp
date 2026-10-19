// Stores submitted files in a local folder (FR-11). Each version gets its own file,
// so earlier versions are never overwritten (BR-03).
#pragma once

#include "domain/Infrastructure.hpp"

#include <filesystem>

namespace lms {

class LocalFileStore : public IFileStore {
public:
    explicit LocalFileStore(std::filesystem::path root);

    void validate(const std::string& path) override;
    StoredFile store(const std::string& path, int assignmentId, int studentId,
                     int versionNo) override;
    void remove(const std::string& storedPath) override;
    void copyAll(const std::string& destinationDir) override;
    void restoreFrom(const std::string& sourceDir) override;

    // SHA-256 of a file as 64 hex characters (libsodium).
    static std::string sha256Of(const std::filesystem::path& file);

private:
    std::filesystem::path root_;
};

} // namespace lms
