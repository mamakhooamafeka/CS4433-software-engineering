#include "infrastructure/LocalFileStore.hpp"

#include "domain/Errors.hpp"
#include "infrastructure/ErrorLog.hpp"

#include <sodium.h>

#include <fstream>
#include <vector>

namespace fs = std::filesystem;

namespace lms {

LocalFileStore::LocalFileStore(fs::path root) : root_(std::move(root)) {
    std::error_code ec;
    fs::create_directories(root_, ec);
    if (ec) {
        ErrorLog::write("create file store " + root_.string() + ": " + ec.message());
        throw DataAccessError("The folder for submitted files could not be created.");
    }
}

// FR-11: one existing, readable, non-empty file of at most 10 MB.
void LocalFileStore::validate(const std::string& path) {
    std::error_code ec;
    const fs::path p(path);
    if (path.empty() || !fs::exists(p, ec)) {
        throw ValidationError("That file does not exist. Check the path and try again.");
    }
    if (!fs::is_regular_file(p, ec)) {
        throw ValidationError("That path is a folder, not a file.");
    }
    const auto size = fs::file_size(p, ec);
    if (ec) {
        throw ValidationError("That file cannot be read.");
    }
    if (size == 0) {
        throw ValidationError("That file is empty.");
    }
    if (size > static_cast<std::uintmax_t>(kMaxFileBytes)) {
        throw ValidationError("That file is larger than 10 MB.");
    }
    std::ifstream in(p, std::ios::binary);
    if (!in) {
        throw ValidationError("That file cannot be read. Is it open in another program?");
    }
}

std::string LocalFileStore::sha256Of(const fs::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in) {
        throw DataAccessError("A stored file could not be read.");
    }
    crypto_hash_sha256_state state;
    crypto_hash_sha256_init(&state);
    std::vector<char> buffer(64 * 1024);
    while (in) {
        in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto got = in.gcount();
        if (got > 0) {
            crypto_hash_sha256_update(&state, reinterpret_cast<const unsigned char*>(buffer.data()),
                                      static_cast<unsigned long long>(got));
        }
    }
    unsigned char digest[crypto_hash_sha256_BYTES];
    crypto_hash_sha256_final(&state, digest);
    char hex[crypto_hash_sha256_BYTES * 2 + 1];
    sodium_bin2hex(hex, sizeof hex, digest, sizeof digest);
    return hex;
}

StoredFile LocalFileStore::store(const std::string& path, int assignmentId, int studentId,
                                 int versionNo) {
    validate(path);
    const fs::path source(path);
    const fs::path folder =
        root_ / std::to_string(assignmentId) / std::to_string(studentId);
    const fs::path target =
        folder / ("v" + std::to_string(versionNo) + "_" + source.filename().string());
    std::error_code ec;
    fs::create_directories(folder, ec);
    if (!ec) fs::copy_file(source, target, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        ErrorLog::write("store " + source.string() + " -> " + target.string() + ": " +
                        ec.message());
        throw DataAccessError("The file could not be saved. Nothing was submitted.");
    }
    StoredFile stored;
    stored.storedPath = target.generic_string();
    stored.originalName = source.filename().string();
    stored.sizeBytes = static_cast<std::int64_t>(fs::file_size(target));
    stored.sha256 = sha256Of(target);
    return stored;
}

// Used to undo a store() when the database write fails (sequence diagram, flow 9a).
void LocalFileStore::remove(const std::string& storedPath) {
    std::error_code ec;
    fs::remove(storedPath, ec);
    if (ec) ErrorLog::write("remove " + storedPath + ": " + ec.message());
}

void LocalFileStore::copyAll(const std::string& destinationDir) {
    std::error_code ec;
    fs::create_directories(destinationDir, ec);
    if (!ec) {
        fs::copy(root_, destinationDir,
                 fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
    }
    if (ec) {
        ErrorLog::write("copyAll to " + destinationDir + ": " + ec.message());
        throw DataAccessError("The submitted files could not be copied to the backup.");
    }
}

} // namespace lms
