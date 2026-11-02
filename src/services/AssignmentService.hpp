// UC09 Create assignment, UC10 Grant extension and UC15 View assignments
// (FR-08, FR-09, FR-10, and FR-24 from change request CR1 in v1.1).
#pragma once

#include "domain/DeadlineCalculator.hpp"
#include "domain/Infrastructure.hpp"
#include "domain/Repositories.hpp"
#include "domain/Roles.hpp"
#include "services/Views.hpp"

#include <vector>

namespace lms {

class AssignmentService {
public:
    AssignmentService(IAssignmentRepository& assignments, ICourseRepository& courses,
                      ISubmissionRepository& submissions, IUserRepository& users, IClock& clock,
                      ITransactionManager& tx)
        : assignments_(assignments),
          courses_(courses),
          submissions_(submissions),
          users_(users),
          clock_(clock),
          tx_(tx),
          deadlines_(assignments) {}

    int createAssignment(const Lecturer& lecturer, int courseId, const std::string& title,
                         const std::string& instructions, DateTime deadline, int maxMark);

    // FR-09 with DC-01 and DC-02: allowed before submission or while unmarked, and only
    // to move the deadline later (BR-02). A LATE submission is re-evaluated.
    void grantExtension(const Lecturer& lecturer, int assignmentId, int studentId,
                        DateTime newDeadline, const std::string& reason);

    // CR1 (FR-24): one later deadline for the whole class. It must be later than the current
    // class deadline (BR-02) and is refused once marks are released (BR-06). LATE submissions
    // are re-evaluated; marked work keeps its mark and is counted for the lecturer instead.
    ClassExtensionResult grantClassExtension(const Lecturer& lecturer, int assignmentId,
                                             DateTime newDeadline, const std::string& reason);

    // CR1: the class deadline in force now, shown before the lecturer types a new one.
    DateTime classDeadline(const Lecturer& lecturer, int assignmentId);

    std::vector<AssignmentView> listForStudent(const Student& student);
    std::vector<Course> coursesOf(const Lecturer& lecturer);
    std::vector<Assignment> assignmentsOf(const User& user, int courseId);
    std::vector<UserRecord> studentsOf(const Lecturer& lecturer, int courseId);

private:
    Course requireCourse(int courseId);
    Assignment requireAssignment(const Lecturer& lecturer, int assignmentId);

    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    ISubmissionRepository& submissions_;
    IUserRepository& users_;
    IClock& clock_;
    ITransactionManager& tx_;
    DeadlineCalculator deadlines_;
};

}  // namespace lms
