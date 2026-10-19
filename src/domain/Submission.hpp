// A student's submission for one assignment and its life cycle (state machine, Figure 3).
// Each public operation is one transition; any event not allowed in the current state
// throws BusinessRuleError, so an invalid state change is impossible.
#pragma once

#include "domain/DateTime.hpp"
#include "domain/Enums.hpp"
#include "domain/Mark.hpp"

#include <optional>
#include <vector>

namespace lms {

class Submission {
public:
    // A brand-new submission with no versions yet; call addVersion() next.
    Submission(int assignmentId, int studentId);

    // Rebuilds a stored submission (used by SqliteSubmissionRepository).
    Submission(int id, int assignmentId, int studentId, SubmissionStatus status,
               std::vector<SubmissionVersion> versions, std::optional<Mark> mark,
               std::optional<int> moderatedBy, std::optional<DateTime> moderatedAt,
               std::optional<int> releasedBy, std::optional<DateTime> releasedAt);

    // First version: SUBMITTED if on time, otherwise LATE (never rejected, C2).
    // Later versions: only while canResubmit() (BR-03).
    void addVersion(SubmissionVersion v, DateTime effectiveDeadline);
    bool canResubmit(DateTime now, DateTime effectiveDeadline) const;

    void recordMark(const Mark& m);                         // SUBMITTED/LATE -> MARKED
    void correctMark(const Mark& m);                        // MARKED -> MARKED
    void approve(int hodId, DateTime at);                   // MARKED -> MODERATED
    void amend(const Mark& m, int hodId, DateTime at);      // MARKED -> MODERATED, RELEASED -> RELEASED
    void release(int hodId, DateTime at);                   // MODERATED -> RELEASED
    void reevaluateLateness(DateTime effectiveDeadline);    // LATE -> SUBMITTED (DC-01)

    const SubmissionVersion& latestVersion() const;
    SubmissionStatus getStatus() const { return status_; }

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }
    int getAssignmentId() const { return assignmentId_; }
    int getStudentId() const { return studentId_; }
    const std::vector<SubmissionVersion>& getVersions() const { return versions_; }
    const std::optional<Mark>& getMark() const { return mark_; }
    std::optional<int> getModeratedBy() const { return moderatedBy_; }
    std::optional<DateTime> getModeratedAt() const { return moderatedAt_; }
    std::optional<int> getReleasedBy() const { return releasedBy_; }
    std::optional<DateTime> getReleasedAt() const { return releasedAt_; }

private:
    void requireStatus(std::initializer_list<SubmissionStatus> allowed, const char* action) const;

    int id_ = 0;
    int assignmentId_;
    int studentId_;
    SubmissionStatus status_ = SubmissionStatus::SUBMITTED;
    std::vector<SubmissionVersion> versions_;
    std::optional<Mark> mark_;
    std::optional<int> moderatedBy_;
    std::optional<DateTime> moderatedAt_;
    std::optional<int> releasedBy_;
    std::optional<DateTime> releasedAt_;
};

} // namespace lms
