#include "presentation/RoleMenus.hpp"

#include "domain/Enums.hpp"

namespace lms {
namespace {

std::string describe(const UserRecord& u) {
    return std::to_string(u.id) + " " + u.username + " | " + u.fullName + " | " +
           toString(u.role) + (u.active ? "" : " | deactivated");
}

} // namespace

void AdminMenu::handle(User& user, MenuOption option) {
    switch (option) {
        case MenuOption::MANAGE_ACCOUNTS: manageAccounts(user); break;
        case MenuOption::RESET_PASSWORD: resetPassword(user); break;
        case MenuOption::MANAGE_DEPARTMENTS_AND_COURSES: manageDepartmentsAndCourses(user); break;
        case MenuOption::ASSIGN_LECTURERS_AND_ENROL: assignLecturersAndEnrol(user); break;
        case MenuOption::BACKUP_AND_RESTORE: backupAndRestore(user); break;
        default: break;
    }
}

std::optional<int> AdminMenu::pickUser(const User& admin, std::optional<Role> role) {
    std::vector<UserRecord> list;
    for (const UserRecord& u : admin_.listUsers(admin)) {
        if (u.active && (!role || u.role == *role)) list.push_back(u);
    }
    std::vector<std::string> items;
    for (const UserRecord& u : list) items.push_back(describe(u));
    const int pick = io_.choose(role ? "Choose a " + toString(*role) : "Choose an account", items);
    if (pick < 0) return std::nullopt;
    return list[static_cast<std::size_t>(pick)].id;
}

std::optional<int> AdminMenu::pickCourse(const User& admin) {
    const auto list = admin_.listCourses(admin);
    std::vector<std::string> items;
    for (const Course& c : list) items.push_back(c.getCode() + " " + c.getTitle());
    const int pick = io_.choose("Choose a course", items);
    if (pick < 0) return std::nullopt;
    return list[static_cast<std::size_t>(pick)].getId();
}

std::optional<int> AdminMenu::pickDepartment(const User& admin) {
    const auto list = admin_.listDepartments(admin);
    std::vector<std::string> items;
    for (const Department& d : list) items.push_back(d.getName());
    const int pick = io_.choose("Choose a department", items);
    if (pick < 0) return std::nullopt;
    return list[static_cast<std::size_t>(pick)].getId();
}

// UC03 and UC04 (FR-04, FR-05).
void AdminMenu::manageAccounts(const User& admin) {
    const int what = io_.choose("Manage accounts", {"List all accounts", "Create an account",
                                                    "Edit a full name", "Deactivate an account"});
    if (what == 0) {
        io_.heading("Accounts");
        for (const UserRecord& u : admin_.listUsers(admin)) io_.info(describe(u));
    } else if (what == 1) {
        const std::vector<Role> roles = {Role::STUDENT, Role::LECTURER, Role::HOD, Role::ADMIN};
        const int r = io_.choose("Role", {"Student", "Lecturer", "Head of department",
                                          "Administrator"});
        if (r < 0) return;
        const std::string username = io_.readText("Username: ", 32);
        const std::string fullName = io_.readText("Full name: ", 100);
        const std::string password = readNewPassword(io_, "Temporary password");
        admin_.createUser(admin, roles[static_cast<std::size_t>(r)], username, fullName, password);
        io_.info("Account created. The user must change the password at first log in.");
    } else if (what == 2) {
        const auto id = pickUser(admin, std::nullopt);
        if (!id) return;
        admin_.editUserName(admin, *id, io_.readText("New full name: ", 100));
        io_.info("The name has been changed.");
    } else if (what == 3) {
        const auto id = pickUser(admin, std::nullopt);
        if (!id) return;
        if (!io_.confirm("Deactivate this account? The user can no longer log in")) return;
        admin_.deactivateUser(admin, *id);
        io_.info("The account has been deactivated. Its records are kept.");
    }
}

// UC05 (FR-05).
void AdminMenu::resetPassword(const User& admin) {
    const auto id = pickUser(admin, std::nullopt);
    if (!id) return;
    admin_.resetPassword(admin, *id, readNewPassword(io_, "Temporary password"));
    io_.info("Password reset. The user must change it at the next log in.");
}

// UC06 (FR-06).
void AdminMenu::manageDepartmentsAndCourses(const User& admin) {
    const int what = io_.choose("Departments and courses",
                                {"List departments and courses", "Create a department",
                                 "Change a department's head", "Create a course"});
    if (what == 0) {
        io_.heading("Departments");
        for (const Department& d : admin_.listDepartments(admin)) io_.info(d.getName());
        io_.heading("Courses");
        for (const Course& c : admin_.listCourses(admin)) {
            io_.info(c.getCode() + " " + c.getTitle());
        }
    } else if (what == 1) {
        const std::string name = io_.readText("Department name: ", 100);
        const auto hod = pickUser(admin, Role::HOD);
        if (!hod) return;
        admin_.createDepartment(admin, name, *hod);
        io_.info("Department created.");
    } else if (what == 2) {
        const auto dept = pickDepartment(admin);
        if (!dept) return;
        const auto hod = pickUser(admin, Role::HOD);
        if (!hod) return;
        admin_.assignHeadOfDepartment(admin, *dept, *hod);
        io_.info("Head of department assigned.");
    } else if (what == 3) {
        const auto dept = pickDepartment(admin);
        if (!dept) return;
        const std::string code = io_.readText("Course code (e.g. CS4433): ", 20);
        const std::string title = io_.readText("Course title: ", 200);
        admin_.createCourse(admin, code, title, *dept);
        io_.info("Course created.");
    }
}

// UC07 (FR-07).
void AdminMenu::assignLecturersAndEnrol(const User& admin) {
    const int what = io_.choose("Lecturers and enrolment",
                                {"Assign a lecturer to a course", "Enrol a student",
                                 "Withdraw a student"});
    if (what < 0) return;
    const auto course = pickCourse(admin);
    if (!course) return;
    const auto person = pickUser(admin, what == 0 ? Role::LECTURER : Role::STUDENT);
    if (!person) return;
    if (what == 0) {
        admin_.assignLecturer(admin, *course, *person);
        io_.info("Lecturer assigned.");
    } else if (what == 1) {
        admin_.enrolStudent(admin, *course, *person);
        io_.info("Student enrolled.");
    } else {
        if (!io_.confirm("Withdraw this student from the course?")) return;
        admin_.withdrawStudent(admin, *course, *person);
        io_.info("Student withdrawn. Their earlier submissions are kept.");
    }
}

// UC08 (FR-21, FR-22).
void AdminMenu::backupAndRestore(const User& admin) {
    const int what = io_.choose("Backup and restore", {"Back up now", "Restore a backup"});
    if (what == 0) {
        io_.info("Backup saved as " + backup_.backupNow() + ".");
    } else if (what == 1) {
        const auto names = backup_.listBackups(admin);
        const int pick = io_.choose("Backups (newest first)", names);
        if (pick < 0) return;
        const std::string& name = names[static_cast<std::size_t>(pick)];
        io_.info("Restoring replaces ALL current data with the copy from " + name + ".");
        if (!io_.confirm("Restore this backup?")) return;
        backup_.restore(admin, name);
        io_.info("Restored. Please log out and log in again.");
    }
}

} // namespace lms
