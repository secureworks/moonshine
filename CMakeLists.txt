cmake_minimum_required(VERSION 3.20)

# Add helper modules (Find*.cmake etc) folder to the CMake path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

cmake_policy(SET CMP0091 NEW)

##############################################
##### Project

project(moonshine
        VERSION 0.7.0
        DESCRIPTION "Moonshine"
        LANGUAGES CXX C)

if(MSVC)
  enable_language(ASM_MASM)
else()
  enable_language(ASM)
endif()

# Generate config.h
configure_file ("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_BINARY_DIR}")

include(ObjectsFirstClass)

#set (CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/cmake/CompilerOptions.cmake")

##############################################
##### Build

# If no build type was specified, set it to Release.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Define compiler and link options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(TargetArch)
target_architecture(ARCHITECTURE)
set(ARCHITECTURE "${ARCHITECTURE}" CACHE INTERNAL "ARCHITECTURE")

message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiling for architecture ${ARCHITECTURE}")
message(STATUS "Compiling for target platform ${CMAKE_SYSTEM_VERSION}")

message(STATUS "CMAKE_C_FLAGS_DEBUG is ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_C_FLAGS_RELEASE is ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_C_FLAGS is ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG=1)
  add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  add_definitions(-DBUILD_DIR="${CMAKE_BINARY_DIR}")
else()
  add_definitions(-DDEBUG=0)
endif()

##############################################
##### Dependencies

set(CPM_SOURCE_CACHE "${PROJECT_SOURCE_DIR}/.cpm_cache" CACHE STRING "")

include(CPM)

find_package(Threads REQUIRED)
#set(Boost_USE_STATIC_LIBS ON)
#set(Boost_USE_RELEASE_LIBS ON)
#set(Boost_USE_DEBUG_LIBS OFF)
#set(Boost_USE_STATIC_RUNTIME ON)
#set(Boost_USE_MULTITHREADED ON)

include(DownloadFmt)
include(DownloadCxxopts)
include(DownloadSpdlog)
include(DownloadOpenSSL)
include(DownloadOatpp)
include(DownloadOatppOpenssl)
include(DownloadOatppSwagger)
include(DownloadOatppSqlite)
include(DownloadOatppWebsocket)
include(DownloadLibHydrogen)
include(DownloadMonocypherCpp)
include(DownloadNlohmannJson)
include(DownloadYas)
include(DownloadSignals)
include(DownloadLuaSocket)
include(DownloadLuaSec)
include(DownloadLuaBinaryheap)
include(DownloadLuaCopas)
include(DownloadLuaCoxpcall)
include(DownloadLuaTimerwheel)
#include(DownloadLuaLuv)

##############################################
##### Code

add_subdirectory(extern)
add_subdirectory(modules)
add_subdirectory(shared)
add_subdirectory(implant)
add_subdirectory(server)
add_subdirectory(apps)
add_subdirectory(examples)

##############################################
##### Tests

if(CMAKE_BUILD_TYPE MATCHES Debug)
    include(DownloadCatch2)
    include(DownloadStandaloneAsio)
    include(DownloadSimpleWebServer)

    # Enable unit testing via "make test" once the code has been compiled.
    include(CTest)
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
# Enable stripping of all files
set(CPACK_STRIP_FILES YES)
include(CPack)
