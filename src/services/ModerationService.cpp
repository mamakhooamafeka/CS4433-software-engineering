#include "services/ModerationService.hpp"

#include "services/MarkingService.hpp"
#include "services/Validation.hpp"

namespace lms {

int ModerationService::requireDepartment(const HeadOfDepartment& hod) {
    requireRole(hod, Role::HOD);
    const std::optional<int> dept = hod.getDepartmentId();
    if (!dept) {
        throw BusinessRuleError("You are not assigned to a department yet. Please see the admin.");
    }
    return *dept;
}

Assignment ModerationService::requireAssignment(const HeadOfDepartment& hod, int assignmentId) {
    requireDepartment(hod);
    auto a = assignments_.findById(assignmentId);
    auto c = a ? courses_.findCourse(a->getCourseId()) : std::nullopt;
    if (!a || !c) throw ValidationError("There is no assignment with that number.");
    requireCourseAccess(hod, *c);  // BR-07: own department only
    return *a;
}

std::vector<AssignmentSummary> ModerationService::listDepartmentAssignments(
    const HeadOfDepartment& hod) {
    std::vector<AssignmentSummary> out;
    for (const Course& c : courses_.coursesOfDepartment(requireDepartment(hod))) {
        for (const Assignment& a : assignments_.byCourse(c.getId())) {
            AssignmentSummary s{a, c.getCode()};
            auto count = [&](SubmissionStatus st) {
                return static_cast<int>(submissions_.byAssignmentAndStatus(a.getId(), st).size());
            };
            s.marked = count(SubmissionStatus::MARKED);
            s.moderated = count(SubmissionStatus::MODERATED);
            s.released = count(SubmissionStatus::RELEASED);
            s.unmarked = count(SubmissionStatus::SUBMITTED) + count(SubmissionStatus::LATE);
            out.push_back(s);
        }
    }
    return out;
}

std::vector<SubmissionView> ModerationService::listWithStatus(
    const HeadOfDepartment& hod, int assignmentId,
    std::initializer_list<SubmissionStatus> statuses) {
    requireAssignment(hod, assignmentId);
    std::vector<SubmissionView> out;
    for (SubmissionStatus st : statuses) {
        for (const Submission& s : submissions_.byAssignmentAndStatus(assignmentId, st)) {
            SubmissionView v;
            v.submissionId = s.getId();
            v.studentId = s.getStudentId();
            v.studentName = nameOf(s.getStudentId());
            v.status = s.getStatus();
            v.latestSubmittedAt = s.latestVersion().submittedAt;
            v.versionCount = static_cast<int>(s.getVersions().size());
            v.mark = s.getMark();
            v.daysLate = v.mark ? v.mark->daysLate : 0;
            out.push_back(v);
        }
    }
    return out;
}

std::vector<SubmissionView> ModerationService::listMarked(const HeadOfDepartment& hod,
                                                          int assignmentId) {
    return listWithStatus(hod, assignmentId, {SubmissionStatus::MARKED});
}

std::vector<SubmissionView> ModerationService::listAmendable(const HeadOfDepartment& hod,
                                                             int assignmentId) {
    return listWithStatus(hod, assignmentId,
                          {SubmissionStatus::MARKED, SubmissionStatus::RELEASED});
}

std::string ModerationService::nameOf(int userId) {
    auto u = users_.findById(userId);
    return u ? u->getFullName() : "user " + std::to_string(userId);
}

void ModerationService::approve(const HeadOfDepartment& hod,
                                const std::vector<int>& submissionIds) {
    requireDepartment(hod);
    Transaction tx(tx_);
    for (int id : submissionIds) {
        auto s = submissions_.findById(id);
        if (!s) throw ValidationError("There is no submission number " + std::to_string(id) + ".");
        requireAssignment(hod, s->getAssignmentId());
        s->approve(hod.getId(), clock_.now());
        submissions_.save(*s);
    }
    tx.commit();
}

Mark ModerationService::amend(const HeadOfDepartment& hod, int submissionId, double rawMark,
                              const std::string& reason) {
    auto s = submissions_.findById(submissionId);
    if (!s) throw ValidationError("There is no submission with that number.");
    const Assignment a = requireAssignment(hod, s->getAssignmentId());
    const std::string why = requireText(reason, "The reason", 1, 500);  // A6: mandatory
    const std::optional<Mark> current = s->getMark();
    if (!current) throw BusinessRuleError("This submission has not been marked yet.");
    requireRawMark(rawMark, a.getMaxMark());

    const Mark& oldMark = *current;
    Mark m = oldMark;  // keeps the lecturer's feedback and daysLate
    const PenaltyResult p = policy_.apply(rawMark, a.getMaxMark(), oldMark.daysLate);
    m.rawMark = roundToTenth(rawMark);
    m.deduction = p.deduction;
    m.finalMark = p.finalMark;

    Transaction tx(tx_);
    s->amend(m, hod.getId(), clock_.now());
    submissions_.save(*s);
    audit_.recordMarkChange(hod.getId(), *s, oldMark, m, ChangeType::AMENDMENT, why);
    tx.commit();
    return m;
}

ReleasePreview ModerationService::releasePreview(const HeadOfDepartment& hod, int assignmentId) {
    const Assignment a = requireAssignment(hod, assignmentId);
    ReleasePreview p;
    p.assignmentTitle = a.getTitle();
    p.toRelease = static_cast<int>(
        submissions_.byAssignmentAndStatus(assignmentId, SubmissionStatus::MODERATED).size());
    for (SubmissionStatus st :
         {SubmissionStatus::SUBMITTED, SubmissionStatus::LATE, SubmissionStatus::MARKED}) {
        p.notReady += static_cast<int>(submissions_.byAssignmentAndStatus(assignmentId, st).size());
    }
    return p;
}

int ModerationService::releaseAll(const HeadOfDepartment& hod, int assignmentId) {
    requireAssignment(hod, assignmentId);
    auto ready = submissions_.byAssignmentAndStatus(assignmentId, SubmissionStatus::MODERATED);
    if (ready.empty()) {
        throw BusinessRuleError("There are no moderated submissions to release.");
    }
    const DateTime now = clock_.now();
    Transaction tx(tx_);  // all released or none (FR-17)
    for (Submission& s : ready) {
        s.release(hod.getId(), now);
        submissions_.save(s);
    }
    tx.commit();
    return static_cast<int>(ready.size());
}

std::vector<AuditEntry> ModerationService::viewAudit(const HeadOfDepartment& hod,
                                                     AuditFilter filter) {
    filter.departmentId = requireDepartment(hod);
    return auditLog_.query(filter);
}

}  // namespace lms
