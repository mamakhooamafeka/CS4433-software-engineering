# Change request CR1: analysis, impact and regression testing (Phase 8)

Moderated LMS v1.0.0 → v1.1.0 · Mafeka M.M. (202321161) · CS4433

## 1. The change request

After release v1.0.0 the school submitted:

> **CR1:** "A lecturer should be able to extend the deadline for a whole class, not just an individual student, and extensions should be shown on the student's assignment list."

The request was clarified into five testable statements before any design work. Each is traced to a requirement in section 3.1.

| # | Clarified meaning | Why |
|---|---|---|
| Q1 | "The whole class" means every student enrolled in the assignment's course, **including students enrolled after the extension** | A class extension describes the assignment, not a list of names; a learner who joins late must not get an earlier deadline than classmates |
| Q2 | A class extension needs a reason and can only move the deadline later, like a personal one | Keeps conflict C3 (reasons are mandatory) and BR-02 (extensions never shorten a deadline) |
| Q3 | A student with a **longer** personal extension keeps it | Taking time away from a learner who already had a valid reason would be unfair |
| Q4 | Work handed in late but before the new deadline becomes on time; **marked** work keeps its mark, and the lecturer is told which marks to review | DC-01 already re-evaluates LATE work; changing a mark automatically would bypass the audit rule that every mark change has a person and a reason (BR-05) |
| Q5 | The student's list shows **that** a deadline was extended and **for whom** (class or personal); the details show the original deadline and the reason of a class extension. A personal reason is not shown | Students must see why their deadline moved; a personal reason (for example a medical note) is private and the screen is on a shared computer (NFR-10) |

## 2. Classification

CR1 is **perfective maintenance**: a change that adds or improves functionality because the users' requirements changed (Sommerville, 2016, ch. 9, "functionality addition or modification"; Lientz and Swanson's perfective category).

| Type | Applies? | Reason |
|---|---|---|
| Corrective (repair a fault) | No | v1.0.0 met FR-09 as specified ("grant **one student** a later deadline") and FR-10. Class-wide extensions were deliberately listed as *Won't have in v1.0* in the SRS, so their absence is not a defect |
| Adaptive (fit a changed environment) | No | The operating system, hardware, SQLite and the school's single-computer set-up are unchanged. The database migration is a consequence of the new feature, not of a change in the environment |
| Perfective (new or improved functionality) | **Yes** | The users asked for a new capability (one action that extends a whole class) and for more information on an existing screen (which extension applies, the original deadline and the reason) |
| Preventive (make future change easier) | Partly, as a side effect | The numbered migration mechanism added for this change (section 4.2) will make every future schema change safe for existing schools |

The change is a **MINOR** version, v1.1.0, under the project's semantic versioning: it adds functionality and keeps every v1.0.0 database working (`CONFIG_MANAGEMENT.md`, section 4).

## 3. Impact analysis

The analysis started from the requirements and followed the traceability links (SRS → use cases → design elements → code → tests). Every item below was checked, including those found **not** to need a change, so the list is complete rather than only the edits.

### 3.1 Requirements and rules

| Item | Impact | v1.1.0 wording (changes in bold) |
|---|---|---|
| FR-09 | Unchanged | Let a lecturer grant one student a later deadline for an unmarked submission, with a mandatory reason |
| **FR-24 (new, Must)** | Added | **Let a lecturer of the course grant the whole class a later deadline for an assignment whose marks are not released, with a mandatory reason that the students can see. It applies to every enrolled student, including later enrolments** (Q1, Q2) |
| FR-10 | Modified | Show a student their enrolled courses and, per assignment, their effective deadline, **whether it was extended for the whole class or for them only, the original deadline and the reason of a class extension**, and their submission status (Q5) |
| BR-02 | Modified | Effective deadline = the latest of **the assignment deadline, every class extension deadline for the assignment** and every extension deadline granted to that student. An extension may only move a deadline later (Q2, Q3) |
| BR-06 | Reused | A class extension is refused once marks for the assignment are released |
| DC-01 | Extended | A class extension re-evaluates every LATE submission of the assignment (Q4) |
| DC-02 | Interpreted | A personal extension is refused for marked work; a class extension is allowed but leaves marked work unchanged and reports it (Q4) |
| C3 | Reused | The reason is mandatory for both kinds of extension |
| NFR-05 | Checked | The database upgrade runs in one transaction (section 4.2) |
| NFR-10 | Checked | Personal extension reasons stay off the student's screen (Q5) |
| NFR-11 | Checked | List lines with the new tags fit in 80 columns (tags `(ext. class)` and `(ext. you)` plus a key) |
| SRS "Won't have in v1.0" | Updated | Class-wide extensions and their display move into v1.1.0 |
| Other FR, NFR and BR | No impact | Checked: login, accounts, marking rules (BR-01), moderation (BR-04, BR-07), audit (BR-05), backups (FR-21) |

