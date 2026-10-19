cmake_minimum_required(VERSION 3.20)
project(ModeratedLMS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LMS_BUILD_TESTS "Build the unit and integration tests" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ---- Dependencies -----------------------------------------------------------
find_package(SQLite3 REQUIRED)
include(cmake/FindSodium.cmake)

# ---- Warnings (coding standard section 8) -----------------------------------
add_library(lms_warnings INTERFACE)
if(MSVC)
  target_compile_options(lms_warnings INTERFACE /W4 /permissive-)
else()
  target_compile_options(lms_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

# ---- Core library: every layer except the console UI -------------------------
# The schema file is embedded so the program can create a new database itself.
file(READ ${CMAKE_SOURCE_DIR}/sql/schema.sql LMS_SCHEMA_SQL)
configure_file(cmake/SchemaSql.hpp.in ${CMAKE_BINARY_DIR}/generated/SchemaSql.hpp @ONLY)

file(GLOB_RECURSE LMS_CORE_SOURCES CONFIGURE_DEPENDS
  src/domain/*.cpp src/services/*.cpp src/data/*.cpp src/infrastructure/*.cpp)
add_library(lms_core STATIC ${LMS_CORE_SOURCES})
target_include_directories(lms_core PUBLIC src ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(lms_core PUBLIC SQLite::SQLite3 Sodium::Sodium PRIVATE lms_warnings)

# ---- Console application ---------------------------------------------------
file(GLOB_RECURSE LMS_UI_SOURCES CONFIGURE_DEPENDS src/presentation/*.cpp)
add_executable(lms app/main.cpp ${LMS_UI_SOURCES})
target_link_libraries(lms PRIVATE lms_core lms_warnings)

# ---- Tests -----------------------------------------------------------------
if(LMS_BUILD_TESTS)
  enable_testing()
  find_package(Catch2 3 QUIET)
  if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.4.0)
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  endif()
  include(Catch)

  file(GLOB_RECURSE LMS_TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
  add_executable(lms_tests ${LMS_TEST_SOURCES})
  target_link_libraries(lms_tests PRIVATE lms_core Catch2::Catch2WithMain lms_warnings)
  catch_discover_tests(lms_tests)
endif()
