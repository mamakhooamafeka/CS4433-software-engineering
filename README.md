# Moderated LMS

A console Learning Management System for a secondary school: assignments, submissions with late penalties and extensions, marking, Head-of-Department moderation and release of marks. CS4433 final project, Mafeka M.M. (202321161).

- C++17, CMake, SQLite 3, libsodium (Argon2id), Catch2
- Layered architecture: presentation, services, domain, data access and infrastructure

**Build and run:** see [docs/BUILD.md](docs/BUILD.md).
**Coding standard:** see [docs/CODING_STANDARD.md](docs/CODING_STANDARD.md).
**Code review and requirement traceability:** see [docs/REVIEW.md](docs/REVIEW.md).

## Quick start (Ubuntu)

```bash
cmake -S . -B build && cmake --build build -j && ctest --test-dir build
./build/lms            # first start asks you to create the administrator
```

On Windows follow the Visual Studio + vcpkg steps in the build guide.
