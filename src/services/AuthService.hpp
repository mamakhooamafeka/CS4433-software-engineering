// UC01 Log in and UC02 Change password (FR-01, FR-02, FR-03, FR-23).
#pragma once

#include "domain/Infrastructure.hpp"
#include "domain/Repositories.hpp"

#include <memory>
#include <string>

namespace lms {

class AuthService {
public:
    AuthService(IUserRepository& users, ICourseRepository& courses,
                IAssignmentRepository& assignments, ISubmissionRepository& submissions,
                IPasswordHasher& hasher, IClock& clock)
        : users_(users),
          courses_(courses),
          assignments_(assignments),
          submissions_(submissions),
          hasher_(hasher),
          clock_(clock) {}

    // Returns the logged-in user, or throws AccessDeniedError with a message that
    // does not reveal whether the username exists.
    std::unique_ptr<User> login(const std::string& username, const std::string& password);

    void changePassword(User& user, const std::string& currentPassword,
                        const std::string& newPassword);

    // FR-23: submissions to mark (lecturer), to moderate (HoD) or assignments
    // still to submit (student). 0 for the administrator.
    int pendingWorkCount(const User& user);

private:
    IUserRepository& users_;
    ICourseRepository& courses_;
    IAssignmentRepository& assignments_;
    ISubmissionRepository& submissions_;
    IPasswordHasher& hasher_;
    IClock& clock_;
};

} // namespace lms
