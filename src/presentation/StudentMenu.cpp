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

// For example "2 day(s) 3 h 15 min" (UC16 step 3).
std::string timeLeft(DateTime now, DateTime deadline) {
    const auto mins = std::chrono::duration_cast<std::chrono::minutes>(deadline - now).count();
    return std::to_string(mins / 1440) + " day(s) " + std::to_string((mins % 1440) / 60) + " h " +
           std::to_string(mins % 60) + " min";
}

// UC16 steps 3, 6, 7 and flows 3a, 3b, 6a. Returns false when the student cannot submit.
bool showPreview(ConsoleIO& io, const SubmissionPreview& p) {
    io.heading("Check before you submit");
    io.info("Assignment: " + p.assignmentTitle);
    io.info("Your deadline: " + showTime(p.effectiveDeadline) + " UTC");
    io.info("Time now:      " + showTime(p.now) + " UTC");
    if (p.effectiveDeadline > p.now) {
        io.info("Time left:     " + timeLeft(p.now, p.effectiveDeadline));
    }
    if (p.isResubmission && !p.canResubmit) {
        io.info("You cannot resubmit: it is past the deadline or already marked (BR-03).");
        io.info("Your version " + std::to_string(p.currentVersion) + " is kept, status " +
                (p.currentStatus ? toString(*p.currentStatus) : "-") + ".");
        return false;
    }
    if (p.daysLate > 0) {
        io.info("This is late by " + std::to_string(p.daysLate) +
                " day(s): " + std::to_string(10 * p.daysLate) +
                "% of the maximum mark will be deducted; zero after 3 days.");
    }
    if (p.isResubmission) {
        io.info("This becomes version " + std::to_string(p.currentVersion + 1) +
                ". Earlier versions are kept, but only the newest is marked.");
    }
    return true;
}

// UC16 step 10.
void showReceipt(ConsoleIO& io, const Receipt& r) {
    io.heading("Submission receipt");
    io.info("Assignment: " + r.assignmentTitle);
    io.info("File: " + r.originalName + " (" + std::to_string(r.sizeBytes) + " bytes)");
    io.info("Version: " + std::to_string(r.versionNo) + ", status " + toString(r.status));
    io.info("Received: " + showTime(r.submittedAt) + " UTC");
    io.info("SHA-256: " + r.sha256);
}

}  // namespace

void StudentMenu::handle(User& user, MenuOption option) {
    const auto& student = dynamic_cast<const Student&>(user);
    switch (option) {
        case MenuOption::VIEW_MY_ASSIGNMENTS:
            viewAssignments(student);
            break;
        case MenuOption::SUBMIT_ASSIGNMENT:
            submit(student);
            break;
        case MenuOption::VIEW_RELEASED_RESULTS:
            viewResults(student);
            break;
        default:
            break;
    }
}

namespace {

// CR1: the list says whether the deadline was extended, and for whom. The tags are short
// so a list line still fits in 80 columns (NFR-11); extensionKey() explains them.
std::string extensionTag(const AssignmentView& v) {
    switch (v.extension) {
        case ExtensionScope::CLASS:
            return " (ext. class)";
        case ExtensionScope::STUDENT:
            return " (ext. you)";
        case ExtensionScope::NONE:
            break;
    }
    return "";
}

// Printed above a list that has at least one extended deadline.
void extensionKey(ConsoleIO& io, const std::vector<AssignmentView>& list) {
    for (const AssignmentView& v : list) {
        if (v.extension != ExtensionScope::NONE) {
            io.info("ext. class = deadline extended for the whole class");
            io.info("ext. you   = deadline extended for you only");
            return;
        }
    }
}

}  // namespace

// UC15 (FR-10, CR1): the deadline shown is the student's own, including any extension.
void StudentMenu::viewAssignments(const Student& student) {
    const auto list = assignments_.listForStudent(student);
    std::vector<std::string> items;
    for (const AssignmentView& v : list) {
        items.push_back(v.course.getCode() + " " + v.assignment.getTitle() + " | due " +
                        showTime(v.effectiveDeadline) + extensionTag(v) + " | " + statusText(v));
    }
    extensionKey(io_, list);
    const int pick = io_.choose("My assignments (choose one to see the instructions)", items);
    if (pick < 0) return;
    const AssignmentView& v = list[static_cast<std::size_t>(pick)];
    io_.heading(v.assignment.getTitle());
    io_.info("Course: " + v.course.getCode() + " " + v.course.getTitle());
    io_.info("Deadline: " + showTime(v.effectiveDeadline) + " UTC");
    if (v.extension == ExtensionScope::CLASS) {
        io_.info("Extended for the whole class from " + showTime(v.assignment.getDeadline()) +
                 " UTC.");
        io_.info("Reason: " + v.classReason);
    } else if (v.extension == ExtensionScope::STUDENT) {
        io_.info("Extended for you only, from " + showTime(v.assignment.getDeadline()) + " UTC.");
    }
    io_.info("Maximum mark: " + std::to_string(v.assignment.getMaxMark()));
    io_.info("Instructions: " + v.assignment.getInstructions());
}

// UC16/UC17, sequence diagram Figure 5: choose, check the file, preview, confirm, receipt.
void StudentMenu::submit(const Student& student) {
    const auto list = submissions_.listOpenAssignments(student);
    std::vector<std::string> items;
    for (const AssignmentView& v : list) {
        items.push_back(v.course.getCode() + " " + v.assignment.getTitle() + " | due " +
                        showTime(v.effectiveDeadline) + extensionTag(v) + " | " + statusText(v));
    }
    extensionKey(io_, list);
    const int pick = io_.choose("Submit an assignment", items);
    if (pick < 0) return;
    const int assignmentId = list[static_cast<std::size_t>(pick)].assignment.getId();

    std::string path;
    const auto preview = askForFile(student, assignmentId, path);
    if (!preview || !showPreview(io_, *preview)) return;
    if (!io_.confirm("Submit this file now?")) return;
    showReceipt(io_, submissions_.submit(student, assignmentId, path));
}

// UC16 steps 4-5 and exception 5a: up to three tries to give a usable file (FR-11).
std::optional<SubmissionPreview> StudentMenu::askForFile(const Student& student, int assignmentId,
                                                         std::string& path) {
    for (int attempt = 0; attempt < 3; ++attempt) {
        path = stripQuotes(io_.readText("Full path of your file: ", 1000));
        try {
            return submissions_.preview(student, assignmentId, path);
        } catch (const ValidationError& e) {  // missing, empty or too big
            io_.showError(e);
        }
    }
    return std::nullopt;
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

}  // namespace lms
