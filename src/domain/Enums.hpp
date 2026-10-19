// The three enumerations from class diagram 2a, with conversions to and from
// the text stored in the database (the CHECK constraints in schema.sql).
#pragma once

#include <string>

namespace lms {

enum class Role { STUDENT, LECTURER, HOD, ADMIN };

enum class SubmissionStatus { SUBMITTED, LATE, MARKED, MODERATED, RELEASED };

enum class ChangeType { INITIAL, CORRECTION, AMENDMENT };

std::string toString(Role r);
std::string toString(SubmissionStatus s);
std::string toString(ChangeType c);

// Each throws DataAccessError for unknown text, which can only come from a damaged database.
Role roleFromString(const std::string& text);
SubmissionStatus statusFromString(const std::string& text);
ChangeType changeTypeFromString(const std::string& text);

}  // namespace lms
