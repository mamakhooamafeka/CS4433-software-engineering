#include "domain/Enums.hpp"

#include "domain/Errors.hpp"

namespace lms {

std::string toString(Role r) {
    switch (r) {
        case Role::STUDENT:
            return "STUDENT";
        case Role::LECTURER:
            return "LECTURER";
        case Role::HOD:
            return "HOD";
        case Role::ADMIN:
            return "ADMIN";
    }
    return "";
}

std::string toString(SubmissionStatus s) {
    switch (s) {
        case SubmissionStatus::SUBMITTED:
            return "SUBMITTED";
        case SubmissionStatus::LATE:
            return "LATE";
        case SubmissionStatus::MARKED:
            return "MARKED";
        case SubmissionStatus::MODERATED:
            return "MODERATED";
        case SubmissionStatus::RELEASED:
            return "RELEASED";
    }
    return "";
}

std::string toString(ChangeType c) {
    switch (c) {
        case ChangeType::INITIAL:
            return "INITIAL";
        case ChangeType::CORRECTION:
            return "CORRECTION";
        case ChangeType::AMENDMENT:
            return "AMENDMENT";
    }
    return "";
}

Role roleFromString(const std::string& text) {
    if (text == "STUDENT") return Role::STUDENT;
    if (text == "LECTURER") return Role::LECTURER;
    if (text == "HOD") return Role::HOD;
    if (text == "ADMIN") return Role::ADMIN;
    throw DataAccessError("The database contains an unknown role.");
}

SubmissionStatus statusFromString(const std::string& text) {
    if (text == "SUBMITTED") return SubmissionStatus::SUBMITTED;
    if (text == "LATE") return SubmissionStatus::LATE;
    if (text == "MARKED") return SubmissionStatus::MARKED;
    if (text == "MODERATED") return SubmissionStatus::MODERATED;
    if (text == "RELEASED") return SubmissionStatus::RELEASED;
    throw DataAccessError("The database contains an unknown submission status.");
}

ChangeType changeTypeFromString(const std::string& text) {
    if (text == "INITIAL") return ChangeType::INITIAL;
    if (text == "CORRECTION") return ChangeType::CORRECTION;
    if (text == "AMENDMENT") return ChangeType::AMENDMENT;
    throw DataAccessError("The database contains an unknown audit change type.");
}

}  // namespace lms
