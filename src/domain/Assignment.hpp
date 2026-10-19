// Assignment and Extension entities (class diagram 2a).
#pragma once

#include "domain/DateTime.hpp"

#include <string>

namespace lms {

class Assignment {
public:
    static constexpr int kMinMaxMark = 1;     // FR-08
    static constexpr int kMaxMaxMark = 1000;  // FR-08

    Assignment(int id, int courseId, std::string title, std::string instructions, DateTime deadline,
               int maxMark, int createdBy, DateTime createdAt)
        : id_(id),
          courseId_(courseId),
          title_(std::move(title)),
          instructions_(std::move(instructions)),
          deadline_(deadline),
          maxMark_(maxMark),
          createdBy_(createdBy),
          createdAt_(createdAt) {}

    int getId() const { return id_; }
    int getCourseId() const { return courseId_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getInstructions() const { return instructions_; }
    DateTime getDeadline() const { return deadline_; }
    int getMaxMark() const { return maxMark_; }
    int getCreatedBy() const { return createdBy_; }
    DateTime getCreatedAt() const { return createdAt_; }
    void setId(int id) { id_ = id; }

private:
    int id_;
    int courseId_;
    std::string title_;
    std::string instructions_;
    DateTime deadline_;
    int maxMark_;
    int createdBy_;
    DateTime createdAt_;
};

// A later deadline for one student (FR-09). A class-wide extension is CR1, not v1.0.
struct Extension {
    int id = 0;
    int assignmentId = 0;
    int studentId = 0;
    DateTime newDeadline;
    std::string reason;
    int grantedBy = 0;
    DateTime grantedAt;
};

}  // namespace lms
