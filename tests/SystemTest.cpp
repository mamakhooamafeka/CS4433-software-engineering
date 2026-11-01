// Phase 5 system tests: each TEST_CASE follows one flow of a fully dressed use case
// (SRS Tables 7-9) through the real menus, with typed input scripted line by line.
// The test IDs match docs/TEST_PLAN.md section 6.
#include "FlowFixture.hpp"
#include "presentation/LoginScreen.hpp"
#include "presentation/RoleMenus.hpp"
#include "services/AdminService.hpp"
#include "services/BackupService.hpp"
#include "services/ModerationService.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>

using namespace lms;
using Catch::Approx;
using std::chrono::hours;

namespace {

bool has(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

// The whole system on an in-memory database: department "Sciences" with HoD hod1,
// course PHY101 taught by lec1, stu1 enrolled (stu2 is not), one assignment out of 100
// due 48 hours after the start. hod2 heads another department.
struct Sys : test::Flow {
    AuthService auth{users, courses, assignments, submissions, hasher, clock};
    AdminService admin{users, courses, hasher, clock};
    ModerationService moderation{assignments, courses,      submissions, users, audit,
                                 policy,      auditService, clock,       db};
    int hodId, otherHodId;

    Sys() {
        hodId = addUser(Role::HOD, "hod1");
        otherHodId = addUser(Role::HOD, "hod2");
        courses.setHeadOfDepartment(courses.findCourse(courseId)->getDepartmentId(), hodId);
        courses.setHeadOfDepartment(courses.insertDepartment("Arts"), otherHodId);
    }

    // Runs a role menu with the typed lines in `script` and returns everything printed.
    std::string asStudent(int id, const std::string& script) {
        std::istringstream in(script);
        std::ostringstream out;
        ConsoleIO io(in, out);
        StudentMenu menu(io, auth, assignmentsSvc, submit);
        menu.run(student(id));
        return out.str();
    }
    std::string asLecturer(const std::string& script) {
        std::istringstream in(script);
        std::ostringstream out;
        ConsoleIO io(in, out);
        LecturerMenu menu(io, auth, assignmentsSvc, marking);
        menu.run(lecturer());
        return out.str();
    }
    std::string asHod(int id, const std::string& script) {
        std::istringstream in(script);
        std::ostringstream out;
        ConsoleIO io(in, out);
        HodMenu menu(io, auth, moderation);
        auto hod = users.findById(id);
        menu.run(*hod);
        return out.str();
    }

    int addStudent(const std::string& name) {
        const int id = addUser(Role::STUDENT, name);
        courses.enrol(courseId, id, clock.now());
        return id;
    }
    int submitAndMark(int stu, double raw) {
        submit.submit(student(stu), asgId, makeFile("w" + std::to_string(stu) + ".txt", 5));
        const int id = submissions.find(asgId, stu)->getId();
        marking.mark(lecturer(), id, raw, "Feedback " + std::to_string(stu));
        return id;
    }
    SubmissionStatus statusOf(int stu) { return submissions.find(asgId, stu)->getStatus(); }
    HeadOfDepartment& hod(int id) {
        hodHolder = users.findById(id);
        return dynamic_cast<HeadOfDepartment&>(*hodHolder);
    }
    std::unique_ptr<User> hodHolder;
};

// Student menu: 1 view, 2 submit, 3 results, 4 change password, 5 log out.
// Lecturer menu: 1 create, 2 extension, 3 mark, 4 correct, 5 password, 6 log out.
// HoD menu: 1 moderate, 2 amend, 3 release, 4 audit, 5 password, 6 log out.

}  // namespace

// ---------------- UC16 Submit assignment ----------------

TEST_CASE("ST-01 UC16 main flow: on-time submission with receipt", "[system][uc16]") {
    Sys s;
    const std::string file = s.makeFile("essay.txt", 120);
    const auto out = s.asStudent(s.stuId, "2\n1\n" + file + "\ny\n5\n");
    CHECK(has(out, "PHY101 Lab report"));   // step 1: open assignments listed
    CHECK(has(out, "Time left:"));          // step 3
    CHECK(has(out, "Submission receipt"));  // step 10
    CHECK(has(out, "120 bytes"));
    CHECK(has(out, "SHA-256: "));
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::SUBMITTED);
}

