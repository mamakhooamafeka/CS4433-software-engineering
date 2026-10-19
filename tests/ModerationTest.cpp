// Integration tests for UC12-UC14 and UC08: moderate, amend, release, audit view, backup.
#include "FlowFixture.hpp"
#include "services/BackupService.hpp"
#include "services/ModerationService.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace lms;
using Catch::Approx;
using lms::test::Flow;
using std::chrono::hours;

namespace {

struct Moderation : Flow {
    ModerationService moderation{assignments, courses, submissions, users, audit,
                                 policy,      auditService, clock, db};
    int hodId, otherHodId, subId;

    Moderation() {
        hodId = addUser(Role::HOD, "hod1");
        otherHodId = addUser(Role::HOD, "hod2");
        courses.setHeadOfDepartment(courses.findCourse(courseId)->getDepartmentId(), hodId);
        courses.setHeadOfDepartment(courses.insertDepartment("Arts"), otherHodId);
        submit.submit(student(stuId), asgId, makeFile("a.txt", 5));
        subId = submissions.find(asgId, stuId)->getId();
        marking.mark(lecturer(), subId, 70, "Good analysis");
    }
    HeadOfDepartment& hod(int id) {
        hodHolder = users.findById(id);
        return dynamic_cast<HeadOfDepartment&>(*hodHolder);
    }
    std::unique_ptr<User> hodHolder;
};

}  // namespace

TEST_CASE("An HoD sees only their own department's marked work (FR-15, BR-07)",
          "[integration][moderation]") {
    Moderation m;
    CHECK(m.moderation.listMarked(m.hod(m.hodId), m.asgId).size() == 1);
    CHECK_THROWS_AS(m.moderation.listMarked(m.hod(m.otherHodId), m.asgId), AccessDeniedError);
    CHECK(m.moderation.listDepartmentAssignments(m.hod(m.otherHodId)).empty());
}

TEST_CASE("Approve, preview and release, then the student sees the result (FR-17, FR-18)",
          "[integration][moderation]") {
    Moderation m;
    m.moderation.approve(m.hod(m.hodId), {m.subId});
    const auto preview = m.moderation.releasePreview(m.hod(m.hodId), m.asgId);
    CHECK(preview.toRelease == 1);
    CHECK(preview.notReady == 0);
    CHECK(m.moderation.releaseAll(m.hod(m.hodId), m.asgId) == 1);

    const auto results = m.submit.listReleasedResults(m.student(m.stuId));
    REQUIRE(results.size() == 1);
    CHECK(results[0].mark.finalMark == Approx(70));
    CHECK(results[0].mark.feedback == "Good analysis");
}

TEST_CASE("Release with nothing moderated is refused", "[integration][moderation]") {
    Moderation m;
    CHECK_THROWS_AS(m.moderation.releaseAll(m.hod(m.hodId), m.asgId), BusinessRuleError);
}

TEST_CASE("Release is all-or-nothing (FR-17)", "[integration][moderation]") {
    Moderation m;
    m.moderation.approve(m.hod(m.hodId), {m.subId});
    // Approving a list that contains a bad id rolls the whole approval back.
    const int stu3 = m.addUser(Role::STUDENT, "stu3");
    m.courses.enrol(m.courseId, stu3, m.clock.now());
    m.submit.submit(m.student(stu3), m.asgId, m.makeFile("b.txt", 5));
    const int sub3 = m.submissions.find(m.asgId, stu3)->getId();
    m.marking.mark(m.lecturer(), sub3, 40, "Fair");
    CHECK_THROWS(m.moderation.approve(m.hod(m.hodId), {sub3, 99999}));
    CHECK(m.submissions.findById(sub3)->getStatus() == SubmissionStatus::MARKED);
}

TEST_CASE("Amend needs a reason, re-applies BR-01 and is audited (FR-16, FR-19)",
          "[integration][moderation]") {
    Moderation m;
    CHECK_THROWS_AS(m.moderation.amend(m.hod(m.hodId), m.subId, 75, " "), ValidationError);
    const Mark amended = m.moderation.amend(m.hod(m.hodId), m.subId, 75, "Marked too harshly");
    CHECK(amended.finalMark == Approx(75));
    CHECK(amended.feedback == "Good analysis");
    CHECK(m.submissions.findById(m.subId)->getStatus() == SubmissionStatus::MODERATED);

    m.moderation.releaseAll(m.hod(m.hodId), m.asgId);
    m.moderation.amend(m.hod(m.hodId), m.subId, 72, "Appeal upheld");   // after release (A6)
    CHECK(m.submissions.findById(m.subId)->getStatus() == SubmissionStatus::RELEASED);

    const auto log = m.moderation.viewAudit(m.hod(m.hodId), AuditFilter{});
    REQUIRE(log.size() == 3);   // initial, amendment, amendment
    CHECK(log[2].changeType == ChangeType::AMENDMENT);
    CHECK(log[2].oldRawMark == Approx(75));
    CHECK(log[2].newRawMark == Approx(72));
    CHECK(log[2].changedBy == m.hodId);
}

TEST_CASE("A lecturer cannot change a mark once it is moderated (BR-07)",
          "[integration][moderation]") {
    Moderation m;
    m.moderation.approve(m.hod(m.hodId), {m.subId});
    CHECK_THROWS_AS(m.marking.correct(m.lecturer(), m.subId, 90, "x", "y"), BusinessRuleError);
}

TEST_CASE("An HoD's audit view never shows another department (FR-20)",
          "[integration][moderation]") {
    Moderation m;
    AuditFilter tryOther;
    tryOther.departmentId = 12345;   // ignored: replaced by the HoD's own department
    CHECK(m.moderation.viewAudit(m.hod(m.otherHodId), tryOther).empty());
    CHECK(m.moderation.viewAudit(m.hod(m.hodId), tryOther).size() == 1);
}

TEST_CASE("Backup once per day, and restore brings the old data back (FR-21)",
          "[integration][backup]") {
    Moderation m;
    BackupService backup(m.db, m.files, m.clock, (m.dir / "backups").string());
    CHECK(backup.backupIfFirstStartToday());
    CHECK_FALSE(backup.backupIfFirstStartToday());   // same day: no second backup
    const auto adminId = m.addUser(Role::ADMIN, "admin");
    const auto admin = m.user(adminId);
    const auto names = backup.listBackups(*admin);
    REQUIRE(names.size() == 1);

    m.moderation.approve(m.hod(m.hodId), {m.subId});   // a change after the backup
    backup.restore(*admin, names[0]);
    CHECK(m.submissions.findById(m.subId)->getStatus() == SubmissionStatus::MARKED);
    CHECK_THROWS_AS(backup.listBackups(*m.user(m.lecId)), AccessDeniedError);
}