### 3.2 Use cases

| Use case | Impact |
|---|---|
| UC10 Grant extension | New alternative flow **3b "Whole class"**: after choosing the assignment the lecturer chooses who gets the extension; for the whole class the system shows the current class deadline, asks for the new deadline and the reason, confirms, stores the class extension, re-evaluates LATE submissions and reports the counts. Exception flows: deadline not later (BR-02), marks released (BR-06), empty reason (C3) |
| UC15 View assignments | Steps 2 and 3 show the extension tag, the original deadline and the class reason |
| UC16 Submit, UC17 Resubmit, UC19 Determine lateness | No change to the steps; the effective deadline they use now includes class extensions (BR-02) |
| UC11 Mark, UC20 Correct mark | No change to the steps; the deduction is worked out from the effective deadline, so a correction after a class extension removes an unfair deduction |
| UC12 Moderate and release, UC13 Amend | No change |
| Use case diagram | No change: CR1 is a new flow of UC10, not a new use case or actor |

### 3.3 Design diagrams

The updated diagrams are in `docs/diagrams/design-v1.1/` as draw.io sources and PNG images, with every changed element in blue.

| Diagram | Impact |
|---|---|
| 01 Component diagram | No change: no new component or interface between layers |
| 02a Class diagram, domain | **Changed:** new `ClassExtension` entity associated with `Assignment` (1 to 0..*); new value object `EffectiveDeadline` and enumeration `ExtensionScope`; `DeadlineCalculator` gains `classDeadline()` and `deadlineFor()` |
| 02b Class diagram, services | **Changed:** `AssignmentService.grantClassExtension()` and `classDeadline()`; `IAssignmentRepository` gains `classExtensionsFor()` and `insertClassExtension()`; `SqliteDatabase` gains `upgradeSchema()`; `LecturerMenu`, `StudentMenu`, `SubmissionService` and `SqliteAssignmentRepository` change inside |
| 03 State machine, Submission | **Changed label only:** the LATE → SUBMITTED transition is triggered by an extension for one student **or the whole class**. No new state or transition |
| 04 Activity diagram | No change: extensions are not a step of the submission-to-release flow |
| 05 Sequence, Submit assignment | No change: `effectiveDeadline()` is still called at the same points |
| 06 Sequence, Moderate and release | No change |
| 07 ERD | **Changed:** new table `class_extension` (section 3.5) |
| Deployment diagram | No change: the upgrade runs inside `lms.exe` on the same computer |

### 3.4 Classes and source files

| File | Change |
|---|---|
| `src/domain/Assignment.hpp` | New `ClassExtension`, `ExtensionScope`, `EffectiveDeadline` |
| `src/domain/DeadlineCalculator.hpp/.cpp` | BR-02 now takes the latest of the assignment deadline, class extensions and the student's own extensions, and says which one applies |
| `src/domain/Repositories.hpp` | Two new functions on `IAssignmentRepository` |
| `src/data/SqliteAssignmentRepository.cpp`, `SqliteRepositories.hpp` | The two functions, with parameterised SQL (NFR-09) |
| `src/data/SqliteDatabase.hpp/.cpp` | `upgradeSchema()`, run when the database opens and after a restore |
| `sql/migrations/001-class-extension.sql`, `cmake/SchemaSql.hpp.in`, `CMakeLists.txt` | Migration 1 embedded in the program; version 1.1.0 |
| `src/services/AssignmentService.hpp/.cpp`, `Views.hpp` | `grantClassExtension()`, `classDeadline()`, `ClassExtensionResult`; `AssignmentView` carries the extension scope and class reason |
| `src/services/SubmissionService.cpp` | Fills the same fields for the submit list |
| `src/presentation/LecturerMenu.cpp`, `RoleMenus.hpp` | "Who gets the extension?" choice and the class flow |
| `src/presentation/StudentMenu.cpp` | Tags, key, original deadline and reason |
| `src/domain/MenuOption.cpp`, `app/main.cpp` | Menu text "Grant an extension"; banner v1.1 |
| Not changed | `Submission`, `LatePenalty` (Strategy), `MarkingService`, `ModerationService`, `AuditService`, `AuthService`, `AdminService`, `BackupService`, `User` hierarchy and `UserFactory` |

