# Appendix B: Updated risk register (final version, after release v1.1.0)

The register from the Project Management Plan (v0.1) was re-scored at each gate: v0.2 at Gate 1 (requirements), v0.3 at Gate 2 (design), and this final version after Phase 8. Likelihood (L) and impact (I) are scored 1 to 5; exposure is L × I. A risk is **Closed** when its cause can no longer occur in this project, and **Open** when it still applies to the school after hand-over.

| ID | Risk | v0.1 | Final | Status | Evidence for the final score |
|---|---|---|---|---|---|
| R01 | Late requirement change | 3×5 = 15 | 1×5 = 5 | Closed | SRS baselined at Gate 1; the only later change, CR1, went through change control with a full [impact analysis](../CR1_IMPACT_ANALYSIS.md). |
| R02 | Schedule slip in Weeks 1 to 3 | 3×5 = 15 | 1×5 = 5 | Closed | All gates met; v1.0.0 and v1.1.0 released before the submission date. |
| R03 | Scope creep | 3×3 = 9 | 1×3 = 3 | Closed | Every feature traces to a requirement in the [traceability matrix](A-traceability-matrix.md); nothing on the out-of-scope list was built. |
| R04 | Implementation effort underestimated | 3×4 = 12 | 1×4 = 4 | Closed | All Must, Should and Could requirements implemented and tested (129 / 129 tests). |
| R05 | Loss of work or tool failure | 2×5 = 10 | 1×4 = 4 | Closed | Hosted GitHub repository; CI builds on Linux and Windows on every push; protected `main` branch. |
| R06 | CR1 larger than expected | 2×3 = 6 | 1×3 = 3 | Closed | CR1 needed exactly one new table and 306 changed lines, because extensions were a separate entity from Phase 3. |
| R07 | Developer unavailability | 3×4 = 12 | 1×4 = 4 | Closed | All phases delivered. |
| R08 | Unresolved stakeholder conflicts | 4×3 = 12 | 1×3 = 3 | Closed | 11 conflicts and ambiguities resolved in writing at Gate 1; CR1 questions Q1 to Q5 clarified in the impact analysis. |
| R09 | Non-expert users struggle with the interface | 3×3 = 9 | 2×3 = 6 | Open | Numbered menus, confirmations and a manual per role; acceptance tests passed by script. Not yet tried by the school's own staff. |
| R10 | Business-rule defects | 4×4 = 16 | 1×4 = 4 | Closed | Decision table, boundary value tests to the second, 95.2% business-logic coverage, mutation check (CR1-09). |
| R11 | Security and data-protection weakness | 2×5 = 10 | 1×5 = 5 | Open | Argon2id hashes, parameterised SQL, scope checks and append-only audit are all tested. Remaining: the database file is not encrypted on disk (see the release notes). |
| R12 | SQLite locking, corruption or data loss | 3×4 = 12 | 1×4 = 4 | Open | Transactions on every multi-step write, integrity check at start-up, daily backup, restore tested (FR-21, CR1-09). Depends on the school keeping a backup disk. |
| R13 | Poor performance with 800 students | 2×3 = 6 | 2×3 = 6 | Open | Keys and usernames are indexed, but response times were not measured with 800 seeded students. Recommended before go-live. |
| R14 | Submission files fill the disk (new at Gate 1) | 2×3 = 6 | 2×3 = 6 | Open | 10 MB limit per file enforced and tested (BV-05). No free-space check before copying; the technician should watch free space. |
| R15 | Code drifts from the design (new at Gate 2) | 3×4 = 12 | 1×4 = 4 | Closed | Code review against the class diagrams ([review record](../REVIEW.md)); v1.1 diagrams updated with CR1. |
| R16 | Password library fails to build on a target OS (new at Gate 2) | 2×3 = 6 | 1×3 = 3 | Closed | libsodium builds in CI on Windows and Linux on every push. |

**Summary:** 10 risks closed, 6 open. The open risks are operational and pass to the school: they are listed with recommendations in the report's professional, legal and ethical issues section and in the [release notes](../RELEASE_NOTES.md).
