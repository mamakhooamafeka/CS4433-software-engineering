// Time type used everywhere (note N1 on class diagram 2a).
// Times are stored in the database as ISO 8601 UTC text, e.g. "2026-11-07T23:59:00Z".
#pragma once

#include <chrono>
#include <string>

namespace lms {

using DateTime = std::chrono::system_clock::time_point;

// Formats a time as "YYYY-MM-DDTHH:MM:SSZ" in UTC.
std::string toIso8601(DateTime t);

// Parses "YYYY-MM-DDTHH:MM:SSZ" or "YYYY-MM-DD HH:MM" (UTC).
// Throws ValidationError if the text is not a real date and time.
DateTime fromIso8601(const std::string& text);

}  // namespace lms
