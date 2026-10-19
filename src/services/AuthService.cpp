#include "services/AuthService.hpp"

#include "domain/Roles.hpp"
#include "services/Validation.hpp"

namespace lms {
namespace {
const char* const kLoginFailed = "Incorrect username or password.";
}  // namespace

std::unique_ptr<User> AuthService::login(const std::string& username, const std::string& password) {
    const DateTime now = clock_.now();
    std::unique_ptr<User> user = users_.findByUsername(username);
    if (!user) {
        throw AccessDeniedError(kLoginFailed);
    }
    if (!user->isActive()) {
        throw AccessDeniedError("This account has been deactivated. Please see the administrator.");
    }
    if (user->isLocked(now)) {
        throw AccessDeniedError(
            "This account is locked for 15 minutes after 5 failed attempts. Please try later.");
    }
    if (!hasher_.verify(password, user->getPasswordHash())) {
        user->registerFailedLogin(now);
        users_.update(*user);
        if (user->isLocked(now)) {
            throw AccessDeniedError(
                "Too many failed attempts. The account is now locked for 15 minutes.");
        }
        throw AccessDeniedError(kLoginFailed);
    }
    if (user->getFailedAttempts() != 0 || user->getLockedUntil()) {
        user->resetFailedLogins();
        users_.update(*user);
    }
    return user;
}

void AuthService::changePassword(User& user, const std::string& currentPassword,
                                 const std::string& newPassword) {
    if (!hasher_.verify(currentPassword, user.getPasswordHash())) {
        throw AccessDeniedError("Your current password is not correct.");
    }
    requirePasswordStrength(newPassword);
    if (newPassword == currentPassword) {
        throw ValidationError("The new password must be different from the current one.");
    }
    user.setPasswordHash(hasher_.hash(newPassword), false);
    users_.update(user);
}

int AuthService::pendingWorkCount(const User& user) {
    int count = 0;
    switch (user.role()) {
        case Role::LECTURER:
            for (const Course& c : courses_.coursesOfLecturer(user.getId())) {
                for (const Assignment& a : assignments_.byCourse(c.getId())) {
                    count += static_cast<int>(
                        submissions_.byAssignmentAndStatus(a.getId(), SubmissionStatus::SUBMITTED)
                            .size() +
                        submissions_.byAssignmentAndStatus(a.getId(), SubmissionStatus::LATE)
                            .size());
                }
            }
            break;
        case Role::HOD: {
            const auto dept = static_cast<const HeadOfDepartment&>(user).getDepartmentId();
            if (!dept) break;
            for (const Course& c : courses_.coursesOfDepartment(*dept)) {
                for (const Assignment& a : assignments_.byCourse(c.getId())) {
                    count += static_cast<int>(
                        submissions_.byAssignmentAndStatus(a.getId(), SubmissionStatus::MARKED)
                            .size());
                }
            }
            break;
        }
        case Role::STUDENT:
            for (const Course& c : courses_.coursesOfStudent(user.getId())) {
                for (const Assignment& a : assignments_.byCourse(c.getId())) {
                    if (!submissions_.hasReleased(a.getId()) &&
                        !submissions_.find(a.getId(), user.getId())) {
                        ++count;
                    }
                }
            }
            break;
        case Role::ADMIN:
            break;
    }
    return count;
}

}  // namespace lms
