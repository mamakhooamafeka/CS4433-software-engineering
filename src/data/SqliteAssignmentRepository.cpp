#include "data/SqliteRepositories.hpp"

namespace lms {
namespace {

const char* const kAssignmentColumns =
    "select id, course_id, title, instructions, deadline, max_mark, created_by, created_at "
    "from assignment ";

Assignment readAssignment(const Statement& s) {
    return Assignment(s.getInt(0), s.getInt(1), s.getText(2), s.getText(3), s.getDateTime(4),
                      s.getInt(5), s.getInt(6), s.getDateTime(7));
}

} // namespace

std::optional<Assignment> SqliteAssignmentRepository::findById(int id) {
    Statement s = db_.prepare(std::string(kAssignmentColumns) + "where id = ?");
    s.bind(1, id);
    if (!s.step()) return std::nullopt;
    return readAssignment(s);
}

std::vector<Assignment> SqliteAssignmentRepository::byCourse(int courseId) {
    Statement s = db_.prepare(std::string(kAssignmentColumns) +
                              "where course_id = ? order by deadline");
    s.bind(1, courseId);
    std::vector<Assignment> out;
    while (s.step()) out.push_back(readAssignment(s));
    return out;
}

int SqliteAssignmentRepository::insert(const Assignment& a) {
    Statement s = db_.prepare(
        "insert into assignment (course_id, title, instructions, deadline, max_mark, created_by, "
        "created_at) values (?, ?, ?, ?, ?, ?, ?)");
    s.bind(1, a.getCourseId()).bind(2, a.getTitle()).bind(3, a.getInstructions());
    s.bind(4, a.getDeadline()).bind(5, a.getMaxMark()).bind(6, a.getCreatedBy());
    s.bind(7, a.getCreatedAt());
    s.run();
    return static_cast<int>(db_.lastInsertId());
}

std::vector<Extension> SqliteAssignmentRepository::extensionsFor(int assignmentId, int studentId) {
    Statement s = db_.prepare(
        "select id, assignment_id, student_id, new_deadline, reason, granted_by, granted_at "
        "from extension where assignment_id = ? and student_id = ? order by granted_at");
    s.bind(1, assignmentId).bind(2, studentId);
    std::vector<Extension> out;
    while (s.step()) {
        Extension e;
        e.id = s.getInt(0);
        e.assignmentId = s.getInt(1);
        e.studentId = s.getInt(2);
        e.newDeadline = s.getDateTime(3);
        e.reason = s.getText(4);
        e.grantedBy = s.getInt(5);
        e.grantedAt = s.getDateTime(6);
        out.push_back(e);
    }
    return out;
}

void SqliteAssignmentRepository::insertExtension(const Extension& e) {
    Statement s = db_.prepare(
        "insert into extension (assignment_id, student_id, new_deadline, reason, granted_by, "
        "granted_at) values (?, ?, ?, ?, ?, ?)");
    s.bind(1, e.assignmentId).bind(2, e.studentId).bind(3, e.newDeadline).bind(4, e.reason);
    s.bind(5, e.grantedBy).bind(6, e.grantedAt);
    s.run();
}

} // namespace lms
