# Changelog

Versions follow semantic versioning (see `docs/CONFIG_MANAGEMENT.md` section 4).

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
- Class-wide extensions (CR1) are planned for v1.1.0.
