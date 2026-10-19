#include "domain/DateTime.hpp"

#include "domain/Errors.hpp"

#include <cstdio>
#include <regex>

namespace lms {
namespace {

// Days since 1970-01-01 for a civil date (Howard Hinnant's algorithm),
// so we do not depend on timegm(), which Windows does not have.
long long daysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2 ? 1 : 0;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<long long>(era) * 146097 + static_cast<long long>(doe) - 719468;
}

void civilFromDays(long long z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y = static_cast<int>(static_cast<long long>(yoe) + era * 400 + (m <= 2 ? 1 : 0));
}

bool isLeap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

unsigned daysInMonth(int y, unsigned m) {
    static const unsigned days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && isLeap(y)) ? 29 : days[m - 1];
}

}  // namespace

std::string toIso8601(DateTime t) {
    using namespace std::chrono;
    const long long secs = duration_cast<seconds>(t.time_since_epoch()).count();
    long long days = secs / 86400;
    long long rem = secs % 86400;
    if (rem < 0) {
        rem += 86400;
        --days;
    }
    int y = 0;
    unsigned m = 0;
    unsigned d = 0;
    civilFromDays(days, y, m, d);
    char buf[64];
    std::snprintf(buf, sizeof buf, "%04d-%02u-%02uT%02lld:%02lld:%02lldZ", y, m, d, rem / 3600,
                  (rem % 3600) / 60, rem % 60);
    return buf;
}

DateTime fromIso8601(const std::string& text) {
    static const std::regex pattern(
        R"(^(\d{4})-(\d{2})-(\d{2})[T ](\d{2}):(\d{2})(?::(\d{2}))?Z?$)");
    std::smatch m;
    if (!std::regex_match(text, m, pattern)) {
        throw ValidationError("Please enter a date and time as YYYY-MM-DD HH:MM.");
    }
    const int year = std::stoi(m[1]);
    const unsigned month = static_cast<unsigned>(std::stoi(m[2]));
    const unsigned day = static_cast<unsigned>(std::stoi(m[3]));
    const int hour = std::stoi(m[4]);
    const int minute = std::stoi(m[5]);
    const int second = m[6].matched ? std::stoi(m[6]) : 0;
    if (month < 1 || month > 12 || day < 1 || day > daysInMonth(year, month) || hour > 23 ||
        minute > 59 || second > 59) {
        throw ValidationError("That date or time does not exist.");
    }
    const long long secs =
        daysFromCivil(year, month, day) * 86400LL + hour * 3600LL + minute * 60LL + second;
    return DateTime{std::chrono::seconds{secs}};
}

}  // namespace lms
