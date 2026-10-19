// The console layer driven by scripted input: wrong typing must be asked again,
// never crash, and the menus must reach the services (NFR-07, WP 4.7).
#include "FlowFixture.hpp"
#include "presentation/LoginScreen.hpp"
#include "presentation/RoleMenus.hpp"

#include <catch2/catch_test_macros.hpp>
#include <sstream>

using namespace lms;
using namespace lms::test;

TEST_CASE("ConsoleIO asks again until the input is valid", "[ui]") {
    std::istringstream in("abc\n7\n3\n12.34\n12.3\nmaybe\ny\n");
    std::ostringstream out;
    ConsoleIO io(in, out);
    CHECK(io.readInt("n: ", 1, 5) == 3);
    CHECK(io.readMark("m: ", 50) == 12.3);
    CHECK(io.confirm("ok?"));
    CHECK(out.str().find("whole number from 1 to 5") != std::string::npos);
    CHECK(out.str().find("at most one decimal") != std::string::npos);
}

TEST_CASE("ConsoleIO throws InputClosed when the input ends", "[ui]") {
    std::istringstream in("");
    std::ostringstream out;
    ConsoleIO io(in, out);
    CHECK_THROWS_AS(io.readText("x: ", 10), InputClosed);
}

TEST_CASE("LoginScreen shows a plain message for a wrong password, then exits", "[ui]") {
    Flow f;
    AuthService auth(f.users, f.courses, f.assignments, f.submissions, f.hasher, f.clock);
    std::istringstream in("stu1\nwrongpass\n\n");
    std::ostringstream out;
    ConsoleIO io(in, out);
    LoginScreen login(io, auth);
    CHECK(login.run() == nullptr);
    CHECK(out.str().find("Incorrect username or password") != std::string::npos);
}

TEST_CASE("A student can submit through the menu and gets a receipt", "[ui]") {
    Flow f;
    AuthService auth(f.users, f.courses, f.assignments, f.submissions, f.hasher, f.clock);
    const std::string file = f.makeFile("essay.txt", 100);
    // 2 = Submit, 1 = first assignment, the path in quotes, confirm, 5 = Log out
    std::istringstream in("2\n1\n\"" + file + "\"\ny\n5\n");
    std::ostringstream out;
    ConsoleIO io(in, out);
    StudentMenu menu(io, auth, f.assignmentsSvc, f.submit);
    Student& student = f.student(f.stuId);
    menu.run(student);
    CHECK(out.str().find("Submission receipt") != std::string::npos);
    CHECK(f.submissions.find(f.asgId, f.stuId).has_value());
}
