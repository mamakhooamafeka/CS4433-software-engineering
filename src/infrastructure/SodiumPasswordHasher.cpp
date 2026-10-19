#include "infrastructure/SodiumPasswordHasher.hpp"

#include "domain/Errors.hpp"
#include "infrastructure/ErrorLog.hpp"

#include <sodium.h>

namespace lms {

SodiumPasswordHasher::SodiumPasswordHasher() {
    if (sodium_init() < 0) {
        ErrorLog::write("sodium_init failed");
        throw DataAccessError("The security library could not start.");
    }
}

// INTERACTIVE limits (64 MiB, about 0.1 s) keep login under 2 s (NFR-01) and memory
// under 100 MB (NFR-03); MODERATE would use 256 MiB.
std::string SodiumPasswordHasher::hash(const std::string& password) {
    char out[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(out, password.c_str(), password.size(),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        ErrorLog::write("crypto_pwhash_str failed (out of memory)");
        throw DataAccessError("The password could not be saved. Please try again.");
    }
    return out;
}

bool SodiumPasswordHasher::verify(const std::string& password, const std::string& storedHash) {
    return crypto_pwhash_str_verify(storedHash.c_str(), password.c_str(), password.size()) == 0;
}

} // namespace lms
