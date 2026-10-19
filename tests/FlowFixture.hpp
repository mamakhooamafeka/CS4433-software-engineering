// Fixture for service-level tests: one department, course, lecturer, two students
// (only stu1 enrolled) and one assignment due 48 hours after the start time.
#pragma once

#include "TestSupport.hpp"
#include "infrastructure/LocalFileStore.hpp"
#include "services/AssignmentService.hpp"
#include "services/MarkingService.hpp"
#include "services/SubmissionService.hpp"

#include <filesystem>
#include <fstream>

namespace lms::test {

namespace fs = std::filesystem;
using std::chrono::hours;

struct Flow : test::World {
    fs::path dir = fs::temp_directory_path() /
                   ("lms_test_" + std::to_string(std::chrono::steady_clock::now()
                                                     .time_since_epoch()
                                                     .count()));
    LocalFileStore files{dir / "store"};
    TenPercentPerDayPolicy policy;
    AuditService auditService{audit, clock};
    AssignmentService assignmentsSvc{assignments, courses, submissions, clock, db};
    SubmissionService submit{assignments, courses, submissions, files, clock, db};
    MarkingService marking{assignments, courses, submissions, users,
                           policy,      auditService, clock, db};
    int lecId, stuId, otherStuId, courseId, asgId;
    DateTime deadline;

    Flow() {
        lecId = addUser(Role::LECTURER, "lec1");
        stuId = addUser(Role::STUDENT, "stu1");
        otherStuId = addUser(Role::STUDENT, "stu2");
        const int dept = courses.insertDepartment("Sciences");
        courseId = courses.insertCourse("PHY101", "Physics", dept);
        courses.addLecturer(courseId, lecId);
        courses.enrol(courseId, stuId, clock.now());
        deadline = clock.now() + hours(48);
        asgId = assignmentsSvc.createAssignment(lecturer(), courseId, "Lab report", "Write it",
                                                deadline, 100);
    }
    ~Flow() {
        std::error_code ec;
        fs::remove_all(dir, ec);
    }
    Lecturer& lecturer() {
        lecHolder = users.findById(lecId);
        return dynamic_cast<Lecturer&>(*lecHolder);
    }
    Student& student(int id) {
        stuHolder = users.findById(id);
        return dynamic_cast<Student&>(*stuHolder);
    }
    std::string makeFile(const std::string& name, std::size_t bytes) {
        fs::create_directories(dir);
        const fs::path p = dir / name;
        std::ofstream(p, std::ios::binary) << std::string(bytes, 'x');
        return p.string();
    }
    std::unique_ptr<User> lecHolder, stuHolder;
};


}  // namespace lms::test
