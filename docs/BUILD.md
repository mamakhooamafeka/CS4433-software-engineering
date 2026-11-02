# Build guide

The Moderated LMS is a C++17 console program built with CMake. It needs **SQLite 3** and **libsodium**; the tests use **Catch2 v3**, which CMake downloads automatically if it isn't installed.

## Windows 10/11 (MSVC + vcpkg)

Tested on Windows with Build Tools for Visual Studio 2022, vcpkg and CMake 4.5: all 69 tests pass.

1. Install a C++ compiler, either one of:
   - **Visual Studio 2022 Community** (or newer), or
   - **Build Tools for Visual Studio 2022** (compiler only, smaller; use it with VS Code and the *C/C++* and *CMake Tools* extensions): https://visualstudio.microsoft.com/visual-cpp-build-tools/

   In the installer tick the workload **"Desktop development with C++"** and check that **MSVC v143** and a **Windows 10/11 SDK** are selected.
2. Install **Git for Windows**: https://git-scm.com/download/win
3. Install **CMake** from https://cmake.org/download/ (*Windows x64 Installer*) and choose **"Add CMake to the PATH"**. (Full Visual Studio already has CMake inside its *Developer PowerShell*, but the Build Tools do not put it on the normal PATH.)
4. Install **vcpkg** and the libraries. Open **PowerShell as administrator** and run:
   ```powershell
   cd C:\
   git clone https://github.com/microsoft/vcpkg.git
   .\vcpkg\bootstrap-vcpkg.bat
   .\vcpkg\vcpkg install sqlite3:x64-windows libsodium:x64-windows catch2:x64-windows
   ```
   This takes 10-20 minutes and ends with *"All requested installations completed successfully"*.
5. Build and test from the project folder (the second line keeps the next one short, so it is not split when copied):
   ```powershell
   cd <path-to>\lms
   $tc = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
   cmake -S . -B build "-DCMAKE_TOOLCHAIN_FILE=$tc"
   cmake --build build --config Release
   ctest --test-dir build -C Release
   ```
   The last line should report *"100% tests passed"*.
6. Run it:
   ```powershell
   .\build\Release\lms.exe
   ```
   or double-click `lms.exe` in `build\Release`.

## Ubuntu 22.04 / 24.04

```bash
sudo apt install build-essential cmake git libsqlite3-dev libsodium-dev catch2 pkg-config
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/lms
```

## First run

The program keeps all its data in one folder, `lms-data` in the folder you start it from (or the folder you give it: `lms.exe D:\lms-data`):

| File / folder | What it holds |
|---|---|
| `lms.db` | The SQLite database (created on first start from `sql/schema.sql`) |
| `submissions/` | Every submitted file, one per version: `<assignment>/<student>/v<n>_<name>` |
| `backups/` | One backup per day, made automatically at the first start of the day |
| `lms-error.log` | Technical details of any error; the screen only shows a plain message |

1. On the very first start there are no accounts, so the program asks you to create the **administrator**.
2. Log in as the administrator and create the other accounts (each gets a temporary password that must be changed at first log in), then a department with its head, a course, and assign the lecturer and enrol students.
3. Log in as the lecturer to create an assignment, as a student to submit, then lecturer (mark) and head of department (moderate, release).

When the program asks for a file, you can paste a path copied with Windows *Copy as path*: the quotes are removed. Dates are typed as `YYYY-MM-DD HH:MM` and are in UTC. To quit, leave the username empty at the log-in screen.

## Quality checks

```bash
ctest --test-dir build --output-on-failure      # all 115 tests
clang-format --dry-run --Werror src/*/*.cpp src/*/*.hpp app/*.cpp tests/*.cpp tests/*.hpp
cppcheck --enable=warning,style,performance,portability --std=c++17 --suppress=useStlAlgorithm \
         --suppress=missingIncludeSystem -I src -I build/generated src app
find src app -name '*.cpp' | xargs clang-tidy -p build --quiet   # settings in .clang-tidy
lizard src app -l cpp                                             # code metrics
```

The results are in [QA_REPORT.md](QA_REPORT.md) (Phase 6) and [REVIEW.md](REVIEW.md) (Phase 4). CI runs the same checks on every push.

## Troubleshooting

| Problem | Fix |
|---|---|
| CMake: *Could not find SQLite3* or *Sodium* | Windows: check the `-DCMAKE_TOOLCHAIN_FILE=.../vcpkg.cmake` option. Ubuntu: install `libsqlite3-dev libsodium-dev`. |
| *The code execution cannot proceed because sqlite3.dll was not found* | Run `lms.exe` from `build\Release`, where vcpkg copies the DLLs. |
| Tests download Catch2 and fail offline | Install `catch2:x64-windows` with vcpkg (or `catch2` with apt), or build with `-DLMS_BUILD_TESTS=OFF`. |
| vcpkg: *Unable to find a valid Visual Studio instance* | The C++ workload is missing: Visual Studio Installer > Modify > tick *Desktop development with C++*, restart, run the vcpkg install again. |
| *cmake is not recognized* | Install CMake with *Add CMake to the PATH* (step 3), then open a new PowerShell window. |
| CMake: *Unknown argument* | The long `cmake -S` line was split when pasted; use the `$tc` lines in step 5. |
| *The database file is damaged* | Log in as administrator and restore the latest backup (menu: Backup and restore). |

## Project layout

| Folder | Layer / purpose |
|---|---|
| `app/` | `main.cpp`: wires the layers together |
| `src/presentation/` | Console menus and input (`ConsoleIO`, `*Menu`) |
| `src/services/` | Application services (`AuthService`, `SubmissionService`, …) |
| `src/domain/` | Entities, business rules, interfaces, exceptions |
| `src/data/` | SQLite repositories (the only place with SQL) |
| `src/infrastructure/` | Password hashing, file store, clock |
| `sql/schema.sql` | Database schema (embedded into the program at build time) |
| `tests/` | Catch2 unit and integration tests |
| `docs/` | This guide and the coding standard |
