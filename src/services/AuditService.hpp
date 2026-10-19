// UC21 Record mark change (FR-19, BR-05). MarkingService and ModerationService call
// it inside the same transaction as the mark change, so a mark can never change
// without its audit row.
#pragma once

#include "domain/Infrastructure.hpp"
#include "domain/Repositories.hpp"

#include <optional>
#include <string>

namespace lms {

class AuditService {
public:
    AuditService(IAuditRepository& audit, IClock& clock) : audit_(audit), clock_(clock) {}

    void recordMarkChange(int changedBy, const Submission& submission,
                          const std::optional<Mark>& oldMark, const Mark& newMark, ChangeType type,
                          const std::string& reason);

private:
    IAuditRepository& audit_;
    IClock& clock_;
};

}  // namespace lms
