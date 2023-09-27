cmake_minimum_required(VERSION 3.20)

# Add helper modules (Find*.cmake etc) folder to the CMake path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

cmake_policy(SET CMP0091 NEW)

set (MOON_STANDALONE_PROJECT ON)
if(DEFINED PROJECT_NAME)
  set(MOON_STANDALONE_PROJECT OFF)
endif()

##############################################
##### Project

project(moon
        VERSION 0.7.1
        LANGUAGES C CXX ASM)

# Generate config.h
configure_file ("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_BINARY_DIR}")

include(target_link_objects)

#set (CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/cmake/CompilerOptions.cmake")

##############################################
##### Options

option(MOON_USE_LUAJIT              "Use LuaJIT" ON)
option(MOON_BUILD_AS_CXX            "Build Moon as C++" OFF)
if(MOON_USE_LUAJIT)
  set(MOON_BUILD_AS_CXX OFF CACHE BOOL "Override option" FORCE)
endif()
option(MOON_SYSTEM_GETPROCADDRESS   "Do not use custom GetProcAddress()" OFF)
option(MOON_SYSTEM_GETMODULEHANDLE  "Do not use custom GetModuleHandle()" OFF)

##############################################
##### Build

# If no build type was specified, set it to Release.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(target_arch)
target_architecture(ARCHITECTURE)
set(ARCHITECTURE "${ARCHITECTURE}" CACHE INTERNAL "ARCHITECTURE")

message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiling for architecture ${ARCHITECTURE}")
message(STATUS "Compiling for target platform ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Option MOON_USE_LUAJIT is ${MOON_USE_LUAJIT}")
message(STATUS "Option MOON_BUILD_AS_CXX is ${MOON_BUILD_AS_CXX}")
message(STATUS "Option MOON_SYSTEM_GETPROCADDRESS is ${MOON_SYSTEM_GETPROCADDRESS}")
message(STATUS "Option MOON_SYSTEM_GETMODULEHANDLE is ${MOON_SYSTEM_GETMODULEHANDLE}")

message(STATUS "CMAKE_C_FLAGS_DEBUG is ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_C_FLAGS_RELEASE is ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_C_FLAGS is ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")

##############################################
##### Dependencies

set(CPM_SOURCE_CACHE "${PROJECT_SOURCE_DIR}/.cpm_cache" CACHE STRING "")

include(apultra)
include(miniz)

##############################################
##### Code

add_subdirectory(extern)
add_subdirectory(src)
add_subdirectory(library)
add_subdirectory(apps)

##############################################
##### Tests

if(CMAKE_BUILD_TYPE MATCHES Debug AND MOON_STANDALONE_PROJECT)
  include(catch2)
  # Enable unit testing via "make test" once the code has been compiled.
  enable_testing()
  add_subdirectory(tests)
endif()

##############################################
##### Install

# If no installation prefix is given manually, install locally.

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  string(TOLOWER ${CMAKE_BUILD_TYPE} ARTIFACT_OUTPUT_DIR)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/${ARTIFACT_OUTPUT_DIR}" CACHE STRING "The install location" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

##############################################
##### Package

set(PROJECT_SYSTEM ${CMAKE_SYSTEM_NAME})
if (PROJECT_SYSTEM STREQUAL "Darwin")
  set(PROJECT_SYSTEM "macos")
endif()
set(PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}_${PROJECT_SYSTEM}_${ARCHITECTURE}")
string(TOLOWER "${PACKAGE_FILE_NAME}"  PACKAGE_FILE_NAME)

# Tell CPack to create a zip file.
set(CPACK_GENERATOR "ZIP")
# Tell CPack what to name the zip file. It will append `.zip`.
set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_FILE_NAME}")
# Tell CPack not to put everything inside an enclosing directory.
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
# Apparently this should be always on but isn't for backwards compatibility.
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)
