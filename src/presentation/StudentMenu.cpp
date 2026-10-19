#include "presentation/RoleMenus.hpp"

#include "domain/Enums.hpp"

namespace lms {
namespace {

// Windows "Copy as path" puts quotes around the path; remove them.
std::string stripQuotes(std::string path) {
    if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
        path = path.substr(1, path.size() - 2);
    }
    return path;
}

std::string statusText(const AssignmentView& v) {
    if (v.closed) return "closed";
    return v.status ? toString(*v.status) : "not submitted";
}

} // namespace

void StudentMenu::handle(User& user, MenuOption option) {
    const auto& student = dynamic_cast<const Student&>(user);
    switch (option) {
        case MenuOption::VIEW_MY_ASSIGNMENTS: viewAssignments(student); break;
        case MenuOption::SUBMIT_ASSIGNMENT: submit(student); break;
        case MenuOption::VIEW_RELEASED_RESULTS: viewResults(student); break;
        default: break;
    }
}

// UC15 (FR-10): the deadline shown is the student's own, including any extension.
void StudentMenu::viewAssignments(const Student& student) {
    const auto list = assignments_.listForStudent(student);
    std::vector<std::string> items;
    for (const AssignmentView& v : list) {
        items.push_back(v.course.getCode() + " " + v.assignment.getTitle() + " | due " +
                        showTime(v.effectiveDeadline) + (v.hasExtension ? " (extended)" : "") +
                        " | " + statusText(v));
    }
    const int pick = io_.choose("My assignments (choose one to see the instructions)", items);
    if (pick < 0) return;
    const AssignmentView& v = list[static_cast<std::size_t>(pick)];
    io_.heading(v.assignment.getTitle());
    io_.info("Course: " + v.course.getCode() + " " + v.course.getTitle());
    io_.info("Deadline: " + showTime(v.effectiveDeadline) + " UTC");
    io_.info("Maximum mark: " + std::to_string(v.assignment.getMaxMark()));
    io_.info("Instructions: " + v.assignment.getInstructions());
}

// UC16/UC17, sequence diagram Figure 5: choose, check the file, preview, confirm, receipt.
void StudentMenu::submit(const Student& student) {
    const auto list = submissions_.listOpenAssignments(student);
    std::vector<std::string> items;
    for (const AssignmentView& v : list) {
        items.push_back(v.course.getCode() + " " + v.assignment.getTitle() + " | due " +
                        showTime(v.effectiveDeadline) + " | " + statusText(v));
    }
    const int pick = io_.choose("Submit an assignment", items);
    if (pick < 0) return;
    const int assignmentId = list[static_cast<std::size_t>(pick)].assignment.getId();

    std::string path;
    std::optional<SubmissionPreview> preview;
    for (int attempt = 0; attempt < 3 && !preview; ++attempt) {
        path = stripQuotes(io_.readText("Full path of your file: ", 1000));
        try {
            preview = submissions_.preview(student, assignmentId, path);
        } catch (const ValidationError& e) {   // missing, empty or too big (FR-11)
            io_.showError(e);
        }
    }
    if (!preview) return;

    io_.heading("Check before you submit");
    io_.info("Assignment: " + preview->assignmentTitle);
    io_.info("Your deadline: " + showTime(preview->effectiveDeadline) + " UTC");
    io_.info("Time now:      " + showTime(preview->now) + " UTC");
    if (preview->isResubmission && !preview->canResubmit) {
        io_.info("You cannot resubmit: it is past the deadline or already marked (BR-03).");
        return;
    }
    if (preview->daysLate > 0) {
        io_.info("This is late by " + std::to_string(preview->daysLate) + " day(s): " +
                 std::to_string(10 * preview->daysLate) +
                 "% of the maximum mark will be deducted; zero after 3 days.");
    }
    if (preview->isResubmission) {
        io_.info("This becomes version " + std::to_string(preview->currentVersion + 1) +
                 ". Earlier versions are kept, but only the newest is marked.");
    }
    if (!io_.confirm("Submit this file now?")) return;

    const Receipt r = submissions_.submit(student, assignmentId, path);
    io_.heading("Submission receipt");
    io_.info("Assignment: " + r.assignmentTitle);
    io_.info("File: " + r.originalName + " (" + std::to_string(r.sizeBytes) + " bytes)");
    io_.info("Version: " + std::to_string(r.versionNo) + ", status " + toString(r.status));
    io_.info("Received: " + showTime(r.submittedAt) + " UTC");
    io_.info("SHA-256: " + r.sha256);
}

// UC18 (FR-18, BR-04): only released results ever reach this screen.
void StudentMenu::viewResults(const Student& student) {
    const auto results = submissions_.listReleasedResults(student);
    io_.heading("My released results");
    if (results.empty()) {
        io_.info("No results have been released yet.");
        return;
    }
    for (const ResultView& r : results) {
        io_.line();
        io_.info(r.courseCode + " " + r.assignmentTitle);
        io_.info("Mark: " + showMark(r.mark.finalMark) + " / " + std::to_string(r.maxMark) +
                 "  (raw " + showMark(r.mark.rawMark) + ", late deduction " +
                 showMark(r.mark.deduction) + ", days late " + std::to_string(r.mark.daysLate) +
                 ")");
        io_.info("Feedback: " + (r.mark.feedback.empty() ? "-" : r.mark.feedback));
    }
}

} // namespace lms
