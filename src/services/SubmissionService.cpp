#include "services/SubmissionService.hpp"

#include "services/Validation.hpp"

namespace lms {

std::vector<AssignmentView> SubmissionService::listOpenAssignments(const Student& student) {
    std::vector<AssignmentView> out;
    for (const Course& c : courses_.coursesOfStudent(student.getId())) {
        for (const Assignment& a : assignments_.byCourse(c.getId())) {
            if (submissions_.hasReleased(a.getId())) continue;
            AssignmentView v{a, c, deadlines_.effectiveDeadline(a, student.getId()), false,
                             std::nullopt, false};
            v.hasExtension = v.effectiveDeadline != a.getDeadline();
            if (auto s = submissions_.find(a.getId(), student.getId())) v.status = s->getStatus();
            out.push_back(v);
        }
    }
    return out;
}

Assignment SubmissionService::requireOpenAssignment(const Student& student, int assignmentId) {
    auto a = assignments_.findById(assignmentId);
    if (!a) throw ValidationError("There is no assignment with that number.");
    auto course = courses_.findCourse(a->getCourseId());
    if (!course) throw ValidationError("There is no assignment with that number.");
    requireCourseAccess(student, *course);
    if (submissions_.hasReleased(assignmentId)) {
        throw BusinessRuleError("Marks for this assignment are released, so it is closed (BR-06).");
    }
    return *a;
}

SubmissionPreview SubmissionService::preview(const Student& student, int assignmentId,
                                             const std::string& path) {
    const Assignment a = requireOpenAssignment(student, assignmentId);
    files_.validate(path);
    SubmissionPreview p;
    p.assignmentTitle = a.getTitle();
    p.effectiveDeadline = deadlines_.effectiveDeadline(a, student.getId());
    p.now = clock_.now();
    p.daysLate = DeadlineCalculator::daysLate(p.now, p.effectiveDeadline);
    if (auto existing = submissions_.find(assignmentId, student.getId())) {
        p.isResubmission = true;
        p.currentVersion = existing->latestVersion().versionNo;
        p.currentStatus = existing->getStatus();
        p.canResubmit = existing->canResubmit(p.now, p.effectiveDeadline);
    }
    return p;
}

Receipt SubmissionService::submit(const Student& student, int assignmentId,
                                  const std::string& path) {
    const Assignment a = requireOpenAssignment(student, assignmentId);
    const DateTime deadline = deadlines_.effectiveDeadline(a, student.getId());
    const DateTime now = clock_.now();   // server time is the submission time (C2)

    Submission sub = submissions_.find(assignmentId, student.getId())
                         .value_or(Submission(assignmentId, student.getId()));
    const bool isNew = sub.getVersions().empty();
    if (!isNew && !sub.canResubmit(now, deadline)) {
        throw BusinessRuleError(
            "Resubmission is not allowed: the deadline has passed or the work is marked (BR-03).");
    }
    const int versionNo = isNew ? 1 : sub.latestVersion().versionNo + 1;

    Transaction tx(tx_);
    const StoredFile stored = files_.store(path, assignmentId, student.getId(), versionNo);
    try {
        SubmissionVersion v;
        v.storedPath = stored.storedPath;
        v.originalName = stored.originalName;
        v.sizeBytes = stored.sizeBytes;
        v.sha256 = stored.sha256;
        v.submittedAt = now;
        sub.addVersion(v, deadline);
        submissions_.save(sub);
        tx.commit();
    } catch (...) {
        files_.remove(stored.storedPath);
        throw;
    }

    Receipt r;
    r.assignmentTitle = a.getTitle();
    r.submittedAt = now;
    r.versionNo = versionNo;
    r.originalName = stored.originalName;
    r.sizeBytes = stored.sizeBytes;
    r.sha256 = stored.sha256;
    r.status = sub.getStatus();
    return r;
}

std::vector<ResultView> SubmissionService::listReleasedResults(const Student& student) {
    std::vector<ResultView> out;
    for (const Submission& s : submissions_.byStudent(student.getId())) {
        if (s.getStatus() != SubmissionStatus::RELEASED || !s.getMark()) continue;
        auto a = assignments_.findById(s.getAssignmentId());
        if (!a) continue;
        auto c = courses_.findCourse(a->getCourseId());
        out.push_back(ResultView{c ? c->getCode() : "", a->getTitle(), a->getMaxMark(),
                                 *s.getMark()});
    }
    return out;
}

} // namespace lms
