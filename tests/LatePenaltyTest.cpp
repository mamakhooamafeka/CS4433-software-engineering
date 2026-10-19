#include "domain/LatePenalty.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;
using lms::TenPercentPerDayPolicy;

// One test per column of the BR-01 decision table (SRS Table 4), M = 100, raw = 75.
TEST_CASE("BR-01 decision table, M = 100, raw = 75", "[unit][penalty]") {
    TenPercentPerDayPolicy policy;
    struct Row { int daysLate; double finalMark; };
    const Row rows[] = {{0, 75.0}, {1, 65.0}, {2, 55.0}, {3, 45.0}, {4, 0.0}, {10, 0.0}};
    for (const Row& r : rows) {
        const auto result = policy.apply(75.0, 100, r.daysLate);
        CAPTURE(r.daysLate);
        CHECK(result.finalMark == Approx(r.finalMark));
        CHECK(result.deduction == Approx(75.0 - r.finalMark));
    }
}

TEST_CASE("The final mark never goes below zero", "[unit][penalty]") {
    TenPercentPerDayPolicy policy;
    const auto result = policy.apply(5.0, 100, 3);   // 5 - 30 would be negative
    CHECK(result.finalMark == Approx(0.0));
    CHECK(result.deduction == Approx(5.0));
}

TEST_CASE("Marks are held to one decimal place", "[unit][penalty]") {
    TenPercentPerDayPolicy policy;
    const auto result = policy.apply(17.26, 25, 1);   // 17.3 - 2.5 = 14.8
    CHECK(result.finalMark == Approx(14.8));
    CHECK(result.deduction == Approx(2.5));
}

TEST_CASE("The penalty is a share of the maximum mark, not of the raw mark", "[unit][penalty]") {
    TenPercentPerDayPolicy policy;
    CHECK(policy.apply(40.0, 50, 2).finalMark == Approx(30.0));   // 40 - 0.2 * 50
}
