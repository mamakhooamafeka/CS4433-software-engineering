// The four role menus (class diagram 2b). Each one only turns typed choices into
// service calls and shows the results: all checks and rules live in the services.
#pragma once

#include "domain/Roles.hpp"
#include "presentation/MenuScreen.hpp"
#include "services/AdminService.hpp"
#include "services/AssignmentService.hpp"
#include "services/BackupService.hpp"
#include "services/MarkingService.hpp"
#include "services/ModerationService.hpp"
#include "services/SubmissionService.hpp"

namespace lms {

class StudentMenu : public MenuScreen {
public:
    StudentMenu(ConsoleIO& io, AuthService& auth, AssignmentService& assignments,
                SubmissionService& submissions)
        : MenuScreen(io, auth), assignments_(assignments), submissions_(submissions) {}

protected:
    void handle(User& user, MenuOption option) override;

private:
    void viewAssignments(const Student& student);
    void submit(const Student& student);
    void viewResults(const Student& student);

    AssignmentService& assignments_;
    SubmissionService& submissions_;
};

class LecturerMenu : public MenuScreen {
public:
    LecturerMenu(ConsoleIO& io, AuthService& auth, AssignmentService& assignments,
                 MarkingService& marking)
        : MenuScreen(io, auth), assignments_(assignments), marking_(marking) {}

protected:
    void handle(User& user, MenuOption option) override;

private:
    std::optional<Course> pickCourse(const Lecturer& lecturer);
    std::optional<Assignment> pickAssignment(const Lecturer& lecturer);
    void createAssignment(const Lecturer& lecturer);
    void grantExtension(const Lecturer& lecturer);
    void markSubmissions(const Lecturer& lecturer);
    void correctMark(const Lecturer& lecturer);

    AssignmentService& assignments_;
    MarkingService& marking_;
};

class HodMenu : public MenuScreen {
public:
    HodMenu(ConsoleIO& io, AuthService& auth, ModerationService& moderation)
        : MenuScreen(io, auth), moderation_(moderation) {}

protected:
    void handle(User& user, MenuOption option) override;

private:
    std::optional<AssignmentSummary> pickAssignment(const HeadOfDepartment& hod);
    void moderate(const HeadOfDepartment& hod);
    void amend(const HeadOfDepartment& hod);
    void release(const HeadOfDepartment& hod);
    void viewAudit(const HeadOfDepartment& hod);

    ModerationService& moderation_;
};

class AdminMenu : public MenuScreen {
public:
    AdminMenu(ConsoleIO& io, AuthService& auth, AdminService& admin, BackupService& backup)
        : MenuScreen(io, auth), admin_(admin), backup_(backup) {}

protected:
    void handle(User& user, MenuOption option) override;

private:
    // Lists accounts (optionally of one role only) and returns the chosen user's id.
    std::optional<int> pickUser(const User& admin, std::optional<Role> role);
    std::optional<int> pickCourse(const User& admin);
    std::optional<int> pickDepartment(const User& admin);
    void manageAccounts(const User& admin);
    void resetPassword(const User& admin);
    void manageDepartmentsAndCourses(const User& admin);
    void assignLecturersAndEnrol(const User& admin);
    void backupAndRestore(const User& admin);

    AdminService& admin_;
    BackupService& backup_;
};

} // namespace lms
