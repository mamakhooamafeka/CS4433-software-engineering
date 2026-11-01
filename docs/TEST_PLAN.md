# Test plan (Phase 5: verification and validation)

Moderated LMS v1.0 · Mafeka M.M. (202321161) · CS4433

## 1. Purpose and scope

This plan says how v1.0 is checked against what was asked for. **Verification** asks "did we build it right?" (the code matches the design and the rules). **Validation** asks "did we build the right thing?" (the use cases work for the people who will use the system).

In scope: every functional requirement FR-01 to FR-23, the business rules BR-01 to BR-07 and DC-01/DC-02, and the testable NFRs (NFR-05 to NFR-09, NFR-12). Out of scope, as agreed in Phase 1: several users at once, network access, and load testing.

## 2. Test levels (V-Model)

Each level on the right side of the V checks the artefact on the matching left side.

| Level | Artefact it verifies | Test basis | Main techniques | Where | Cases |
|---|---|---|---|---|---|
| Unit | Class diagrams 2a/2b and the business rules (Phase 3 design, SRS rules) | BR-01 decision table, state machine of `Submission`, class responsibilities | Decision table, state transition, **BVA**, **EP** | `tests/*Test.cpp` tagged `[unit]` and `[bva]` | 26 + 18 |
| Integration | Architecture (layers, repositories) and sequence diagrams (Phase 3) | Service + real SQLite repositories + file store, wired as in `main.cpp` | Scenario per sequence diagram, transaction rollback, injection text | `[integration]` | 39 |
| System | SRS: FR/NFR and the fully dressed use cases UC16, UC11, UC12 (Phase 2) | Main, alternative and exception flows of each use case | Use-case flow testing through the real menus with scripted keyboard input | `tests/SystemTest.cpp` `[system]`, `tests/MenuTest.cpp` `[ui]` | 27 + 4 |
| Acceptance | Stakeholder needs and the Phase 1 success criteria | Real tasks done by each role on the real Windows program | Scripted walkthrough by the user, results recorded by hand | `docs/ACCEPTANCE_TESTS.md` | 8 |

Automated total: **114 test cases, 366 assertions.** All automated tests run with one command (`ctest`) and in CI on every push.

## 3. Entry and exit criteria

| Level | Entry criteria (may start when) | Exit criteria (done when) |
|---|---|---|
| Unit | Class compiles with zero warnings; the rule it implements is written in the SRS | Every BR has at least one test; every BVA row in section 5 has a test; 100% of unit tests pass |
| Integration | Units it uses pass; schema loads into an in-memory database | Each sequence diagram has a scenario test; rollback test passes; 100% pass |
| System | Integration tests pass; the menus build into `lms_ui` | Every main, alternative and exception flow of UC16, UC11 and UC12 has a test or a written reason why not (section 6); 100% pass; no open High or Critical defect |
| Acceptance | System tests pass in CI; Windows build passes; build guide followed on a clean data folder | All 8 acceptance tests pass on Windows; no open defect above Low |
| Whole phase | — | Line coverage of business logic (`src/domain` + `src/services`) **at least 70%** (NFR-12), measured in CI, and every defect in `DEFECT_LOG.md` closed or accepted |

## 4. Environment and tools

| Item | Choice |
|---|---|
| Test framework | Catch2 v3 (`Catch2WithMain`), one executable `lms_tests`, registered with CTest by `catch_discover_tests` |
| Test doubles | `FixedClock` (time can be set to the exact second), in-memory SQLite (`:memory:`), temporary folder for files; the real Argon2id hasher |
| Scripted UI input | `ConsoleIO` takes any `std::istream`, so a test feeds typed lines such as `"2\n1\n<path>\ny\n5\n"` |
| Coverage | GCC `--coverage -O0` + gcovr; HTML report in `docs/test-results/coverage/index.html` |
| CI | GitHub Actions, `.github/workflows/ci.yml`: Linux (GCC, tests, coverage gate at 70%) and Windows (MSVC + vcpkg, tests) |
| Platforms | Ubuntu 24.04 GCC 13; Windows 10/11 MSVC Build Tools 2022 |

## 5. Equivalence partitioning and boundary value analysis

EP splits each input into classes that the program should treat the same way; one value from each class is tested. BVA then tests the values on, just below and just above each limit, because that is where off-by-one mistakes hide.

### 5.1 Deadline and days late (BR-01, BR-02)

`d` = number of started 24-hour periods after the effective deadline `D`.

