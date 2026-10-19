// Every action a menu can offer. Each User subclass returns only the options
// its role may use (FR-01), so the menus never show a forbidden action.
#pragma once

#include <string>

namespace lms {

enum class MenuOption {
    // every role
    CHANGE_PASSWORD,
    LOG_OUT,
    // student
    VIEW_MY_ASSIGNMENTS,
    SUBMIT_ASSIGNMENT,
    VIEW_RELEASED_RESULTS,
    // lecturer
    CREATE_ASSIGNMENT,
    GRANT_EXTENSION,
    MARK_SUBMISSIONS,
    CORRECT_MARK,
    // head of department
    MODERATE_MARKS,
    AMEND_MARK,
    RELEASE_MARKS,
    VIEW_AUDIT_LOG,
    // administrator
    MANAGE_ACCOUNTS,
    RESET_PASSWORD,
    MANAGE_DEPARTMENTS_AND_COURSES,
    ASSIGN_LECTURERS_AND_ENROL,
    BACKUP_AND_RESTORE
};

// The text shown in the menu.
std::string label(MenuOption option);

}  // namespace lms
