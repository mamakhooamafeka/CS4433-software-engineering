#include "domain/DeadlineCalculator.hpp"

#include <algorithm>

namespace lms {

DateTime DeadlineCalculator::classDeadline(const Assignment& a) const {
    DateTime deadline = a.getDeadline();
    for (const ClassExtension& e : assignments_.classExtensionsFor(a.getId())) {
        deadline = std::max(deadline, e.newDeadline);
    }
    return deadline;
}

EffectiveDeadline DeadlineCalculator::deadlineFor(const Assignment& a, int studentId) const {
    EffectiveDeadline out;
    out.deadline = a.getDeadline();
    for (const ClassExtension& e : assignments_.classExtensionsFor(a.getId())) {
        if (e.newDeadline > out.deadline) {
            out.deadline = e.newDeadline;
            out.scope = ExtensionScope::CLASS;
            out.classReason = e.reason;
        }
    }
    // A student's own extension counts only if it is later than the class deadline.
    for (const Extension& e : assignments_.extensionsFor(a.getId(), studentId)) {
        if (e.newDeadline > out.deadline) {
            out.deadline = e.newDeadline;
            out.scope = ExtensionScope::STUDENT;
        }
    }
    return out;
}

DateTime DeadlineCalculator::effectiveDeadline(const Assignment& a, int studentId) const {
    return deadlineFor(a, studentId).deadline;
}

int DeadlineCalculator::daysLate(DateTime submittedAt, DateTime effectiveDeadline) {
    if (submittedAt <= effectiveDeadline) {
        return 0;
    }
    using namespace std::chrono;
    const auto lateBy = duration_cast<seconds>(submittedAt - effectiveDeadline).count();
    constexpr long long kDay = 24LL * 60 * 60;
    return static_cast<int>((lateBy + kDay - 1) / kDay);  // ceiling division
}

}  // namespace lms
