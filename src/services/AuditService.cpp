#include "services/AuditService.hpp"

namespace lms {

void AuditService::recordMarkChange(int changedBy, const Submission& submission,
                                    const std::optional<Mark>& oldMark, const Mark& newMark,
                                    ChangeType type, const std::string& reason) {
    AuditEntry e;
    e.submissionId = submission.getId();
    e.changedBy = changedBy;
    e.changedAt = clock_.now();
    e.changeType = type;
    if (oldMark) {
        e.oldRawMark = oldMark->rawMark;
        e.oldFinalMark = oldMark->finalMark;
    }
    e.newRawMark = newMark.rawMark;
    e.newFinalMark = newMark.finalMark;
    e.reason = reason;
    audit_.append(e);
}

} // namespace lms
