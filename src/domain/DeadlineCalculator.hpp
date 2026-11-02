// BR-02 (effective deadline) and the days-late part of BR-01.
#pragma once

#include "domain/Assignment.hpp"
#include "domain/Repositories.hpp"

namespace lms {

class DeadlineCalculator {
public:
    explicit DeadlineCalculator(IAssignmentRepository& assignments) : assignments_(assignments) {}

    // CR1: the deadline for the whole class: the latest class extension, if any,
    // otherwise the assignment deadline.
    DateTime classDeadline(const Assignment& a) const;

    // The latest of the class deadline and this student's own extensions (BR-02),
    // with where it came from, so the student's list can show it (CR1).
    EffectiveDeadline deadlineFor(const Assignment& a, int studentId) const;

    // Just the date and time of deadlineFor().
    DateTime effectiveDeadline(const Assignment& a, int studentId) const;

    // d = 0 if on time, otherwise the number of started 24-hour periods after the deadline.
    static int daysLate(DateTime submittedAt, DateTime effectiveDeadline);

private:
    IAssignmentRepository& assignments_;
};

}  // namespace lms
