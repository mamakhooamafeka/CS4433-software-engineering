// BR-02 (effective deadline) and the days-late part of BR-01.
#pragma once

#include "domain/Assignment.hpp"
#include "domain/Repositories.hpp"

namespace lms {

class DeadlineCalculator {
public:
    explicit DeadlineCalculator(IAssignmentRepository& assignments) : assignments_(assignments) {}

    // The latest extension granted to this student, if any; otherwise the assignment deadline.
    DateTime effectiveDeadline(const Assignment& a, int studentId) const;

    // d = 0 if on time, otherwise the number of started 24-hour periods after the deadline.
    static int daysLate(DateTime submittedAt, DateTime effectiveDeadline);

private:
    IAssignmentRepository& assignments_;
};

}  // namespace lms
