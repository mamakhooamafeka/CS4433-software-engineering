// SQLite implementations of the Repository interfaces (class diagram 2b).
// Every query in the program is in the matching .cpp files, and every one uses
// ? parameters (NFR-09).
#pragma once

#include "data/SqliteDatabase.hpp"
#include "domain/Repositories.hpp"

namespace lms {

class SqliteUserRepository : public IUserRepository {
public:
    explicit SqliteUserRepository(SqliteDatabase& db) : db_(db) {}
    std::unique_ptr<User> findByUsername(const std::string& username) override;
    std::unique_ptr<User> findById(int id) override;
    int insert(const User& user) override;
    void update(const User& user) override;
    std::vector<UserRecord> listAll() override;

private:
    std::unique_ptr<User> load(Statement& s);
    void loadScope(UserRecord& rec);
    SqliteDatabase& db_;
};

class SqliteCourseRepository : public ICourseRepository {
public:
    explicit SqliteCourseRepository(SqliteDatabase& db) : db_(db) {}
    std::optional<Course> findCourse(int id) override;
    std::optional<Department> findDepartment(int id) override;
    std::vector<Course> coursesOfLecturer(int lecturerId) override;
    std::vector<Course> coursesOfStudent(int studentId) override;
    std::vector<Course> coursesOfDepartment(int departmentId) override;
    std::vector<Department> listDepartments() override;
    std::vector<Course> listCourses() override;
    int insertDepartment(const std::string& name) override;
    void setHeadOfDepartment(int departmentId, int hodUserId) override;
    int insertCourse(const std::string& code, const std::string& title,
                     int departmentId) override;
    void addLecturer(int courseId, int lecturerId) override;
    void enrol(int courseId, int studentId, DateTime at) override;
    void withdraw(int courseId, int studentId) override;
    bool isEnrolled(int courseId, int studentId) override;

private:
    std::vector<Course> courses(Statement& s);
    SqliteDatabase& db_;
};

class SqliteAssignmentRepository : public IAssignmentRepository {
public:
    explicit SqliteAssignmentRepository(SqliteDatabase& db) : db_(db) {}
    std::optional<Assignment> findById(int id) override;
    std::vector<Assignment> byCourse(int courseId) override;
    int insert(const Assignment& a) override;
    std::vector<Extension> extensionsFor(int assignmentId, int studentId) override;
    void insertExtension(const Extension& e) override;

private:
    SqliteDatabase& db_;
};

class SqliteSubmissionRepository : public ISubmissionRepository {
public:
    explicit SqliteSubmissionRepository(SqliteDatabase& db) : db_(db) {}
    std::optional<Submission> find(int assignmentId, int studentId) override;
    std::optional<Submission> findById(int id) override;
    std::vector<Submission> byAssignmentAndStatus(int assignmentId,
                                                  SubmissionStatus status) override;
    std::vector<Submission> byStudent(int studentId) override;
    bool hasReleased(int assignmentId) override;
    void save(Submission& s) override;

private:
    std::vector<Submission> loadAll(Statement& s);
    std::vector<SubmissionVersion> versionsOf(int submissionId);
    SqliteDatabase& db_;
};

class SqliteAuditRepository : public IAuditRepository {
public:
    explicit SqliteAuditRepository(SqliteDatabase& db) : db_(db) {}
    void append(const AuditEntry& e) override;
    std::vector<AuditEntry> query(const AuditFilter& filter) override;

private:
    SqliteDatabase& db_;
};

} // namespace lms