306 lines of production code were added or changed in 21 files. The late-penalty Strategy, the state machine and the marking and moderation services did not change at all, because Phase 3 isolated "where the deadline comes from" in `DeadlineCalculator` and the `extension` entity (risk R06).

### 3.5 Database tables

| Table | Impact |
|---|---|
| `class_extension` (new) | `id` PK; `assignment_id` FK → `assignment`; `new_deadline`; `reason` (CHECK not blank); `granted_by` FK → `user_account`; `granted_at`; index on `assignment_id`. In 3NF: every column depends only on the key; the class is identified through the assignment's course, so no course or student column is stored twice |
| `extension` | No change |
| `submission` | No column change; LATE rows can become SUBMITTED, by the same rule as in v1.0.0 |
| `assignment` | No change: the original deadline is kept, which is what lets the student see it (Q5) |
| `mark_audit` | No change: no mark is changed automatically, so no audit row is needed |
| Schema version | `pragma user_version` is now 1 (it was 0 in v1.0.0) |

### 3.6 Test cases

Existing tests that exercise the changed code. They had to pass **unchanged**:

| Existing test | What it protects |
|---|---|
| "Effective deadline is the latest extension (BR-02)" (UserTest) | BR-02 for personal extensions after `DeadlineCalculator` was rewritten |
| "An extension turns LATE back into SUBMITTED (DC-01)" (SubmissionTest) | The state transition reused by class extensions |
| "An extension needs a reason, moves the deadline later and fixes LATE (DC-01)", "An extension can be granted before the student submits (DC-02)" (SubmissionFlowTest) | UC10 main flow for one student |
| BV-01, BV-03, BV-11 (BoundaryValueTest) | Days late, the deadline second and the extension boundaries |
| ST-01 to ST-09 (SystemTest) | The student's submit and assignment lists that now carry tags |
| ST-10 to ST-14 | Marking and correction, which use the effective deadline |
| AT-03, AT-04, AT-05 | Creating assignments, submitting, and late submission on Windows |

The only edit to existing test code: the test fake `FakeAssignments` in `UserTest.cpp` gained the two new repository functions (3 lines), because a C++ class must implement every pure virtual function of its interface. No test case was changed.

New tests are listed in section 5.

### 3.7 Other documents

| Document | Change |
|---|---|
| `docs/manuals/USER_MANUAL_LECTURER.md` | Section 4 rewritten (4.1 one student, 4.2 whole class); problems table |
| `docs/manuals/USER_MANUAL_STUDENT.md` | Section 3 (tags, key, details) and section 5 |
| Other two manuals | Version number only |
| `docs/INSTALL.md` | Version, package name, upgrade note in section 10 |
| `docs/RELEASE_NOTES.md`, `CHANGELOG.md`, `README.md` | v1.1.0 |
| `docs/CONFIG_MANAGEMENT.md`, `docs/DEPLOYMENT.md` | Release table; version wording |
| `docs/TEST_PLAN.md`, `docs/TEST_RESULTS.md`, `docs/ACCEPTANCE_TESTS.md` | Regression section and AT-09 |

### 3.8 Risk R06

The Phase 1 risk register predicted CR1 (R06: "Change request CR1 larger than expected", trigger "CR1 needs a schema change beyond one new column or table"). The change needed exactly **one new table** and no change to existing tables, so the trigger was not reached and the mitigation (model extensions as a separate entity, keep the penalty rule isolated) worked. R06 can be closed.

## 4. Design decisions

### 4.1 How to store a class extension

