// Integration tests: the real SQLite repositories against an in-memory database
// created from the real schema.
#include "data/SqliteRepositories.hpp"
#include "domain/Roles.hpp"
#include "domain/UserFactory.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace lms;

namespace {

struct Fixture {
    SqliteDatabase db{":memory:"};
    SqliteUserRepository users{db};
    SqliteCourseRepository courses{db};
    SqliteAssignmentRepository assignments{db};
    SqliteSubmissionRepository submissions{db};
    SqliteAuditRepository audit{db};
    DateTime t0 = fromIso8601("2026-11-01 08:00");

    int addUser(Role role, const std::string& username) {
        return users.insert(*UserFactory::createNew(role, username, "Name " + username, "hash"));
    }
};

SubmissionVersion version(DateTime t) {
    SubmissionVersion v;
    v.storedPath = "submissions/a" + toIso8601(t);
    v.originalName = "work.pdf";
    v.sizeBytes = 1234;
    v.sha256 = std::string(64, 'b');
    v.submittedAt = t;
    return v;
}

}  // namespace

TEST_CASE("A new database gets the schema and passes the integrity check", "[integration][db]") {
    SqliteDatabase db(":memory:");
    CHECK(db.integrityCheck());
}

TEST_CASE("Users round-trip with their role and scope", "[integration][db]") {
    Fixture f;
    const int hod = f.addUser(Role::HOD, "hod1");
    const int lec = f.addUser(Role::LECTURER, "lec1");
    const int stu = f.addUser(Role::STUDENT, "stu1");
    const int dept = f.courses.insertDepartment("Sciences");
    f.courses.setHeadOfDepartment(dept, hod);
    const int course = f.courses.insertCourse("PHY101", "Physics", dept);
    f.courses.addLecturer(course, lec);
    f.courses.enrol(course, stu, f.t0);

    auto student = f.users.findByUsername("stu1");
    REQUIRE(student);
    CHECK(student->role() == Role::STUDENT);
    CHECK(student->canAccessCourse(*f.courses.findCourse(course)));
    auto hodUser = f.users.findById(hod);
    REQUIRE(hodUser);
    CHECK(dynamic_cast<HeadOfDepartment&>(*hodUser).getDepartmentId() == dept);

    f.courses.withdraw(course, stu);
    CHECK_FALSE(f.users.findById(stu)->canAccessCourse(*f.courses.findCourse(course)));
}

TEST_CASE("Usernames are unique and case-insensitive", "[integration][db]") {
    Fixture f;
    f.addUser(Role::STUDENT, "Thabo");
    CHECK_THROWS_AS(f.addUser(Role::STUDENT, "thabo"), ValidationError);
}

TEST_CASE("Failed logins and lockout are saved", "[integration][db]") {
    Fixture f;
    f.addUser(Role::STUDENT, "stu1");
    auto u = f.users.findByUsername("stu1");
    for (int i = 0; i < 5; ++i) u->registerFailedLogin(f.t0);
    f.users.update(*u);
    CHECK(f.users.findByUsername("stu1")->isLocked(f.t0 + std::chrono::minutes(1)));
}

TEST_CASE("A submission, its versions and its mark are saved and reloaded", "[integration][db]") {
    Fixture f;
    const int lec = f.addUser(Role::LECTURER, "lec1");
    const int stu = f.addUser(Role::STUDENT, "stu1");
    const int dept = f.courses.insertDepartment("Sciences");
    const int course = f.courses.insertCourse("PHY101", "Physics", dept);
    const DateTime deadline = f.t0 + std::chrono::hours(48);
    const int asg = f.assignments.insert(
        Assignment(0, course, "Lab report", "Write it up", deadline, 50, lec, f.t0));

    Submission s(asg, stu);
    s.addVersion(version(f.t0), deadline);
    f.submissions.save(s);
    REQUIRE(s.getId() > 0);
    s.addVersion(version(f.t0 + std::chrono::hours(1)), deadline);
    f.submissions.save(s);

    auto loaded = f.submissions.find(asg, stu);
    REQUIRE(loaded);
    CHECK(loaded->getVersions().size() == 2);
    CHECK(loaded->getStatus() == SubmissionStatus::SUBMITTED);

    Mark m;
    m.rawMark = 40;
    m.deduction = 0;
    m.finalMark = 40;
    m.feedback = "Clear method";
    m.markedBy = lec;
    m.markedAt = f.t0 + std::chrono::hours(72);
    loaded->recordMark(m);
    f.submissions.save(*loaded);
    auto marked = f.submissions.findById(s.getId());
    REQUIRE(marked->getMark());
    CHECK(marked->getMark()->finalMark == 40);
    CHECK(f.submissions.byAssignmentAndStatus(asg, SubmissionStatus::MARKED).size() == 1);
    CHECK_FALSE(f.submissions.hasReleased(asg));
}

