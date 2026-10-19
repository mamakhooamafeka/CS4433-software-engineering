#include "domain/Errors.hpp"
#include "domain/Submission.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace lms;
using std::chrono::seconds;

namespace {

const DateTime kDeadline = fromIso8601("2026-11-07 23:59");

SubmissionVersion versionAt(DateTime t) {
    SubmissionVersion v;
    v.storedPath = "submissions/x";
    v.originalName = "essay.pdf";
    v.sizeBytes = 100;
    v.sha256 = std::string(64, 'a');
    v.submittedAt = t;
    return v;
}

Mark someMark() {
    Mark m;
    m.rawMark = 70;
    m.finalMark = 70;
    m.feedback = "Good work";
    m.markedBy = 2;
    return m;
}

}  // namespace

TEST_CASE("A submission exactly at the deadline is SUBMITTED", "[unit][submission]") {
    Submission s(1, 10);
    s.addVersion(versionAt(kDeadline), kDeadline);
    CHECK(s.getStatus() == SubmissionStatus::SUBMITTED);
}

TEST_CASE("A submission one second after the deadline is LATE, not rejected",
          "[unit][submission]") {
    Submission s(1, 10);
    s.addVersion(versionAt(kDeadline + seconds(1)), kDeadline);
    CHECK(s.getStatus() == SubmissionStatus::LATE);
    CHECK(s.getVersions().size() == 1);
}

TEST_CASE("Resubmission before the deadline adds a version and keeps the old one",
          "[unit][submission]") {
    Submission s(1, 10);
    s.addVersion(versionAt(kDeadline - seconds(100)), kDeadline);
    s.addVersion(versionAt(kDeadline - seconds(10)), kDeadline);
    REQUIRE(s.getVersions().size() == 2);
    CHECK(s.latestVersion().versionNo == 2);
    CHECK(s.getVersions()[0].versionNo == 1);
}

TEST_CASE("Resubmission after the deadline is refused (BR-03)", "[unit][submission]") {
    Submission s(1, 10);
    s.addVersion(versionAt(kDeadline - seconds(100)), kDeadline);
    CHECK_THROWS_AS(s.addVersion(versionAt(kDeadline + seconds(1)), kDeadline), BusinessRuleError);
}

TEST_CASE("Resubmission after marking is refused (BR-03)", "[unit][submission]") {
    Submission s(1, 10);
    s.addVersion(versionAt(kDeadline - seconds(100)), kDeadline);
    s.recordMark(someMark());
    CHECK_FALSE(s.canResubmit(kDeadline - seconds(50), kDeadline));
}

TEST_CASE("Full life cycle: mark, approve, release", "[unit][submission]") {
    Submission s(1, 10);
    s.addVersion(versionAt(kDeadline), kDeadline);
    s.recordMark(someMark());
    CHECK(s.getStatus() == SubmissionStatus::MARKED);
    s.correctMark(someMark());
    CHECK(s.getStatus() == SubmissionStatus::MARKED);
    s.approve(5, kDeadline);
    CHECK(s.getStatus() == SubmissionStatus::MODERATED);
    CHECK(s.getModeratedBy() == 5);
    s.release(5, kDeadline);
    CHECK(s.getStatus() == SubmissionStatus::RELEASED);
    CHECK(s.getReleasedBy() == 5);
}

TEST_CASE("Amend moves MARKED to MODERATED and keeps RELEASED as RELEASED", "[unit][submission]") {
    Submission s(1, 10);
    s.addVersion(versionAt(kDeadline), kDeadline);
    s.recordMark(someMark());
    s.amend(someMark(), 5, kDeadline);
    CHECK(s.getStatus() == SubmissionStatus::MODERATED);
    s.release(5, kDeadline);
    s.amend(someMark(), 5, kDeadline);
    CHECK(s.getStatus() == SubmissionStatus::RELEASED);
}

TEST_CASE("Events not allowed in a state are refused", "[unit][submission]") {
    Submission s(1, 10);
    s.addVersion(versionAt(kDeadline), kDeadline);
    CHECK_THROWS_AS(s.approve(5, kDeadline), BusinessRuleError);  // not marked yet
    CHECK_THROWS_AS(s.release(5, kDeadline), BusinessRuleError);
    s.recordMark(someMark());
    CHECK_THROWS_AS(s.recordMark(someMark()), BusinessRuleError);  // already marked
    s.approve(5, kDeadline);
    CHECK_THROWS_AS(s.correctMark(someMark()), BusinessRuleError);  // lecturer can't change now
}

TEST_CASE("An extension turns LATE back into SUBMITTED (DC-01)", "[unit][submission]") {
    Submission s(1, 10);
    const DateTime submittedAt = kDeadline + std::chrono::hours(5);
    s.addVersion(versionAt(submittedAt), kDeadline);
    REQUIRE(s.getStatus() == SubmissionStatus::LATE);
    s.reevaluateLateness(kDeadline + std::chrono::hours(1));  // still too early
    CHECK(s.getStatus() == SubmissionStatus::LATE);
    s.reevaluateLateness(kDeadline + std::chrono::hours(24));
    CHECK(s.getStatus() == SubmissionStatus::SUBMITTED);
}
