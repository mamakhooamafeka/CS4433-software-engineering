// Repository/DAO interfaces (class diagram 2b). Services depend on these, never on
// SQLite, so the database can be swapped or replaced by fakes in tests
// (dependency inversion). The Sqlite* classes in src/data implement them.
#pragma once

#include "domain/Assignment.hpp"
#include "domain/Course.hpp"
#include "domain/Mark.hpp"
#include "domain/Submission.hpp"
#include "domain/User.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace lms {

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual std::unique_ptr<User> findByUsername(const std::string& username) = 0;
    virtual std::unique_ptr<User> findById(int id) = 0;
    virtual int insert(const User& user) = 0;
    virtual void update(const User& user) = 0;
    virtual std::vector<UserRecord> listAll() = 0;
    // Active enrolled students of a course, for choosing who gets an extension.
    virtual std::vector<UserRecord> studentsOfCourse(int courseId) = 0;
};

class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;
    virtual std::optional<Course> findCourse(int id) = 0;
    virtual std::optional<Department> findDepartment(int id) = 0;
    virtual std::vector<Course> coursesOfLecturer(int lecturerId) = 0;
    virtual std::vector<Course> coursesOfStudent(int studentId) = 0;
    virtual std::vector<Course> coursesOfDepartment(int departmentId) = 0;
    virtual std::vector<Department> listDepartments() = 0;
    virtual std::vector<Course> listCourses() = 0;
    virtual int insertDepartment(const std::string& name) = 0;
    virtual void setHeadOfDepartment(int departmentId, int hodUserId) = 0;
    virtual int insertCourse(const std::string& code, const std::string& title,
                             int departmentId) = 0;
    virtual void addLecturer(int courseId, int lecturerId) = 0;
    virtual void enrol(int courseId, int studentId, DateTime at) = 0;
    virtual void withdraw(int courseId, int studentId) = 0;
    virtual bool isEnrolled(int courseId, int studentId) = 0;
};

class IAssignmentRepository {
public:
    virtual ~IAssignmentRepository() = default;
    virtual std::optional<Assignment> findById(int id) = 0;
    virtual std::vector<Assignment> byCourse(int courseId) = 0;
    virtual int insert(const Assignment& a) = 0;
    virtual std::vector<Extension> extensionsFor(int assignmentId, int studentId) = 0;
    virtual void insertExtension(const Extension& e) = 0;
};

class ISubmissionRepository {
public:
    virtual ~ISubmissionRepository() = default;
    virtual std::optional<Submission> find(int assignmentId, int studentId) = 0;
    virtual std::optional<Submission> findById(int id) = 0;
    virtual std::vector<Submission> byAssignmentAndStatus(int assignmentId,
                                                          SubmissionStatus status) = 0;
    virtual std::vector<Submission> byStudent(int studentId) = 0;
    virtual bool hasReleased(int assignmentId) = 0;
    // Inserts or updates the submission and any new versions; sets the id when inserting.
    virtual void save(Submission& s) = 0;
};

class IAuditRepository {
public:
    virtual ~IAuditRepository() = default;
    virtual void append(const AuditEntry& e) = 0;
    virtual std::vector<AuditEntry> query(const AuditFilter& filter) = 0;
};

// Groups several repository calls into one all-or-nothing database transaction.
class ITransactionManager {
public:
    virtual ~ITransactionManager() = default;
    virtual void begin() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
};

// RAII helper: rolls back automatically unless commit() was reached, so an exception
// in the middle of a release or submission leaves the database unchanged (NFR-05).
class Transaction {
public:
    explicit Transaction(ITransactionManager& tm) : tm_(tm) { tm_.begin(); }
    ~Transaction() {
        if (!done_) {
            try {
                tm_.rollback();
            } catch (...) {
                // Never throw from a destructor.
            }
        }
    }
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    void commit() {
        tm_.commit();
        done_ = true;
    }

private:
    ITransactionManager& tm_;
    bool done_ = false;
};

}  // namespace lms
