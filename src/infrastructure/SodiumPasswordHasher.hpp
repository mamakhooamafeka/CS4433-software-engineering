// Password hashing with libsodium's Argon2id (NFR-08). The stored string contains
// the algorithm, its parameters and a random salt, so no separate salt column is needed.
#pragma once

#include "domain/Infrastructure.hpp"

namespace lms {

class SodiumPasswordHasher : public IPasswordHasher {
public:
    SodiumPasswordHasher();   // throws DataAccessError if libsodium cannot start
    std::string hash(const std::string& password) override;
    bool verify(const std::string& password, const std::string& storedHash) override;
};

} // namespace lms
