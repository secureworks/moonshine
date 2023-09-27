# Define a name that will be used for the target
set(TARGET "moon_static")

# Here we define the target.
# https://cmake.org/cmake/help/latest/command/add_executable.html
add_library(${TARGET} STATIC)
add_library(moon::static ALIAS ${TARGET})

# Specify sources to use when building the target.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PUBLIC $<TARGET_OBJECTS:moon_object>)

# Define expected compiler features
# https://cmake.org/cmake/help/latest/command/target_compile_features.html
target_compile_features(${TARGET} PRIVATE)

# Set library properties that affect how the target is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "moon")
set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MOON_BUILD_AS_CXX)
  set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)
else()
  set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE C)
endif()
if (WIN32)
  set_target_properties(${TARGET} PROPERTIES PREFIX "")
endif()

# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(${TARGET} PUBLIC)

# Specify libraries or flags to use when linking the target.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_libraries(${TARGET} PUBLIC moon_object)

# Specify compile time options when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(${TARGET} PRIVATE)

# Specify options to the link step for an target.
# https://cmake.org/cmake/help/git-stage/command/target_link_options.html
target_link_options(${TARGET} PRIVATE)

##############################################
##### Install

include(GNUInstallDirs)

set(TARGET_SYSTEM ${CMAKE_SYSTEM_NAME})
string(TOLOWER "${TARGET_SYSTEM}" TARGET_SYSTEM)
if (TARGET_SYSTEM STREQUAL "darwin")
  set(TARGET_SYSTEM "macos")
endif()

# The INSTALL(TARGET) command specifies where to install specific targets.
install(TARGETS ${TARGET} ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${TARGET_SYSTEM}/${ARCHITECTURE}" COMPONENT ${PROJECT_NAME})
