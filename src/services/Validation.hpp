// Input checks used at the service boundary, so every entry point validates
// the same way whatever screen the data came from.
#pragma once

#include "domain/Course.hpp"
#include "domain/Errors.hpp"
#include "domain/User.hpp"

#include <string>

namespace lms {

// Trims spaces and checks the length is between minLen and maxLen characters.
std::string requireText(const std::string& value, const std::string& fieldName, std::size_t minLen,
                        std::size_t maxLen);

// 3-32 characters: letters, digits, dot, dash or underscore.
std::string requireUsername(const std::string& value);

// FR-03: at least 8 characters.
void requirePasswordStrength(const std::string& password);

void requireRole(const User& user, Role role);

// BR-07: throws AccessDeniedError unless the user may act on this course.
void requireCourseAccess(const User& user, const Course& course);

}  // namespace lms
