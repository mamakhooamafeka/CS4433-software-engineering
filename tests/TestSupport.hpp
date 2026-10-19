// Shared test fixture: real repositories on an in-memory database, the real
// Argon2id hasher and a clock the test can move.
#pragma once

#include "data/SqliteRepositories.hpp"
#include "domain/UserFactory.hpp"
#include "infrastructure/SodiumPasswordHasher.hpp"

#include <string>

namespace lms::test {

class FixedClock : public IClock {
public:
    explicit FixedClock(DateTime t) : t_(t) {}
    DateTime now() const override { return t_; }
    void set(DateTime t) { t_ = t; }
    void advance(std::chrono::seconds s) { t_ += s; }

private:
    DateTime t_;
};

struct World {
    SqliteDatabase db{":memory:"};
    SqliteUserRepository users{db};
    SqliteCourseRepository courses{db};
    SqliteAssignmentRepository assignments{db};
    SqliteSubmissionRepository submissions{db};
    SqliteAuditRepository audit{db};
    SodiumPasswordHasher hasher;
    FixedClock clock{fromIso8601("2026-11-01 08:00")};

    int addUser(Role role, const std::string& username, const std::string& password = "password1") {
        auto u = UserFactory::createNew(role, username, "Name " + username, hasher.hash(password));
        u->setPasswordHash(u->getPasswordHash(), false);
        return users.insert(*u);
    }
    std::unique_ptr<User> user(int id) { return users.findById(id); }
};

}  // namespace lms::test
