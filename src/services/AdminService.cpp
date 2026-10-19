#include "services/AdminService.hpp"

#include "domain/UserFactory.hpp"
#include "services/Validation.hpp"

namespace lms {

std::unique_ptr<User> AdminService::requireUserWithRole(int userId, Role role) {
    auto user = users_.findById(userId);
    if (!user) {
        throw ValidationError("There is no user with that number.");
    }
    if (user->role() != role) {
        throw ValidationError("That user is not a " + toString(role) + ".");
    }
    return user;
}

Course AdminService::requireCourse(int courseId) {
    auto course = courses_.findCourse(courseId);
    if (!course) {
        throw ValidationError("There is no course with that number.");
    }
    return *course;
}

bool AdminService::needsFirstAdmin() {
    return users_.listAll().empty();
}

int AdminService::createFirstAdmin(const std::string& username, const std::string& fullName,
                                   const std::string& password) {
    if (!needsFirstAdmin()) {
        throw AccessDeniedError("An administrator already exists.");
    }
    requirePasswordStrength(password);
    auto admin = UserFactory::createNew(Role::ADMIN, requireUsername(username),
                                        requireText(fullName, "The full name", 1, 100),
                                        hasher_.hash(password));
    admin->setPasswordHash(admin->getPasswordHash(), false);
    return users_.insert(*admin);
}

int AdminService::createUser(const User& admin, Role role, const std::string& username,
                             const std::string& fullName, const std::string& temporaryPassword) {
    requireRole(admin, Role::ADMIN);
    const std::string name = requireUsername(username);
    const std::string full = requireText(fullName, "The full name", 1, 100);
    requirePasswordStrength(temporaryPassword);
    auto user = UserFactory::createNew(role, name, full, hasher_.hash(temporaryPassword));
    return users_.insert(*user);
}

void AdminService::editUserName(const User& admin, int userId, const std::string& fullName) {
    requireRole(admin, Role::ADMIN);
    auto user = users_.findById(userId);
    if (!user) throw ValidationError("There is no user with that number.");
    user->setFullName(requireText(fullName, "The full name", 1, 100));
    users_.update(*user);
}

// FR-04: the record is kept; the user just cannot log in any more.
void AdminService::deactivateUser(const User& admin, int userId) {
    requireRole(admin, Role::ADMIN);
    if (userId == admin.getId()) {
        throw BusinessRuleError("You cannot deactivate your own account.");
    }
    auto user = users_.findById(userId);
    if (!user) throw ValidationError("There is no user with that number.");
    user->deactivate();
    users_.update(*user);
}

// FR-05: a temporary password that must be changed at the next login.
void AdminService::resetPassword(const User& admin, int userId,
                                 const std::string& temporaryPassword) {
    requireRole(admin, Role::ADMIN);
    requirePasswordStrength(temporaryPassword);
    auto user = users_.findById(userId);
    if (!user) throw ValidationError("There is no user with that number.");
    user->setPasswordHash(hasher_.hash(temporaryPassword), true);
    user->resetFailedLogins();
    users_.update(*user);
}

std::vector<UserRecord> AdminService::listUsers(const User& admin) {
    requireRole(admin, Role::ADMIN);
    return users_.listAll();
}

int AdminService::createDepartment(const User& admin, const std::string& name, int hodUserId) {
    requireRole(admin, Role::ADMIN);
    const int id = courses_.insertDepartment(requireText(name, "The department name", 1, 100));
    if (hodUserId > 0) {
        assignHeadOfDepartment(admin, id, hodUserId);
    }
    return id;
}

void AdminService::assignHeadOfDepartment(const User& admin, int departmentId, int hodUserId) {
    requireRole(admin, Role::ADMIN);
    if (!courses_.findDepartment(departmentId)) {
        throw ValidationError("There is no department with that number.");
    }
    requireUserWithRole(hodUserId, Role::HOD);
    courses_.setHeadOfDepartment(departmentId, hodUserId);
}

int AdminService::createCourse(const User& admin, const std::string& code,
                               const std::string& title, int departmentId) {
    requireRole(admin, Role::ADMIN);
    if (!courses_.findDepartment(departmentId)) {
        throw ValidationError("There is no department with that number.");
    }
    return courses_.insertCourse(requireText(code, "The course code", 1, 20),
                                 requireText(title, "The course title", 1, 100), departmentId);
}

void AdminService::assignLecturer(const User& admin, int courseId, int lecturerId) {
    requireRole(admin, Role::ADMIN);
    requireCourse(courseId);
    requireUserWithRole(lecturerId, Role::LECTURER);
    courses_.addLecturer(courseId, lecturerId);
}

void AdminService::enrolStudent(const User& admin, int courseId, int studentId) {
    requireRole(admin, Role::ADMIN);
    requireCourse(courseId);
    requireUserWithRole(studentId, Role::STUDENT);
    courses_.enrol(courseId, studentId, clock_.now());
}

void AdminService::withdrawStudent(const User& admin, int courseId, int studentId) {
    requireRole(admin, Role::ADMIN);
    requireCourse(courseId);
    if (!courses_.isEnrolled(courseId, studentId)) {
        throw ValidationError("That student is not enrolled in that course.");
    }
    courses_.withdraw(courseId, studentId);
}

std::vector<Department> AdminService::listDepartments(const User& admin) {
    requireRole(admin, Role::ADMIN);
    return courses_.listDepartments();
}

std::vector<Course> AdminService::listCourses(const User& admin) {
    requireRole(admin, Role::ADMIN);
    return courses_.listCourses();
}

} // namespace lms
