// Integration tests for UC09-UC11, UC16-UC17: create, extend, submit, resubmit, mark.
#include "TestSupport.hpp"
#include "infrastructure/LocalFileStore.hpp"
#include "services/AssignmentService.hpp"
#include "services/MarkingService.hpp"
#include "services/SubmissionService.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

using namespace lms;
using Catch::Approx;
using std::chrono::hours;
using std::chrono::seconds;
namespace fs = std::filesystem;

namespace {

struct Flow : test::World {
    fs::path dir = fs::temp_directory_path() /
                   ("lms_test_" + std::to_string(std::chrono::steady_clock::now()
                                                     .time_since_epoch()
                                                     .count()));
    LocalFileStore files{dir / "store"};
    TenPercentPerDayPolicy policy;
    AuditService auditService{audit, clock};
    AssignmentService assignmentsSvc{assignments, courses, submissions, clock, db};
    SubmissionService submit{assignments, courses, submissions, files, clock, db};
    MarkingService marking{assignments, courses, submissions, users,
                           policy,      auditService, clock, db};
    int lecId, stuId, otherStuId, courseId, asgId;
    DateTime deadline;

    Flow() {
        lecId = addUser(Role::LECTURER, "lec1");
        stuId = addUser(Role::STUDENT, "stu1");
        otherStuId = addUser(Role::STUDENT, "stu2");
        const int dept = courses.insertDepartment("Sciences");
        courseId = courses.insertCourse("PHY101", "Physics", dept);
        courses.addLecturer(courseId, lecId);
        courses.enrol(courseId, stuId, clock.now());
        deadline = clock.now() + hours(48);
        asgId = assignmentsSvc.createAssignment(lecturer(), courseId, "Lab report", "Write it",
                                                deadline, 100);
    }
    ~Flow() {
        std::error_code ec;
        fs::remove_all(dir, ec);
    }
    Lecturer& lecturer() {
        lecHolder = users.findById(lecId);
        return dynamic_cast<Lecturer&>(*lecHolder);
    }
    Student& student(int id) {
        stuHolder = users.findById(id);
        return dynamic_cast<Student&>(*stuHolder);
    }
    std::string makeFile(const std::string& name, std::size_t bytes) {
        fs::create_directories(dir);
        const fs::path p = dir / name;
        std::ofstream(p, std::ios::binary) << std::string(bytes, 'x');
        return p.string();
    }
    std::unique_ptr<User> lecHolder, stuHolder;
};

}  // namespace

TEST_CASE("On-time submission is stored with size and SHA-256 (FR-11)", "[integration][submit]") {
    Flow f;
    const auto receipt = f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 10));
    CHECK(receipt.status == SubmissionStatus::SUBMITTED);
    CHECK(receipt.sizeBytes == 10);
    CHECK(receipt.sha256.size() == 64);
    CHECK(fs::exists(f.submissions.find(f.asgId, f.stuId)->latestVersion().storedPath));
}

TEST_CASE("Late submission is accepted and marked LATE (C2)", "[integration][submit]") {
    Flow f;
    f.clock.set(f.deadline + seconds(1));
    const auto preview = f.submit.preview(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 10));
    CHECK(preview.daysLate == 1);
    CHECK(f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 10)).status ==
          SubmissionStatus::LATE);
}

TEST_CASE("Files are validated: missing, empty and over 10 MB are refused",
          "[integration][submit]") {
    Flow f;
    auto& s = f.student(f.stuId);
    CHECK_THROWS_AS(f.submit.submit(s, f.asgId, (f.dir / "nope.txt").string()), ValidationError);
    CHECK_THROWS_AS(f.submit.submit(s, f.asgId, f.makeFile("empty.txt", 0)), ValidationError);
    CHECK_THROWS_AS(f.submit.submit(s, f.asgId, f.makeFile("big.bin", 10 * 1024 * 1024 + 1)),
                    ValidationError);
    CHECK_NOTHROW(f.submit.submit(s, f.asgId, f.makeFile("max.bin", 10 * 1024 * 1024)));
}

TEST_CASE("A student not enrolled in the course cannot submit (BR-07)", "[integration][submit]") {
    Flow f;
    CHECK_THROWS_AS(f.submit.submit(f.student(f.otherStuId), f.asgId, f.makeFile("a.txt", 5)),
                    AccessDeniedError);
}

TEST_CASE("Resubmission keeps every version until the deadline (FR-12, BR-03)",
          "[integration][submit]") {
    Flow f;
    f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("v1.txt", 5));
    f.clock.advance(hours(1));
    const auto r2 = f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("v2.txt", 6));
    CHECK(r2.versionNo == 2);
    const auto sub = f.submissions.find(f.asgId, f.stuId);
    REQUIRE(sub->getVersions().size() == 2);
    CHECK(fs::exists(sub->getVersions()[0].storedPath));   // the old file is still there

    f.clock.set(f.deadline + seconds(1));
    CHECK_THROWS_AS(f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("v3.txt", 7)),
                    BusinessRuleError);
}

