#include "domain/LatePenalty.hpp"

#include <algorithm>
#include <cmath>

namespace lms {

double roundToTenth(double value) {
    return std::round(value * 10.0) / 10.0;
}

PenaltyResult TenPercentPerDayPolicy::apply(double rawMark, int maxMark, int daysLate) const {
    const double raw = roundToTenth(rawMark);
    double finalMark = raw;
    if (daysLate > maxPenalisedDays_) {
        finalMark = 0.0;
    } else if (daysLate > 0) {
        finalMark = std::max(0.0, raw - ratePerDay_ * daysLate * maxMark);
    }
    finalMark = roundToTenth(finalMark);
    return PenaltyResult{roundToTenth(raw - finalMark), finalMark};
}

}  // namespace lms
