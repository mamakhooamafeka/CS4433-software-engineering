# Quality assurance report (Phase 6)

Moderated LMS v1.0.0 · Mafeka M.M. (202321161) · CS4433

## 1. Summary

| Check | Before Phase 6 | After Phase 6 |
|---|---|---|
| GCC warnings (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion`) | 0 | 0 |
| cppcheck findings (warning, style, performance, portability) | 23 style (`useStlAlgorithm`) | 0 (the 23 are accepted, section 2.3) |
| clang-tidy findings (broad check set, section 2.2) | 128 | 0 (19 fixed, 109 accepted with reasons) |
| Functions over the coding-standard limits (40 lines or complexity 10) | 11 | 7, each justified (section 3.3) |
| Average cyclomatic complexity per function | 2.5 | 2.5 |
| Automated tests | 114 | 115 (375 assertions) |
| Line coverage, business logic | 93.7% | 93.7% |
| Layer violations found in review | 1 | 0 |

Static analysis now runs in CI on every push (`.github/workflows/ci.yml`, job "Static analysis"), so a new finding fails the build.

## 2. Static analysis

### 2.1 Tools

| Tool | Version | Settings |
|---|---|---|
| cppcheck | 2.13 | `--enable=warning,style,performance,portability --std=c++17` |
| clang-tidy | 18 | `.clang-tidy` in the repository root: families `bugprone`, `performance`, `modernize`, `readability`, `cppcoreguidelines`, `misc`, with the exclusions in section 2.3 |
| Compiler | GCC 13, MSVC 2022 | `-Wall -Wextra -Wpedantic -Wshadow -Wconversion`; `/W4 /permissive-` |

The earlier Phase 4 review (`REVIEW.md`) ran clang-tidy with only `bugprone`, `performance` and `modernize-use-override`. Phase 6 widened it to six full check families, which is why new findings appeared.

### 2.2 clang-tidy findings and what was done

| Check | Found | Fixed | Action |
|---|---|---|---|
| `misc-const-correctness` | 9 | 9 | Variables that never change are now `const` |
| `cppcoreguidelines-special-member-functions` | 16 | 5 | RAII owners (`SqliteDatabase`, `Transaction`, `EchoOff`, `MenuScreen`) now also delete move operations. The other 11 are pure interfaces and `User`; accepted (2.3) |
| `cppcoreguidelines-pro-type-static-cast-downcast` | 1 | 1 | `AuthService` now uses `dynamic_cast` to reach `HeadOfDepartment` |
| `readability-function-cognitive-complexity` | 1 | 1 | `HodMenu::moderate` (26) split: number reading moved to `readChosenNumbers` |
| `readability-avoid-nested-conditional-operator` | 1 | 1 | Replaced by a `switch` in `pendingWhat()` |
| `readability-named-parameter` | 1 | 1 | Parameter named in `Administrator::canAccessCourse` |
| `performance-inefficient-vector-operation` | 13 | 1 | `reserve()` added in the data layer, where lists can be long. The 12 in menus build lists of a few items for the screen; accepted |
| `cppcoreguidelines-avoid-const-or-ref-data-members` | 54 | 0 | Accepted (2.3) |
| C arrays, array decay, varargs, `reinterpret_cast` (5 checks) | 20 | 0 | Accepted (2.3) |
| `bugprone-empty-catch` | 3 | 0 | Accepted (2.3) |
| `performance-enum-size` | 4 | 0 | Accepted (2.3) |
| `modernize-use-auto`, `modernize-return-braced-init-list` | 4 | 0 | Accepted (2.3) |
| **Total** | **128** | **19** | 109 accepted |

### 2.3 Findings accepted on purpose

| Finding | Why it is kept |
|---|---|
| Reference data members (54) | Services and repositories receive their dependencies as references, which is the dependency-injection design from Phase 3. `main.cpp` creates every object and keeps it alive for the whole run, so a reference can never dangle. |
| C arrays, array decay, varargs, `reinterpret_cast` (20) | Only at the edges where the code calls C libraries: `std::snprintf` for formatting, libsodium's fixed-size hash buffer, SQLite's `unsigned char*` text, and `std::ifstream::read` into a byte buffer. These APIs require exactly these types. |
| Special member functions on interfaces and `User` (11) | Pure interfaces are never copied; the rule of five adds ten lines of boilerplate each with no benefit. |
| Empty `catch` (3) | Two in `ConsoleIO` mean "not a number, ask again" and the message is printed right after. One is in the `Transaction` destructor, which must never throw. Each has a comment. |
| Enum base type (4) | Enums are stored as text in the database; saving 3 bytes per value has no effect. |
| `auto` and braced returns (4) | The explicit type reads more clearly in date arithmetic and in repository mapping code. |
| cppcheck `useStlAlgorithm` (23) | Plain loops that build short menu lists are easier to read and explain than `std::transform` with a lambda. |
| Menu `push_back` without `reserve` (12) | Lists shown on screen have a handful of items; the cost is not measurable. |

## 3. Code metrics

Measured with lizard 1.17 (NLOC = non-comment, non-blank lines of code; CCN = cyclomatic complexity).

### 3.1 Size

| Part | Files | Lines | NLOC |
|---|---|---|---|
| Domain (`src/domain`) | 24 | 1,238 | 929 |
| Services (`src/services`) | 19 | 1,386 | 1,128 |
| Data access (`src/data`) | 8 | 959 | 798 |
| Infrastructure (`src/infrastructure`) | 7 | 277 | 212 |
| Console UI (`src/presentation`) | 11 | 1,225 | 1,006 |
| Program start (`app`) | 1 | 121 | 106 |
| **Production code** | **70** | **5,206** | **4,179** |
| Tests (`tests`) | 14 | 1,730 | 1,476 |
| Database schema (`sql/schema.sql`) | 1 | 133 | — |

Test code is 35% of the size of the production code.

### 3.2 Complexity

342 functions, average 8.5 NLOC and CCN 2.5.

| CCN | Functions | Share |
|---|---|---|
| 1 to 5 (simple) | 307 | 89.8% |
| 6 to 10 (moderate) | 32 | 9.4% |
| Over 10 | 3 | 0.9% |

Key functions, the ones that carry the business rules:

| Function | Rule | NLOC | CCN |
|---|---|---|---|
| `TenPercentPerDayPolicy::apply` | BR-01 late penalty | 11 | 3 |
| `DeadlineCalculator::daysLate` | BR-01 days late | 9 | 2 |
| `DeadlineCalculator::effectiveDeadline` | BR-02 extensions | 7 | 2 |
| `Submission::addVersion` | BR-03 resubmission | 14 | 4 |
| `Submission::reevaluateLateness` | DC-01 | 6 | 4 |
| `SubmissionService::submit` | UC16, NFR-05 | 39 | 5 |
| `MarkingService::computeMark` | UC11, BR-01 | 16 | 1 |
| `ModerationService::amend` | UC13 | 22 | 3 |
| `ModerationService::releaseAll` | FR-17 atomic release | 15 | 3 |
| `AssignmentService::grantExtension` | UC10, BR-02, DC-01 | 32 | 8 |
| `AuthService::login` | FR-02 lockout | 28 | 8 |

All key functions are within the coding-standard limits (under 40 lines and CCN under 10).

### 3.3 Functions over the limits

| Function | Before | After | Decision |
|---|---|---|---|
| `StudentMenu::submit` | 55 lines, CCN 14 | 17 lines, CCN 6 | Split into `askForFile`, `showPreview`, `showReceipt` |
| `AuthService::pendingWorkCount` | 41 lines, CCN 14 | 18 lines, CCN 6 | Three near-identical loops replaced by `countSubmissions` and `countUnsubmitted` |
| `HodMenu::viewAudit` | 30 lines, CCN 11 | 16 lines, CCN 6 | Filter choice moved to `chooseAuditFilter` |
| `HodMenu::moderate` | cognitive complexity 26 | CCN 9 | Number reading moved to `readChosenNumbers` |
| `label` (MenuOption text) | CCN 19 | — | Kept: a flat `switch` with one `return` per menu option; each case is trivial |
| `AdminMenu::manageAccounts`, `manageDepartmentsAndCourses` | CCN 12 and 11 | — | Kept: one short branch per menu choice, read top to bottom |
| `run` in `main.cpp` | 66 lines | — | Kept: the composition root that creates and connects every object; splitting it would hide the wiring |
| `SubmissionService::submit` | 42 lines | — | Kept: the file copy, database save and clean-up on failure belong together for NFR-05 |
| `SqliteSubmissionRepository::loadAll`, `save` | 41 lines each | — | Kept: one line per database column |

## 4. Self code review

### 4.1 Checklist

The same checklist is in `.github/pull_request_template.md`, so every future pull request is reviewed against it.

| ID | Check | Result for the whole code base |
|---|---|---|
| C1 | Builds with zero warnings (GCC and MSVC) | Pass |
| C2 | All tests pass; behaviour has tests | Finding QR1, fixed |
| C3 | Layers respected (Phase 3 architecture) | Finding QR2, fixed |
| C4 | Input validated at the service boundary | Pass: `Validation.cpp` helpers used by every service entry point |
| C5 | SQL uses `?` parameters only | Pass: the only string joins add constant column lists to constant clauses (QR12) |
| C6 | Plain error messages; no secrets in logs | Pass (QR13) |
| C7 | RAII, no `new` / `delete` | Pass; move operations made explicit (QR8) |
| C8 | Function size and complexity | Findings QR3–QR6, fixed; exceptions in 3.3 |
| C9 | `const` and `override` | Findings QR7, QR9, fixed |
| C10 | Business rules match the SRS and design | Pass: checked against the traceability table in `REVIEW.md` and the Phase 5 system tests |
| C11 | Formatting and static analysis clean | Pass after section 2; now enforced in CI (QR15) |
| C12 | Documentation up to date | Pass: `CHANGELOG.md`, `CONFIG_MANAGEMENT.md`, this report |

### 4.2 Review record

| ID | Check | Finding | Change made | Verified by |
|---|---|---|---|---|
| QR1 | C2 | FR-23 (pending-work count on the main menu) had no test | New integration test for all four roles | Test "FR-23 pending work counts for each role" passes |
| QR2 | C3 | `BackupService` (services layer) included the concrete `SqliteDatabase` class from the data layer | New `IDatabaseBackup` interface in the domain; `SqliteDatabase` implements it; `BackupService` depends only on the interface | Builds; backup and restore test passes; no `data/` include left in `src/services` |
| QR3 | C8 | `StudentMenu::submit` 55 lines, CCN 14 | Split into three helpers | System tests ST-01 to ST-09 pass |
| QR4 | C8 | `AuthService::pendingWorkCount` 41 lines, CCN 14, duplicated loops | Two helpers | QR1 test passes |
| QR5 | C8 | `HodMenu::viewAudit` CCN 11 | `chooseAuditFilter` | Full suite passes |
| QR6 | C8 | `HodMenu::moderate` cognitive complexity 26 | `readChosenNumbers` | ST-16, ST-17 pass |
| QR7 | C9 | 9 variables could be `const` | Made `const` | clang-tidy clean |
| QR8 | C7 | 4 RAII classes did not state their move operations | Deleted explicitly | clang-tidy clean |
| QR9 | C9 | `static_cast` used to downcast `User` | `dynamic_cast` | QR1 test passes |
| QR10 | C11 | Nested `?:` in the main menu | `pendingWhat()` with a `switch` | Full suite passes |
| QR11 | C11 | No `reserve()` when loading submissions | Added | Full suite passes |
| QR12 | C5 | Checked every `prepare(...)` call | No change needed | — |
| QR13 | C6 | Checked that passwords never reach output or the error log | No change needed | — |
| QR14 | C8 | Seven functions still over a limit | Kept with reasons (3.3) | — |
| QR15 | C11 | Static analysis ran only by hand | `.clang-tidy` added and a CI job that fails on any finding | CI "Static analysis" job |

After all changes: 115 / 115 tests pass, business-logic coverage 93.7% (991 of 1,058 lines), cppcheck and clang-tidy report nothing.
