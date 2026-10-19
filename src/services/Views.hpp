// Read-only data the services hand to the menus. Keeping them separate from the
// entities means a screen can never change an entity by accident.
#pragma once

#include "domain/Assignment.hpp"
#include "domain/Course.hpp"
#include "domain/Mark.hpp"

#include <optional>
#include <string>

namespace lms {

struct AssignmentView {
    Assignment assignment;
    Course course;
    DateTime effectiveDeadline;
    bool hasExtension = false;
    std::optional<SubmissionStatus> status;   // empty = not submitted yet
    bool closed = false;                      // marks released (BR-06)
};

struct SubmissionPreview {
    std::string assignmentTitle;
    DateTime effectiveDeadline;
    DateTime now;
    int daysLate = 0;
    bool isResubmission = false;
    bool canResubmit = true;
    int currentVersion = 0;
    std::optional<SubmissionStatus> currentStatus;
};

struct Receipt {
    std::string assignmentTitle;
    DateTime submittedAt;
    int versionNo = 1;
    std::string originalName;
    std::int64_t sizeBytes = 0;
    std::string sha256;
    SubmissionStatus status = SubmissionStatus::SUBMITTED;
};

struct ResultView {
    std::string courseCode;
    std::string assignmentTitle;
    int maxMark = 0;
    Mark mark;
};

struct SubmissionView {
    int submissionId = 0;
    int studentId = 0;
    std::string studentName;
    SubmissionStatus status = SubmissionStatus::SUBMITTED;
    DateTime latestSubmittedAt;
    int versionCount = 0;
    std::string filePath;   // latest version, so the lecturer can open it
    int daysLate = 0;
    std::optional<Mark> mark;
};

struct AssignmentSummary {
    Assignment assignment;
    std::string courseCode;
    int marked = 0;
    int moderated = 0;
    int released = 0;
    int unmarked = 0;
};

struct ReleasePreview {
    std::string assignmentTitle;
    int toRelease = 0;     // MODERATED now
    int notReady = 0;      // SUBMITTED, LATE or MARKED: stay hidden
};

} // namespace lms
