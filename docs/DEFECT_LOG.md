# Defect log

Severity scale: **Critical** wrong mark, lost data or security hole · **High** a use case cannot be completed · **Medium** works but differs from the specification · **Low** cosmetic, wording, or build/documentation with a workaround.

Every fix was retested with the test that found it, then the whole suite was run again (regression): 114 / 114 pass.

## Found by the Phase 5 system tests

These six tests failed on the v1.0 code and pass after the fixes (run on 31 Oct 2026: 44 new cases, 38 passed, 6 failed before; 44 / 44 after).

| ID | Found by | Severity | Defect | Cause | Fix | Retest |
|---|---|---|---|---|---|---|
| D01 | ST-11 | Medium | UC11 5a: empty feedback threw the lecturer back to the main menu and the typed mark was lost, instead of asking for the feedback again | The menu read feedback with "empty allowed"; only the service rejected it, so the error ended the whole step | `LecturerMenu::markSubmissions` reads feedback with `readText(..., 2000)`, which asks again until it is not empty | ST-11 pass |
| D02 | ST-10, ST-12 | Medium | UC11 steps 6–7: the mark was saved at once, without first showing the late deduction and final mark and without asking for confirmation (FR-22) | No way to work out a mark without saving it | New `MarkingService::preview` (same calculation, nothing saved); the menu shows raw, deduction, days late and final mark, then asks "Save this mark?" | ST-10, ST-12 pass |
| D03 | ST-06 | Low | UC16 3b: a refused resubmission gave the reason but did not show the existing submission | Not implemented in the menu | Shows "Your version N is kept, status ..." | ST-06 pass |
| D04 | ST-01 | Low | UC16 step 3: time remaining before the deadline was not shown | Only the deadline and the time now were printed | New `timeLeft()` shows "x day(s) y h z min" | ST-01 pass |
| D05 | ST-16 | Low | UC12 step 3: the HoD's moderation list did not show the lecturer's feedback | `describe()` left feedback out | Feedback added to each line | ST-16 pass |
| D06 | Test design for ST-16 | Low | UC12 step 3: the extension reason is not shown in the moderation list | `SubmissionView` does not carry extensions | **Accepted for v1.0** (deferred): days late already reflects the extension, and the reason is kept in the database; planned with CR1 in Phase 8 | — |

## Found by static analysis and the code review (Phase 4, WP 4.8)

| ID | Found by | Severity | Defect | Cause | Fix | Retest |
|---|---|---|---|---|---|---|
| D07 | MSVC build | High | Program did not compile on Windows | `windows.h` defines `min`/`max` macros that break `std::min` | `NOMINMAX` and `WIN32_LEAN_AND_MEAN` before the include | Windows build, 69 / 69 then |
| D08 | clang-tidy | Medium | Optionals read twice (`getMark()` then `*getMark()`), a possible crash if the second read is empty | Repeated calls | Read once into a local, then check | Full suite |
| D09 | clang-tidy | Medium | Audit view used `*oldRawMark` after checking only `oldFinalMark` | Incomplete check | Check both | ModerationTest |
| D10 | clang-tidy | Medium | `int` multiplication before widening to `long long` in time code (overflow risk for far dates) | Missing `LL` suffix | `LL` literals | DateTimeTest far-future case |
| D11 | Code review | Medium | Lecturers could not see where the submitted file was stored (UC11 step 3) | Path not in `SubmissionView` | `SubmissionView::filePath`, shown when marking | ST-10 checks "File:" |
| D12 | cppcheck | Low | `ConsoleIO` could be built implicitly from a stream | Constructor not `explicit` | Made `explicit` | Build |
| D13 | clang-tidy | Low | Unneeded copy of the old mark in `ModerationService::amend` | Copy instead of reference | `const` reference | ModerationTest |

## Found while running the program on Windows (acceptance)

| ID | Found by | Severity | Defect | Cause | Fix | Retest |
|---|---|---|---|---|---|---|
| D14 | Student, creating an account | Medium | An administrator could not cancel "Create account" half way; they had to finish it | Username and full name were required inputs with no way out | Pressing Enter on an empty Username or Full name now cancels (`AdminMenu.cpp`) | ST-27 pass, AT-08 |
| D15 | Student, after deleting the old build folder | Medium | All data (including the admin account) disappeared and the program asked for a new first admin | `lms-data` is created in the folder the program is started from, which was inside the deleted folder | Build guide and README now run the program as `lms.exe C:\lms-data`, outside the code folder | AT-01 restart step |
| D16 | Student, following BUILD.md | Low | Windows build guide assumed "Developer PowerShell", a ready C++ workload and CMake on PATH; the long cmake line split when pasted | Guide written from a Visual Studio install, not Build Tools | BUILD.md rewritten: normal PowerShell, add the C++ workload with Modify, install CMake, `$tc` variable for the toolchain path, troubleshooting section | Student build: 69 / 69 |

## Summary

| Severity | Found | Fixed | Accepted |
|---|---|---|---|
| Critical | 0 | 0 | 0 |
| High | 1 | 1 | 0 |
| Medium | 8 | 8 | 0 |
| Low | 7 | 6 | 1 (D06) |
| **Total** | **16** | **15** | **1** |
