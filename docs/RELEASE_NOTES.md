# Moderated LMS v1.0.0 release notes

The first release of the Moderated LMS, the school's offline system for handing in assignments, marking them, checking the marks in each department and releasing them to learners. It is release v1.0 of the project brief; the tag is `v1.0.0` because the project uses semantic versioning (`docs/CONFIG_MANAGEMENT.md`, section 4).

## Who it is for and what it does

| Role | What they can do in v1.0.0 |
|---|---|
| Student | See their courses, assignments and own deadlines (including extensions); hand in a file up to 10 MB with a preview and a receipt; hand in new versions until the deadline; see marks and feedback once released |
| Lecturer | Create assignments; give one student an extension with a reason; mark with the late penalty worked out by the system and shown before saving; correct a mark with a reason before moderation |
| Head of department | Approve all or some marks; amend a mark with a reason; release all moderated marks of an assignment at once; read the audit log of the department |
| Administrator | Create, rename and deactivate accounts; reset passwords; set up departments, courses, lecturers and enrolments; back up and restore |
| Everyone | Secure log in (passwords protected with Argon2id); own password chosen at first log in; account locked for 15 minutes after 5 wrong passwords |

The school's rules are built in: 10% of the maximum mark off for each day late and 0 after 3 days (BR-01), extensions only move a deadline later (BR-02), no new version after the deadline or after marking (BR-03), learners see marks only after release (BR-04), and every mark change is kept in an audit log that cannot be edited (FR-20).

## System requirements

| Item | Requirement |
|---|---|
| Computer | One Windows 10 or 11 computer, 64-bit, at least 4 GB memory, 2 GB free disk |
| Network | None. The system works fully offline |
| Other software | None. The Visual C++ runtime is included in the package |
| Backup | An external USB hard disk for the daily second copy (recommended) |

## What is in the package

`ModeratedLMS-1.0.0-windows-x64.zip`, attached to the GitHub Release, built and tested automatically by `.github/workflows/release.yml` from the `v1.0.0` tag:

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
| Automated tests | 115 tests (unit, integration and system), all passing on Windows and Linux |
| Acceptance tests | AT-01 to AT-08 run by hand on Windows, all passed |
| Line coverage of the business logic | 93.7% (target 70%) |
| Static analysis | cppcheck and clang-tidy report no findings; checked on every push |
| Defects | 16 found during testing and review (D01 to D16). 15 fixed, 1 accepted (D06) |

Details: `docs/TEST_RESULTS.md`, `docs/DEFECT_LOG.md` and `docs/QA_REPORT.md`.

## Known limitations

| Limitation | Effect | Workaround or plan |
|---|---|---|
| One computer, one user at a time | People queue to use the system | Supported set-up by design (Phase 1): SQLite on a shared network drive is not reliable |
| Times are typed and shown in UTC | Users must subtract 2 hours from Lesotho time | Explained in every manual; showing local time is a candidate for a later version |
| Extensions are for one student at a time | A class-wide extension needs one entry per learner | Change request CR1, planned for v1.1.0 |
| A deadline cannot be edited after the assignment is created | A wrong deadline is fixed with extensions | Covered by CR1 in v1.1.0 |
| The extension reason is not shown in the HoD's moderation list (D06) | The HoD sees the days late but not why an extension was given | Accepted for v1.0.0; the reason is stored and can be added to the screen later |
| The database file is not encrypted on disk | Someone with access to the Windows account could copy `lms.db` | Keep the computer supervised and locked when not in use; keep the backup disk locked away (Administrator manual, section 10) |
| No e-mail or SMS notifications | Learners must log in to see new assignments and marks | The school has no e-mail infrastructure (Phase 1) |

## Licences

The Moderated LMS uses SQLite (public domain) and libsodium (ISC licence). Both allow a compiled program to be given to the school without royalties. Their licence texts are in the `licences` folder of the package.

## Support

Problems are first reported to the school's administrator. The technician sends `C:\lms-data\lms-error.log` with any fault report.
