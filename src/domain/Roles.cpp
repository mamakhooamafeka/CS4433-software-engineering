#include "domain/Roles.hpp"

namespace lms {

Student::Student(UserRecord record, std::set<int> enrolledCourseIds)
    : User(std::move(record)), enrolledCourseIds_(std::move(enrolledCourseIds)) {}

std::vector<MenuOption> Student::menuOptions() const {
    return {MenuOption::VIEW_MY_ASSIGNMENTS, MenuOption::SUBMIT_ASSIGNMENT,
            MenuOption::VIEW_RELEASED_RESULTS, MenuOption::CHANGE_PASSWORD, MenuOption::LOG_OUT};
}

bool Student::canAccessCourse(const Course& c) const {
    return enrolledCourseIds_.count(c.getId()) > 0;
}

Lecturer::Lecturer(UserRecord record, std::set<int> taughtCourseIds)
    : User(std::move(record)), taughtCourseIds_(std::move(taughtCourseIds)) {}

std::vector<MenuOption> Lecturer::menuOptions() const {
    return {MenuOption::CREATE_ASSIGNMENT, MenuOption::GRANT_EXTENSION,
            MenuOption::MARK_SUBMISSIONS,  MenuOption::CORRECT_MARK,
            MenuOption::CHANGE_PASSWORD,   MenuOption::LOG_OUT};
}

bool Lecturer::canAccessCourse(const Course& c) const {
    return taughtCourseIds_.count(c.getId()) > 0;
}

HeadOfDepartment::HeadOfDepartment(UserRecord record, std::optional<int> departmentId)
    : User(std::move(record)), departmentId_(departmentId) {}

std::vector<MenuOption> HeadOfDepartment::menuOptions() const {
    return {MenuOption::MODERATE_MARKS, MenuOption::AMEND_MARK,      MenuOption::RELEASE_MARKS,
            MenuOption::VIEW_AUDIT_LOG, MenuOption::CHANGE_PASSWORD, MenuOption::LOG_OUT};
}

bool HeadOfDepartment::canAccessCourse(const Course& c) const {
    return departmentId_.has_value() && c.getDepartmentId() == *departmentId_;
}

Administrator::Administrator(UserRecord record) : User(std::move(record)) {}

std::vector<MenuOption> Administrator::menuOptions() const {
    return {MenuOption::MANAGE_ACCOUNTS,
            MenuOption::RESET_PASSWORD,
            MenuOption::MANAGE_DEPARTMENTS_AND_COURSES,
            MenuOption::ASSIGN_LECTURERS_AND_ENROL,
            MenuOption::BACKUP_AND_RESTORE,
            MenuOption::CHANGE_PASSWORD,
            MenuOption::LOG_OUT};
}

} // namespace lms