TEST_CASE("ST-02 UC16 3a: resubmission before the deadline makes version 2", "[system][uc16]") {
    Sys s;
    s.submit.submit(s.student(s.stuId), s.asgId, s.makeFile("v1.txt", 5));
    const auto out = s.asStudent(s.stuId, "2\n1\n" + s.makeFile("v2.txt", 6) + "\ny\n5\n");
    CHECK(has(out, "This becomes version 2"));
    CHECK(s.submissions.find(s.asgId, s.stuId)->getVersions().size() == 2);
}

TEST_CASE("ST-03 UC16 6a: late submission warns about the deduction and is stored as LATE",
          "[system][uc16]") {
    Sys s;
    s.clock.set(s.deadline + hours(30));  // 2 started days late
    const auto out = s.asStudent(s.stuId, "2\n1\n" + s.makeFile("a.txt", 5) + "\ny\n5\n");
    CHECK(has(out, "late by 2 day(s): 20% of the maximum mark will be deducted"));
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::LATE);
}

TEST_CASE("ST-04 UC16 8a: cancelling at the confirmation stores nothing", "[system][uc16]") {
    Sys s;
    s.asStudent(s.stuId, "2\n1\n" + s.makeFile("a.txt", 5) + "\nn\n5\n");
    CHECK_FALSE(s.submissions.find(s.asgId, s.stuId).has_value());
}

TEST_CASE("ST-05 UC16 1a: a student with no open assignments is told so", "[system][uc16]") {
    Sys s;
    const auto out = s.asStudent(s.otherStuId, "2\n5\n");  // stu2 is not enrolled
    CHECK(has(out, "There is nothing to show."));
}

TEST_CASE("ST-06 UC16 3b: resubmission after marking is refused with the reason",
          "[system][uc16]") {
    Sys s;
    s.submitAndMark(s.stuId, 60);
    const auto out = s.asStudent(s.stuId, "2\n1\n" + s.makeFile("again.txt", 5) + "\n5\n");
    CHECK(has(out, "You cannot resubmit"));
    CHECK(has(out, "Your version 1 is kept, status MARKED"));
    CHECK(s.submissions.find(s.asgId, s.stuId)->getVersions().size() == 1);
}

TEST_CASE("ST-07 UC16 5a: three bad files return to the menu with plain errors", "[system][uc16]") {
    Sys s;
    const std::string empty = s.makeFile("empty.txt", 0);
    const std::string missing = (s.dir / "missing.txt").string();
    const auto out =
        s.asStudent(s.stuId, "2\n1\n" + missing + "\n" + empty + "\n" + missing + "\n5\n");
    CHECK(has(out, "! "));  // plain-language error lines
    CHECK_FALSE(has(out, "Check before you submit"));
    CHECK_FALSE(s.submissions.find(s.asgId, s.stuId).has_value());
}

TEST_CASE("ST-08 UC16 5a: a bad file then a good one continues normally", "[system][uc16]") {
    Sys s;
    const std::string missing = (s.dir / "missing.txt").string();
    const auto out =
        s.asStudent(s.stuId, "2\n1\n" + missing + "\n\"" + s.makeFile("ok.txt", 5) + "\"\ny\n5\n");
    CHECK(has(out, "Submission receipt"));
}

TEST_CASE("ST-09 UC16 precondition: a released assignment is closed (BR-06)", "[system][uc16]") {
    Sys s;
    const int sub = s.submitAndMark(s.stuId, 60);
    s.moderation.approve(s.hod(s.hodId), {sub});
    s.moderation.releaseAll(s.hod(s.hodId), s.asgId);
    const auto out = s.asStudent(s.stuId, "2\n5\n");
    CHECK(has(out, "There is nothing to show."));
}