| Option | Decision | Reason |
|---|---|---|
| A. New table `class_extension` (chosen) | **Chosen** | One row per decision, with its own reason, author and time. Students enrolled later are covered automatically (Q1). The list can tell class from personal extensions (Q5). Existing tables and data are untouched, so the upgrade is a single `create table` |
| B. Make `extension.student_id` nullable, where NULL means "whole class" | Rejected | A NULL foreign key with a hidden meaning is easy to misread in queries. SQLite cannot drop a NOT NULL constraint, so the `extension` table would have to be copied and rebuilt on every school's database, which is the riskiest kind of migration |
| C. Insert one `extension` row per enrolled student | Rejected | Students enrolled after the extension would not get it (fails Q1); the list could not tell a class extension from a personal one (fails Q5); one decision would be stored many times (an update anomaly) |
| D. Change `assignment.deadline` | Rejected | The original deadline, which students must see (Q5), would be lost, and so would who changed it, when and why. It would also break BR-02's rule that a deadline only moves through extensions |

### 4.2 How existing databases are upgraded

v1.0.0 creates its tables only when the database is new, so v1.1.0 adds a numbered migration. `SqliteDatabase::upgradeSchema()` reads `pragma user_version` and runs every migration above it, each in its own transaction that also sets the new version. A power cut leaves the old or the new version, never half (NFR-05). The same step runs after a backup is restored, so a v1.0.0 backup still works (FR-21). A brand-new database is created by the v1.0.0 schema followed by the migrations, so new and upgraded databases are identical by construction.

### 4.3 How the lecturer reaches it

The whole-class choice is a question inside the existing option `2. Grant an extension`, not a new menu option. Every menu number stays the same, so the manuals' other sections, the acceptance tests and the scripted system tests did not need renumbering, and users who learned v1.0.0 find everything where it was.

### 4.4 Marked work

A class extension does not change marks by itself. A mark change must be made by a person with a reason and audited (BR-05, FR-19, FR-20). Instead the lecturer is told how many marked submissions carry a late deduction although they are on time under the new deadline. A correction (UC20) recalculates the deduction with the new deadline. A mark the HoD already moderated is changed by the HoD (UC13).

## 5. Regression testing

### 5.1 Strategy

1. **Re-run the whole v1.0.0 suite unchanged** (115 test cases) after the change, on Linux (GCC) and Windows (MSVC), locally and in CI. A full re-run was chosen over a selected subset because the suite runs in under a minute, and `DeadlineCalculator`, which every lateness calculation uses, was rewritten.
2. **Add new tests for CR1** at every V-Model level, using equivalence partitioning and boundary values for the new deadline.
3. **Test the upgrade** on a database made exactly as v1.0.0 makes it, and on a real v1.0.0 database (the one used for the Phase 7 manual screens).
4. Keep the CI gates: business-logic coverage of at least 70%, cppcheck and clang-tidy with zero findings.
5. **Acceptance test AT-09** (`ACCEPTANCE_TESTS.md`), run through the real program with each step typed by a script.

### 5.2 New test cases (`tests/ClassExtensionTest.cpp`)

| ID | Level | What it checks | Requirement |
|---|---|---|---|
| CR1-01 | Unit | The latest of class and personal extensions wins, and the scope and class reason are reported; an earlier personal extension does not override a later class one | BR-02, FR-10 |
| CR1-02 | Integration | A class extension reaches every enrolled student, also one enrolled afterwards; a student who is not enrolled still cannot submit | FR-24 (Q1), FR-02 |
| CR1-03 | Integration (BVA) | Empty reason refused; one second before and exactly on the class deadline refused; one second after accepted; the new class deadline becomes the next limit | FR-24, BR-02, C3 |
| CR1-04 | Integration | A LATE submission before the new deadline becomes SUBMITTED; one after it stays LATE and is then marked 1 day late | DC-01, BR-01 |
| CR1-05 | Integration | A longer personal extension is kept; a new personal extension must beat the class deadline | BR-02 (Q3) |
| CR1-06 | Integration | Marked work keeps its mark and is counted; a correction removes the deduction | DC-02, UC20 (Q4) |
| CR1-07 | Integration | Refused after release, and for a lecturer of another course | BR-06, BR-07 |
| CR1-08 | Data | A v1.0.0 database is upgraded in place, keeps its data, and opening it again changes nothing | NFR-05 |
| CR1-09 | Data | A v1.0.0 backup is upgraded when restored | FR-21 |
| CR1-10 | System | The lecturer's whole-class flow through the real menu | UC10 3b |
| CR1-11 | System | Answering `n`, or an earlier date, stores nothing and shows the BR-02 message | UC10 3b exceptions |
| CR1-12 | System | The student's list shows `(ext. class)` and the key; the details show the original deadline and the reason; the submit list shows the tag | FR-10, UC15 |
| CR1-13 | System | A personal extension shows `(ext. you)` and its reason is not shown; a classmate sees only the class extension | FR-10, NFR-10 |
| CR1-14 | System | A list with no extension looks exactly as in v1.0.0 | Regression of UC15 |

