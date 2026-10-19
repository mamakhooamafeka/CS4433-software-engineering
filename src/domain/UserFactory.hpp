// Simple Factory (class diagram 2a): the only place that decides which User
// subclass to build for a role, so callers never switch on Role themselves.
#pragma once

#include "domain/User.hpp"

#include <memory>
#include <string>

namespace lms {

class UserFactory {
public:
    // Rebuilds a stored user from its record (used by SqliteUserRepository).
    static std::unique_ptr<User> create(const UserRecord& rec);

    // A new account: active, id 0 until inserted, and must change its password (FR-05).
    static std::unique_ptr<User> createNew(Role role, const std::string& username,
                                           const std::string& fullName,
                                           const std::string& passwordHash);
};

} // namespace lms