// ---------------- UC11 Mark submission ----------------

TEST_CASE("ST-10 UC11 main flow with 4a: bad marks are asked again, penalty shown, saved",
          "[system][uc11]") {
    Sys s;
    s.clock.set(s.deadline + hours(1));  // 1 day late
    s.submit.submit(s.student(s.stuId), s.asgId, s.makeFile("a.txt", 5));
    // 3 = Mark, course 1, assignment 1, submission 1, then "abc", "101", "12.34" refused
    const auto out = s.asLecturer("3\n1\n1\n1\nabc\n101\n12.34\n80\nClear and complete\ny\n6\n");
    CHECK(has(out, "File: "));  // step 3: stored file location
    CHECK(has(out, "Please enter a mark from 0 to 100"));
    CHECK(has(out, "late deduction 10.0 (1 day(s) late), final 70.0"));  // step 6
    CHECK(has(out, "Saved."));
    const auto mark = s.submissions.find(s.asgId, s.stuId)->getMark();
    REQUIRE(mark.has_value());
    CHECK(mark->finalMark == Approx(70));
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::MARKED);
    CHECK(s.submit.listReleasedResults(s.student(s.stuId)).empty());  // step 9, BR-04
}

TEST_CASE("ST-11 UC11 5a: empty feedback is refused and asked again", "[system][uc11]") {
    Sys s;
    s.submit.submit(s.student(s.stuId), s.asgId, s.makeFile("a.txt", 5));
    const auto out = s.asLecturer("3\n1\n1\n1\n55\n\n   \nGood effort\ny\n6\n");
    CHECK(has(out, "This cannot be empty."));
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::MARKED);
}

TEST_CASE("ST-12 UC11 step 7: not confirming leaves the submission unmarked", "[system][uc11]") {
    Sys s;
    s.submit.submit(s.student(s.stuId), s.asgId, s.makeFile("a.txt", 5));
    s.asLecturer("3\n1\n1\n1\n55\nOK\nn\n0\n6\n");
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::SUBMITTED);
}

TEST_CASE("ST-13 UC11 2a: the marking list shows only unmarked submissions", "[system][uc11]") {
    Sys s;
    const int stu3 = s.addStudent("stu3");
    s.submitAndMark(s.stuId, 50);
    s.submit.submit(s.student(stu3), s.asgId, s.makeFile("b.txt", 5));
    const auto out = s.asLecturer("3\n1\n1\n0\n6\n");
    CHECK(has(out, "Name stu3"));
    CHECK_FALSE(has(out, "Name stu1"));
}

TEST_CASE("ST-14 UC11 3a: correcting a MARKED submission needs a reason and is audited",
          "[system][uc11]") {
    Sys s;
    s.submitAndMark(s.stuId, 50);
    // 4 = Correct, course, assignment, submission, raw 58, keep feedback, empty reason refused
    const auto out = s.asLecturer("4\n1\n1\n1\n58\n\n\nMissed page 3\ny\n6\n");
    CHECK(has(out, "Corrected. Final mark is now 58.0"));
    AuditFilter all;
    all.departmentId = s.courses.findCourse(s.courseId)->getDepartmentId();
    const auto log = s.audit.query(all);
    REQUIRE(log.size() == 2);
    CHECK(log[1].reason == "Missed page 3");
}

TEST_CASE("ST-15 UC11 3b: a MODERATED mark cannot be corrected by the lecturer", "[system][uc11]") {
    Sys s;
    const int sub = s.submitAndMark(s.stuId, 50);
    s.moderation.approve(s.hod(s.hodId), {sub});
    const auto out = s.asLecturer("4\n1\n1\n6\n");
    CHECK(has(out, "There is nothing to show."));
    CHECK(s.submissions.findById(sub)->getMark()->rawMark == Approx(50));
}

