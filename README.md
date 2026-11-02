# Moderated LMS

A console Learning Management System for a secondary school: assignments, submissions with late penalties and extensions, marking, Head-of-Department moderation and release of marks. CS4433 final project, Mafeka M.M. (202321161).

- C++17, CMake, SQLite 3, libsodium (Argon2id), Catch2
- Layered architecture: presentation, services, domain, data access and infrastructure

**Build and run:** see [docs/BUILD.md](docs/BUILD.md).
**Coding standard:** see [docs/CODING_STANDARD.md](docs/CODING_STANDARD.md).
**Code review and requirement traceability:** see [docs/REVIEW.md](docs/REVIEW.md).
**Deployment (Phase 7):** [deployment diagram](docs/DEPLOYMENT.md), [installation guide](docs/INSTALL.md), user manuals for [students](docs/manuals/USER_MANUAL_STUDENT.md), [lecturers](docs/manuals/USER_MANUAL_LECTURER.md), [heads of department](docs/manuals/USER_MANUAL_HEAD_OF_DEPARTMENT.md) and [administrators](docs/manuals/USER_MANUAL_ADMINISTRATOR.md), [release notes](docs/RELEASE_NOTES.md). Download the ready Windows program from the repository's Releases page.
**Configuration management and QA (Phase 6):** [branching, commits and releases](docs/CONFIG_MANAGEMENT.md), [static analysis, metrics and code review](docs/QA_REPORT.md), [changelog](CHANGELOG.md).
**Testing (Phase 5):** [test plan](docs/TEST_PLAN.md), [test results and coverage](docs/TEST_RESULTS.md), [defect log](docs/DEFECT_LOG.md), [acceptance tests](docs/ACCEPTANCE_TESTS.md). CI runs every test on Linux and Windows on each push (`.github/workflows/ci.yml`).

## Quick start (Ubuntu)

```bash
cmake -S . -B build && cmake --build build -j && ctest --test-dir build
./build/lms            # first start asks you to create the administrator
```

On Windows follow the Visual Studio + vcpkg steps in the build guide.
