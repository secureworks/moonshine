# Define a name that will be used for the target
set(TARGET "win32mod")

# Here we define the target.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(${TARGET} MODULE)

set(${TARGET}_SOURCES win32mod.c)

# Specify sources to use when building the target.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PRIVATE ${${TARGET}_SOURCES})

# Set library properties that affect how the target is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(${TARGET} PROPERTIES LANGUAGE C)
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE C)
set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${TARGET} PROPERTIES PREFIX "")

# Specify libraries or flags to use when linking the target.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_libraries(${TARGET} PRIVATE)
target_link_libraries(${TARGET} PRIVATE liblua51)

# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(${TARGET} PRIVATE $<TARGET_PROPERTY:liblua51,INCLUDE_DIRECTORIES>)

# Specify compile time options when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(${TARGET} PRIVATE)

# Add options to the link step for a target.
# https://cmake.org/cmake/help/latest/command/target_link_options.html
target_link_options(${TARGET} PRIVATE)

##############################################
##### Install

#include(GNUInstallDirs)

# The INSTALL(TARGET) command specifies where to install specific targets.
install(TARGETS ${TARGET} LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR} COMPONENT ${PROJECT_NAME})
