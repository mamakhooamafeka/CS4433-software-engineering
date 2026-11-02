# Appendix A: Requirements traceability matrix (v1.1.0)

Each functional requirement is linked to its use case, the design elements that realise it, the source files that implement it and the tests that verify it. Every requirement has at least one passing test (129 / 129 tests pass, see [the test results](../TEST_RESULTS.md)).

**Diagram key** (all in [docs/diagrams](../diagrams/README.md)):
UCD = [use case diagram](../diagrams/requirements/use-case-diagram.png),
CMP = [component diagram](../diagrams/design-v1.0/01-component-diagram.png),
CD-a = [domain class diagram](../diagrams/design-v1.1/02a-class-diagram-domain.png),
CD-b = [services class diagram](../diagrams/design-v1.1/02b-class-diagram-services.png),
SM = [submission state machine](../diagrams/design-v1.1/03-state-machine-submission.png),
ACT = [activity diagram](../diagrams/design-v1.0/04-activity-submission-to-release.png),
SQ-1 = [sequence: submit assignment](../diagrams/design-v1.0/05-sequence-submit-assignment.png),
SQ-2 = [sequence: moderate and release](../diagrams/design-v1.0/06-sequence-moderate-release.png),
ERD = [entity relationship diagram](../diagrams/design-v1.1/07-erd.png).

Source files are under `src/` and tests under `tests/`. ST = system test, BV = boundary value test, AT = acceptance test ([acceptance tests](../ACCEPTANCE_TESTS.md)).

| FR | Requirement (short) | Use case | Classes and diagrams | Tables | Source files | Tests |
|---|---|---|---|---|---|---|
| FR-01 | Log in; only the role's menu options | UC01 | `User` (abstract), role subclasses, `AuthService`; CD-a, CD-b | `user_account` | `domain/User.*`, `domain/Roles.*`, `domain/MenuOption.*`, `services/AuthService.*`, `presentation/LoginScreen.*` | "Each role sees only its own menu options", "Only an administrator can manage accounts"; AT-01 |
| FR-02 | Lock for 15 minutes after 5 failed logins | UC01 | `User`, `AuthService`; CD-a | `user_account` | `domain/User.*`, `services/AuthService.*` | "Five wrong passwords lock the account", BV-14, BV-15 |
| FR-03 | Change own password | UC02 | `AuthService`, `Validation`; CD-b | `user_account` | `services/AuthService.*`, `services/Validation.*` | "Changing a password needs the current one", BV-12, ST-24 |
| FR-04 | Create, edit, deactivate accounts | UC03 | `AdminService`, `UserFactory` (Factory); CD-b | `user_account` | `services/AdminService.*`, `domain/UserFactory.*`, `presentation/AdminMenu.cpp` | "A deactivated user cannot log in", ST-27 |
| FR-05 | Reset to a temporary password | UC07 | `AdminService`, `User`; CD-b | `user_account` | `services/AdminService.*`, `presentation/LoginScreen.*` | "A reset password must be changed at the next login", ST-24 |
| FR-06 | Manage departments and courses | UC04 | `Course`, `Department`, `AdminService`; CD-a | `department`, `course` | `domain/Course.hpp`, `services/AdminService.*` | "The administrator sets up departments, courses, lecturers and students", "One HoD cannot head two departments" |
| FR-07 | Assign lecturers, enrol students | UC05, UC06 | `AdminService`; CD-b | `course_lecturer`, `enrolment` | `services/AdminService.*`, `presentation/AdminMenu.cpp` | "The administrator sets up departments, …", "A student not enrolled in the course cannot submit" |
| FR-08 | Create an assignment | UC09 | `Assignment`, `AssignmentService`; CD-a, CD-b | `assignment` | `domain/Assignment.hpp`, `services/AssignmentService.*` | BV-08, BV-09, BV-10 |
| FR-09 | Grant an extension (one student, or whole class since v1.1) | UC10 | `Extension`, `ClassExtension`, `DeadlineCalculator`, `AssignmentService`; CD-a, CD-b, SM | `extension`, `class_extension` | `domain/DeadlineCalculator.*`, `services/AssignmentService.*`, `presentation/LecturerMenu.cpp` | BV-11, "An extension turns LATE back into SUBMITTED", CR1-01 to CR1-11; AT-09 |
| FR-10 | Show courses, assignments and effective deadlines | UC15 | `SubmissionService`, `AssignmentView`; CD-b | `assignment`, `extension`, `class_extension` | `services/SubmissionService.*`, `presentation/StudentMenu.cpp` | ST-01, CR1-12, CR1-13, CR1-14 |
| FR-11 | Accept one file up to 10 MB, with timestamp | UC16, UC19 | `Submission`, `IFileStore`, `LocalFileStore`; CD-a, SQ-1, SM | `submission`, `submission_version` | `services/SubmissionService.*`, `infrastructure/LocalFileStore.*` | BV-05, "On-time submission is stored with size and SHA-256", ST-01 to ST-08 |
| FR-12 | Resubmit before the deadline, keep versions | UC17 | `Submission`; SQ-1, SM | `submission_version` | `domain/Submission.*`, `services/SubmissionService.*` | BV-04, "Resubmission keeps every version", ST-02 |
| FR-13 | Record raw mark and feedback | UC11, UC20 | `MarkingService`, `ILatePenaltyPolicy`, `TenPercentPerDayPolicy` (Strategy); CD-b, ACT | `submission` | `services/MarkingService.*`, `domain/LatePenalty.*` | BV-06, BV-17, "Marks outside 0..M are refused", ST-10 to ST-15 |
| FR-14 | Withhold marks until release | UC11, UC18 | `SubmissionService`; SM | `submission` | `services/SubmissionService.*` | "Students never see marks before release", ST-10 |
| FR-15 | HoD lists own department's marked work | UC12 | `ModerationService`; SQ-2 | `submission`, `course` | `services/ModerationService.*`, `presentation/HodMenu.cpp` | "An HoD sees only their own department's marked work", ST-23 |
| FR-16 | HoD amends a mark with a reason | UC13, UC20 | `ModerationService`, `ILatePenaltyPolicy`; SQ-2 | `submission`, `mark_audit` | `services/ModerationService.*` | "Amend needs a reason, re-applies BR-01 and is audited", ST-18, ST-21 |
| FR-17 | Release all moderated work at once | UC12 | `ModerationService`, `Transaction`; SQ-2, SM | `submission` | `services/ModerationService.*` | "Release is all-or-nothing", ST-16, ST-20 |
| FR-18 | Student sees released mark and feedback | UC18 | `SubmissionService`; SM | `submission` | `services/SubmissionService.*`, `presentation/StudentMenu.cpp` | "Approve, preview and release, then the student sees the result", ST-16 |
| FR-19 | Append every mark change to the audit log | UC21 | `AuditService`, `AuditEntry`; CD-b, ERD | `mark_audit` (with triggers) | `services/AuditService.*`, `sql/schema.sql` | "The audit log is append-only and filterable", ST-14, ST-21 |
| FR-20 | HoD views the department's audit log | UC14 | `AuditService`; CD-b | `mark_audit` | `services/AuditService.*`, `presentation/HodMenu.cpp` | "An HoD's audit view never shows another department" |
| FR-21 | Daily backup and restore | UC08 | `BackupService`, `SqliteDatabase`; CMP | all tables | `services/BackupService.*`, `data/SqliteDatabase.*` | "Backup once per day, and restore brings the old data back", CR1-09 |
| FR-22 | Confirm before irreversible actions | UC03, UC08, UC12 | `ConsoleIO`; CD-b | none | `presentation/ConsoleIO.*` | ST-04, ST-12, ST-20 |
| FR-23 | Show pending work after login | UC01 | `AuthService`, `MenuScreen`; CD-b | none | `presentation/MenuScreen.*`, `services/AuthService.*` | "FR-23 pending work counts for each role" |

