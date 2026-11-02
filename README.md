# Moderated LMS

A console Learning Management System for a secondary school: assignments, submissions with late penalties and extensions, marking, Head-of-Department moderation and release of marks. CS4433 final project, Mafeka M.M. (202321161).

- C++17, CMake, SQLite 3, libsodium (Argon2id), Catch2
- Layered architecture: presentation, services, domain, data access and infrastructure

## Final submission

- **Final report:** [report/Final_Report_Mafeka_202321161.pdf](report/Final_Report_Mafeka_202321161.pdf) (Word version: [.docx](report/Final_Report_Mafeka_202321161.docx))
- **Appendices A to H:** [docs/appendices](docs/appendices/README.md) (traceability matrix, risk register, test results, user manuals, deployment, QA, CR1)
- **Diagrams D1 to D12:** [docs/diagrams](docs/diagrams/README.md) (planning, use cases, design v1.0 and v1.1, deployment)
- **Application:** the source code in this repository, and the ready Windows program on the [Releases page](https://github.com/mamakhooamafeka/CS4433-software-engineering/releases/latest) (v1.1.0)

**Build and run:** see [docs/BUILD.md](docs/BUILD.md).
**Coding standard:** see [docs/CODING_STANDARD.md](docs/CODING_STANDARD.md).
**Code review and requirement traceability:** see [docs/REVIEW.md](docs/REVIEW.md).
**Deployment (Phase 7):** [deployment diagram](docs/DEPLOYMENT.md), [installation guide](docs/INSTALL.md), user manuals for [students](docs/manuals/USER_MANUAL_STUDENT.md), [lecturers](docs/manuals/USER_MANUAL_LECTURER.md), [heads of department](docs/manuals/USER_MANUAL_HEAD_OF_DEPARTMENT.md) and [administrators](docs/manuals/USER_MANUAL_ADMINISTRATOR.md), [release notes](docs/RELEASE_NOTES.md). Download the ready Windows program from the repository's Releases page.
**Maintenance (Phase 8, release v1.1.0):** [change request CR1: classification, impact analysis and regression tests](docs/CR1_IMPACT_ANALYSIS.md), [updated design diagrams](docs/diagrams/design-v1.1/).
**Configuration management and QA (Phase 6):** [branching, commits and releases](docs/CONFIG_MANAGEMENT.md), [static analysis, metrics and code review](docs/QA_REPORT.md), [changelog](CHANGELOG.md).
**Testing (Phase 5):** [test plan](docs/TEST_PLAN.md), [test results and coverage](docs/TEST_RESULTS.md), [defect log](docs/DEFECT_LOG.md), [acceptance tests](docs/ACCEPTANCE_TESTS.md). CI runs every test on Linux and Windows on each push (`.github/workflows/ci.yml`).

## Quick start (Ubuntu)

```bash
cmake -S . -B build && cmake --build build -j && ctest --test-dir build
./build/lms            # first start asks you to create the administrator
```

On Windows follow the Visual Studio + vcpkg steps in the build guide.
