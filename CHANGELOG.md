# Changelog

Versions follow semantic versioning (see `docs/CONFIG_MANAGEMENT.md` section 4).

## v1.1.0

Change request CR1 (Phase 8). Analysis, impact and regression results: `docs/CR1_IMPACT_ANALYSIS.md`.

### Features
- Lecturer: `Grant an extension` can now extend the deadline for the whole class (FR-24), with a reason that students see. It must move the class deadline later (BR-02), is refused once marks are released (BR-06), re-evaluates LATE submissions (DC-01) and reports marked work that may need a correction (UC20).
- Student: the assignment and submit lists mark extended deadlines as `(ext. class)` or `(ext. you)`; the details show the original deadline and the class reason (FR-10).

### Data
- New table `class_extension`, added by migration 1 (`sql/migrations/001-class-extension.sql`). The program upgrades a v1.0.0 database, or a restored v1.0.0 backup, on first use; `pragma user_version` records the version.

### Quality
- 14 new tests (CR1-01 to CR1-14); the 115 tests of v1.0.0 pass unchanged (129 in total). Business-logic coverage 95.2%. cppcheck and clang-tidy report no findings.

### Design documents
- Updated class diagrams, state machine and ERD in `docs/diagrams/design-v1.1/` (changes in blue).

## v1.0.0

First release of the Moderated LMS.

### Features
- Log in with Argon2id password hashing, forced change of temporary passwords and a 15-minute lock after five failed attempts (FR-01 to FR-05).
- Administrator: accounts, departments, courses, lecturer assignment, enrolment, backup and restore (FR-04 to FR-07, FR-21).
- Lecturer: assignments, per-student extensions, marking with the late penalty worked out by the system, corrections with a reason (FR-08, FR-09, FR-13, FR-19).
- Student: submissions with checks, receipts with SHA-256, resubmission before the deadline, released results (FR-10 to FR-12, FR-18).
- Head of Department: moderation, amendment with a reason, all-or-nothing release, audit log (FR-15 to FR-17, FR-20).

### Quality
- 115 automated tests (unit, integration, system); 93.7% line coverage of the business logic.
- CI on Linux and Windows, with cppcheck and clang-tidy.
- Defects D01 to D16 fixed or accepted (`docs/DEFECT_LOG.md`); static analysis and review findings fixed (`docs/QA_REPORT.md`).

### Deployment
- Windows installation package `ModeratedLMS-1.0.0-windows-x64.zip` with the Visual C++ runtime included, built and attached to the GitHub Release by `release.yml`.
- `start-lms.bat` (always uses `C:\lms-data`) and `copy-backups.bat` (daily copy of the backups to a second disk).
- Deployment diagram, installation guide, a user manual for each role and release notes (`docs/DEPLOYMENT.md`).

### Known limitations
- One user at a time on one computer.
- Times are entered and shown in UTC.
- Class-wide extensions (CR1) were planned for v1.1.0.
