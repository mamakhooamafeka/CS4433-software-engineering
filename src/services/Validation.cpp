#include "services/Validation.hpp"

#include <cctype>

namespace lms {

std::string requireText(const std::string& value, const std::string& fieldName,
                        std::size_t minLen, std::size_t maxLen) {
    const auto first = value.find_first_not_of(" \t\r\n");
    const auto last = value.find_last_not_of(" \t\r\n");
    const std::string trimmed =
        first == std::string::npos ? "" : value.substr(first, last - first + 1);
    if (trimmed.size() < minLen) {
        throw ValidationError(minLen == 1 ? fieldName + " cannot be empty."
                                          : fieldName + " must have at least " +
                                                std::to_string(minLen) + " characters.");
    }
    if (trimmed.size() > maxLen) {
        throw ValidationError(fieldName + " can have at most " + std::to_string(maxLen) +
                              " characters.");
    }
    return trimmed;
}

std::string requireUsername(const std::string& value) {
    const std::string name = requireText(value, "The username", 3, 32);
    for (const char c : name) {
        const auto uc = static_cast<unsigned char>(c);
        if (!std::isalnum(uc) && c != '.' && c != '-' && c != '_') {
            throw ValidationError(
                "A username may only use letters, digits, dots, dashes and underscores.");
        }
    }
    return name;
}

void requirePasswordStrength(const std::string& password) {
    if (password.size() < 8) {
        throw ValidationError("The new password must have at least 8 characters.");
    }
    if (password.size() > 128) {
        throw ValidationError("The new password can have at most 128 characters.");
    }
}

void requireRole(const User& user, Role role) {
    if (user.role() != role) {
        throw AccessDeniedError("Your role does not allow this action.");
    }
}

void requireCourseAccess(const User& user, const Course& course) {
    if (!user.canAccessCourse(course)) {
        throw AccessDeniedError("You can only act on courses you are responsible for.");
    }
}

} // namespace lms
