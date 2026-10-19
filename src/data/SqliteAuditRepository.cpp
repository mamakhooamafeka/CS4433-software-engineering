#include "data/SqliteRepositories.hpp"

namespace lms {

// Append-only: there is no update or delete method, and database triggers refuse
// both even from outside the program (BR-05).
void SqliteAuditRepository::append(const AuditEntry& e) {
    Statement s = db_.prepare(
        "insert into mark_audit (submission_id, changed_by, changed_at, change_type, "
        "old_raw_mark, old_final_mark, new_raw_mark, new_final_mark, reason) "
        "values (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    s.bind(1, e.submissionId).bind(2, e.changedBy).bind(3, e.changedAt);
    s.bind(4, toString(e.changeType)).bind(5, e.oldRawMark).bind(6, e.oldFinalMark);
    s.bind(7, e.newRawMark).bind(8, e.newFinalMark).bind(9, e.reason);
    s.run();
}

// Optional filters use "(? is null or column = ?)", so the SQL text never changes
// and every value is still a bound parameter (NFR-09).
std::vector<AuditEntry> SqliteAuditRepository::query(const AuditFilter& f) {
    Statement s = db_.prepare(
        "select m.id, m.submission_id, m.changed_by, m.changed_at, m.change_type, "
        "m.old_raw_mark, m.old_final_mark, m.new_raw_mark, m.new_final_mark, m.reason "
        "from mark_audit m "
        "join submission s on s.id = m.submission_id "
        "join assignment a on a.id = s.assignment_id "
        "join course c on c.id = a.course_id "
        "where c.department_id = ? "
        "and (? is null or c.id = ?) "
        "and (? is null or a.id = ?) "
        "and (? is null or s.student_id = ?) "
        "order by m.changed_at, m.id");
    s.bind(1, f.departmentId);
    s.bind(2, f.courseId).bind(3, f.courseId);
    s.bind(4, f.assignmentId).bind(5, f.assignmentId);
    s.bind(6, f.studentId).bind(7, f.studentId);
    std::vector<AuditEntry> out;
    while (s.step()) {
        AuditEntry e;
        e.id = s.getInt(0);
        e.submissionId = s.getInt(1);
        e.changedBy = s.getInt(2);
        e.changedAt = s.getDateTime(3);
        e.changeType = changeTypeFromString(s.getText(4));
        e.oldRawMark = s.getOptionalDouble(5);
        e.oldFinalMark = s.getOptionalDouble(6);
        e.newRawMark = s.getDouble(7);
        e.newFinalMark = s.getDouble(8);
        e.reason = s.getText(9);
        out.push_back(e);
    }
    return out;
}

}  // namespace lms
