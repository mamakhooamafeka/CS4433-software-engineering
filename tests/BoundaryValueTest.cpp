// Phase 5: boundary value analysis (BVA) and equivalence partitioning (EP).
// Each TEST_CASE name starts with the test ID used in docs/TEST_PLAN.md section 5.
// For every limit we test the value just inside, on, and just outside it.
#include "FlowFixture.hpp"
#include "domain/DeadlineCalculator.hpp"
#include "presentation/ConsoleIO.hpp"
#include "services/AdminService.hpp"
#include "services/AuthService.hpp"
#include "services/Validation.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>

using namespace lms;
using Catch::Approx;
using lms::test::Flow;
using std::chrono::hours;
using std::chrono::minutes;
using std::chrono::seconds;

namespace {
struct AuthFlow : Flow {
    AuthService auth{users, courses, assignments, submissions, hasher, clock};
    AdminService admin{users, courses, hasher, clock};
};
}  // namespace

TEST_CASE("BV-01 days late at every 24-hour boundary (BR-01)", "[bva][deadline]") {
    const DateTime d = fromIso8601("2026-11-07 23:59");
    CHECK(DeadlineCalculator::daysLate(d - seconds(1), d) == 0);  // just before
    CHECK(DeadlineCalculator::daysLate(d, d) == 0);               // exactly on
    CHECK(DeadlineCalculator::daysLate(d + seconds(1), d) == 1);  // just after
    CHECK(DeadlineCalculator::daysLate(d + hours(24), d) == 1);
    CHECK(DeadlineCalculator::daysLate(d + hours(24) + seconds(1), d) == 2);
    CHECK(DeadlineCalculator::daysLate(d + hours(48), d) == 2);
    CHECK(DeadlineCalculator::daysLate(d + hours(48) + seconds(1), d) == 3);
    CHECK(DeadlineCalculator::daysLate(d + hours(72), d) == 3);
    CHECK(DeadlineCalculator::daysLate(d + hours(72) + seconds(1), d) == 4);
}

TEST_CASE("BV-02 penalty at 3 and 4 days late (BR-01 cut-off)", "[bva][penalty]") {
    TenPercentPerDayPolicy policy;
    CHECK(policy.apply(100.0, 100, 3).finalMark == Approx(70.0));  // last day with a mark
    CHECK(policy.apply(100.0, 100, 4).finalMark == Approx(0.0));   // first day of zero
}

TEST_CASE("BV-03 submission status on the deadline second (C2)", "[bva][submit]") {
    Flow on;
    on.clock.set(on.deadline);
    CHECK(on.submit.submit(on.student(on.stuId), on.asgId, on.makeFile("a.txt", 5)).status ==
          SubmissionStatus::SUBMITTED);

    Flow after;
    after.clock.set(after.deadline + seconds(1));
    CHECK(after.submit.submit(after.student(after.stuId), after.asgId, after.makeFile("a.txt", 5))
              .status == SubmissionStatus::LATE);
}

TEST_CASE("BV-04 resubmission allowed on the deadline, refused one second later (BR-03)",
          "[bva][submit]") {
    Flow f;
    f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("v1.txt", 5));
    f.clock.set(f.deadline);
    CHECK(f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("v2.txt", 5)).versionNo == 2);
    f.clock.set(f.deadline + seconds(1));
    CHECK_THROWS_AS(f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("v3.txt", 5)),
                    BusinessRuleError);
}

TEST_CASE("BV-05 file size 0, 1, 10 MiB and 10 MiB + 1 byte (FR-11)", "[bva][submit]") {
    constexpr std::size_t kMiB = 1024 * 1024;
    Flow f;
    const auto& s = f.student(f.stuId);
    CHECK_THROWS_AS(f.submit.preview(s, f.asgId, f.makeFile("zero.txt", 0)), ValidationError);
    CHECK_NOTHROW(f.submit.preview(s, f.asgId, f.makeFile("one.txt", 1)));
    CHECK_NOTHROW(f.submit.preview(s, f.asgId, f.makeFile("max.bin", 10 * kMiB)));
    CHECK_THROWS_AS(f.submit.preview(s, f.asgId, f.makeFile("big.bin", 10 * kMiB + 1)),
                    ValidationError);
}

