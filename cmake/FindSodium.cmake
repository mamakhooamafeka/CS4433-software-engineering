# Finds libsodium and defines the imported target Sodium::Sodium.
# Linux/macOS: uses pkg-config. Windows (vcpkg): uses the unofficial-sodium package.

if(TARGET Sodium::Sodium)
  return()
endif()

find_package(unofficial-sodium CONFIG QUIET)
if(unofficial-sodium_FOUND)
  add_library(Sodium::Sodium INTERFACE IMPORTED)
  target_link_libraries(Sodium::Sodium INTERFACE unofficial-sodium::sodium)
  return()
endif()

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(SODIUM QUIET IMPORTED_TARGET libsodium)
endif()
if(SODIUM_FOUND)
  add_library(Sodium::Sodium INTERFACE IMPORTED)
  target_link_libraries(Sodium::Sodium INTERFACE PkgConfig::SODIUM)
  return()
endif()

find_path(SODIUM_INCLUDE_DIR sodium.h)
find_library(SODIUM_LIBRARY NAMES sodium libsodium)
if(SODIUM_INCLUDE_DIR AND SODIUM_LIBRARY)
  add_library(Sodium::Sodium UNKNOWN IMPORTED)
  set_target_properties(Sodium::Sodium PROPERTIES
    IMPORTED_LOCATION ${SODIUM_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${SODIUM_INCLUDE_DIR})
  return()
endif()

message(FATAL_ERROR "libsodium not found. See docs/BUILD.md for how to install it.")
