#include "domain/Submission.hpp"

#include "domain/Errors.hpp"

#include <algorithm>
#include <string>

namespace lms {

Submission::Submission(int assignmentId, int studentId)
    : assignmentId_(assignmentId), studentId_(studentId) {}

Submission::Submission(int id, int assignmentId, int studentId, SubmissionStatus status,
                       std::vector<SubmissionVersion> versions, std::optional<Mark> mark,
                       std::optional<int> moderatedBy, std::optional<DateTime> moderatedAt,
                       std::optional<int> releasedBy, std::optional<DateTime> releasedAt)
    : id_(id),
      assignmentId_(assignmentId),
      studentId_(studentId),
      status_(status),
      versions_(std::move(versions)),
      mark_(std::move(mark)),
      moderatedBy_(moderatedBy),
      moderatedAt_(moderatedAt),
      releasedBy_(releasedBy),
      releasedAt_(releasedAt) {}

void Submission::requireStatus(std::initializer_list<SubmissionStatus> allowed,
                               const char* action) const {
    if (std::find(allowed.begin(), allowed.end(), status_) == allowed.end()) {
        throw BusinessRuleError(std::string("You cannot ") + action + " a submission that is " +
                                toString(status_) + ".");
    }
}

void Submission::addVersion(SubmissionVersion v, DateTime effectiveDeadline) {
    if (versions_.empty()) {
        v.versionNo = 1;
        status_ = v.submittedAt <= effectiveDeadline ? SubmissionStatus::SUBMITTED
                                                     : SubmissionStatus::LATE;
    } else {
        if (!canResubmit(v.submittedAt, effectiveDeadline)) {
            throw BusinessRuleError(
                "Resubmission is not allowed: the deadline has passed or the work is marked.");
        }
        v.versionNo = versions_.back().versionNo + 1;
    }
    versions_.push_back(std::move(v));
}

// BR-03: only before the effective deadline and before marking.
bool Submission::canResubmit(DateTime now, DateTime effectiveDeadline) const {
    return !versions_.empty() && status_ == SubmissionStatus::SUBMITTED && now <= effectiveDeadline;
}

void Submission::recordMark(const Mark& m) {
    requireStatus({SubmissionStatus::SUBMITTED, SubmissionStatus::LATE}, "mark");
    mark_ = m;
    status_ = SubmissionStatus::MARKED;
}

void Submission::correctMark(const Mark& m) {
    requireStatus({SubmissionStatus::MARKED}, "correct the mark of");
    mark_ = m;
}

void Submission::approve(int hodId, DateTime at) {
    requireStatus({SubmissionStatus::MARKED}, "approve");
    moderatedBy_ = hodId;
    moderatedAt_ = at;
    status_ = SubmissionStatus::MODERATED;
}

// Only an HoD may change a MODERATED or RELEASED mark (BR-07); the service checks the role.
void Submission::amend(const Mark& m, int hodId, DateTime at) {
    requireStatus({SubmissionStatus::MARKED, SubmissionStatus::RELEASED}, "amend");
    mark_ = m;
    if (status_ == SubmissionStatus::MARKED) {
        moderatedBy_ = hodId;
        moderatedAt_ = at;
        status_ = SubmissionStatus::MODERATED;
    }
}

void Submission::release(int hodId, DateTime at) {
    requireStatus({SubmissionStatus::MODERATED}, "release");
    releasedBy_ = hodId;
    releasedAt_ = at;
    status_ = SubmissionStatus::RELEASED;
}

// DC-01: an extension can turn a LATE submission into an on-time one.
void Submission::reevaluateLateness(DateTime effectiveDeadline) {
    if (status_ == SubmissionStatus::LATE && !versions_.empty() &&
        latestVersion().submittedAt <= effectiveDeadline) {
        status_ = SubmissionStatus::SUBMITTED;
    }
}

const SubmissionVersion& Submission::latestVersion() const {
    if (versions_.empty()) {
        throw BusinessRuleError("This submission has no uploaded file.");
    }
    return versions_.back();
}

}  // namespace lms