| Class | Submitted at | Expected `d` | Expected status / penalty | Test |
|---|---|---|---|---|
| On time (valid) | `D - 1 s` | 0 | SUBMITTED, no deduction | BV-01 |
| On the boundary | `D` exactly | 0 | SUBMITTED | BV-01, BV-03 |
| Late 1 day | `D + 1 s` | 1 | LATE, 10% of M | BV-01, BV-03 |
| Upper end of day 1 | `D + 24 h` | 1 | 10% | BV-01 |
| Start of day 2 | `D + 24 h + 1 s` | 2 | 20% | BV-01 |
| Day 2 / day 3 edge | `D + 48 h`, `D + 48 h + 1 s` | 2, 3 | 20%, 30% | BV-01 |
| Last day with a mark | `D + 72 h` | 3 | 30% | BV-01, BV-02 |
| Zero class | `D + 72 h + 1 s` | 4 | final mark 0 | BV-01, BV-02 |

### 5.2 Late penalty (BR-01 decision table, M = 100, raw = 75)

| `d` | 0 | 1 | 2 | 3 | 4 | 10 |
|---|---|---|---|---|---|---|
| Final mark | 75 | 65 | 55 | 45 | 0 | 0 |

Test: "BR-01 decision table" in `LatePenaltyTest.cpp`; never below zero (raw 5, d 3 → 0); penalty is a share of M, not of raw.

### 5.3 Resubmission (BR-03)

| Class | Input | Expected | Test |
|---|---|---|---|
| Before deadline, unmarked (valid) | `D` exactly | version 2 stored, version 1 kept | BV-04, ST-02 |
| After deadline (invalid) | `D + 1 s` | refused, reason shown | BV-04 |
| Already marked (invalid) | any time | refused, existing version kept | ST-06 |
| Assignment released (invalid) | any time | not offered; service refuses (BR-06) | ST-09, ModerationTest |

### 5.4 File to submit (FR-11)

| Class | Size / state | Expected | Test |
|---|---|---|---|
| Missing (invalid) | no such path | plain error, ask again | ST-07, ST-08 |
| Empty (invalid) | 0 bytes | refused | BV-05 |
| Smallest valid | 1 byte | accepted | BV-05 |
| Largest valid | 10 MiB = 10,485,760 bytes | accepted | BV-05 |
| Too big (invalid) | 10 MiB + 1 byte | refused | BV-05 |
| Path in quotes (valid) | `"C:\...\file.docx"` | quotes removed, accepted | ST-08, MenuTest |

### 5.5 Raw mark (FR-13), M = 100

| Class | Typed / given | Expected | Test |
|---|---|---|---|
| Below range (invalid) | -0.1 | refused / asked again | BV-06, BV-17 |
| Lower bound | 0 | accepted | BV-06, BV-17 |
| Upper bound | 100 | accepted | BV-06, BV-17 |
| Above range (invalid) | 100.1 | refused / asked again | BV-06, BV-17 |
| Two decimals typed (invalid at the keyboard) | 12.34 | asked again | BV-17, ST-10 |
| Two decimals given to the service | 67.26 | stored as 67.3 | BV-07 |
| Not a number (invalid) | `abc`, empty | asked again | ST-10 |

### 5.6 Assignment fields (FR-08)

| Input | Invalid below | Valid lower | Valid upper | Invalid above | Test |
|---|---|---|---|---|---|
| Maximum mark M | 0 | 1 | 1000 | 1001 | BV-08 |
| Deadline | `now - 1 s`, `now` | `now + 1 s` | far future (year 2099) | — | BV-09, DateTimeTest |
| Title length | 0 (only spaces) | 1 | 200 | 201 | BV-10 |

### 5.7 Extension (BR-02, DC-01, DC-02)

| Class | New deadline | Expected | Test |
|---|---|---|---|
| Earlier (invalid) | `D - 1 s` | refused (BR-02) | BV-11 |
| Equal (invalid) | `D` | refused | BV-11 |
| Later (valid) | `D + 1 s` | granted; becomes the new limit | BV-11 |
| Missing reason (invalid) | empty | refused | SubmissionFlowTest |
| Before the student submits | — | allowed (DC-02) | SubmissionFlowTest |

### 5.8 Accounts and login (FR-02, FR-03)

| Input | Invalid below | Valid lower | Valid upper | Invalid above | Test |
|---|---|---|---|---|---|
| Password length | 7 | 8 | 128 | 129 | BV-12, ST-24 |
| Username length | 2 | 3 | 32 | 33 | BV-13 |
| Username characters | — | letters, digits, `.` `-` `_` | — | space or other symbols | BV-13 |
| Wrong passwords in a row | — | 4 (not locked) | — | 5 (locked) | BV-14 |
| Lock time | — | 14 min 59 s (still locked) | 15 min (open again) | — | BV-15 |

