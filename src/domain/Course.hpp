// Course and Department entities (class diagram 2a). Each course belongs to exactly one
// department, and each department has at most one HoD (FR-06, ASM-04).
#pragma once

#include <optional>
#include <string>

namespace lms {

class Department {
public:
    Department(int id, std::string name, std::optional<int> hodUserId)
        : id_(id), name_(std::move(name)), hodUserId_(hodUserId) {}

    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    std::optional<int> getHodUserId() const { return hodUserId_; }

private:
    int id_;
    std::string name_;
    std::optional<int> hodUserId_;
};

class Course {
public:
    Course(int id, std::string code, std::string title, int departmentId)
        : id_(id), code_(std::move(code)), title_(std::move(title)), departmentId_(departmentId) {}

    int getId() const { return id_; }
    const std::string& getCode() const { return code_; }
    const std::string& getTitle() const { return title_; }
    int getDepartmentId() const { return departmentId_; }

private:
    int id_;
    std::string code_;
    std::string title_;
    int departmentId_;
};

}  // namespace lms
