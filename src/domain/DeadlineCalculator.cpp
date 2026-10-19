#include "domain/DeadlineCalculator.hpp"

#include <algorithm>

namespace lms {

DateTime DeadlineCalculator::effectiveDeadline(const Assignment& a, int studentId) const {
    DateTime deadline = a.getDeadline();
    for (const Extension& e : assignments_.extensionsFor(a.getId(), studentId)) {
        deadline = std::max(deadline, e.newDeadline);
    }
    return deadline;
}

int DeadlineCalculator::daysLate(DateTime submittedAt, DateTime effectiveDeadline) {
    if (submittedAt <= effectiveDeadline) {
        return 0;
    }
    using namespace std::chrono;
    const auto lateBy = duration_cast<seconds>(submittedAt - effectiveDeadline).count();
    constexpr long long kDay = 24 * 60 * 60;
    return static_cast<int>((lateBy + kDay - 1) / kDay);   // ceiling division
}

} // namespace lms
