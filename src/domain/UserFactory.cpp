#include "domain/UserFactory.hpp"

#include "domain/Errors.hpp"
#include "domain/Roles.hpp"

namespace lms {

std::unique_ptr<User> UserFactory::create(const UserRecord& rec) {
    switch (rec.role) {
        case Role::STUDENT: return std::make_unique<Student>(rec, rec.courseIds);
        case Role::LECTURER: return std::make_unique<Lecturer>(rec, rec.courseIds);
        case Role::HOD: return std::make_unique<HeadOfDepartment>(rec, rec.departmentId);
        case Role::ADMIN: return std::make_unique<Administrator>(rec);
    }
    throw DataAccessError("The database contains an unknown role.");
}

std::unique_ptr<User> UserFactory::createNew(Role role, const std::string& username,
                                             const std::string& fullName,
                                             const std::string& passwordHash) {
    UserRecord rec;
    rec.role = role;
    rec.username = username;
    rec.fullName = fullName;
    rec.passwordHash = passwordHash;
    rec.active = true;
    rec.mustChangePassword = true;
    return create(rec);
}

} // namespace lms