TEST_CASE("Marking applies the BR-01 penalty and writes an audit row", "[integration][mark]") {
    Flow f;
    f.clock.set(f.deadline + hours(25));   // 2 days late
    f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 5));
    const int subId = f.submissions.find(f.asgId, f.stuId)->getId();
    const Mark m = f.marking.mark(f.lecturer(), subId, 80, "Well structured");
    CHECK(m.daysLate == 2);
    CHECK(m.finalMark == Approx(60));
    CHECK(m.deduction == Approx(20));
    AuditFilter filter;
    filter.departmentId = f.courses.findCourse(f.courseId)->getDepartmentId();
    const auto rows = f.audit.query(filter);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].changeType == ChangeType::INITIAL);
    CHECK(rows[0].changedBy == f.lecId);
}

TEST_CASE("Marks outside 0..M and empty feedback are refused (FR-13)", "[integration][mark]") {
    Flow f;
    f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 5));
    const int subId = f.submissions.find(f.asgId, f.stuId)->getId();
    CHECK_THROWS_AS(f.marking.mark(f.lecturer(), subId, -1, "x"), ValidationError);
    CHECK_THROWS_AS(f.marking.mark(f.lecturer(), subId, 100.1, "x"), ValidationError);
    CHECK_THROWS_AS(f.marking.mark(f.lecturer(), subId, 50, "   "), ValidationError);
    CHECK_NOTHROW(f.marking.mark(f.lecturer(), subId, 100, "Perfect"));
}

TEST_CASE("A correction needs a reason and is audited with old and new values",
          "[integration][mark]") {
    Flow f;
    f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 5));
    const int subId = f.submissions.find(f.asgId, f.stuId)->getId();
    f.marking.mark(f.lecturer(), subId, 50, "OK");
    CHECK_THROWS_AS(f.marking.correct(f.lecturer(), subId, 55, "OK", ""), ValidationError);
    f.marking.correct(f.lecturer(), subId, 55, "Better", "Missed a page");
    AuditFilter filter;
    filter.departmentId = f.courses.findCourse(f.courseId)->getDepartmentId();
    const auto rows = f.audit.query(filter);
    REQUIRE(rows.size() == 2);
    CHECK(rows[1].changeType == ChangeType::CORRECTION);
    CHECK(rows[1].oldRawMark == Approx(50));
    CHECK(rows[1].newRawMark == Approx(55));
    CHECK(rows[1].reason == "Missed a page");
}

TEST_CASE("An extension needs a reason, moves the deadline later and fixes LATE (DC-01)",
          "[integration][extension]") {
    Flow f;
    f.clock.set(f.deadline + hours(2));
    f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 5));
    REQUIRE(f.submissions.find(f.asgId, f.stuId)->getStatus() == SubmissionStatus::LATE);

    CHECK_THROWS_AS(f.assignmentsSvc.grantExtension(f.lecturer(), f.asgId, f.stuId,
                                                    f.deadline + hours(24), ""),
                    ValidationError);
    CHECK_THROWS_AS(f.assignmentsSvc.grantExtension(f.lecturer(), f.asgId, f.stuId,
                                                    f.deadline - hours(1), "Ill"),
                    BusinessRuleError);
    f.assignmentsSvc.grantExtension(f.lecturer(), f.asgId, f.stuId, f.deadline + hours(24),
                                    "Medical note");
    CHECK(f.submissions.find(f.asgId, f.stuId)->getStatus() == SubmissionStatus::SUBMITTED);

    const int subId = f.submissions.find(f.asgId, f.stuId)->getId();
    CHECK(f.marking.mark(f.lecturer(), subId, 70, "Good").daysLate == 0);   // no penalty now
}

TEST_CASE("An extension can be granted before the student submits (DC-02)",
          "[integration][extension]") {
    Flow f;
    f.assignmentsSvc.grantExtension(f.lecturer(), f.asgId, f.stuId, f.deadline + hours(24),
                                    "Sports tour");
    f.clock.set(f.deadline + hours(12));
    CHECK(f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 5)).status ==
          SubmissionStatus::SUBMITTED);
}

TEST_CASE("Students never see marks before release (FR-14, BR-04)", "[integration][mark]") {
    Flow f;
    f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 5));
    const int subId = f.submissions.find(f.asgId, f.stuId)->getId();
    f.marking.mark(f.lecturer(), subId, 70, "Good");
    CHECK(f.submit.listReleasedResults(f.student(f.stuId)).empty());
}
