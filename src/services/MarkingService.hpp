// UC11 Mark submission and UC20 Correct mark (FR-13, FR-19). The late penalty comes
// from the injected ILatePenaltyPolicy (Strategy), never from code in this class.
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

class MarkingService {
public:
    MarkingService(IAssignmentRepository& assignments, ICourseRepository& courses,
                   ISubmissionRepository& submissions, IUserRepository& users,
                   ILatePenaltyPolicy& policy, AuditService& audit, IClock& clock,
                   ITransactionManager& tx)
        : assignments_(assignments),
          courses_(courses),
          submissions_(submissions),
          users_(users),
          policy_(policy),
          audit_(audit),
          clock_(clock),
          tx_(tx),
          deadlines_(assignments) {}

    std::vector<SubmissionView> listSubmissions(const Lecturer& lecturer, int assignmentId,
                                                bool unmarkedOnly);
    // Works out the mark (with the late penalty) without saving it, so the lecturer
    // can see raw, deduction and final mark before confirming (UC11 steps 6-7).
    Mark preview(const Lecturer& lecturer, int submissionId, double rawMark,
                 const std::string& feedback);
    Mark mark(const Lecturer& lecturer, int submissionId, double rawMark,
              const std::string& feedback);
    Mark correct(const Lecturer& lecturer, int submissionId, double rawMark,
                 const std::string& feedback, const std::string& reason);

private:
    struct Context {
        Submission submission;
        Assignment assignment;
    };
    Context load(const User& user, int submissionId);
    Mark computeMark(const Context& ctx, double rawMark, const std::string& feedback, int markedBy);

    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    ISubmissionRepository& submissions_;
    IUserRepository& users_;
    ILatePenaltyPolicy& policy_;
    AuditService& audit_;
    IClock& clock_;
    ITransactionManager& tx_;
    DeadlineCalculator deadlines_;
};

// Shared by MarkingService and ModerationService: checks 0 <= raw <= max (FR-13).
void requireRawMark(double rawMark, int maxMark);

}  // namespace lms
