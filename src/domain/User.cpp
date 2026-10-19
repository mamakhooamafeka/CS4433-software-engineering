#include "domain/User.hpp"

namespace lms {

User::User(UserRecord record)
    : id_(record.id),
      username_(std::move(record.username)),
      fullName_(std::move(record.fullName)),
      passwordHash_(std::move(record.passwordHash)),
      active_(record.active),
      failedAttempts_(record.failedAttempts),
      lockedUntil_(record.lockedUntil),
      mustChangePassword_(record.mustChangePassword) {}

bool User::isLocked(DateTime now) const {
    return lockedUntil_.has_value() && now < *lockedUntil_;
}

// FR-02: the fifth consecutive failure locks the account for 15 minutes.
void User::registerFailedLogin(DateTime now) {
    ++failedAttempts_;
    if (failedAttempts_ >= kMaxFailedAttempts) {
        lockedUntil_ = now + std::chrono::minutes(kLockMinutes);
        failedAttempts_ = 0;
    }
}

void User::resetFailedLogins() {
    failedAttempts_ = 0;
    lockedUntil_.reset();
}

void User::setPasswordHash(const std::string& hash, bool mustChange) {
    passwordHash_ = hash;
    mustChangePassword_ = mustChange;
}

UserRecord User::toRecord() const {
    UserRecord r;
    r.id = id_;
    r.username = username_;
    r.fullName = fullName_;
    r.passwordHash = passwordHash_;
    r.role = role();
    r.active = active_;
    r.failedAttempts = failedAttempts_;
    r.lockedUntil = lockedUntil_;
    r.mustChangePassword = mustChangePassword_;
    return r;
}

} // namespace lms
