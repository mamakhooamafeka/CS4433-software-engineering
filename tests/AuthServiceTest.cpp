#include "TestSupport.hpp"
#include "services/AdminService.hpp"
#include "services/AuthService.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace lms;
using lms::test::World;

namespace {
struct AuthWorld : World {
    AuthService auth{users, courses, assignments, submissions, hasher, clock};
    AdminService admin{users, courses, hasher, clock};
};
}  // namespace

TEST_CASE("Passwords are stored as Argon2id hashes, never as plain text (NFR-08)",
          "[integration][auth]") {
    AuthWorld w;
    const int id = w.addUser(Role::STUDENT, "stu1", "secret123");
    const std::string stored = w.user(id)->getPasswordHash();
    CHECK(stored.find("secret123") == std::string::npos);
    CHECK(stored.rfind("$argon2id$", 0) == 0);
    // The same password hashed twice gives different strings: each has its own salt.
    CHECK(w.hasher.hash("secret123") != w.hasher.hash("secret123"));
}

TEST_CASE("Login succeeds with the right password and fails with a wrong one",
          "[integration][auth]") {
    AuthWorld w;
    w.addUser(Role::LECTURER, "lec1", "secret123");
    CHECK(w.auth.login("lec1", "secret123")->role() == Role::LECTURER);
    CHECK_THROWS_AS(w.auth.login("lec1", "wrong"), AccessDeniedError);
    CHECK_THROWS_AS(w.auth.login("nobody", "secret123"), AccessDeniedError);
}

TEST_CASE("Five wrong passwords lock the account for 15 minutes (FR-02)", "[integration][auth]") {
    AuthWorld w;
    w.addUser(Role::STUDENT, "stu1", "secret123");
    for (int i = 0; i < 5; ++i) {
        CHECK_THROWS_AS(w.auth.login("stu1", "wrong"), AccessDeniedError);
    }
    // Even the right password is refused while locked.
    CHECK_THROWS_AS(w.auth.login("stu1", "secret123"), AccessDeniedError);
    w.clock.advance(std::chrono::minutes(15));
    CHECK_NOTHROW(w.auth.login("stu1", "secret123"));
}

TEST_CASE("A deactivated user cannot log in (FR-04)", "[integration][auth]") {
    AuthWorld w;
    const int adminId = w.addUser(Role::ADMIN, "admin");
    const int stu = w.addUser(Role::STUDENT, "stu1", "secret123");
    w.admin.deactivateUser(*w.user(adminId), stu);
    CHECK_THROWS_AS(w.auth.login("stu1", "secret123"), AccessDeniedError);
    CHECK(w.user(stu) != nullptr);   // the record is kept
}

TEST_CASE("Changing a password needs the current one and 8+ characters (FR-03)",
          "[integration][auth]") {
    AuthWorld w;
    const int id = w.addUser(Role::STUDENT, "stu1", "secret123");
    auto u = w.user(id);
    CHECK_THROWS_AS(w.auth.changePassword(*u, "wrong", "newpass99"), AccessDeniedError);
    CHECK_THROWS_AS(w.auth.changePassword(*u, "secret123", "short"), ValidationError);
    w.auth.changePassword(*u, "secret123", "newpass99");
    CHECK_NOTHROW(w.auth.login("stu1", "newpass99"));
}

TEST_CASE("A reset password must be changed at the next login (FR-05)", "[integration][auth]") {
    AuthWorld w;
    const int adminId = w.addUser(Role::ADMIN, "admin");
    const int stu = w.addUser(Role::STUDENT, "stu1", "secret123");
    w.admin.resetPassword(*w.user(adminId), stu, "temporary1");
    auto u = w.auth.login("stu1", "temporary1");
    CHECK(u->mustChangePassword());
}