// ---------------- UC12 Moderate and release marks ----------------

TEST_CASE("ST-16 UC12 main flow: approve all, release, the student sees the result",
          "[system][uc12]") {
    Sys s;
    s.submitAndMark(s.stuId, 64);
    // 1 = Moderate, assignment 1, approve all, y; 3 = Release, assignment 1, y; 6 = log out
    const auto out = s.asHod(s.hodId, "1\n1\n1\ny\n3\n1\ny\n6\n");
    CHECK(has(out, "marked 1, moderated 0, released 0, unmarked 0"));  // step 1
    CHECK(has(out, "Feedback " + std::to_string(s.stuId)));            // step 3
    CHECK(has(out, "1 moderated mark(s) will be released"));           // step 6
    CHECK(has(out, "1 mark(s) released."));                            // step 9
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::RELEASED);
    const auto results = s.asStudent(s.stuId, "3\n5\n");
    CHECK(has(results, "Mark: 64.0 / 100"));
}

TEST_CASE("ST-17 UC12 4: approve only some, with bad numbers asked again", "[system][uc12]") {
    Sys s;
    const int stu3 = s.addStudent("stu3");
    s.submitAndMark(s.stuId, 40);
    s.submitAndMark(stu3, 70);
    s.asHod(s.hodId, "1\n1\n2\n9\nabc\n2\ny\n6\n");
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::MARKED);
    CHECK(s.statusOf(stu3) == SubmissionStatus::MODERATED);
}

TEST_CASE("ST-18 UC12 4a: amend with a reason re-applies the penalty and is audited",
          "[system][uc12]") {
    Sys s;
    s.clock.set(s.deadline + hours(1));  // 1 day late
    s.submitAndMark(s.stuId, 60);        // final 50
    const auto out = s.asHod(s.hodId, "2\n1\n1\n70\nMarked too harshly\ny\n6\n");
    CHECK(has(out, "Final mark is now 60.0"));
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::MODERATED);
}

TEST_CASE("ST-19 UC12 5a and 6: stop after approving; preview counts the rest", "[system][uc12]") {
    Sys s;
    const int stu3 = s.addStudent("stu3");
    s.submitAndMark(s.stuId, 40);
    s.submit.submit(s.student(stu3), s.asgId, s.makeFile("b.txt", 5));  // not marked
    s.asHod(s.hodId, "1\n1\n1\ny\n6\n");                                // approve, then log out
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::MODERATED);
    const auto out = s.asHod(s.hodId, "3\n1\nn\n6\n");
    CHECK(has(out, "1 submission(s) are not moderated yet"));
}

TEST_CASE("ST-20 UC12 7a: cancelling the release changes nothing", "[system][uc12]") {
    Sys s;
    s.submitAndMark(s.stuId, 40);
    s.asHod(s.hodId, "1\n1\n1\ny\n3\n1\nn\n6\n");
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::MODERATED);
    CHECK(s.submit.listReleasedResults(s.student(s.stuId)).empty());
}

TEST_CASE("ST-21 UC12 9a: amending a released mark updates what the student sees",
          "[system][uc12]") {
    Sys s;
    s.submitAndMark(s.stuId, 40);
    s.asHod(s.hodId, "1\n1\n1\ny\n3\n1\ny\n6\n");
    const auto out = s.asHod(s.hodId, "2\n1\n1\n45\nAppeal upheld\ny\n6\n");
    CHECK(has(out, "already released"));
    CHECK(has(s.asStudent(s.stuId, "3\n5\n"), "Mark: 45.0 / 100"));
    const auto log = s.moderation.viewAudit(s.hod(s.hodId), AuditFilter{});
    REQUIRE(log.size() == 2);  // initial mark, then the amendment with both values
    CHECK(log[1].oldRawMark == Approx(40));
    CHECK(log[1].newRawMark == Approx(45));
}

