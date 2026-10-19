# Code review checkpoint (WP 4.8)

Review of the complete v1.0 code against the Phase 3 design and the coding standard.

## 1. Automated checks

| Check | Command | Result |
|---|---|---|
| Compiler warnings | GCC `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` | 0 warnings |
| Tests | `ctest` | 69 / 69 passed (unit, integration, UI) |
| Formatting | `clang-format --dry-run --Werror` | clean |
| cppcheck 2.x | warning, style, performance, portability | clean (only `useStlAlgorithm` suppressed, see 3) |
| clang-tidy | `bugprone-*`, `performance-*`, `modernize-use-override` | clean after the fixes in 2 |
| Windows target | MinGW-w64 syntax check of every source file | clean |

## 2. Findings fixed in this review

| # | Finding | Fix |
|---|---|---|
| R1 | `ConsoleIO` constructor could convert implicitly from a stream | made `explicit` |
| R2 | `windows.h` defines `min`/`max` macros that break `std::min` with MSVC | `NOMINMAX` and `WIN32_LEAN_AND_MEAN` before the include |
| R3 | Optionals read twice (`getMark()` then `*getMark()`), hard to prove safe | read once into a local, then check |
| R4 | Audit view used `*oldRawMark` after checking only `oldFinalMark` | check both |
| R5 | Mixed `int` arithmetic before widening to `long long` in time code | `LL` literals |
| R6 | Unneeded copy of the old mark in `ModerationService::amend` | const reference |
| R7 | Lecturers could not see where the submitted file was saved | `SubmissionView::filePath` shown when marking |

## 3. Decisions kept on purpose

- **Plain loops instead of `std::transform`** (cppcheck `useStlAlgorithm`): the loops that build menu lists are easier to read and explain; the lists are short.
- **Empty `catch` in `ConsoleIO::readInt` / `readMark`**: a failed number conversion simply means "ask again"; the message is printed right after.

## 4. Design fidelity (Phase 3 class diagrams 2a / 2b)

Every class in the diagrams exists with the same name and responsibility. Additions not in the diagrams, all small helpers:

| Addition | Why |
|---|---|
| `IFileStore::restoreFrom`, `copyAll` | backup and restore of submitted files (FR-21) |
| Extra repository queries (`listAll`, `studentsOfCourse`, `byCourse`, `byStudent`, …) | menus need to list things to choose from |
| `AdminService::editUserName`, `createFirstAdmin` | UC03 edit, and the very first start with no accounts |
| `ModerationService::listAmendable`, `nameOf` | UC13 needs MARKED and RELEASED lists; audit view shows names |
| `UserRecord::courseIds`, `departmentId` | scope of authority (BR-07) loaded with the user |
| `MenuScreen::run` as a Template Method | shared menu loop and error handling for the four role menus |

## 5. Requirement traceability (where each rule lives)

| Requirement | Code | Test |
|---|---|---|
| FR-01 role menus | `Roles.cpp` `menuOptions()`, `MenuScreen::run` | `UserTest` |
| FR-02 lock after 5 failures | `User::registerFailedLogin`, `AuthService::login` | `AuthServiceTest` |
| FR-03 password length | `requirePasswordStrength` | `AuthServiceTest` |
| FR-05 temporary password | `LoginScreen::run`, `AdminService::resetPassword` | `AuthServiceTest`, `AdminServiceTest` |
| FR-08 assignment | `AssignmentService::createAssignment` | `FlowFixture` (every flow test) |
| FR-09 extension, DC-01, DC-02 | `AssignmentService::grantExtension`, `Submission::reevaluateLateness` | `SubmissionFlowTest` |
| FR-11 file checks, SHA-256 | `LocalFileStore::validate`, `sha256Of` | `SubmissionFlowTest` |
| BR-01 late penalty | `TenPercentPerDayPolicy` (Strategy) | `LatePenaltyTest` |
| BR-03 resubmission | `Submission::canResubmit` | `SubmissionTest`, `SubmissionFlowTest` |
| BR-04 students see released only | `SubmissionService::listReleasedResults` | `SubmissionFlowTest`, `ModerationTest` |
| BR-05 append-only audit | schema triggers, `AuditService` | `RepositoryTest`, `ModerationTest` |
| BR-06 closed after release | `SubmissionService::requireOpenAssignment` | `ModerationTest` |
| BR-07 scope of authority | `canAccessCourse`, `requireCourseAccess` | `UserTest`, `ModerationTest` |
| FR-17 atomic release | `ModerationService::releaseAll` + `Transaction` | `ModerationTest` |
| FR-21 backup and restore | `BackupService` | `ModerationTest` |
| FR-22 confirmations | `ConsoleIO::confirm` before every irreversible action | `MenuTest` |
| NFR-07 plain errors | `LmsError` hierarchy, `ErrorLog`, `MenuScreen::run` | `MenuTest` |
| NFR-09 parameterised SQL | `Statement::bind` only | `RepositoryTest` (injection test) |

## 6. Known limitations of v1.0

- One user at a time on one computer (SQLite file, console program), as scoped in Phase 1.
- Times are entered and shown in UTC; Lesotho time is UTC+2.
- Class-wide extensions (CR1) are not part of v1.0.
