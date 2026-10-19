// Value objects stored with a submission (class diagram 2a).
#pragma once

#include "domain/DateTime.hpp"
#include "domain/Enums.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace lms {

// One uploaded file. Earlier versions are kept read-only (BR-03).
struct SubmissionVersion {
    int versionNo = 1;
    std::string storedPath;
    std::string originalName;
    std::int64_t sizeBytes = 0;
    std::string sha256;
    DateTime submittedAt;
};

// A mark with its late-penalty snapshot. finalMark = rawMark - deduction (BR-01).
struct Mark {
    double rawMark = 0.0;
    double deduction = 0.0;
    double finalMark = 0.0;
    int daysLate = 0;
    std::string feedback;
    int markedBy = 0;
    DateTime markedAt;
};

// One row of the append-only audit log (FR-19, BR-05).
struct AuditEntry {
    int id = 0;
    int submissionId = 0;
    int changedBy = 0;
    DateTime changedAt;
    ChangeType changeType = ChangeType::INITIAL;
    std::optional<double> oldRawMark;
    std::optional<double> oldFinalMark;
    double newRawMark = 0.0;
    double newFinalMark = 0.0;
    std::string reason;
};

// Filters for an HoD's view of the audit log (FR-20). Empty = no filter.
struct AuditFilter {
    int departmentId = 0;
    std::optional<int> courseId;
    std::optional<int> assignmentId;
    std::optional<int> studentId;
};

} // namespace lms
