# Configuration management

Repository: https://github.com/mamakhooamafeka/CS4433-software-engineering

## 1. What is under version control

Everything needed to build, test and understand the system is in Git:

| Item | Where |
|---|---|
| Source code | `src/`, `app/` |
| Database schema | `sql/schema.sql` (built into the program at compile time) |
| Build definition | `CMakeLists.txt`, `cmake/` |
| Tests | `tests/` |
| Quality configuration | `.clang-format`, `.clang-tidy` |
| CI pipeline | `.github/workflows/ci.yml` |
| Pull request checklist | `.github/pull_request_template.md` |
| Documentation | `README.md`, `docs/` |

Never committed (listed in `.gitignore`): build folders, the database file (`*.db`), submitted files, backups, the `lms-data` folder and log files. These are produced by running the program, contain user data, or can be rebuilt from the sources.

## 2. Branching strategy

The project uses **GitHub Flow**: one long-lived branch, `main`, and short-lived branches for each change.

```
main  ──●──────────●───────────●──────────●──  (always builds, all tests pass)
         \        /             \        /
          ●──●──●                ●──●──●
     feature/phase6-qa       fix/D17-short-description
```

| Rule | Why |
|---|---|
| `main` always builds and passes every test | Anyone can build a working program from it at any time, and every tag on it is releasable |
| Every change is made on its own branch, named by type | The purpose of a branch is clear from its name |
| A branch is merged into `main` only by a pull request | The change is reviewed against the checklist, and CI runs on it before it reaches `main` |
| A pull request can only be merged when CI is green | Broken code cannot reach `main` (branch protection, section 6) |
| A branch is deleted after it is merged | The branch list shows only work in progress |

Branch names:

| Prefix | Use | Example |
|---|---|---|
| `feature/` | New functionality or a work package | `feature/cr1-class-extension` |
| `fix/` | A defect from `DEFECT_LOG.md`, named by its ID | `fix/D17-audit-filter` |
| `docs/` | Documentation only | `docs/user-manual` |
| `test/` | Tests only | `test/backup-restore` |

GitHub Flow was chosen over Git Flow (separate `develop`, `release` and `hotfix` branches) because there is one developer, one supported version at a time, and CI already keeps `main` releasable. Git Flow's extra branches would add merging work without adding safety.

History note: during Phases 4 and 5 the work packages were committed directly to `main`, one commit per work package, before the CI pipeline and this strategy existed. From Phase 6 onwards every change goes through a branch and pull request.

## 3. Commit messages

| Rule | Example |
|---|---|
| Short summary in the imperative, at most 72 characters | `Add Argon2id password hashing` |
| One logical change per commit | Not "fix stuff and add tests" |
| Mention the requirement, rule or defect ID when there is one | `Ask again for empty feedback (UC11 5a, D01)` |
| A blank line and a longer explanation when the reason is not obvious | |

## 4. Versions and tags

Releases follow **semantic versioning**, `vMAJOR.MINOR.PATCH`:

| Part | Goes up when | Example |
|---|---|---|
| MAJOR | A change breaks existing data or how users work | `v2.0.0` |
| MINOR | New functionality that keeps existing data working | `v1.1.0` for change request CR1 |
| PATCH | Defect fixes only | `v1.0.1` |

Each release is an **annotated tag** on a commit of `main`, with a GitHub Release that holds the release notes:

| Tag | Content |
|---|---|
| `v1.0.0` | First release: all of Phases 4 to 6 (implementation, tests, quality fixes) |
| `v1.1.0` | Planned: change request CR1 (Phase 8) |

## 5. Release procedure

1. All work for the release is merged into `main` by pull requests, and CI on `main` is green.
2. `CHANGELOG.md` is updated, and the version in `CMakeLists.txt` (`project(... VERSION x.y.z)`) is checked.
3. An annotated tag is created on that commit: `git tag -a v1.0.0 -m "Release 1.0.0"`, then `git push origin v1.0.0`.
4. A GitHub Release is created from the tag, with the release notes from `CHANGELOG.md`.

## 6. Branch protection on GitHub

`main` is protected (Settings > Branches > Add branch protection rule):

- Require a pull request before merging.
- Require status checks to pass before merging: `Linux build, tests and coverage`, `Windows build and tests (MSVC + vcpkg)`, and `Static analysis (cppcheck + clang-tidy)`.

## 7. Continuous integration

`.github/workflows/ci.yml` runs on every push to any branch and on every pull request:

| Job | What it does | Fails when |
|---|---|---|
| Linux build, tests and coverage | Builds with GCC, runs all tests, measures coverage | A test fails, or business-logic coverage is under 70% |
| Windows build and tests | Builds with MSVC and vcpkg, runs all tests | The build or a test fails |
| Static analysis | Runs cppcheck and clang-tidy with the project settings | Any finding |

Evidence: every run is listed under the repository's **Actions** tab. The first green run is linked in `docs/TEST_RESULTS.md`.
