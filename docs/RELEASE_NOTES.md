# Moderated LMS v1.1.0 release notes

Release v1.1 of the Moderated LMS, the school's offline system for handing in assignments, marking them, checking the marks in each department and releasing them to learners. It carries out the school's change request CR1. The tag is `v1.1.0`: a MINOR version, because it adds functionality and keeps all existing data working (`docs/CONFIG_MANAGEMENT.md`, section 4).

## What is new in v1.1.0 (change request CR1)

| Who | What changed |
|---|---|
| Lecturer | `2. Grant an extension` now asks **who** gets it: one student (as before) or **the whole class**. A class extension needs a reason, can only move the deadline later, also covers students enrolled later, and turns late work handed in before the new deadline into on-time work. The lecturer is told how many late submissions became on time and how many marked ones may need a correction |
| Student | The assignment list shows extended deadlines as `(ext. class)` or `(ext. you)`, with a key above the list. The assignment details show the original deadline and, for a class extension, the lecturer's reason. The reason for a personal extension is not shown on screen, because it may be private |
| Technician | Nothing to do beyond installing the new version (`INSTALL.md`, section 10). The first start upgrades the database by itself and keeps all data |

The analysis of the change, the updated design diagrams and the regression test results are in `docs/CR1_IMPACT_ANALYSIS.md`.

## Upgrading from v1.0.0

1. Ask everyone to log out, and let the administrator make a backup (`5. Back up or restore`, `1. Back up now`).
2. Install v1.1.0 over the program folder as described in `INSTALL.md`, section 10. Do not touch `C:\lms-data`.
3. Start the program. It adds the new table for class extensions to the database (migration 1) in one step, and keeps every account, mark and submitted file. Backups made by v1.0.0 can still be restored; they are upgraded the same way.

## Who it is for and what it does

| Role | What they can do in v1.1.0 |
|---|---|
| Student | See their courses, assignments and own deadlines, with extensions marked as class or personal; hand in a file up to 10 MB with a preview and a receipt; hand in new versions until the deadline; see marks and feedback once released |
| Lecturer | Create assignments; give one student or the whole class an extension with a reason; mark with the late penalty worked out by the system and shown before saving; correct a mark with a reason before moderation |
| Head of department | Approve all or some marks; amend a mark with a reason; release all moderated marks of an assignment at once; read the audit log of the department |
| Administrator | Create, rename and deactivate accounts; reset passwords; set up departments, courses, lecturers and enrolments; back up and restore |
| Everyone | Secure log in (passwords protected with Argon2id); own password chosen at first log in; account locked for 15 minutes after 5 wrong passwords |

The school's rules are built in: 10% of the maximum mark off for each day late and 0 after 3 days (BR-01), extensions, for one student or the whole class, only move a deadline later (BR-02), no new version after the deadline or after marking (BR-03), learners see marks only after release (BR-04), and every mark change is kept in an audit log that cannot be edited (FR-20).

## System requirements

| Item | Requirement |
|---|---|
| Computer | One Windows 10 or 11 computer, 64-bit, at least 4 GB memory, 2 GB free disk |
| Network | None. The system works fully offline |
| Other software | None. The Visual C++ runtime is included in the package |
| Backup | An external USB hard disk for the daily second copy (recommended) |

## What is in the package

`ModeratedLMS-1.1.0-windows-x64.zip`, attached to the GitHub Release, built and tested automatically by `.github/workflows/release.yml` from the `v1.1.0` tag:

| File | Purpose |
|---|---|
| `lms.exe` and the DLL files | The program and its libraries |
| `start-lms.bat` | Starts the program with the data folder `C:\lms-data` |
| `copy-backups.bat` | Copies the daily backups to the external disk |
| `docs\INSTALL.md` | Installation guide for the technician |
| `docs\USER_MANUAL_*.md` | One manual each for students, lecturers, heads of department and administrators |
| `docs\RELEASE_NOTES.md` | This file |
| `licences\` | Licences of SQLite and libsodium |

## Installing

Follow `docs\INSTALL.md`. A new installation takes about 20 minutes, including the scheduled backup copy.

## Quality of this release

| Measure | Result |
|---|---|
| Automated tests | 129 tests (unit, integration and system): the 115 test cases of v1.0.0, unchanged, plus 14 new test cases for CR1. All pass on Windows and Linux |
| Acceptance tests | AT-01 to AT-08 run by hand on Windows, all passed |
| Line coverage of the business logic | 95.2% (target 70%; 93.7% in v1.0.0) |
| Static analysis | cppcheck and clang-tidy report no findings; checked on every push |
| Defects | 16 found during testing and review (D01 to D16). 15 fixed, 1 accepted (D06) |

Details: `docs/TEST_RESULTS.md`, `docs/DEFECT_LOG.md` and `docs/QA_REPORT.md`.

## Known limitations

| Limitation | Effect | Workaround or plan |
|---|---|---|
| One computer, one user at a time | People queue to use the system | Supported set-up by design (Phase 1): SQLite on a shared network drive is not reliable |
| Times are typed and shown in UTC | Users must subtract 2 hours from Lesotho time | Explained in every manual; showing local time is a candidate for a later version |
| A deadline can be moved later but never earlier | A deadline set too late cannot be pulled back | By design (BR-02): moving a deadline earlier would be unfair to learners who planned for it. Create a new assignment if needed |
| Marked work keeps its mark after a class extension | A late deduction may stay on a marked submission | The lecturer is told how many, and corrects them with `Correct a mark`, which works the deduction out again |
| The extension reason is not shown in the HoD's moderation list (D06) | The HoD sees the days late but not why an extension was given | Accepted; the reasons of both kinds of extension are stored and can be added to the screen later |
| The database file is not encrypted on disk | Someone with access to the Windows account could copy `lms.db` | Keep the computer supervised and locked when not in use; keep the backup disk locked away (Administrator manual, section 10) |
| No e-mail or SMS notifications | Learners must log in to see new assignments and marks | The school has no e-mail infrastructure (Phase 1) |

## Licences

The Moderated LMS uses SQLite (public domain) and libsodium (ISC licence). Both allow a compiled program to be given to the school without royalties. Their licence texts are in the `licences` folder of the package.

## Support

Problems are first reported to the school's administrator. The technician sends `C:\lms-data\lms-error.log` with any fault report.