### 5.3 Results

| Run | Result |
|---|---|
| v1.0.0 test cases on the v1.1.0 code (Linux, GCC 13.3) | **115 / 115 passed** |
| New CR1 test cases | **14 / 14 passed** |
| Whole suite | **129 / 129 passed**, 0 compiler warnings with `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` |
| Business-logic line coverage | **95.2%** (1,062 of 1,116 lines; v1.0.0: 93.7%). All new service and domain lines are covered |
| cppcheck and clang-tidy | 0 findings. clang-tidy found one unchecked `std::optional` access in the first version of `grantClassExtension`; it was fixed before commit |
| Mutation check | Removing the upgrade after a restore makes CR1-09 fail, so the test really detects the fault it is meant to find |
| Real v1.0.0 database (Phase 7 data) opened with v1.1.0 | Upgraded to `user_version` 1, all accounts, marks and submissions kept; a class extension then worked through the menus |
| Windows (MSVC + vcpkg), student laptop | 129 / 129 passed; `[cr1]`: 14 test cases, 74 assertions, all passed |
| AT-09 acceptance test | Pass: the expected screens of every step appeared |
| GitHub Actions | Run on the pull request (screenshot in the Phase 8 chapter) |

### 5.4 Screens from the real program (v1.1.0 on the v1.0.0 database)

Lecturer:

```
Who gets the extension?
-----------------------
  1. One student
  2. The whole class
  0. Back
Choose: 2
  The deadline for the whole class is now YYYY-MM-DD 12:00 UTC.
New deadline (YYYY-MM-DD HH:MM, UTC): YYYY-MM-DD 12:00
Reason (the students will see it): School closed for the inter-schools sports day
Extend "Essay 2" for the whole class of CS4433 to YYYY-MM-DD 12:00? (y/n): y
  The extension has been granted to all 1 student(s) of CS4433.
```

Student:

```
  ext. class = deadline extended for the whole class
  ext. you   = deadline extended for you only

My assignments (choose one to see the instructions)
---------------------------------------------------
  1. CS4433 Essay 1 | due YYYY-MM-DD 12:00 (ext. you) | closed
  2. CS4433 Essay 2 | due YYYY-MM-DD 12:00 (ext. class) | not submitted
  0. Back
Choose: 2

Essay 2
-------
  Course: CS4433 Software Engineering
  Deadline: YYYY-MM-DD 12:00 UTC
  Extended for the whole class from YYYY-MM-DD 12:00 UTC.
  Reason: School closed for the inter-schools sports day
```

## 6. Traceability for CR1

| Requirement | Use case | Design | Code | Tests |
|---|---|---|---|---|
| FR-24 | UC10 flow 3b | 02a `ClassExtension`; 02b `AssignmentService.grantClassExtension`; 07 `class_extension` | `AssignmentService.cpp`, `LecturerMenu.cpp`, `SqliteAssignmentRepository.cpp`, migration 1 | CR1-02, 03, 06, 07, 10, 11; AT-09 |
| FR-10 (modified) | UC15 | 02a `EffectiveDeadline`, `ExtensionScope`; 02b `StudentMenu` | `DeadlineCalculator.cpp`, `StudentMenu.cpp`, `Views.hpp` | CR1-01, 12, 13, 14; AT-09 |
| BR-02 (modified) | UC10, UC16, UC19 | 02a `DeadlineCalculator` | `DeadlineCalculator.cpp` | CR1-01, 03, 05; BV-11; existing BR-02 unit test |
| DC-01 / DC-02 | UC10 | 03 LATE → SUBMITTED | `AssignmentService.cpp` | CR1-04, 06 |
| NFR-05 (upgrade) | — | 02b `SqliteDatabase.upgradeSchema` | `SqliteDatabase.cpp` | CR1-08, 09 |

## References

Lientz, B.P. and Swanson, E.B. (1980) *Software Maintenance Management*. Reading, MA: Addison-Wesley.

Sommerville, I. (2016) *Software Engineering*. 10th edn. Harlow: Pearson.
