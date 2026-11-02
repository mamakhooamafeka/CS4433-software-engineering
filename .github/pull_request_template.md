## What this changes

<!-- One or two sentences. Name the requirement, rule or defect ID (FR-xx, BR-xx, Dxx). -->

## Review checklist (docs/QA_REPORT.md section 4)

- [ ] C1 Builds with zero warnings (GCC and MSVC)
- [ ] C2 All tests pass; new or changed behaviour has a test
- [ ] C3 Layers respected: no SQL outside `src/data`, no SQLite or libsodium outside data/infrastructure, services use only domain interfaces
- [ ] C4 User input is validated at the service boundary (length, range, format)
- [ ] C5 SQL uses `?` parameters only; no user text joined into SQL
- [ ] C6 Errors use the `LmsError` classes with plain messages; no passwords or SQL in messages or logs
- [ ] C7 Resources are owned by RAII classes; no `new` or `delete`
- [ ] C8 Functions under 40 lines and cyclomatic complexity under 10, or the exception is justified
- [ ] C9 `const` where nothing changes; `override` on every overriding function
- [ ] C10 Business rules match the SRS and design, and the code names the rule ID
- [ ] C11 clang-format, cppcheck and clang-tidy are clean (CI green)
- [ ] C12 Documentation updated (README, docs, CHANGELOG)
