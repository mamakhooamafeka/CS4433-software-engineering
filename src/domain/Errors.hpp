// Exception hierarchy shared by every layer (class diagram 2b, NFR-07).
// The console shows what() to the user, so messages must be plain language
// with no internal detail such as SQL text or file paths.
#pragma once

#include <stdexcept>
#include <string>

namespace lms {

class LmsError : public std::runtime_error {
public:
    explicit LmsError(const std::string& message) : std::runtime_error(message) {}
};

// Input the user typed is not acceptable (wrong format, out of range, empty).
class ValidationError : public LmsError {
public:
    using LmsError::LmsError;
};

// The user's role or course does not allow this action.
class AccessDeniedError : public LmsError {
public:
    using LmsError::LmsError;
};

// The action breaks a business rule, e.g. marking a released submission.
class BusinessRuleError : public LmsError {
public:
    using LmsError::LmsError;
};

// The database or file store failed. The cause is logged, not shown.
class DataAccessError : public LmsError {
public:
    using LmsError::LmsError;
};

}  // namespace lms
