// Strategy pattern for the late penalty (class diagram 2a). MarkingService and
// ModerationService depend only on ILatePenaltyPolicy, so a different school rule
// can be added as a new class without changing them (open/closed principle).
#pragma once

namespace lms {

struct PenaltyResult {
    double deduction = 0.0;
    double finalMark = 0.0;
};

class ILatePenaltyPolicy {
public:
    virtual ~ILatePenaltyPolicy() = default;
    virtual PenaltyResult apply(double rawMark, int maxMark, int daysLate) const = 0;
};

// BR-01: 10% of the maximum mark per day late for days 1-3, and 0 from day 4.
// The final mark never goes below 0 and is held to one decimal place.
class TenPercentPerDayPolicy : public ILatePenaltyPolicy {
public:
    PenaltyResult apply(double rawMark, int maxMark, int daysLate) const override;

private:
    double ratePerDay_ = 0.10;
    int maxPenalisedDays_ = 3;
};

// Rounds to one decimal place (BR-01).
double roundToTenth(double value);

}  // namespace lms
