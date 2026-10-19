#include "TestSupport.hpp"
#include "services/AdminService.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace lms;
using lms::test::World;

namespace {
struct AdminWorld : World {
    AdminService admin{users, courses, hasher, clock};
};
}  // namespace

TEST_CASE("The first administrator can only be created on an empty system",
          "[integration][admin]") {
    AdminWorld w;
    REQUIRE(w.admin.needsFirstAdmin());
    w.admin.createFirstAdmin("admin", "School Admin", "adminpass1");
    CHECK_FALSE(w.admin.needsFirstAdmin());
    CHECK_THROWS_AS(w.admin.createFirstAdmin("admin2", "Other", "adminpass1"), AccessDeniedError);
}

TEST_CASE("Only an administrator can manage accounts (FR-01, BR-07)", "[integration][admin]") {
    AdminWorld w;
    const int lec = w.addUser(Role::LECTURER, "lec1");
    CHECK_THROWS_AS(w.admin.createUser(*w.user(lec), Role::STUDENT, "stu1", "S", "password1"),
                    AccessDeniedError);
}

TEST_CASE("The administrator sets up departments, courses, lecturers and students",
          "[integration][admin]") {
    AdminWorld w;
    const int adminId = w.addUser(Role::ADMIN, "admin");
    auto admin = w.user(adminId);
    const int hod = w.admin.createUser(*admin, Role::HOD, "hod1", "Mrs Head", "password1");
    const int lec = w.admin.createUser(*admin, Role::LECTURER, "lec1", "Mr Teach", "password1");
    const int stu = w.admin.createUser(*admin, Role::STUDENT, "stu1", "Thabo", "password1");
    const int dept = w.admin.createDepartment(*admin, "Sciences", hod);
    const int course = w.admin.createCourse(*admin, "PHY101", "Physics", dept);
    w.admin.assignLecturer(*admin, course, lec);
    w.admin.enrolStudent(*admin, course, stu);

    const Course c = *w.courses.findCourse(course);
    CHECK(w.user(lec)->canAccessCourse(c));
    CHECK(w.user(stu)->canAccessCourse(c));
    CHECK(w.user(hod)->canAccessCourse(c));

    w.admin.withdrawStudent(*admin, course, stu);
    CHECK_FALSE(w.user(stu)->canAccessCourse(c));
}

TEST_CASE("Wrong roles and bad input are refused with clear errors", "[integration][admin]") {
    AdminWorld w;
    const int adminId = w.addUser(Role::ADMIN, "admin");
    auto admin = w.user(adminId);
    const int stu = w.addUser(Role::STUDENT, "stu1");
    const int dept = w.admin.createDepartment(*admin, "Sciences", 0);
    const int course = w.admin.createCourse(*admin, "PHY101", "Physics", dept);

    CHECK_THROWS_AS(w.admin.assignLecturer(*admin, course, stu), ValidationError);   // not a lecturer
    CHECK_THROWS_AS(w.admin.assignHeadOfDepartment(*admin, dept, stu), ValidationError);
    CHECK_THROWS_AS(w.admin.createCourse(*admin, "X1", "X", 999), ValidationError);  // no such dept
    CHECK_THROWS_AS(w.admin.createUser(*admin, Role::STUDENT, "a b", "Name", "password1"),
                    ValidationError);                                            // space in username
    CHECK_THROWS_AS(w.admin.createUser(*admin, Role::STUDENT, "ok_name", "", "password1"),
                    ValidationError);                                            // empty name
    CHECK_THROWS_AS(w.admin.deactivateUser(*admin, adminId), BusinessRuleError); // self
}

TEST_CASE("One HoD cannot head two departments (ASM-04)", "[integration][admin]") {
    AdminWorld w;
    auto admin = w.user(w.addUser(Role::ADMIN, "admin"));
    const int hod = w.addUser(Role::HOD, "hod1");
    w.admin.createDepartment(*admin, "Sciences", hod);
    const int arts = w.admin.createDepartment(*admin, "Arts", 0);
    CHECK_THROWS_AS(w.admin.assignHeadOfDepartment(*admin, arts, hod), BusinessRuleError);
}