TEST_CASE("BV-06 raw mark -0.1, 0, M and M + 0.1 (FR-13)", "[bva][mark]") {
    Flow f;
    f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 5));
    const int subId = f.submissions.find(f.asgId, f.stuId)->getId();
    CHECK_THROWS_AS(f.marking.mark(f.lecturer(), subId, -0.1, "x"), ValidationError);
    CHECK_THROWS_AS(f.marking.mark(f.lecturer(), subId, 100.1, "x"), ValidationError);
    CHECK(f.marking.mark(f.lecturer(), subId, 0.0, "Nothing relevant").finalMark == Approx(0.0));
    CHECK(f.marking.correct(f.lecturer(), subId, 100.0, "Full", "Re-read").finalMark ==
          Approx(100.0));
}

TEST_CASE("BV-07 raw mark with two decimals is rounded to one (EP: valid class)", "[bva][mark]") {
    Flow f;
    f.submit.submit(f.student(f.stuId), f.asgId, f.makeFile("a.txt", 5));
    const int subId = f.submissions.find(f.asgId, f.stuId)->getId();
    CHECK(f.marking.mark(f.lecturer(), subId, 67.26, "Good").rawMark == Approx(67.3));
}

TEST_CASE("BV-08 maximum mark 0, 1, 1000 and 1001 (FR-08)", "[bva][assignment]") {
    Flow f;
    const DateTime due = f.clock.now() + hours(1);
    auto create = [&](int max) {
        return f.assignmentsSvc.createAssignment(f.lecturer(), f.courseId, "T", "I", due, max);
    };
    CHECK_THROWS_AS(create(0), ValidationError);
    CHECK_NOTHROW(create(1));
    CHECK_NOTHROW(create(1000));
    CHECK_THROWS_AS(create(1001), ValidationError);
}

TEST_CASE("BV-09 new deadline equal to now is refused, one second later accepted (FR-08)",
          "[bva][assignment]") {
    Flow f;
    auto create = [&](DateTime due) {
        return f.assignmentsSvc.createAssignment(f.lecturer(), f.courseId, "T", "I", due, 10);
    };
    CHECK_THROWS_AS(create(f.clock.now() - seconds(1)), ValidationError);
    CHECK_THROWS_AS(create(f.clock.now()), ValidationError);
    CHECK_NOTHROW(create(f.clock.now() + seconds(1)));
}

TEST_CASE("BV-10 title length 0, 1, 200 and 201 characters (FR-08)", "[bva][assignment]") {
    Flow f;
    const DateTime due = f.clock.now() + hours(1);
    auto create = [&](const std::string& title) {
        return f.assignmentsSvc.createAssignment(f.lecturer(), f.courseId, title, "I", due, 10);
    };
    CHECK_THROWS_AS(create("   "), ValidationError);
    CHECK_NOTHROW(create("A"));
    CHECK_NOTHROW(create(std::string(200, 'a')));
    CHECK_THROWS_AS(create(std::string(201, 'a')), ValidationError);
}

TEST_CASE("BV-11 extension earlier, equal to and one second after the deadline (BR-02)",
          "[bva][extension]") {
    Flow f;
    auto extend = [&](DateTime to) {
        f.assignmentsSvc.grantExtension(f.lecturer(), f.asgId, f.stuId, to, "Ill");
    };
    CHECK_THROWS_AS(extend(f.deadline - seconds(1)), BusinessRuleError);
    CHECK_THROWS_AS(extend(f.deadline), BusinessRuleError);
    CHECK_NOTHROW(extend(f.deadline + seconds(1)));
    // The new effective deadline is now the limit for the next extension.
    CHECK_THROWS_AS(extend(f.deadline + seconds(1)), BusinessRuleError);
}

