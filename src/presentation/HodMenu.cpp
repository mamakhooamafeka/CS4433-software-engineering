#include "presentation/RoleMenus.hpp"

#include "domain/Enums.hpp"

#include <set>
#include <sstream>

namespace lms {
namespace {

std::string describe(const SubmissionView& v) {
    std::string text = v.studentName + " | " + toString(v.status);
    if (v.mark) {
        text += " | raw " + showMark(v.mark->rawMark) + " - " + showMark(v.mark->deduction) +
                " = " + showMark(v.mark->finalMark);
    }
    if (v.daysLate > 0) text += " | " + std::to_string(v.daysLate) + "d late";
    return text;
}

}  // namespace

void HodMenu::handle(User& user, MenuOption option) {
    const auto& hod = dynamic_cast<const HeadOfDepartment&>(user);
    switch (option) {
        case MenuOption::MODERATE_MARKS:
            moderate(hod);
            break;
        case MenuOption::AMEND_MARK:
            amend(hod);
            break;
        case MenuOption::RELEASE_MARKS:
            release(hod);
            break;
        case MenuOption::VIEW_AUDIT_LOG:
            viewAudit(hod);
            break;
        default:
            break;
    }
}

std::optional<AssignmentSummary> HodMenu::pickAssignment(const HeadOfDepartment& hod) {
    const auto list = moderation_.listDepartmentAssignments(hod);
    std::vector<std::string> items;
    for (const AssignmentSummary& s : list) {
        items.push_back(s.courseCode + " " + s.assignment.getTitle() + " | marked " +
                        std::to_string(s.marked) + ", moderated " + std::to_string(s.moderated) +
                        ", released " + std::to_string(s.released) + ", unmarked " +
                        std::to_string(s.unmarked));
    }
    const int pick = io_.choose("Assignments in my department", items);
    if (pick < 0) return std::nullopt;
    return list[static_cast<std::size_t>(pick)];
}

// UC12 part 1 (FR-15): approve all marked submissions, or only the chosen ones.
void HodMenu::moderate(const HeadOfDepartment& hod) {
    const auto a = pickAssignment(hod);
    if (!a) return;
    const auto list = moderation_.listMarked(hod, a->assignment.getId());
    io_.heading("Marked submissions: " + a->assignment.getTitle());
    if (list.empty()) {
        io_.info("There are no marked submissions waiting for moderation.");
        return;
    }
    for (std::size_t i = 0; i < list.size(); ++i) {
        io_.info(std::to_string(i + 1) + ". " + describe(list[i]));
    }
    const int how = io_.choose("Approve", {"Approve all", "Approve some (type their numbers)"});
    if (how < 0) return;

    std::vector<int> ids;
    if (how == 0) {
        for (const SubmissionView& v : list) ids.push_back(v.submissionId);
    } else {
        while (ids.empty()) {
            std::istringstream numbers(
                io_.readText("Numbers separated by spaces, e.g. 1 3 4: ", 500));
            std::set<int> chosen;
            int n = 0;
            bool ok = true;
            while (numbers >> n) {
                if (n < 1 || n > static_cast<int>(list.size())) ok = false;
                chosen.insert(n);
            }
            if (!ok || !numbers.eof() || chosen.empty()) {
                io_.info("Please type numbers from 1 to " + std::to_string(list.size()) + ".");
                continue;
            }
            for (int c : chosen) ids.push_back(list[static_cast<std::size_t>(c - 1)].submissionId);
        }
    }
    if (!io_.confirm("Approve " + std::to_string(ids.size()) + " mark(s)?")) return;
    moderation_.approve(hod, ids);
    io_.info(std::to_string(ids.size()) + " mark(s) approved. Release them when ready.");
}

// UC13 (FR-16): the reason is required and recorded in the audit log.
void HodMenu::amend(const HeadOfDepartment& hod) {
    const auto a = pickAssignment(hod);
    if (!a) return;
    const auto list = moderation_.listAmendable(hod, a->assignment.getId());
    std::vector<std::string> items;
    for (const SubmissionView& v : list) items.push_back(describe(v));
    const int pick = io_.choose("Choose the mark to amend", items);
    if (pick < 0) return;
    const SubmissionView& v = list[static_cast<std::size_t>(pick)];
    if (v.status == SubmissionStatus::RELEASED) {
        io_.info("This result is already released: the student will see the new mark.");
    }
    const int max = a->assignment.getMaxMark();
    const double raw = io_.readMark("New raw mark (0-" + std::to_string(max) + "): ", max);
    const std::string reason = io_.readText("Reason for the amendment: ", 500);
    if (!io_.confirm("Change the raw mark of " + v.studentName + " to " + showMark(raw) + "?")) {
        return;
    }
    const Mark m = moderation_.amend(hod, v.submissionId, raw, reason);
    io_.info("Amended. Final mark is now " + showMark(m.finalMark) + ".");
}

// UC12 part 2 (FR-17, FR-22): preview, confirm, then release everything at once.
void HodMenu::release(const HeadOfDepartment& hod) {
    const auto a = pickAssignment(hod);
    if (!a) return;
    const ReleasePreview p = moderation_.releasePreview(hod, a->assignment.getId());
    io_.heading("Release marks: " + p.assignmentTitle);
    io_.info(std::to_string(p.toRelease) + " moderated mark(s) will be released.");
    if (p.notReady > 0) {
        io_.info(std::to_string(p.notReady) +
                 " submission(s) are not moderated yet and stay hidden from students.");
    }
    if (p.toRelease == 0) return;
    io_.info("Students see released marks at once, and the assignment closes (BR-06).");
    if (!io_.confirm("Release now?")) return;
    const int n = moderation_.releaseAll(hod, a->assignment.getId());
    io_.info(std::to_string(n) + " mark(s) released.");
}

// UC14 (FR-20): always limited to the HoD's own department by the service.
void HodMenu::viewAudit(const HeadOfDepartment& hod) {
    AuditFilter filter;
    const int how =
        io_.choose("Audit log", {"All mark changes in my department", "Only one course",
                                 "Only one assignment", "Only one student (by number)"});
    if (how < 0) return;
    if (how == 1 || how == 2) {
        const auto a = pickAssignment(hod);
        if (!a) return;
        if (how == 1) {
            filter.courseId = a->assignment.getCourseId();
        } else {
            filter.assignmentId = a->assignment.getId();
        }
    } else if (how == 3) {
        filter.studentId = io_.readInt("Student user number: ", 1, 1000000000);
    }
    const auto entries = moderation_.viewAudit(hod, filter);
    io_.heading("Audit log (" + std::to_string(entries.size()) + " entries)");
    for (const AuditEntry& e : entries) {
        const std::string before = e.oldRawMark && e.oldFinalMark
                                       ? showMark(*e.oldRawMark) + "/" + showMark(*e.oldFinalMark)
                                       : "-";
        io_.info(showTime(e.changedAt) + " " + toString(e.changeType) + " submission " +
                 std::to_string(e.submissionId) + ": " + before + " -> " + showMark(e.newRawMark) +
                 "/" + showMark(e.newFinalMark));
        io_.info("    by " + moderation_.nameOf(e.changedBy) +
                 (e.reason.empty() ? "" : ", reason: " + e.reason));
    }
}

}  // namespace lms
