// Administrator use cases UC03-UC07 (FR-04 to FR-07). Every method first checks
// that the acting user is an administrator.
#pragma once

#include "domain/Infrastructure.hpp"
#include "domain/Repositories.hpp"

#include <string>
#include <vector>

namespace lms {

class AdminService {
public:
    AdminService(IUserRepository& users, ICourseRepository& courses, IPasswordHasher& hasher,
                 IClock& clock)
        : users_(users), courses_(courses), hasher_(hasher), clock_(clock) {}

    // First start-up only: creates the first administrator when no accounts exist.
    bool needsFirstAdmin();
    int createFirstAdmin(const std::string& username, const std::string& fullName,
                         const std::string& password);

    int createUser(const User& admin, Role role, const std::string& username,
                   const std::string& fullName, const std::string& temporaryPassword);
    void editUserName(const User& admin, int userId, const std::string& fullName);
    void deactivateUser(const User& admin, int userId);
    void resetPassword(const User& admin, int userId, const std::string& temporaryPassword);
    std::vector<UserRecord> listUsers(const User& admin);

    int createDepartment(const User& admin, const std::string& name, int hodUserId);
    void assignHeadOfDepartment(const User& admin, int departmentId, int hodUserId);
    int createCourse(const User& admin, const std::string& code, const std::string& title,
                     int departmentId);
    void assignLecturer(const User& admin, int courseId, int lecturerId);
    void enrolStudent(const User& admin, int courseId, int studentId);
    void withdrawStudent(const User& admin, int courseId, int studentId);
    std::vector<Department> listDepartments(const User& admin);
    std::vector<Course> listCourses(const User& admin);

private:
    std::unique_ptr<User> requireUserWithRole(int userId, Role role);
    Course requireCourse(int courseId);

    IUserRepository& users_;
    ICourseRepository& courses_;
    IPasswordHasher& hasher_;
    IClock& clock_;
};

} // namespace lms