TEST_CASE("BV-12 password length 7, 8, 128 and 129 characters (FR-03)", "[bva][auth]") {
    CHECK_THROWS_AS(requirePasswordStrength(std::string(7, 'a')), ValidationError);
    CHECK_NOTHROW(requirePasswordStrength(std::string(8, 'a')));
    CHECK_NOTHROW(requirePasswordStrength(std::string(128, 'a')));
    CHECK_THROWS_AS(requirePasswordStrength(std::string(129, 'a')), ValidationError);
}

TEST_CASE("BV-13 username length 2, 3, 32 and 33 characters", "[bva][admin]") {
    CHECK_THROWS_AS(requireUsername("ab"), ValidationError);
    CHECK(requireUsername("abc") == "abc");
    CHECK_NOTHROW(requireUsername(std::string(32, 'u')));
    CHECK_THROWS_AS(requireUsername(std::string(33, 'u')), ValidationError);
    CHECK_THROWS_AS(requireUsername("bad name"), ValidationError);  // EP: invalid characters
}

TEST_CASE("BV-14 four failed logins do not lock, the fifth does (FR-02)", "[bva][auth]") {
    AuthFlow f;
    for (int i = 0; i < 4; ++i) CHECK_THROWS(f.auth.login("stu1", "wrong"));
    CHECK_NOTHROW(f.auth.login("stu1", "password1"));  // 4 failures: still open, count resets

    for (int i = 0; i < 5; ++i) CHECK_THROWS(f.auth.login("stu1", "wrong"));
    CHECK_THROWS_AS(f.auth.login("stu1", "password1"), AccessDeniedError);  // locked
}

TEST_CASE("BV-15 lock lasts exactly 15 minutes (FR-02)", "[bva][auth]") {
    AuthFlow f;
    for (int i = 0; i < 5; ++i) CHECK_THROWS(f.auth.login("stu1", "wrong"));
    f.clock.advance(minutes(15) - seconds(1));
    CHECK_THROWS_AS(f.auth.login("stu1", "password1"), AccessDeniedError);  // 14:59
    f.clock.advance(seconds(1));
    CHECK_NOTHROW(f.auth.login("stu1", "password1"));  // 15:00
}

TEST_CASE("BV-16 menu choice 0, 1, n and n + 1 (EP: numbers, words, empty)", "[bva][ui]") {
    std::istringstream in("4\n-1\nabc\n\n3\n0\n1\n");
    std::ostringstream out;
    ConsoleIO io(in, out);
    const std::vector<std::string> items{"a", "b", "c"};
    CHECK(io.choose("Pick", items) == 2);   // 4, -1, abc and empty re-asked, then 3
    CHECK(io.choose("Pick", items) == -1);  // 0 = Back
    CHECK(io.choose("Pick", items) == 0);   // 1 = first item
}

TEST_CASE("BV-17 typed mark: -0.1, 0, M, M + 0.1 and two decimals (FR-13)", "[bva][ui]") {
    std::istringstream in("-0.1\n100.1\n12.34\n0\n100\n99.9\n");
    std::ostringstream out;
    ConsoleIO io(in, out);
    CHECK(io.readMark("m: ", 100) == 0.0);  // -0.1, 100.1 and 12.34 re-asked first
    CHECK(io.readMark("m: ", 100) == 100.0);
    CHECK(io.readMark("m: ", 100) == Approx(99.9));
}

TEST_CASE("BV-18 typed text at the maximum length and one over", "[bva][ui]") {
    std::istringstream in(std::string(11, 'x') + "\n" + std::string(10, 'x') + "\n");
    std::ostringstream out;
    ConsoleIO io(in, out);
    CHECK(io.readText("t: ", 10) == std::string(10, 'x'));
    CHECK(out.str().find("at most 10 characters") != std::string::npos);
}
