// UC12 Moderate and release marks, UC13 Amend mark and UC14 View audit log
// (FR-15, FR-16, FR-17, FR-20). Follows sequence diagram Figure 6.
#pragma once

#include "domain/DeadlineCalculator.hpp"
#include "domain/Infrastructure.hpp"
#include "domain/LatePenalty.hpp"
#include "domain/Repositories.hpp"
#include "domain/Roles.hpp"
#include "services/AuditService.hpp"
#include "services/Views.hpp"

#include <vector>

namespace lms {

class ModerationService {
public:
    ModerationService(IAssignmentRepository& assignments, ICourseRepository& courses,
                      ISubmissionRepository& submissions, IUserRepository& users,
                      IAuditRepository& auditLog, ILatePenaltyPolicy& policy, AuditService& audit,
                      IClock& clock, ITransactionManager& tx)
        : assignments_(assignments),
          courses_(courses),
          submissions_(submissions),
          users_(users),
          auditLog_(auditLog),
          policy_(policy),
          audit_(audit),
          clock_(clock),
          tx_(tx),
          deadlines_(assignments) {}

    std::vector<AssignmentSummary> listDepartmentAssignments(const HeadOfDepartment& hod);
    std::vector<SubmissionView> listMarked(const HeadOfDepartment& hod, int assignmentId);
    // Approves several submissions in one transaction: all or none.
    void approve(const HeadOfDepartment& hod, const std::vector<int>& submissionIds);
    // FR-16: changes the raw mark, re-applies BR-01, keeps the lecturer's feedback.
    Mark amend(const HeadOfDepartment& hod, int submissionId, double rawMark,
               const std::string& reason);
    ReleasePreview releasePreview(const HeadOfDepartment& hod, int assignmentId);
    // FR-17: releases every MODERATED submission of the assignment atomically.
    int releaseAll(const HeadOfDepartment& hod, int assignmentId);
    // FR-20: always limited to the HoD's own department.
    std::vector<AuditEntry> viewAudit(const HeadOfDepartment& hod, AuditFilter filter);

private:
    int requireDepartment(const HeadOfDepartment& hod);
    Assignment requireAssignment(const HeadOfDepartment& hod, int assignmentId);

    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    ISubmissionRepository& submissions_;
    IUserRepository& users_;
    IAuditRepository& auditLog_;
    ILatePenaltyPolicy& policy_;
    AuditService& audit_;
    IClock& clock_;
    ITransactionManager& tx_;
    DeadlineCalculator deadlines_;
};

} // namespace lms
