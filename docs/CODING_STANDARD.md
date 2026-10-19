# Coding standard: Moderated LMS

This project follows the **C++ Core Guidelines** (Stroustrup and Sutter), with the project rules below. Where a rule here is stricter, this document wins. `clang-format` (`.clang-format`) enforces the layout, and `cppcheck`/`clang-tidy` check the rest (Phase 6).

## 1. Language and build
- C++17 only (`CMAKE_CXX_STANDARD 17`), no compiler extensions.
- The build is CMake. No source file is compiled outside `CMakeLists.txt`.
- Code must compile without warnings at `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` (GCC/Clang) and `/W4` (MSVC).

## 2. Layers (from the Phase 3 architecture)
- `src/presentation` → `src/services` → `src/domain`. `src/data` and `src/infrastructure` implement interfaces declared in `src/domain`.
- A layer may only include headers from the layer directly below it or from `domain`. The UI never includes `sqlite3.h` or `sodium.h`.
- **All SQL lives in `src/data`.** Every statement uses `sqlite3_prepare_v2` with `?` parameters bound by `sqlite3_bind_*`. Building SQL by joining strings with user input is forbidden (NFR-09).

## 3. Naming
| Kind | Style | Example |
|---|---|---|
| Classes, structs, enums | PascalCase | `SubmissionService`, `SubmissionStatus` |
| Interfaces | `I` + PascalCase | `ILatePenaltyPolicy`, `ISubmissionRepository` |
| Functions and methods | camelCase, verb first | `recordMark()`, `effectiveDeadline()` |
| Variables and parameters | camelCase | `rawMark`, `studentId` |
| Private data members | camelCase + trailing `_` | `status_`, `versions_` |
| Constants | `k` + PascalCase | `kMaxFileBytes` |
| Enum values | UPPER_CASE (they match the database text) | `SubmissionStatus::LATE` |
| Files | one class per file, named after it | `Submission.hpp`, `Submission.cpp` |

Everything is in `namespace lms`.

## 4. Files
- Headers use `#pragma once`.
- Include order: own header, project headers, standard library, third-party; one blank line between groups.
- Each header starts with a one-line comment saying what the class is for and, where it applies, the requirement it serves (e.g. `// BR-01 late penalty`).

## 5. Memory and resources
- No `new`/`delete` in application code. Own objects with `std::unique_ptr`, share with references.
- Every resource (database handle, prepared statement, transaction) is wrapped in a RAII class, so it is released even when an exception is thrown.
- Use `std::optional` for values that may be missing (e.g. `lockedUntil`, `mark`), never magic numbers like `-1`.

## 6. Errors
- Report errors with the exceptions in `domain/Errors.hpp` (`ValidationError`, `AccessDeniedError`, `BusinessRuleError`, `DataAccessError`). Never return error codes.
- Exception messages are plain language for the user, with no SQL, paths or stack detail (NFR-07).
- Every menu action is wrapped in `try`/`catch (const LmsError&)` in the presentation layer, so no input or database error ends the program.
- Low-level errors (SQLite return codes, file errors) are translated to `DataAccessError` inside `src/data` / `src/infrastructure`.

## 7. Security
- Passwords are only handled as `const std::string&`, never logged or printed, and stored only as libsodium Argon2id hashes (NFR-08).
- All user input is validated at the service boundary (length, range, format) before use.

## 8. Functions and classes
- Functions do one thing and stay short (aim for under 40 lines; cyclomatic complexity under 10).
- Mark member functions `const` when they don't change state, and use `override` on every overriding function.
- Pass large objects by `const&`. Return by value.
- Prefer `enum class` over plain `enum`.

## 9. Comments
- Explain *why*, not *what*. Reference the rule or requirement ID (BR-xx, FR-xx, DC-xx) when code enforces it.

## 10. Tests
- Tests use Catch2 v3 and live in `tests/`, one file per class under test, named `<Class>Test.cpp`.
- Test names describe the behaviour, e.g. `"A submission one second after the deadline is LATE"`.
- Tag tests by V-Model level: `[unit]`, `[integration]`, `[system]`.

## 11. Git
- Commit messages: short imperative summary (≤ 72 characters), e.g. `Add Argon2id password hashing`.
- One logical change per commit. `main` must always build and pass the tests.