## Business rules and non-functional requirements

| ID | Where it lives | Tests |
|---|---|---|
| BR-01 late penalty | `domain/LatePenalty.*` (Strategy) | LatePenaltyTest, BV-01 to BV-03 |
| BR-02 effective deadline | `domain/DeadlineCalculator.*` | BV-11, CR1-01, CR1-03, CR1-05 |
| BR-03 resubmission | `Submission::canResubmit` | BV-04, "Resubmission after the deadline is refused" |
| BR-04 marks hidden until release | `SubmissionService::listReleasedResults` | "Students never see marks before release" |
| BR-05 append-only audit | `sql/schema.sql` triggers, `AuditService` | "The audit log is append-only and filterable" |
| BR-06 closed after release | `SubmissionService::requireOpenAssignment` | ST-09, CR1-07 |
| BR-07 scope of authority | `canAccessCourse`, `requireCourseAccess` | "Course access follows BR-07", ST-23, CR1-07 |
| NFR-05 power loss | `Transaction`, `pragma integrity_check` | "A transaction that is not committed is rolled back" |
| NFR-07 plain errors, no crash | `LmsError`, `MenuScreen::run` | ST-25, ST-26 |
| NFR-08 password hashes | `infrastructure/SodiumPasswordHasher.*` (Argon2id) | "Passwords are stored as Argon2id hashes" |
| NFR-09 parameterised SQL | `data/` (`Statement::bind` only) | "SQL injection text is stored as plain data" |
| NFR-10 data protection | data dictionary, BR-07 checks | all cross-role refusal tests |
| NFR-11 80-column text | `presentation/ConsoleIO.*` | inspection, CR1-12 |
| NFR-12 coverage | CI coverage job | 95.2% business-logic line coverage |