TEST_CASE("The database refuses a mark that breaks final = raw - deduction", "[integration][db]") {
    Fixture f;
    const int lec = f.addUser(Role::LECTURER, "lec1");
    const int stu = f.addUser(Role::STUDENT, "stu1");
    const int course = f.courses.insertCourse("PHY101", "Physics", f.courses.insertDepartment("S"));
    const int asg = f.assignments.insert(Assignment(0, course, "T", "I", f.t0, 50, lec, f.t0));
    Submission s(asg, stu);
    s.addVersion(version(f.t0), f.t0);
    Mark bad;
    bad.rawMark = 40;
    bad.deduction = 5;
    bad.finalMark = 40;  // should be 35
    bad.feedback = "x";
    bad.markedBy = lec;
    bad.markedAt = f.t0;
    s.recordMark(bad);
    CHECK_THROWS_AS(f.submissions.save(s), DataAccessError);
}

TEST_CASE("The audit log is append-only and filterable", "[integration][db]") {
    Fixture f;
    const int lec = f.addUser(Role::LECTURER, "lec1");
    const int stu = f.addUser(Role::STUDENT, "stu1");
    const int dept = f.courses.insertDepartment("Sciences");
    const int course = f.courses.insertCourse("PHY101", "Physics", dept);
    const int asg = f.assignments.insert(Assignment(0, course, "T", "I", f.t0, 50, lec, f.t0));
    Submission s(asg, stu);
    s.addVersion(version(f.t0), f.t0);
    f.submissions.save(s);

    AuditEntry e;
    e.submissionId = s.getId();
    e.changedBy = lec;
    e.changedAt = f.t0;
    e.changeType = ChangeType::INITIAL;
    e.newRawMark = 30;
    e.newFinalMark = 30;
    e.reason = "Initial mark";
    f.audit.append(e);

    AuditFilter filter;
    filter.departmentId = dept;
    CHECK(f.audit.query(filter).size() == 1);
    filter.studentId = stu + 100;
    CHECK(f.audit.query(filter).empty());

    // Even direct SQL cannot change or delete an audit row.
    CHECK_THROWS(f.db.prepare("update mark_audit set reason = 'x'").run());
    CHECK_THROWS(f.db.prepare("delete from mark_audit").run());
}

TEST_CASE("A transaction that is not committed is rolled back", "[integration][db]") {
    Fixture f;
    {
        Transaction tx(f.db);
        f.addUser(Role::STUDENT, "temp");
    }
    CHECK(f.users.findByUsername("temp") == nullptr);
    {
        Transaction tx(f.db);
        f.addUser(Role::STUDENT, "kept");
        tx.commit();
    }
    CHECK(f.users.findByUsername("kept") != nullptr);
}

TEST_CASE("SQL injection text is stored as plain data (NFR-09)", "[integration][db]") {
    Fixture f;
    const std::string evil = "x'; drop table user_account; --";
    CHECK(f.users.findByUsername(evil) == nullptr);
    f.addUser(Role::STUDENT, evil);
    auto u = f.users.findByUsername(evil);
    REQUIRE(u != nullptr);  // the table still exists and the text is just a name
    CHECK(u->getUsername() == evil);
}
