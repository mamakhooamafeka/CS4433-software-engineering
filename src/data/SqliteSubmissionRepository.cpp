#include "data/SqliteRepositories.hpp"

namespace lms {
namespace {

const char* const kSubmissionColumns =
    "select id, assignment_id, student_id, status, raw_mark, deduction, final_mark, days_late, "
    "feedback, marked_by, marked_at, moderated_by, moderated_at, released_by, released_at "
    "from submission ";

} // namespace

std::vector<SubmissionVersion> SqliteSubmissionRepository::versionsOf(int submissionId) {
    Statement s = db_.prepare(
        "select version_no, stored_path, original_name, size_bytes, sha256, submitted_at "
        "from submission_version where submission_id = ? order by version_no");
    s.bind(1, submissionId);
    std::vector<SubmissionVersion> out;
    while (s.step()) {
        SubmissionVersion v;
        v.versionNo = s.getInt(0);
        v.storedPath = s.getText(1);
        v.originalName = s.getText(2);
        v.sizeBytes = s.getInt64(3);
        v.sha256 = s.getText(4);
        v.submittedAt = s.getDateTime(5);
        out.push_back(v);
    }
    return out;
}

std::vector<Submission> SqliteSubmissionRepository::loadAll(Statement& s) {
    struct Row {
        int id, assignmentId, studentId;
        SubmissionStatus status;
        std::optional<Mark> mark;
        std::optional<int> moderatedBy, releasedBy;
        std::optional<DateTime> moderatedAt, releasedAt;
    };
    // Read all rows first: versionsOf() needs its own statement on the same connection.
    std::vector<Row> rows;
    while (s.step()) {
        Row r{s.getInt(0), s.getInt(1), s.getInt(2), statusFromString(s.getText(3)), std::nullopt,
              s.getOptionalInt(11), s.getOptionalInt(13), s.getOptionalDateTime(12),
              s.getOptionalDateTime(14)};
        if (!s.isNull(4)) {
            Mark m;
            m.rawMark = s.getDouble(4);
            m.deduction = s.getDouble(5);
            m.finalMark = s.getDouble(6);
            m.daysLate = s.getInt(7);
            m.feedback = s.getText(8);
            m.markedBy = s.getInt(9);
            m.markedAt = s.getDateTime(10);
            r.mark = m;
        }
        rows.push_back(r);
    }
    std::vector<Submission> out;
    for (const Row& r : rows) {
        out.emplace_back(r.id, r.assignmentId, r.studentId, r.status, versionsOf(r.id), r.mark,
                         r.moderatedBy, r.moderatedAt, r.releasedBy, r.releasedAt);
    }
    return out;
}

std::optional<Submission> SqliteSubmissionRepository::find(int assignmentId, int studentId) {
    Statement s = db_.prepare(std::string(kSubmissionColumns) +
                              "where assignment_id = ? and student_id = ?");
    s.bind(1, assignmentId).bind(2, studentId);
    auto found = loadAll(s);
    if (found.empty()) return std::nullopt;
    return std::move(found.front());
}

std::optional<Submission> SqliteSubmissionRepository::findById(int id) {
    Statement s = db_.prepare(std::string(kSubmissionColumns) + "where id = ?");
    s.bind(1, id);
    auto found = loadAll(s);
    if (found.empty()) return std::nullopt;
    return std::move(found.front());
}

std::vector<Submission> SqliteSubmissionRepository::byAssignmentAndStatus(
    int assignmentId, SubmissionStatus status) {
    Statement s = db_.prepare(std::string(kSubmissionColumns) +
                              "where assignment_id = ? and status = ? order by id");
    s.bind(1, assignmentId).bind(2, toString(status));
    return loadAll(s);
}

std::vector<Submission> SqliteSubmissionRepository::byStudent(int studentId) {
    Statement s = db_.prepare(std::string(kSubmissionColumns) + "where student_id = ? order by id");
    s.bind(1, studentId);
    return loadAll(s);
}

bool SqliteSubmissionRepository::hasReleased(int assignmentId) {
    Statement s = db_.prepare(
        "select 1 from submission where assignment_id = ? and status = 'RELEASED' limit 1");
    s.bind(1, assignmentId);
    return s.step();
}

void SqliteSubmissionRepository::save(Submission& sub) {
    const auto& m = sub.getMark();
    const std::string sqlInsert =
        "insert into submission (status, raw_mark, deduction, final_mark, days_late, feedback, "
        "marked_by, marked_at, moderated_by, moderated_at, released_by, released_at, "
        "assignment_id, student_id) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    const std::string sqlUpdate =
        "update submission set status = ?, raw_mark = ?, deduction = ?, final_mark = ?, "
        "days_late = ?, feedback = ?, marked_by = ?, marked_at = ?, moderated_by = ?, "
        "moderated_at = ?, released_by = ?, released_at = ? where assignment_id = ? and "
        "student_id = ?";
    Statement s = db_.prepare(sub.getId() == 0 ? sqlInsert : sqlUpdate);
    s.bind(1, toString(sub.getStatus()));
    if (m) {
        s.bind(2, m->rawMark).bind(3, m->deduction).bind(4, m->finalMark).bind(5, m->daysLate);
        s.bind(6, m->feedback).bind(7, m->markedBy).bind(8, m->markedAt);
    } else {
        for (int i = 2; i <= 8; ++i) s.bindNull(i);
    }
    s.bind(9, sub.getModeratedBy()).bind(10, sub.getModeratedAt());
    s.bind(11, sub.getReleasedBy()).bind(12, sub.getReleasedAt());
    s.bind(13, sub.getAssignmentId()).bind(14, sub.getStudentId());
    s.run();
    if (sub.getId() == 0) sub.setId(static_cast<int>(db_.lastInsertId()));

    // Versions are append-only (BR-03), so only versions not yet stored are inserted.
    Statement last = db_.prepare(
        "select coalesce(max(version_no), 0) from submission_version where submission_id = ?");
    last.bind(1, sub.getId());
    last.step();
    const int stored = last.getInt(0);
    for (const SubmissionVersion& v : sub.getVersions()) {
        if (v.versionNo <= stored) continue;
        Statement ins = db_.prepare(
            "insert into submission_version (submission_id, version_no, stored_path, "
            "original_name, size_bytes, sha256, submitted_at) values (?, ?, ?, ?, ?, ?, ?)");
        ins.bind(1, sub.getId()).bind(2, v.versionNo).bind(3, v.storedPath);
        ins.bind(4, v.originalName).bind(5, v.sizeBytes).bind(6, v.sha256).bind(7, v.submittedAt);
        ins.run();
    }
}

} // namespace lms
