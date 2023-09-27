include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (NOT DEFINED llwythwr_INSTALL_CMAKEDIR)
    set(llwythwr_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/llwythwr"
            CACHE STRING "Path to llwythwr CMake files")
endif ()

install(TARGETS llwythwr EXPORT llwythwr_Targets
        RUNTIME COMPONENT llwythwr_Runtime
        LIBRARY COMPONENT llwythwr_Runtime
        NAMELINK_COMPONENT llwythwr_Development
        ARCHIVE COMPONENT llwythwr_Development
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(DIRECTORY "${llwythwr_SOURCE_DIR}/include/" "${llwythwr_BINARY_DIR}/include/"
        TYPE INCLUDE
        COMPONENT llwythwr_Development)

if (BUILD_SHARED_LIBS)
    set(type shared)
elseif (LLWYTHWR_STATIC_LIBS)
    set(type static)
else ()
    set(type object)
endif ()

install(EXPORT llwythwr_Targets
        DESTINATION "${llwythwr_INSTALL_CMAKEDIR}"
        NAMESPACE Libllwythwr::
        FILE llwythwr-${type}-targets.cmake
        COMPONENT llwythwr_Development)

write_basic_package_version_file(
        llwythwrConfigVersion.cmake
        COMPATIBILITY SameMajorVersion)

install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/llwythwrConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/llwythwrConfigVersion.cmake"
        DESTINATION "${llwythwr_INSTALL_CMAKEDIR}"
        COMPONENT llwythwr_Development)

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
