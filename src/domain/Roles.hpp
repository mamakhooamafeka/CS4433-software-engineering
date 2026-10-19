// The four concrete roles (class diagram 2a). Each overrides role(), menuOptions()
// and canAccessCourse(), which is how BR-07 (scope of authority) is enforced.
#pragma once

#include "domain/User.hpp"

#include <set>

namespace lms {

class Student : public User {
public:
    Student(UserRecord record, std::set<int> enrolledCourseIds);

    Role role() const override { return Role::STUDENT; }
    std::vector<MenuOption> menuOptions() const override;
    bool canAccessCourse(const Course& c) const override;

private:
    std::set<int> enrolledCourseIds_;
};

class Lecturer : public User {
public:
    Lecturer(UserRecord record, std::set<int> taughtCourseIds);

    Role role() const override { return Role::LECTURER; }
    std::vector<MenuOption> menuOptions() const override;
    bool canAccessCourse(const Course& c) const override;

private:
    std::set<int> taughtCourseIds_;
};

class HeadOfDepartment : public User {
public:
    // departmentId is empty until the administrator assigns the HoD to a department.
    HeadOfDepartment(UserRecord record, std::optional<int> departmentId);

    Role role() const override { return Role::HOD; }
    std::vector<MenuOption> menuOptions() const override;
    bool canAccessCourse(const Course& c) const override;
    std::optional<int> getDepartmentId() const { return departmentId_; }

private:
    std::optional<int> departmentId_;
};

class Administrator : public User {
public:
    explicit Administrator(UserRecord record);

    Role role() const override { return Role::ADMIN; }
    std::vector<MenuOption> menuOptions() const override;
    // The administrator manages records but has no academic access to marks (BR-07).
    bool canAccessCourse(const Course&) const override { return false; }
};

} // namespace lms
