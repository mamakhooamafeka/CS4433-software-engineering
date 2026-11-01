#include "presentation/RoleMenus.hpp"

#include "domain/Enums.hpp"

namespace lms {
namespace {

std::string describe(const SubmissionView& v) {
    std::string text = v.studentName + " | " + toString(v.status) + " | v" +
                       std::to_string(v.versionCount) + " " + showTime(v.latestSubmittedAt);
    if (v.daysLate > 0) text += " | " + std::to_string(v.daysLate) + " day(s) late";
    if (v.mark) text += " | final " + showMark(v.mark->finalMark);
    return text;
}

}  // namespace

void LecturerMenu::handle(User& user, MenuOption option) {
    const auto& lecturer = dynamic_cast<const Lecturer&>(user);
    switch (option) {
        case MenuOption::CREATE_ASSIGNMENT:
            createAssignment(lecturer);
            break;
        case MenuOption::GRANT_EXTENSION:
            grantExtension(lecturer);
            break;
        case MenuOption::MARK_SUBMISSIONS:
            markSubmissions(lecturer);
            break;
        case MenuOption::CORRECT_MARK:
            correctMark(lecturer);
            break;
        default:
            break;
    }
}

std::optional<Course> LecturerMenu::pickCourse(const Lecturer& lecturer) {
    const auto courses = assignments_.coursesOf(lecturer);
    std::vector<std::string> items;
    for (const Course& c : courses) items.push_back(c.getCode() + " " + c.getTitle());
    const int pick = io_.choose("Choose a course", items);
    if (pick < 0) return std::nullopt;
    return courses[static_cast<std::size_t>(pick)];
}

std::optional<Assignment> LecturerMenu::pickAssignment(const Lecturer& lecturer) {
    const auto course = pickCourse(lecturer);
    if (!course) return std::nullopt;
    const auto list = assignments_.assignmentsOf(lecturer, course->getId());
    std::vector<std::string> items;
    for (const Assignment& a : list) {
        items.push_back(a.getTitle() + " | due " + showTime(a.getDeadline()));
    }
    const int pick = io_.choose("Choose an assignment", items);
    if (pick < 0) return std::nullopt;
    return list[static_cast<std::size_t>(pick)];
}

// UC09 (FR-08).
void LecturerMenu::createAssignment(const Lecturer& lecturer) {
    const auto course = pickCourse(lecturer);
    if (!course) return;
    io_.heading("New assignment for " + course->getCode());
    const std::string title = io_.readText("Title: ", 200);
    const std::string instructions = io_.readText("Instructions: ", 2000);
    const DateTime deadline = io_.readDateTime("Deadline (YYYY-MM-DD HH:MM, UTC): ");
    const int maxMark =
        io_.readInt("Maximum mark (1-1000): ", Assignment::kMinMaxMark, Assignment::kMaxMaxMark);
    if (!io_.confirm("Create \"" + title + "\" due " + showTime(deadline) + "?")) return;
    assignments_.createAssignment(lecturer, course->getId(), title, instructions, deadline,
                                  maxMark);
    io_.info("The assignment has been created. Enrolled students can see it now.");
}

// UC10 (FR-09, BR-02, DC-01, DC-02).
void LecturerMenu::grantExtension(const Lecturer& lecturer) {
    const auto course = pickCourse(lecturer);
    if (!course) return;
    const auto list = assignments_.assignmentsOf(lecturer, course->getId());
    std::vector<std::string> items;
    for (const Assignment& a : list) {
        items.push_back(a.getTitle() + " | due " + showTime(a.getDeadline()));
    }
    const int pickA = io_.choose("Choose an assignment", items);
    if (pickA < 0) return;
    const Assignment& a = list[static_cast<std::size_t>(pickA)];

    const auto students = assignments_.studentsOf(lecturer, course->getId());
    items.clear();
    for (const UserRecord& s : students) items.push_back(s.fullName + " (" + s.username + ")");
    const int pickS = io_.choose("Choose the student", items);
    if (pickS < 0) return;
    const UserRecord& s = students[static_cast<std::size_t>(pickS)];

    const DateTime newDeadline = io_.readDateTime("New deadline (YYYY-MM-DD HH:MM, UTC): ");
    const std::string reason = io_.readText("Reason: ", 500);
    if (!io_.confirm("Extend \"" + a.getTitle() + "\" for " + s.fullName + " to " +
                     showTime(newDeadline) + "?")) {
        return;
    }
    assignments_.grantExtension(lecturer, a.getId(), s.id, newDeadline, reason);
    io_.info("The extension has been granted.");
}

// UC11 (FR-13): the late penalty is worked out by the service, not typed in.
void LecturerMenu::markSubmissions(const Lecturer& lecturer) {
    const auto a = pickAssignment(lecturer);
    if (!a) return;
    while (true) {
        const auto list = marking_.listSubmissions(lecturer, a->getId(), true);
        std::vector<std::string> items;
        for (const SubmissionView& v : list) items.push_back(describe(v));
        const int pick = io_.choose("Unmarked submissions for " + a->getTitle(), items);
        if (pick < 0) return;
        const SubmissionView& v = list[static_cast<std::size_t>(pick)];
        io_.info("File: " + v.filePath);
        const double raw =
            io_.readMark("Raw mark (0-" + std::to_string(a->getMaxMark()) + "): ", a->getMaxMark());
        const std::string feedback = io_.readText("Feedback: ", 2000);  // UC11 5a: asks again
        const Mark p = marking_.preview(lecturer, v.submissionId, raw, feedback);
        io_.info("Raw " + showMark(p.rawMark) + ", late deduction " + showMark(p.deduction) + " (" +
                 std::to_string(p.daysLate) + " day(s) late), final " + showMark(p.finalMark) +
                 ".");
        if (!io_.confirm("Save this mark?")) continue;  // UC11 step 7
        marking_.mark(lecturer, v.submissionId, raw, feedback);
        io_.info("Saved. Students see it only after the HoD releases it.");
    }
}

// UC20 (FR-19): a correction needs a reason, which goes into the audit log.
void LecturerMenu::correctMark(const Lecturer& lecturer) {
    const auto a = pickAssignment(lecturer);
    if (!a) return;
    std::vector<SubmissionView> marked;
    for (const SubmissionView& v : marking_.listSubmissions(lecturer, a->getId(), false)) {
        if (v.status == SubmissionStatus::MARKED && v.mark) marked.push_back(v);
    }
    std::vector<std::string> items;
    for (const SubmissionView& v : marked) items.push_back(describe(v));
    const int pick = io_.choose("Marked submissions (not yet moderated)", items);
    if (pick < 0) return;
    const SubmissionView& v = marked[static_cast<std::size_t>(pick)];
    const Mark old = v.mark.value_or(Mark{});
    io_.info("Current: raw " + showMark(old.rawMark) + ", final " + showMark(old.finalMark) +
             ". Feedback: " + old.feedback);
    const double raw = io_.readMark(
        "Corrected raw mark (0-" + std::to_string(a->getMaxMark()) + "): ", a->getMaxMark());
    std::string feedback = io_.readText("New feedback (Enter keeps the old): ", 2000, true);
    if (feedback.empty()) feedback = old.feedback;
    const std::string reason = io_.readText("Reason for the correction: ", 500);
    if (!io_.confirm("Save the corrected mark?")) return;
    const Mark m = marking_.correct(lecturer, v.submissionId, raw, feedback, reason);
    io_.info("Corrected. Final mark is now " + showMark(m.finalMark) + ".");
}

}  // namespace lms
