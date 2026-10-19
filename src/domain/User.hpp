// Abstract base class for the four roles (class diagram 2a). Callers use User&
// and the virtual functions, so role-specific behaviour is polymorphic.
#pragma once

#include "domain/Course.hpp"
#include "domain/DateTime.hpp"
#include "domain/Enums.hpp"
#include "domain/MenuOption.hpp"

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace lms {

// The stored fields of a user account, as read from or written to user_account.
struct UserRecord {
    int id = 0;
    std::string username;
    std::string fullName;
    std::string passwordHash;
    Role role = Role::STUDENT;
    bool active = true;
    int failedAttempts = 0;
    std::optional<DateTime> lockedUntil;
    bool mustChangePassword = true;
    // Scope, loaded by the repository: enrolled courses (student), taught courses
    // (lecturer) or the department headed (HoD). Not columns of user_account.
    std::set<int> courseIds;
    std::optional<int> departmentId;
};

class User {
public:
    static constexpr int kMaxFailedAttempts = 5;  // FR-02
    static constexpr int kLockMinutes = 15;       // FR-02

    explicit User(UserRecord record);
    virtual ~User() = default;

    virtual Role role() const = 0;
    virtual std::vector<MenuOption> menuOptions() const = 0;
    virtual bool canAccessCourse(const Course& c) const = 0;

    bool isLocked(DateTime now) const;
    void registerFailedLogin(DateTime now);
    void resetFailedLogins();

    int getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getFullName() const { return fullName_; }
    const std::string& getPasswordHash() const { return passwordHash_; }
    bool isActive() const { return active_; }
    bool mustChangePassword() const { return mustChangePassword_; }
    int getFailedAttempts() const { return failedAttempts_; }
    std::optional<DateTime> getLockedUntil() const { return lockedUntil_; }

    void setId(int id) { id_ = id; }
    void setFullName(const std::string& name) { fullName_ = name; }
    void setPasswordHash(const std::string& hash, bool mustChange);
    void deactivate() { active_ = false; }

    // The record to save, including the role from the subclass.
    UserRecord toRecord() const;

protected:
    int id_;
    std::string username_;
    std::string fullName_;
    std::string passwordHash_;
    bool active_;
    int failedAttempts_;
    std::optional<DateTime> lockedUntil_;
    bool mustChangePassword_;
};

}  // namespace lms
