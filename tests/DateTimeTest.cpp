#include "domain/DateTime.hpp"
#include "domain/Errors.hpp"

#include <catch2/catch_test_macros.hpp>

using lms::fromIso8601;
using lms::toIso8601;

TEST_CASE("ISO 8601 text round-trips through DateTime", "[unit][datetime]") {
    CHECK(toIso8601(fromIso8601("2026-11-07T23:59:00Z")) == "2026-11-07T23:59:00Z");
    CHECK(toIso8601(fromIso8601("1970-01-01T00:00:00Z")) == "1970-01-01T00:00:00Z");
}

TEST_CASE("Console format without seconds is accepted", "[unit][datetime]") {
    CHECK(toIso8601(fromIso8601("2026-11-07 08:30")) == "2026-11-07T08:30:00Z");
}

TEST_CASE("Leap days are handled (boundary values)", "[unit][datetime]") {
    CHECK_NOTHROW(fromIso8601("2028-02-29 12:00"));
    CHECK_THROWS_AS(fromIso8601("2026-02-29 12:00"), lms::ValidationError);
}

TEST_CASE("Invalid dates are rejected with ValidationError", "[unit][datetime]") {
    CHECK_THROWS_AS(fromIso8601(""), lms::ValidationError);
    CHECK_THROWS_AS(fromIso8601("7 Nov 2026"), lms::ValidationError);
    CHECK_THROWS_AS(fromIso8601("2026-13-01 10:00"), lms::ValidationError);
    CHECK_THROWS_AS(fromIso8601("2026-11-31 10:00"), lms::ValidationError);
    CHECK_THROWS_AS(fromIso8601("2026-11-07 24:00"), lms::ValidationError);
}

TEST_CASE("One day apart is exactly 86400 seconds", "[unit][datetime]") {
    const auto a = fromIso8601("2026-11-07 23:59");
    const auto b = fromIso8601("2026-11-08 23:59");
    CHECK(std::chrono::duration_cast<std::chrono::seconds>(b - a).count() == 86400);
}
