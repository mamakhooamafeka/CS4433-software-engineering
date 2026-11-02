// Phase 8 regression tests for change request CR1 (release v1.1.0, FR-24): a lecturer can
// extend the deadline for the whole class, and students see extensions in their list.
// The test IDs match docs/CR1_IMPACT_ANALYSIS.md section 6.
#include "FlowFixture.hpp"
#include "SchemaSql.hpp"
#include "presentation/RoleMenus.hpp"
#include "services/ModerationService.hpp"

#include <sqlite3.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>

using namespace lms;
using Catch::Approx;
using std::chrono::hours;
using std::chrono::seconds;
namespace fs = std::filesystem;

namespace {

bool has(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

// The Phase 5 fixture plus a HoD and the menus, as in SystemTest.cpp.
struct Cr1 : test::Flow {
    AuthService auth{users, courses, assignments, submissions, hasher, clock};
    ModerationService moderation{assignments, courses,      submissions, users, audit,
                                 policy,      auditService, clock,       db};
    int hodId;
    std::unique_ptr<User> hodHolder;

    Cr1() {
        hodId = addUser(Role::HOD, "hod1");
        courses.setHeadOfDepartment(courses.findCourse(courseId)->getDepartmentId(), hodId);
    }
    HeadOfDepartment& hod() {
        hodHolder = users.findById(hodId);
        return dynamic_cast<HeadOfDepartment&>(*hodHolder);
    }
    int enrolNew(const std::string& name) {
        const int id = addUser(Role::STUDENT, name);
        courses.enrol(courseId, id, clock.now());
        return id;
    }
    ClassExtensionResult extendClass(DateTime to, const std::string& reason = "Exam week") {
        return assignmentsSvc.grantClassExtension(lecturer(), asgId, to, reason);
    }
    DateTime deadlineOf(int stu) {
        return DeadlineCalculator(assignments).effectiveDeadline(*assignments.findById(asgId), stu);
    }
    SubmissionStatus statusOf(int stu) { return submissions.find(asgId, stu)->getStatus(); }
    int submitFile(int stu) {
        submit.submit(student(stu), asgId, makeFile("w" + std::to_string(stu) + ".txt", 5));
        return submissions.find(asgId, stu)->getId();
    }
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
};

// A fake repository, so DeadlineCalculator is tested without a database.
class FakeAssignments : public IAssignmentRepository {
public:
    std::vector<Extension> own;
    std::vector<ClassExtension> cls;
    std::optional<Assignment> findById(int) override { return std::nullopt; }
    std::vector<Assignment> byCourse(int) override { return {}; }
    int insert(const Assignment&) override { return 0; }
    std::vector<Extension> extensionsFor(int, int) override { return own; }
    void insertExtension(const Extension&) override {}
    std::vector<ClassExtension> classExtensionsFor(int) override { return cls; }
    void insertClassExtension(const ClassExtension&) override {}
};

ClassExtension classExt(DateTime to, const std::string& reason) {
    ClassExtension e;
    e.newDeadline = to;
    e.reason = reason;
    return e;
}

Extension ownExt(DateTime to) {
    Extension e;
    e.newDeadline = to;
    return e;
}

// Writes a database exactly as release v1.0.0 made it: the v1.0 tables, user_version 0.
void makeV10Database(const std::string& path) {
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    REQUIRE(sqlite3_exec(db, data::kSchemaSql, nullptr, nullptr, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_exec(db, "insert into department (name) values ('Sciences');", nullptr, nullptr,
                         nullptr) == SQLITE_OK);
    sqlite3_close(db);
}

int count(SqliteDatabase& db, const std::string& sql) {
    Statement s = db.prepare(sql);
    s.step();
    return s.getInt(0);
}

}  // namespace

// ---------------- Unit: DeadlineCalculator ----------------

TEST_CASE("CR1-01 the latest of the class and own extensions wins, and says which", "[unit][cr1]") {
    FakeAssignments repo;
    const DateTime due = fromIso8601("2026-11-10 12:00");
    const Assignment a(1, 1, "Essay", "Write", due, 100, 1, fromIso8601("2026-11-01 08:00"));
    DeadlineCalculator calc(repo);

    CHECK(calc.deadlineFor(a, 7).scope == ExtensionScope::NONE);
    CHECK(calc.deadlineFor(a, 7).deadline == due);

    repo.cls = {classExt(due + hours(24), "Exam week"), classExt(due + hours(48), "Power cut")};
    const EffectiveDeadline c = calc.deadlineFor(a, 7);
    CHECK(c.scope == ExtensionScope::CLASS);
    CHECK(c.deadline == due + hours(48));
    CHECK(c.classReason == "Power cut");
    CHECK(calc.classDeadline(a) == due + hours(48));

    repo.own = {ownExt(due + hours(24))};  // earlier than the class deadline: no effect
    CHECK(calc.deadlineFor(a, 7).scope == ExtensionScope::CLASS);
    CHECK(calc.deadlineFor(a, 7).deadline == due + hours(48));

    repo.own = {ownExt(due + hours(72))};  // later: the student's own deadline wins
    CHECK(calc.deadlineFor(a, 7).scope == ExtensionScope::STUDENT);
    CHECK(calc.effectiveDeadline(a, 7) == due + hours(72));
    CHECK(calc.classDeadline(a) == due + hours(48));  // the class deadline is unchanged
}

// ---------------- Integration: AssignmentService ----------------

TEST_CASE("CR1-02 a class extension reaches every student, also one enrolled later",
          "[integration][cr1]") {
    Cr1 f;
    const int stu3 = f.enrolNew("stu3");
    const auto r = f.extendClass(f.deadline + hours(24));
    CHECK(r.studentsInClass == 2);
    const int stu4 = f.enrolNew("stu4");  // enrolled after the extension
    CHECK(f.deadlineOf(f.stuId) == f.deadline + hours(24));
    CHECK(f.deadlineOf(stu3) == f.deadline + hours(24));
    CHECK(f.deadlineOf(stu4) == f.deadline + hours(24));
    // stu2 is not enrolled, so the class extension gives stu2 nothing to submit to (FR-02).
    CHECK_THROWS_AS(f.submit.submit(f.student(f.otherStuId), f.asgId, f.makeFile("x.txt", 5)),
                    AccessDeniedError);
}

TEST_CASE("CR1-03 a class extension needs a reason and only moves the deadline later (BR-02)",
          "[integration][cr1][bva]") {
    Cr1 f;
    CHECK_THROWS_AS(f.extendClass(f.deadline + hours(24), "   "), ValidationError);
    CHECK_THROWS_AS(f.extendClass(f.deadline - seconds(1)), BusinessRuleError);
    CHECK_THROWS_AS(f.extendClass(f.deadline), BusinessRuleError);
    CHECK_NOTHROW(f.extendClass(f.deadline + seconds(1)));
    // The new class deadline is the limit for the next class extension.
    CHECK_THROWS_AS(f.extendClass(f.deadline + seconds(1)), BusinessRuleError);
    CHECK_NOTHROW(f.extendClass(f.deadline + hours(24)));
    CHECK(f.assignments.classExtensionsFor(f.asgId).size() == 2);
}

TEST_CASE("CR1-04 LATE work handed in before the new class deadline becomes on time (DC-01)",
          "[integration][cr1]") {
    Cr1 f;
    const int stu3 = f.enrolNew("stu3");
    f.clock.set(f.deadline + hours(2));
    f.submitFile(f.stuId);  // 2 hours late
    f.clock.set(f.deadline + hours(30));
    f.submitFile(stu3);  // 30 hours late
    REQUIRE(f.statusOf(f.stuId) == SubmissionStatus::LATE);
    REQUIRE(f.statusOf(stu3) == SubmissionStatus::LATE);

    const auto r = f.extendClass(f.deadline + hours(24));
    CHECK(r.nowOnTime == 1);
    CHECK(f.statusOf(f.stuId) == SubmissionStatus::SUBMITTED);
    CHECK(f.statusOf(stu3) == SubmissionStatus::LATE);  // still after the new deadline

    // Marking uses the new deadline: stu3 is 6 hours late, so 1 started day.
    const int sub3 = f.submissions.find(f.asgId, stu3)->getId();
    CHECK(f.marking.mark(f.lecturer(), sub3, 80, "Good").daysLate == 1);
}

TEST_CASE("CR1-05 a student's own longer extension is kept after a class extension",
          "[integration][cr1]") {
    Cr1 f;
    f.assignmentsSvc.grantExtension(f.lecturer(), f.asgId, f.stuId, f.deadline + hours(72),
                                    "Medical note");
    f.extendClass(f.deadline + hours(24));
    CHECK(f.deadlineOf(f.stuId) == f.deadline + hours(72));
    // An own extension after a class extension must beat the class deadline (BR-02).
    const int stu3 = f.enrolNew("stu3");
    CHECK_THROWS_AS(
        f.assignmentsSvc.grantExtension(f.lecturer(), f.asgId, stu3, f.deadline + hours(12), "Ill"),
        BusinessRuleError);
    CHECK_NOTHROW(f.assignmentsSvc.grantExtension(f.lecturer(), f.asgId, stu3,
                                                  f.deadline + hours(36), "Ill"));
}

TEST_CASE("CR1-06 marked work keeps its mark; the lecturer is told and can correct it",
          "[integration][cr1]") {
    Cr1 f;
    f.clock.set(f.deadline + hours(2));
    const int sub = f.submitFile(f.stuId);
    const Mark before = f.marking.mark(f.lecturer(), sub, 80, "Good");
    REQUIRE(before.deduction == Approx(10));

    const auto r = f.extendClass(f.deadline + hours(24));
    CHECK(r.nowOnTime == 0);
    CHECK(r.markedAndLate == 1);
    CHECK(f.statusOf(f.stuId) == SubmissionStatus::MARKED);
    CHECK(f.submissions.findById(sub)->getMark()->finalMark == Approx(70));  // unchanged

    // UC20: a correction works the penalty out again with the new deadline.
    const Mark after = f.marking.correct(f.lecturer(), sub, 80, "Good", "Class extension");
    CHECK(after.daysLate == 0);
    CHECK(after.finalMark == Approx(80));
}

TEST_CASE("CR1-07 no class extension after release, or by a lecturer of another course",
          "[integration][cr1]") {
    Cr1 f;
    const int sub = f.submitFile(f.stuId);
    f.marking.mark(f.lecturer(), sub, 70, "Good");
    const int otherLec = f.addUser(Role::LECTURER, "lec2");
    auto other = f.users.findById(otherLec);
    CHECK_THROWS_AS(f.assignmentsSvc.grantClassExtension(dynamic_cast<Lecturer&>(*other), f.asgId,
                                                         f.deadline + hours(24), "Why"),
                    AccessDeniedError);
    f.moderation.approve(f.hod(), {sub});
    f.moderation.releaseAll(f.hod(), f.asgId);
    CHECK_THROWS_AS(f.extendClass(f.deadline + hours(24)), BusinessRuleError);
    CHECK(f.assignments.classExtensionsFor(f.asgId).empty());
}

// ---------------- Data: upgrading a v1.0.0 database ----------------

TEST_CASE("CR1-08 a v1.0.0 database is upgraded in place and keeps its data", "[data][cr1]") {
    const fs::path file = fs::temp_directory_path() / "lms_cr1_upgrade.db";
    fs::remove(file);
    makeV10Database(file.string());
    {
        SqliteDatabase db(file.string());
        CHECK(count(db, "pragma user_version") == 1);
        CHECK(count(db, "select count(*) from sqlite_master where name = 'class_extension'") == 1);
        CHECK(count(db, "select count(*) from department") == 1);
    }
    {
        SqliteDatabase again(file.string());  // opening twice must not fail or repeat it
        CHECK(count(again, "pragma user_version") == 1);
    }
    fs::remove(file);
}

TEST_CASE("CR1-09 restoring a backup made by v1.0.0 upgrades it", "[data][cr1]") {
    const fs::path file = fs::temp_directory_path() / "lms_cr1_restore.db";
    fs::remove(file);
    makeV10Database(file.string());
    SqliteDatabase db(":memory:");
    db.restoreFrom(file.string());
    CHECK(count(db, "pragma user_version") == 1);
    CHECK(count(db, "select count(*) from sqlite_master where name = 'class_extension'") == 1);
    CHECK(count(db, "select count(*) from department") == 1);
    fs::remove(file);
}

// ---------------- System: through the menus ----------------
// Lecturer menu: 1 create, 2 extension, 3 mark, 4 correct, 5 password, 6 log out.
// Student menu: 1 view, 2 submit, 3 results, 4 change password, 5 log out.

TEST_CASE("CR1-10 the lecturer extends for the whole class through the menu", "[system][cr1]") {
    Cr1 f;
    f.enrolNew("stu3");
    const auto out = f.asLecturer("2\n1\n1\n2\n2026-11-05 12:00\nPower cut on Monday\ny\n6\n");
    CHECK(has(out, "Who gets the extension?"));
    CHECK(has(out, "The deadline for the whole class is now 2026-11-03 08:00 UTC."));
    CHECK(has(out, "The extension has been granted to all 2 student(s) of PHY101."));
    CHECK(f.deadlineOf(f.stuId) == fromIso8601("2026-11-05 12:00"));
}

TEST_CASE("CR1-11 cancelling or an earlier date stores nothing", "[system][cr1]") {
    Cr1 f;
    f.asLecturer("2\n1\n1\n2\n2026-11-05 12:00\nPower cut\nn\n6\n");
    CHECK(f.assignments.classExtensionsFor(f.asgId).empty());
    const auto out = f.asLecturer("2\n1\n1\n2\n2026-11-02 12:00\nPower cut\ny\n6\n");
    CHECK(has(out, "An extension can only move the deadline later (BR-02)."));
    CHECK(f.assignments.classExtensionsFor(f.asgId).empty());
}

TEST_CASE("CR1-12 the student's list shows a class extension with its reason", "[system][cr1]") {
    Cr1 f;
    f.extendClass(fromIso8601("2026-11-05 12:00"), "Power cut on Monday");
    const auto out = f.asStudent(f.stuId, "1\n1\n5\n");
    CHECK(has(out, "PHY101 Lab report | due 2026-11-05 12:00 (ext. class) | not submitted"));
    CHECK(has(out, "ext. class = deadline extended for the whole class"));
    CHECK(has(out, "Extended for the whole class from 2026-11-03 08:00 UTC."));
    CHECK(has(out, "Reason: Power cut on Monday"));
    // The submit list shows it too.
    CHECK(has(f.asStudent(f.stuId, "2\n0\n5\n"), "(ext. class)"));
}

TEST_CASE("CR1-13 an own extension shows as 'ext. you' and its reason stays private",
          "[system][cr1]") {
    Cr1 f;
    f.extendClass(fromIso8601("2026-11-05 12:00"), "Power cut");
    f.assignmentsSvc.grantExtension(f.lecturer(), f.asgId, f.stuId, fromIso8601("2026-11-07 12:00"),
                                    "Medical note");
    const auto out = f.asStudent(f.stuId, "1\n1\n5\n");
    CHECK(has(out, "due 2026-11-07 12:00 (ext. you)"));
    CHECK(has(out, "Extended for you only, from 2026-11-03 08:00 UTC."));
    CHECK_FALSE(has(out, "Medical note"));
    // A classmate sees only the class extension.
    const int stu3 = f.enrolNew("stu3");
    const auto other = f.asStudent(stu3, "1\n0\n5\n");
    CHECK(has(other, "due 2026-11-05 12:00 (ext. class)"));
    CHECK_FALSE(has(other, "ext. you)"));
}

TEST_CASE("CR1-14 a list with no extension looks exactly as in v1.0.0", "[system][cr1]") {
    Cr1 f;
    const auto out = f.asStudent(f.stuId, "1\n0\n5\n");
    CHECK(has(out, "PHY101 Lab report | due 2026-11-03 08:00 | not submitted"));
    CHECK_FALSE(has(out, "ext."));
}
