#include "services/AssignmentService.hpp"

#include "services/Validation.hpp"

namespace lms {

Course AssignmentService::requireCourse(int courseId) {
    auto course = courses_.findCourse(courseId);
    if (!course) throw ValidationError("There is no course with that number.");
    return *course;
}

int AssignmentService::createAssignment(const Lecturer& lecturer, int courseId,
                                        const std::string& title, const std::string& instructions,
                                        DateTime deadline, int maxMark) {
    requireCourseAccess(lecturer, requireCourse(courseId));
    const std::string t = requireText(title, "The title", 1, 200);
    const std::string i = requireText(instructions, "The instructions", 1, 5000);
    if (maxMark < Assignment::kMinMaxMark || maxMark > Assignment::kMaxMaxMark) {
        throw ValidationError("The maximum mark must be between 1 and 1000.");
    }
    const DateTime now = clock_.now();
    if (deadline <= now) {
        throw ValidationError("The deadline must be in the future.");
    }
    return assignments_.insert(
        Assignment(0, courseId, t, i, deadline, maxMark, lecturer.getId(), now));
}

void AssignmentService::grantExtension(const Lecturer& lecturer, int assignmentId, int studentId,
                                       DateTime newDeadline, const std::string& reason) {
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw ValidationError("There is no assignment with that number.");
    requireCourseAccess(lecturer, requireCourse(assignment->getCourseId()));
    if (!courses_.isEnrolled(assignment->getCourseId(), studentId)) {
        throw ValidationError("That student is not enrolled in this course.");
    }
    const std::string why = requireText(reason, "The reason", 1, 500);  // C3: mandatory
    if (newDeadline <= deadlines_.effectiveDeadline(*assignment, studentId)) {
        throw BusinessRuleError("An extension can only move the deadline later (BR-02).");
    }
    auto submission = submissions_.find(assignmentId, studentId);
    if (submission && submission->getStatus() != SubmissionStatus::SUBMITTED &&
        submission->getStatus() != SubmissionStatus::LATE) {
        throw BusinessRuleError("This submission is already marked, so it cannot be extended.");
    }

    Extension e;
    e.assignmentId = assignmentId;
    e.studentId = studentId;
    e.newDeadline = newDeadline;
    e.reason = why;
    e.grantedBy = lecturer.getId();
    e.grantedAt = clock_.now();

    Transaction tx(tx_);
    assignments_.insertExtension(e);
    if (submission) {
        submission->reevaluateLateness(newDeadline);  // DC-01
        submissions_.save(*submission);
    }
    tx.commit();
}

Assignment AssignmentService::requireAssignment(const Lecturer& lecturer, int assignmentId) {
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw ValidationError("There is no assignment with that number.");
    requireCourseAccess(lecturer, requireCourse(assignment->getCourseId()));
    return *assignment;
}

DateTime AssignmentService::classDeadline(const Lecturer& lecturer, int assignmentId) {
    return deadlines_.classDeadline(requireAssignment(lecturer, assignmentId));
}

ClassExtensionResult AssignmentService::grantClassExtension(const Lecturer& lecturer,
                                                            int assignmentId, DateTime newDeadline,
                                                            const std::string& reason) {
    const Assignment a = requireAssignment(lecturer, assignmentId);
    const std::string why = requireText(reason, "The reason", 1, 500);  // C3: mandatory
    if (submissions_.hasReleased(assignmentId)) {
        throw BusinessRuleError(
            "Marks for this assignment are released, so it cannot be extended (BR-06).");
    }
    const DateTime oldClassDeadline = deadlines_.classDeadline(a);
    if (newDeadline <= oldClassDeadline) {
        throw BusinessRuleError("An extension can only move the deadline later (BR-02).");
    }

    ClassExtension e;
    e.assignmentId = assignmentId;
    e.newDeadline = newDeadline;
    e.reason = why;
    e.grantedBy = lecturer.getId();
    e.grantedAt = clock_.now();

    ClassExtensionResult result;
    result.studentsInClass = static_cast<int>(users_.studentsOfCourse(a.getCourseId()).size());

    Transaction tx(tx_);
    assignments_.insertClassExtension(e);
    // DC-01 for everyone: each student's deadline is now the later of the new class deadline
    // and their own extension, so a student with a longer extension keeps it.
    for (Submission& s : submissions_.byAssignmentAndStatus(assignmentId, SubmissionStatus::LATE)) {
        s.reevaluateLateness(deadlines_.effectiveDeadline(a, s.getStudentId()));
        if (s.getStatus() == SubmissionStatus::SUBMITTED) {
            submissions_.save(s);
            ++result.nowOnTime;
        }
    }
    tx.commit();

    // Marked work is never changed by an extension (DC-02). Count the marks that carry a late
    // deduction although the work is on time under the new deadline, so the lecturer can
    // correct them (UC20) or ask the HoD to amend them (UC13).
    for (const SubmissionStatus st : {SubmissionStatus::MARKED, SubmissionStatus::MODERATED}) {
        for (const Submission& s : submissions_.byAssignmentAndStatus(assignmentId, st)) {
            const std::optional<Mark>& m = s.getMark();
            if (m.has_value() && m->daysLate > 0 &&
                s.latestVersion().submittedAt <= deadlines_.effectiveDeadline(a, s.getStudentId())) {
                ++result.markedAndLate;
            }
        }
    }
    return result;
}

std::vector<AssignmentView> AssignmentService::listForStudent(const Student& student) {
    std::vector<AssignmentView> out;
    for (const Course& c : courses_.coursesOfStudent(student.getId())) {
        for (const Assignment& a : assignments_.byCourse(c.getId())) {
            const EffectiveDeadline d = deadlines_.deadlineFor(a, student.getId());
            AssignmentView v{a,           c,       d.deadline, std::nullopt,
                             submissions_.hasReleased(a.getId()), d.scope, d.classReason};
            if (auto s = submissions_.find(a.getId(), student.getId())) {
                v.status = s->getStatus();
            }
            out.push_back(v);
        }
    }
    return out;
}

std::vector<Course> AssignmentService::coursesOf(const Lecturer& lecturer) {
    return courses_.coursesOfLecturer(lecturer.getId());
}

std::vector<Assignment> AssignmentService::assignmentsOf(const User& user, int courseId) {
    requireCourseAccess(user, requireCourse(courseId));
    return assignments_.byCourse(courseId);
}

std::vector<UserRecord> AssignmentService::studentsOf(const Lecturer& lecturer, int courseId) {
    requireCourseAccess(lecturer, requireCourse(courseId));
    return users_.studentsOfCourse(courseId);
}

}  // namespace lms
