#include "data/SqliteRepositories.hpp"

#include "domain/UserFactory.hpp"

namespace lms {
namespace {

const char* const kUserColumns =
    "select id, username, full_name, password_hash, role, active, failed_attempts, "
    "locked_until, must_change_password from user_account ";

UserRecord readRecord(const Statement& s) {
    UserRecord r;
    r.id = s.getInt(0);
    r.username = s.getText(1);
    r.fullName = s.getText(2);
    r.passwordHash = s.getText(3);
    r.role = roleFromString(s.getText(4));
    r.active = s.getBool(5);
    r.failedAttempts = s.getInt(6);
    r.lockedUntil = s.getOptionalDateTime(7);
    r.mustChangePassword = s.getBool(8);
    return r;
}

} // namespace

// Adds the courses or department that limit what this user may see (BR-07).
void SqliteUserRepository::loadScope(UserRecord& rec) {
    if (rec.role == Role::STUDENT) {
        Statement s = db_.prepare(
            "select course_id from enrolment where student_id = ? and withdrawn = 0");
        s.bind(1, rec.id);
        while (s.step()) rec.courseIds.insert(s.getInt(0));
    } else if (rec.role == Role::LECTURER) {
        Statement s = db_.prepare("select course_id from course_lecturer where lecturer_id = ?");
        s.bind(1, rec.id);
        while (s.step()) rec.courseIds.insert(s.getInt(0));
    } else if (rec.role == Role::HOD) {
        Statement s = db_.prepare("select id from department where hod_user_id = ?");
        s.bind(1, rec.id);
        if (s.step()) rec.departmentId = s.getInt(0);
    }
}

std::unique_ptr<User> SqliteUserRepository::load(Statement& s) {
    if (!s.step()) return nullptr;
    UserRecord rec = readRecord(s);
    loadScope(rec);
    return UserFactory::create(rec);
}

std::unique_ptr<User> SqliteUserRepository::findByUsername(const std::string& username) {
    Statement s = db_.prepare(std::string(kUserColumns) + "where username = ?");
    s.bind(1, username);
    return load(s);
}

std::unique_ptr<User> SqliteUserRepository::findById(int id) {
    Statement s = db_.prepare(std::string(kUserColumns) + "where id = ?");
    s.bind(1, id);
    return load(s);
}

int SqliteUserRepository::insert(const User& user) {
    const UserRecord r = user.toRecord();
    Statement s = db_.prepare(
        "insert into user_account (username, full_name, password_hash, role, active, "
        "failed_attempts, locked_until, must_change_password) values (?, ?, ?, ?, ?, ?, ?, ?)");
    s.bind(1, r.username).bind(2, r.fullName).bind(3, r.passwordHash).bind(4, toString(r.role));
    s.bind(5, r.active).bind(6, r.failedAttempts).bind(7, r.lockedUntil);
    s.bind(8, r.mustChangePassword);
    try {
        s.run();
    } catch (const ConstraintViolation&) {
        throw ValidationError("That username is already taken, or a field is too long or empty.");
    }
    return static_cast<int>(db_.lastInsertId());
}

void SqliteUserRepository::update(const User& user) {
    const UserRecord r = user.toRecord();
    Statement s = db_.prepare(
        "update user_account set full_name = ?, password_hash = ?, active = ?, "
        "failed_attempts = ?, locked_until = ?, must_change_password = ? where id = ?");
    s.bind(1, r.fullName).bind(2, r.passwordHash).bind(3, r.active).bind(4, r.failedAttempts);
    s.bind(5, r.lockedUntil).bind(6, r.mustChangePassword).bind(7, r.id);
    s.run();
}

std::vector<UserRecord> SqliteUserRepository::listAll() {
    Statement s = db_.prepare(std::string(kUserColumns) + "order by role, username");
    std::vector<UserRecord> out;
    while (s.step()) out.push_back(readRecord(s));
    return out;
}

} // namespace lms