### 5.9 Menu choices (FR-22, NFR-07)

| Class | Typed (menu of 3) | Expected | Test |
|---|---|---|---|
| Back | 0 | returns to the previous menu | BV-16 |
| Valid | 1, 3 | that item | BV-16 |
| Out of range (invalid) | 4, -1 | asked again | BV-16 |
| Not a number / empty (invalid) | `abc`, empty line | asked again | BV-16 |
| Text too long (invalid) | max + 1 characters | asked again | BV-18 |

## 6. System tests from the use cases

Every flow of the three fully dressed use cases (SRS Tables 7–9) is driven through the real menus.

| ID | Use case flow | Expected result |
|---|---|---|
| ST-01 | UC16 main flow | Open assignment listed, time left shown, receipt with size and SHA-256, status SUBMITTED |
| ST-02 | UC16 3a resubmit | "This becomes version 2"; both versions kept |
| ST-03 | UC16 6a late | Warning "late by 2 day(s): 20% ..."; stored as LATE |
| ST-04 | UC16 8a cancel | Nothing stored |
| ST-05 | UC16 1a none open | "There is nothing to show." |
| ST-06 | UC16 3b refused | Reason shown and existing version and status shown; nothing added |
| ST-07 | UC16 5a three bad files | Plain errors; back to the menu; nothing stored |
| ST-08 | UC16 5a then a good file | Continues to the receipt |
| ST-09 | UC16 precondition (BR-06) | Released assignment no longer offered |
| ST-10 | UC11 main flow + 4a | Bad marks asked again; deduction and final shown before saving; MARKED; student cannot see it |
| ST-11 | UC11 5a | Empty feedback asked again (not thrown back to the menu) |
| ST-12 | UC11 step 7 | Answering "n" saves nothing |
| ST-13 | UC11 2a | Only unmarked submissions listed |
| ST-14 | UC11 3a correct | Empty reason asked again; audit keeps old and new values |
| ST-15 | UC11 3b | MODERATED mark not offered for correction |
| ST-16 | UC12 main flow | Counts shown, feedback shown, approve all, preview, release; student sees 64.0 / 100 |
| ST-17 | UC12 step 4 (some) | Bad numbers asked again; only the chosen one MODERATED |
| ST-18 | UC12 4a amend | Penalty re-applied (70 − 10 = 60); MODERATED |
| ST-19 | UC12 5a and step 6 | Stop after approving; preview counts the not-ready one |
| ST-20 | UC12 7a cancel | Nothing released |
| ST-21 | UC12 9a amend after release | Warning shown; student sees the new mark; audit keeps both values |
| ST-22 | UC12 1a | "There are no marked submissions waiting for moderation." |
| ST-23 | UC12 2a | Other department's HoD sees nothing; nothing changes |
| ST-24 | UC01 first login | Temporary password must be changed; 7-character password refused |
| ST-25 | NFR-07 | 100 invalid inputs (empty, words, huge numbers, `%s%n`, SQL text, 5,000 characters) cause zero crashes |
| ST-26 | NFR-07 | Input ending mid-way ends the program cleanly |
| ST-27 | UC03 cancel (D14) | Enter on an empty username or full name cancels account creation |

Flows checked at integration level instead of through the menus, with the reason:

| Flow | Why not through the menus | Covered by |
|---|---|---|
| UC16 9a, UC11 8a, UC12 8a (database error → rollback) | A real disk or database failure cannot be produced on demand from the keyboard | "A transaction that is not committed is rolled back" (RepositoryTest), "Release is all-or-nothing" (ModerationTest), "The database refuses a mark that breaks final = raw − deduction" |

## 7. Acceptance tests

Run by the student on the Windows build, in the role of each stakeholder: see `docs/ACCEPTANCE_TESTS.md`.

## 8. Defects

Severity scale used in `docs/DEFECT_LOG.md`:

| Severity | Meaning |
|---|---|
| Critical | Wrong mark, lost data or a security hole |
| High | A use case cannot be completed |
| Medium | A use case works but differs from its specification |
| Low | Cosmetic, wording, or a build/documentation problem with a workaround |

Each defect is retested by the test that found it, and the whole suite is run again (regression) before it is closed.

## 9. How to run

```
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

One group only, for example the system tests: `build\Release\lms_tests.exe "[system]"` (Windows) or `./build/lms_tests "[system]"` (Linux).
