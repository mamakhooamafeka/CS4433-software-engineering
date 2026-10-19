#include "data/SqliteRepositories.hpp"

namespace lms {

std::vector<Course> SqliteCourseRepository::courses(Statement& s) {
    std::vector<Course> out;
    while (s.step()) {
        out.emplace_back(s.getInt(0), s.getText(1), s.getText(2), s.getInt(3));
    }
    return out;
}

std::optional<Course> SqliteCourseRepository::findCourse(int id) {
    Statement s = db_.prepare("select id, code, title, department_id from course where id = ?");
    s.bind(1, id);
    auto found = courses(s);
    if (found.empty()) return std::nullopt;
    return found.front();
}

std::optional<Department> SqliteCourseRepository::findDepartment(int id) {
    Statement s = db_.prepare("select id, name, hod_user_id from department where id = ?");
    s.bind(1, id);
    if (!s.step()) return std::nullopt;
    return Department(s.getInt(0), s.getText(1), s.getOptionalInt(2));
}

std::vector<Course> SqliteCourseRepository::coursesOfLecturer(int lecturerId) {
    Statement s = db_.prepare(
        "select c.id, c.code, c.title, c.department_id from course c "
        "join course_lecturer cl on cl.course_id = c.id where cl.lecturer_id = ? order by c.code");
    s.bind(1, lecturerId);
    return courses(s);
}

std::vector<Course> SqliteCourseRepository::coursesOfStudent(int studentId) {
    Statement s = db_.prepare(
        "select c.id, c.code, c.title, c.department_id from course c "
        "join enrolment e on e.course_id = c.id where e.student_id = ? and e.withdrawn = 0 "
        "order by c.code");
    s.bind(1, studentId);
    return courses(s);
}

std::vector<Course> SqliteCourseRepository::coursesOfDepartment(int departmentId) {
    Statement s = db_.prepare(
        "select id, code, title, department_id from course where department_id = ? order by code");
    s.bind(1, departmentId);
    return courses(s);
}

std::vector<Department> SqliteCourseRepository::listDepartments() {
    Statement s = db_.prepare("select id, name, hod_user_id from department order by name");
    std::vector<Department> out;
    while (s.step()) out.emplace_back(s.getInt(0), s.getText(1), s.getOptionalInt(2));
    return out;
}

std::vector<Course> SqliteCourseRepository::listCourses() {
    Statement s = db_.prepare("select id, code, title, department_id from course order by code");
    return courses(s);
}

int SqliteCourseRepository::insertDepartment(const std::string& name) {
    Statement s = db_.prepare("insert into department (name) values (?)");
    s.bind(1, name);
    try {
        s.run();
    } catch (const ConstraintViolation&) {
        throw ValidationError("A department with that name already exists.");
    }
    return static_cast<int>(db_.lastInsertId());
}

void SqliteCourseRepository::setHeadOfDepartment(int departmentId, int hodUserId) {
    Statement s = db_.prepare("update department set hod_user_id = ? where id = ?");
    s.bind(1, hodUserId).bind(2, departmentId);
    try {
        s.run();
    } catch (const ConstraintViolation&) {
        throw BusinessRuleError("That HoD already heads another department (ASM-04).");
    }
}

int SqliteCourseRepository::insertCourse(const std::string& code, const std::string& title,
                                         int departmentId) {
    Statement s = db_.prepare("insert into course (code, title, department_id) values (?, ?, ?)");
    s.bind(1, code).bind(2, title).bind(3, departmentId);
    try {
        s.run();
    } catch (const ConstraintViolation&) {
        throw ValidationError("That course code already exists, or the department is unknown.");
    }
    return static_cast<int>(db_.lastInsertId());
}

void SqliteCourseRepository::addLecturer(int courseId, int lecturerId) {
    Statement s = db_.prepare(
        "insert or ignore into course_lecturer (course_id, lecturer_id) values (?, ?)");
    s.bind(1, courseId).bind(2, lecturerId);
    s.run();
}

// Re-enrolling a withdrawn student re-activates the existing row.
void SqliteCourseRepository::enrol(int courseId, int studentId, DateTime at) {
    Statement s = db_.prepare(
        "insert into enrolment (course_id, student_id, enrolled_at, withdrawn) values (?, ?, ?, 0) "
        "on conflict (course_id, student_id) do update set withdrawn = 0");
    s.bind(1, courseId).bind(2, studentId).bind(3, at);
    s.run();
}

// Withdrawal keeps the row, so the student's past submissions stay linked (FR-04).
void SqliteCourseRepository::withdraw(int courseId, int studentId) {
    Statement s = db_.prepare(
        "update enrolment set withdrawn = 1 where course_id = ? and student_id = ?");
    s.bind(1, courseId).bind(2, studentId);
    s.run();
}

bool SqliteCourseRepository::isEnrolled(int courseId, int studentId) {
    Statement s = db_.prepare(
        "select 1 from enrolment where course_id = ? and student_id = ? and withdrawn = 0");
    s.bind(1, courseId).bind(2, studentId);
    return s.step();
}

} // namespace lms
