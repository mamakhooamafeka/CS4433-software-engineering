// Entry point: builds every object once and passes each one the interfaces it needs
// (dependency injection by hand), then runs the log-in / menu loop.
// Usage: lms [data-folder]   (default "lms-data" in the current folder)
#include "data/SqliteRepositories.hpp"
#include "domain/LatePenalty.hpp"
#include "infrastructure/ErrorLog.hpp"
#include "infrastructure/LocalFileStore.hpp"
#include "infrastructure/SodiumPasswordHasher.hpp"
#include "infrastructure/SystemClock.hpp"
#include "presentation/LoginScreen.hpp"
#include "presentation/RoleMenus.hpp"
#include "services/AuditService.hpp"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
using namespace lms;

namespace {

void createFirstAdmin(ConsoleIO& io, AdminService& admin) {
    io.heading("First start: create the administrator account");
    while (true) {
        const std::string username = io.readText("Username: ", 32);
        const std::string fullName = io.readText("Full name: ", 100);
        const std::string password = readNewPassword(io, "Password");
        try {
            admin.createFirstAdmin(username, fullName, password);
            io.info("Administrator created. Please log in.");
            return;
        } catch (const LmsError& e) {
            io.showError(e);
        }
    }
}

int run(const fs::path& dataDir) {
    std::error_code ec;
    fs::create_directories(dataDir, ec);
    ErrorLog::setPath((dataDir / "lms-error.log").string());

    SqliteDatabase db((dataDir / "lms.db").string());
    if (!db.integrityCheck()) {
        std::cout << "The database file is damaged. Please restore a backup.\n";
    }
    SqliteUserRepository users(db);
    SqliteCourseRepository courses(db);
    SqliteAssignmentRepository assignments(db);
    SqliteSubmissionRepository submissions(db);
    SqliteAuditRepository auditLog(db);
    SodiumPasswordHasher hasher;
    SystemClock clock;
    LocalFileStore files(dataDir / "submissions");
    TenPercentPerDayPolicy policy;

    AuthService auth(users, courses, assignments, submissions, hasher, clock);
    AdminService admin(users, courses, hasher, clock);
    AuditService audit(auditLog, clock);
    AssignmentService assignmentSvc(assignments, courses, submissions, users, clock, db);
    SubmissionService submissionSvc(assignments, courses, submissions, files, clock, db);
    MarkingService marking(assignments, courses, submissions, users, policy, audit, clock, db);
    ModerationService moderation(assignments, courses, submissions, users, auditLog, policy,
                                 audit, clock, db);
    BackupService backup(db, files, clock, (dataDir / "backups").string());

    ConsoleIO io;
    io.line("Moderated LMS v1.0. All times are UTC.");
    try {
        if (backup.backupIfFirstStartToday()) io.info("Today's backup has been made.");
    } catch (const LmsError& e) {
        io.showError(e);   // a failed backup must not stop people working
    }
    if (admin.needsFirstAdmin()) createFirstAdmin(io, admin);

    StudentMenu studentMenu(io, auth, assignmentSvc, submissionSvc);
    LecturerMenu lecturerMenu(io, auth, assignmentSvc, marking);
    HodMenu hodMenu(io, auth, moderation);
    AdminMenu adminMenu(io, auth, admin, backup);

    LoginScreen login(io, auth);
    while (std::unique_ptr<User> user = login.run()) {
        MenuScreen* menu = &adminMenu;
        switch (user->role()) {
            case Role::STUDENT: menu = &studentMenu; break;
            case Role::LECTURER: menu = &lecturerMenu; break;
            case Role::HOD: menu = &hodMenu; break;
            case Role::ADMIN: menu = &adminMenu; break;
        }
        menu->run(*user);
        io.info("You have logged out.");
    }
    io.line("Goodbye.");
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    const fs::path dataDir = argc > 1 ? fs::path(argv[1]) : fs::path("lms-data");
    try {
        return run(dataDir);
    } catch (const InputClosed&) {
        return 0;   // Ctrl+Z / Ctrl+D: leave quietly
    } catch (const LmsError& e) {
        std::cerr << "\n" << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        ErrorLog::write(std::string("unexpected: ") + e.what());
        std::cerr << "\nSomething went wrong. The details are in lms-error.log.\n";
        return 1;
    }
}
