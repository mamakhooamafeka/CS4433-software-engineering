// UC16 Submit assignment, UC17 Resubmit and UC18 View released results
// (FR-11, FR-12, FR-14, FR-18). Follows sequence diagram Figure 5.
#pragma once

#include "domain/DeadlineCalculator.hpp"
#include "domain/Infrastructure.hpp"
#include "domain/Repositories.hpp"
#include "domain/Roles.hpp"
#include "services/Views.hpp"

#include <vector>

namespace lms {

class SubmissionService {
public:
    SubmissionService(IAssignmentRepository& assignments, ICourseRepository& courses,
                      ISubmissionRepository& submissions, IFileStore& files, IClock& clock,
                      ITransactionManager& tx)
        : assignments_(assignments),
          courses_(courses),
          submissions_(submissions),
          files_(files),
          clock_(clock),
          tx_(tx),
          deadlines_(assignments) {}

    // Assignments of enrolled courses whose marks are not yet released (BR-06).
    std::vector<AssignmentView> listOpenAssignments(const Student& student);

    // Checks everything and shows lateness before the student confirms (flows 1-8).
    SubmissionPreview preview(const Student& student, int assignmentId, const std::string& path);

    // Stores the file and the submission in one transaction; on a database error the
    // copied file is removed again, so nothing half-saved remains (flow 9a).
    Receipt submit(const Student& student, int assignmentId, const std::string& path);

    // BR-04: only RELEASED submissions are ever returned to a student.
    std::vector<ResultView> listReleasedResults(const Student& student);

private:
    Assignment requireOpenAssignment(const Student& student, int assignmentId);

    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    ISubmissionRepository& submissions_;
    IFileStore& files_;
    IClock& clock_;
    ITransactionManager& tx_;
    DeadlineCalculator deadlines_;
};

} // namespace lms
