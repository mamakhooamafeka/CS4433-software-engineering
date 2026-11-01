#include "services/MarkingService.hpp"

#include "services/Validation.hpp"

#include <cmath>

namespace lms {

void requireRawMark(double rawMark, int maxMark) {
    if (!std::isfinite(rawMark) || rawMark < 0.0 || rawMark > maxMark) {
        throw ValidationError("The mark must be between 0 and " + std::to_string(maxMark) + ".");
    }
}

MarkingService::Context MarkingService::load(const User& user, int submissionId) {
    auto sub = submissions_.findById(submissionId);
    if (!sub) throw ValidationError("There is no submission with that number.");
    auto a = assignments_.findById(sub->getAssignmentId());
    auto c = a ? courses_.findCourse(a->getCourseId()) : std::nullopt;
    if (!a || !c) throw ValidationError("There is no submission with that number.");
    requireCourseAccess(user, *c);
    return Context{*sub, *a};
}

// BR-01 applied to the latest version and the deadline in force now (BR-02).
Mark MarkingService::computeMark(const Context& ctx, double rawMark, const std::string& feedback,
                                 int markedBy) {
    requireRawMark(rawMark, ctx.assignment.getMaxMark());
    const DateTime deadline =
        deadlines_.effectiveDeadline(ctx.assignment, ctx.submission.getStudentId());
    Mark m;
    m.daysLate = DeadlineCalculator::daysLate(ctx.submission.latestVersion().submittedAt, deadline);
    const PenaltyResult p = policy_.apply(rawMark, ctx.assignment.getMaxMark(), m.daysLate);
    m.rawMark = roundToTenth(rawMark);
    m.deduction = p.deduction;
    m.finalMark = p.finalMark;
    m.feedback = requireText(feedback, "The feedback", 1, 2000);
    m.markedBy = markedBy;
    m.markedAt = clock_.now();
    return m;
}

std::vector<SubmissionView> MarkingService::listSubmissions(const Lecturer& lecturer,
                                                            int assignmentId, bool unmarkedOnly) {
    auto a = assignments_.findById(assignmentId);
    auto c = a ? courses_.findCourse(a->getCourseId()) : std::nullopt;
    if (!a || !c) throw ValidationError("There is no assignment with that number.");
    requireCourseAccess(lecturer, *c);

    std::vector<SubmissionStatus> statuses = {SubmissionStatus::SUBMITTED, SubmissionStatus::LATE};
    if (!unmarkedOnly) {
        statuses.insert(statuses.end(), {SubmissionStatus::MARKED, SubmissionStatus::MODERATED,
                                         SubmissionStatus::RELEASED});
    }
    std::vector<SubmissionView> out;
    for (SubmissionStatus st : statuses) {
        for (const Submission& s : submissions_.byAssignmentAndStatus(assignmentId, st)) {
            SubmissionView v;
            v.submissionId = s.getId();
            v.studentId = s.getStudentId();
            auto student = users_.findById(s.getStudentId());
            v.studentName = student ? student->getFullName() : "?";
            v.status = s.getStatus();
            v.latestSubmittedAt = s.latestVersion().submittedAt;
            v.versionCount = static_cast<int>(s.getVersions().size());
            v.filePath = s.latestVersion().storedPath;
            v.daysLate = DeadlineCalculator::daysLate(
                v.latestSubmittedAt, deadlines_.effectiveDeadline(*a, s.getStudentId()));
            v.mark = s.getMark();
            out.push_back(v);
        }
    }
    return out;
}

Mark MarkingService::preview(const Lecturer& lecturer, int submissionId, double rawMark,
                             const std::string& feedback) {
    const Context ctx = load(lecturer, submissionId);
    return computeMark(ctx, rawMark, feedback, lecturer.getId());
}

Mark MarkingService::mark(const Lecturer& lecturer, int submissionId, double rawMark,
                          const std::string& feedback) {
    Context ctx = load(lecturer, submissionId);
    const Mark m = computeMark(ctx, rawMark, feedback, lecturer.getId());
    Transaction tx(tx_);
    ctx.submission.recordMark(m);
    submissions_.save(ctx.submission);
    audit_.recordMarkChange(lecturer.getId(), ctx.submission, std::nullopt, m, ChangeType::INITIAL,
                            "Initial mark");
    tx.commit();
    return m;
}

Mark MarkingService::correct(const Lecturer& lecturer, int submissionId, double rawMark,
                             const std::string& feedback, const std::string& reason) {
    Context ctx = load(lecturer, submissionId);
    const std::string why = requireText(reason, "The reason", 1, 500);
    const auto oldMark = ctx.submission.getMark();
    const Mark m = computeMark(ctx, rawMark, feedback, lecturer.getId());
    Transaction tx(tx_);
    ctx.submission.correctMark(m);  // only while MARKED (BR-07)
    submissions_.save(ctx.submission);
    audit_.recordMarkChange(lecturer.getId(), ctx.submission, oldMark, m, ChangeType::CORRECTION,
                            why);
    tx.commit();
    return m;
}

}  // namespace lms
