include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (NOT DEFINED deinamig_INSTALL_CMAKEDIR)
    set(deinamig_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/deinamig"
            CACHE STRING "Path to deinamig CMake files")
endif ()

install(TARGETS deinamig EXPORT deinamig_Targets
        RUNTIME COMPONENT deinamig_Runtime
        LIBRARY COMPONENT deinamig_Runtime
        NAMELINK_COMPONENT deinamig_Development
        ARCHIVE COMPONENT deinamig_Development
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(DIRECTORY "${deinamig_SOURCE_DIR}/include/" "${deinamig_BINARY_DIR}/include/"
        TYPE INCLUDE
        COMPONENT deinamig_Development)

if (BUILD_SHARED_LIBS)
    set(type shared)
elseif (DEINAMIG_STATIC_LIBS)
    set(type static)
else ()
    set(type object)
endif ()

install(EXPORT deinamig_Targets
        DESTINATION "${deinamig_INSTALL_CMAKEDIR}"
        NAMESPACE Libdeinamig::
        FILE deinamig-${type}-targets.cmake
        COMPONENT deinamig_Development)

write_basic_package_version_file(
        deinamigConfigVersion.cmake
        COMPATIBILITY SameMajorVersion)

install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/deinamigConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/deinamigConfigVersion.cmake"
        DESTINATION "${deinamig_INSTALL_CMAKEDIR}"
        COMPONENT deinamig_Development)

## Use macos instead of darwin as operating system type

string(TOLOWER "${CMAKE_SYSTEM_NAME}"  PROJECT_SYSTEM)
if (PROJECT_SYSTEM STREQUAL "darwin")
    set(PROJECT_SYSTEM "macos")
endif()

## If no installation prefix is given, install locally.

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/_install/${PROJECT_SYSTEM}-${CMAKE_SYSTEM_PROCESSOR}" CACHE STRING "The install location" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

## Packaging

set(PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}_${PROJECT_SYSTEM}_${CMAKE_SYSTEM_PROCESSOR}")
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
