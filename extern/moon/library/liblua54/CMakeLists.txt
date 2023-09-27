# Define a name that will be used for the target
set(TARGET "liblua54")

# Here we define the target.
# https://cmake.org/cmake/help/latest/command/add_executable.html
add_library(${TARGET} SHARED)

# Specify sources to use when building the target.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PRIVATE $<TARGET_OBJECTS:moon_object>)

# Define expected compiler features
# https://cmake.org/cmake/help/latest/command/target_compile_features.html
target_compile_features(${TARGET} PRIVATE)

# Set library properties that affect how the target is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "lua54")
set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE C)
if (WIN32)
  set_target_properties(${TARGET} PROPERTIES PREFIX "")
endif()

# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(${TARGET} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Specify libraries or flags to use when linking the target.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_libraries(${TARGET} PRIVATE moon_object)

# Specify compile time options when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(${TARGET} PRIVATE)

# Specify options to the link step for an target.
# https://cmake.org/cmake/help/git-stage/command/target_link_options.html
target_link_options(${TARGET} PRIVATE)

##############################################
##### Install

#add_custom_command(
#        TARGET ${TARGET} POST_BUILD
#        DEPENDS "${TARGET}"
#        COMMAND $<$<CONFIG:Release>:${CMAKE_STRIP}>
#        ARGS $<TARGET_FILE:${TARGET}>
#)

include(GNUInstallDirs)

set(TARGET_SYSTEM ${CMAKE_SYSTEM_NAME})
string(TOLOWER "${TARGET_SYSTEM}" TARGET_SYSTEM)
if (TARGET_SYSTEM STREQUAL "darwin")
  set(TARGET_SYSTEM "macos")
endif()

# The INSTALL(TARGET) command specifies where to install specific targets.
install(TARGETS ${TARGET}
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${TARGET_SYSTEM}/${ARCHITECTURE}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}/${TARGET_SYSTEM}/${ARCHITECTURE}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${TARGET_SYSTEM}/${ARCHITECTURE}"
        COMPONENT ${PROJECT_NAME})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT ${PROJECT_NAME} FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