TEST_CASE("ST-22 UC12 1a: nothing marked yet is reported plainly", "[system][uc12]") {
    Sys s;
    s.submit.submit(s.student(s.stuId), s.asgId, s.makeFile("a.txt", 5));
    const auto out = s.asHod(s.hodId, "1\n1\n6\n");
    CHECK(has(out, "There are no marked submissions waiting for moderation."));
}

TEST_CASE("ST-23 UC12 2a: another department's HoD cannot see the assignment (BR-07)",
          "[system][uc12]") {
    Sys s;
    s.submitAndMark(s.stuId, 40);
    const auto out = s.asHod(s.otherHodId, "1\n6\n");
    CHECK(has(out, "There is nothing to show."));
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::MARKED);
}

// ---------------- Login and robustness (NFR-07) ----------------

TEST_CASE("ST-24 UC01: a new account must change its temporary password at first login",
          "[system][uc01]") {
    Sys s;
    auto admin = s.users.findById(s.addUser(Role::ADMIN, "admin"));
    s.admin.createUser(*admin, Role::STUDENT, "thabo", "Thabo M", "TempPass1");
    std::istringstream in("thabo\nTempPass1\nshort\nNewPass99\nNewPass99\n");
    std::ostringstream out;
    ConsoleIO io(in, out);
    LoginScreen login(io, s.auth);
    const auto user = login.run();
    REQUIRE(user != nullptr);
    CHECK(has(out.str(), "temporary password"));
    CHECK(has(out.str(), "at least 8 characters"));
    CHECK_NOTHROW(s.auth.login("thabo", "NewPass99"));
}

TEST_CASE("ST-25 NFR-07: 100 invalid inputs cause zero crashes", "[system][nfr07]") {
    Sys s;
    const std::vector<std::string> junk = {"",
                                           "abc",
                                           "-1",
                                           "0",
                                           "99",
                                           "1.5",
                                           "99999999999999999999",
                                           "%s%n%x",
                                           "'; DROP TABLE users;--",
                                           std::string(5000, 'z')};
    std::string script;
    for (int i = 0; i < 100; ++i) script += junk[static_cast<std::size_t>(i) % junk.size()] + "\n";
    // The same 100 lines at the student main menu, then at the lecturer mark prompt.
    CHECK_NOTHROW(s.asStudent(s.stuId, script + "5\n"));
    s.submit.submit(s.student(s.stuId), s.asgId, s.makeFile("a.txt", 5));
    std::string markScript = "3\n1\n1\n1\n";
    for (int i = 0; i < 100; ++i) {
        const std::string& j = junk[static_cast<std::size_t>(i) % junk.size()];
        if (j == "0" || j == "99" || j == "1.5") continue;  // these are valid marks
        markScript += j + "\n";
    }
    markScript += "50\nFine\ny\n6\n";
    CHECK_NOTHROW(s.asLecturer(markScript));
    CHECK(s.statusOf(s.stuId) == SubmissionStatus::MARKED);
}

TEST_CASE("ST-26 NFR-07: the input ending mid-way ends the program cleanly", "[system][nfr07]") {
    Sys s;
    CHECK_THROWS_AS(s.asStudent(s.stuId, "2\n1\n"), InputClosed);  // main.cpp exits with 0
}

TEST_CASE("ST-27 UC03: pressing Enter on an empty username cancels account creation (D14)",
          "[system][uc03]") {
    Sys s;
    auto admin = s.users.findById(s.addUser(Role::ADMIN, "admin"));
    BackupService backup(s.db, s.files, s.clock, (s.dir / "backups").string());
    const auto before = s.admin.listUsers(*admin).size();
    std::istringstream in("1\n2\n1\n\n1\n2\n1\nnewstu\n\n7\n");
    std::ostringstream out;
    ConsoleIO io(in, out);
    AdminMenu menu(io, s.auth, s.admin, backup);
    menu.run(*admin);
    CHECK(s.admin.listUsers(*admin).size() == before);  // both attempts cancelled
}
