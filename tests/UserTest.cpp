#include "domain/DeadlineCalculator.hpp"
#include "domain/Roles.hpp"
#include "domain/UserFactory.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

using namespace lms;

namespace {

UserRecord record(Role role) {
    UserRecord r;
    r.id = 7;
    r.username = "user";
    r.fullName = "A User";
    r.passwordHash = "hash";
    r.role = role;
    return r;
}

bool offers(const User& u, MenuOption o) {
    const auto options = u.menuOptions();
    return std::find(options.begin(), options.end(), o) != options.end();
}

// A fake repository with fixed extensions, so DeadlineCalculator can be tested alone.
class FakeAssignments : public IAssignmentRepository {
public:
    std::vector<Extension> extensions;
    std::optional<Assignment> findById(int) override { return std::nullopt; }
    std::vector<Assignment> byCourse(int) override { return {}; }
    int insert(const Assignment&) override { return 0; }
    std::vector<Extension> extensionsFor(int, int) override { return extensions; }
    void insertExtension(const Extension&) override {}
};

}  // namespace

TEST_CASE("UserFactory builds the right subclass for each role", "[unit][user]") {
    CHECK(dynamic_cast<Student*>(UserFactory::create(record(Role::STUDENT)).get()) != nullptr);
    CHECK(dynamic_cast<Lecturer*>(UserFactory::create(record(Role::LECTURER)).get()) != nullptr);
    CHECK(dynamic_cast<HeadOfDepartment*>(UserFactory::create(record(Role::HOD)).get()) !=
          nullptr);
    CHECK(dynamic_cast<Administrator*>(UserFactory::create(record(Role::ADMIN)).get()) != nullptr);
}

TEST_CASE("New accounts must change their password", "[unit][user]") {
    auto u = UserFactory::createNew(Role::STUDENT, "thabo", "Thabo M", "hash");
    CHECK(u->mustChangePassword());
    CHECK(u->isActive());
    CHECK(u->toRecord().role == Role::STUDENT);
}

TEST_CASE("Each role sees only its own menu options (FR-01)", "[unit][user]") {
    auto student = UserFactory::create(record(Role::STUDENT));
    auto lecturer = UserFactory::create(record(Role::LECTURER));
    auto hod = UserFactory::create(record(Role::HOD));
    auto admin = UserFactory::create(record(Role::ADMIN));
    CHECK(offers(*student, MenuOption::SUBMIT_ASSIGNMENT));
    CHECK_FALSE(offers(*student, MenuOption::MARK_SUBMISSIONS));
    CHECK(offers(*lecturer, MenuOption::MARK_SUBMISSIONS));
    CHECK_FALSE(offers(*lecturer, MenuOption::RELEASE_MARKS));
    CHECK(offers(*hod, MenuOption::RELEASE_MARKS));
    CHECK_FALSE(offers(*hod, MenuOption::MANAGE_ACCOUNTS));
    CHECK(offers(*admin, MenuOption::MANAGE_ACCOUNTS));
    CHECK_FALSE(offers(*admin, MenuOption::MARK_SUBMISSIONS));
}

TEST_CASE("Course access follows BR-07", "[unit][user]") {
    const Course maths(1, "MAT101", "Maths", 100);
    const Course art(2, "ART101", "Art", 200);
    auto rec = record(Role::LECTURER);
    rec.courseIds = {1};
    auto lecturer = UserFactory::create(rec);
    CHECK(lecturer->canAccessCourse(maths));
    CHECK_FALSE(lecturer->canAccessCourse(art));

    auto hodRec = record(Role::HOD);
    hodRec.departmentId = 200;
    auto hod = UserFactory::create(hodRec);
    CHECK(hod->canAccessCourse(art));
    CHECK_FALSE(hod->canAccessCourse(maths));
    CHECK_FALSE(UserFactory::create(record(Role::ADMIN))->canAccessCourse(maths));
}

TEST_CASE("Five failed logins lock the account for 15 minutes (FR-02)", "[unit][user]") {
    auto u = UserFactory::create(record(Role::STUDENT));
    const DateTime t = fromIso8601("2026-11-01 10:00");
    for (int i = 0; i < 4; ++i) u->registerFailedLogin(t);
    CHECK_FALSE(u->isLocked(t));
    u->registerFailedLogin(t);
    CHECK(u->isLocked(t + std::chrono::minutes(14)));
    CHECK_FALSE(u->isLocked(t + std::chrono::minutes(15)));
}

TEST_CASE("Effective deadline is the latest extension (BR-02)", "[unit][deadline]") {
    FakeAssignments repo;
    const DateTime d = fromIso8601("2026-11-07 23:59");
    const Assignment a(1, 1, "Essay", "Write", d, 100, 2, d);
    DeadlineCalculator calc(repo);
    CHECK(calc.effectiveDeadline(a, 10) == d);

    Extension e1;
    e1.newDeadline = d + std::chrono::hours(24);
    Extension e2;
    e2.newDeadline = d + std::chrono::hours(48);
    repo.extensions = {e2, e1};
    CHECK(calc.effectiveDeadline(a, 10) == d + std::chrono::hours(48));
}

TEST_CASE("Days late counts started 24-hour periods (BR-01 boundaries)", "[unit][deadline]") {
    const DateTime d = fromIso8601("2026-11-07 23:59");
    using std::chrono::hours;
    using std::chrono::seconds;
    CHECK(DeadlineCalculator::daysLate(d, d) == 0);
    CHECK(DeadlineCalculator::daysLate(d + seconds(1), d) == 1);
    CHECK(DeadlineCalculator::daysLate(d + hours(24), d) == 1);
    CHECK(DeadlineCalculator::daysLate(d + hours(24) + seconds(1), d) == 2);
    CHECK(DeadlineCalculator::daysLate(d + hours(72) + seconds(1), d) == 4);
}
