#include "domain/MenuOption.hpp"

namespace lms {

std::string label(MenuOption option) {
    switch (option) {
        case MenuOption::CHANGE_PASSWORD:
            return "Change my password";
        case MenuOption::LOG_OUT:
            return "Log out";
        case MenuOption::VIEW_MY_ASSIGNMENTS:
            return "View my courses and assignments";
        case MenuOption::SUBMIT_ASSIGNMENT:
            return "Submit an assignment";
        case MenuOption::VIEW_RELEASED_RESULTS:
            return "View my released results";
        case MenuOption::CREATE_ASSIGNMENT:
            return "Create an assignment";
        case MenuOption::GRANT_EXTENSION:
            return "Grant a student an extension";
        case MenuOption::MARK_SUBMISSIONS:
            return "Mark submissions";
        case MenuOption::CORRECT_MARK:
            return "Correct a mark";
        case MenuOption::MODERATE_MARKS:
            return "Moderate marked submissions";
        case MenuOption::AMEND_MARK:
            return "Amend a mark";
        case MenuOption::RELEASE_MARKS:
            return "Release marks for an assignment";
        case MenuOption::VIEW_AUDIT_LOG:
            return "View the audit log";
        case MenuOption::MANAGE_ACCOUNTS:
            return "Manage user accounts";
        case MenuOption::RESET_PASSWORD:
            return "Reset a user's password";
        case MenuOption::MANAGE_DEPARTMENTS_AND_COURSES:
            return "Manage departments and courses";
        case MenuOption::ASSIGN_LECTURERS_AND_ENROL:
            return "Assign lecturers and enrol students";
        case MenuOption::BACKUP_AND_RESTORE:
            return "Back up or restore";
    }
    return "";
}

}  // namespace lms
