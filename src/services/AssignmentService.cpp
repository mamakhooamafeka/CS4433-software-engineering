#include "services/AssignmentService.hpp"

#include "services/Validation.hpp"

namespace lms {

Course AssignmentService::requireCourse(int courseId) {
    auto course = courses_.findCourse(courseId);
    if (!course) throw ValidationError("There is no course with that number.");
    return *course;
}

int AssignmentService::createAssignment(const Lecturer& lecturer, int courseId,
                                        const std::string& title,
                                        const std::string& instructions, DateTime deadline,
                                        int maxMark) {
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
    const std::string why = requireText(reason, "The reason", 1, 500);   // C3: mandatory
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
        submission->reevaluateLateness(newDeadline);   // DC-01
        submissions_.save(*submission);
    }
    tx.commit();
}

std::vector<AssignmentView> AssignmentService::listForStudent(const Student& student) {
    std::vector<AssignmentView> out;
    for (const Course& c : courses_.coursesOfStudent(student.getId())) {
        for (const Assignment& a : assignments_.byCourse(c.getId())) {
            AssignmentView v{a, c, deadlines_.effectiveDeadline(a, student.getId()), false,
                             std::nullopt, submissions_.hasReleased(a.getId())};
            v.hasExtension = v.effectiveDeadline != a.getDeadline();
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

} // namespace lms
