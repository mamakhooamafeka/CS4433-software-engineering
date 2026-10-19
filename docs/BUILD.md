# Build guide

The Moderated LMS is a C++17 console program built with CMake. It needs **SQLite 3** and **libsodium**; the tests use **Catch2 v3**, which CMake downloads automatically if it isn't installed.

## Windows 10/11 (Visual Studio + vcpkg)

1. Install **Visual Studio 2022 Community** with the workload **"Desktop development with C++"** (this includes CMake and the MSVC compiler).
2. Install **Git for Windows**: https://git-scm.com/download/win
3. Install **vcpkg** (the package manager for the libraries). Open *Developer PowerShell for VS 2022* and run:
   ```powershell
   cd C:\
   git clone https://github.com/microsoft/vcpkg.git
   .\vcpkg\bootstrap-vcpkg.bat
   .\vcpkg\vcpkg install sqlite3:x64-windows libsodium:x64-windows catch2:x64-windows
   ```
4. Build and test from the project folder:
   ```powershell
   cd <path-to>\lms
   cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build build --config Release
   ctest --test-dir build -C Release --output-on-failure
   ```
5. Run it:
   ```powershell
   .\build\Release\lms.exe
   ```

Tip: you can also open the `lms` folder directly in Visual Studio (*File > Open > Folder*). It reads `CMakeLists.txt`; add the toolchain file under *Project > CMake Settings*.

## Ubuntu 22.04 / 24.04

```bash
sudo apt install build-essential cmake git libsqlite3-dev libsodium-dev catch2 pkg-config
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/lms
```

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
