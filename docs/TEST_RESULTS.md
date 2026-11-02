# Test results (Phase 5)

Results for the commit "Phase 5: verification and validation".

## 1. Automated tests

| Level | Tag | Cases | Passed | Failed |
|---|---|---|---|---|
| Unit | `[unit]` | 26 | 26 | 0 |
| Unit and integration boundary values | `[bva]` | 18 | 18 | 0 |
| Integration | `[integration]` | 39 | 39 | 0 |
| UI (menus with scripted input) | MenuTest `[ui]` | 4 | 4 | 0 |
| System (use-case flows) | `[system]` | 27 | 27 | 0 |
| **Total** | | **114** | **114** | **0** |

366 assertions, all passed. Full run time about 27 seconds (most of it is the Argon2id hashing, which is slow on purpose).

| Platform | Compiler | Result |
|---|---|---|
| Ubuntu 24.04 | GCC 13.3, `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` | 0 warnings, 114 / 114 pass |
| Windows 10/11 (student laptop) | MSVC Build Tools 2022 + vcpkg | 114 / 114 pass |
| GitHub Actions | Linux and Windows jobs in `.github/workflows/ci.yml` | both jobs pass on every push; first run linked in section 4 |

Machine-readable results: `docs/test-results/test-results.xml` (JUnit format, readable by CI tools).

### Before and after the fixes

The first 44 new test cases were run against the v1.0 code: **38 passed and 6 failed** (ST-01, ST-06, ST-10, ST-11, ST-12, ST-16). ST-27 was added later, for D14. The failures were real differences from the use case specifications and are logged as D01–D05 in `DEFECT_LOG.md`. After the fixes all 114 cases pass, which also confirms the fixes did not break any of the 69 older tests (regression).

## 2. Coverage (NFR-12: at least 70% of business logic)

Measured with GCC `--coverage` and gcovr 8 over the full test run.

| Layer | Lines | Covered | Line coverage |
|---|---|---|---|
| Domain (`src/domain`) | 392 | 375 | 95.7% |
| Services (`src/services`) | 669 | 619 | 92.5% |
| **Business logic (domain + services)** | **1,061** | **994** | **93.7%** |
| Data access (`src/data`) | 527 | 478 | 90.7% |
| Infrastructure (`src/infrastructure`) | 114 | 86 | 75.4% |
| Console UI (`src/presentation`) | 676 | 430 | 63.6% |
| All source code | 2,378 | 1,988 | 83.6% |

Business-logic coverage went from 83.4% (end of Phase 4) to **93.7%**. The CI pipeline fails the build if it ever drops below 70%.

What is not covered, and why: administrator menu screens that only list things (checked by hand in AT-01), file-store error branches for a disk that cannot be written, and `main.cpp` start-up (checked by hand on Windows).

Full report with every line marked: open `docs/test-results/coverage/index.html` in a web browser. Text versions: `coverage-business-logic.txt` and `coverage-all-src.txt`.

## 3. Acceptance tests

Run by hand on Windows using `docs/ACCEPTANCE_TESTS.md`. All 8 passed, so the acceptance exit criterion is met.

| ID | Test | Result |
|---|---|---|
| AT-01 | Administrator sets up the system | Pass |
| AT-02 | First log in and lockout | Pass |
| AT-03 | Lecturer creates assignments | Pass |
| AT-04 | Student submits and resubmits on time | Pass |
| AT-05 | Late submission with warning | Pass |
| AT-06 | Lecturer marks with the penalty shown first | Pass |
| AT-07 | HoD moderates and releases | Pass |
| AT-08 | Cancel, backup and restore | Pass |

## 4. Windows and CI evidence

| Item | Value |
|---|---|
| Windows `ctest` on the laptop | 114 / 114 passed |
| GitHub Actions run (link) | https://github.com/mamakhooamafeka/CS4433-software-engineering/actions/runs/41294443447 |
| Linux job | passed (1 min 30 s); business-logic coverage 993 / 1,060 lines = 93.7% |
| Windows job | passed (3 min 43 s) |

The CI coverage count (993 / 1,060) differs from the local count (994 / 1,061) by one line because CI uses a newer GCC version; both are 93.7%.

## 5. Regression results for v1.1.0 (change request CR1)

| Run | Result |
|---|---|
| The 115 v1.0.0 test cases, unchanged, on the v1.1.0 code (Linux, GCC 13.3) | 115 / 115 passed |
| New CR1 test cases (`[cr1]`) | 14 / 14 passed |
| Whole suite | 129 / 129 passed, 0 compiler warnings |
| Business-logic line coverage | 1,062 / 1,116 lines = 95.2% (v1.0.0: 93.7%) |
| cppcheck, clang-tidy | 0 findings |
| AT-09 on Windows | See `ACCEPTANCE_TESTS.md` |

Details and the list of affected tests: `docs/CR1_IMPACT_ANALYSIS.md`, section 5.
