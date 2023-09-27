# Define a name that will be used for the targets
set(TARGET "implant_tests")

# Here we define the targets.
# https://cmake.org/cmake/help/latest/command/add_executable.html
add_executable(${TARGET})

# Specify sources to use when building the targets.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PRIVATE
        $<TARGET_OBJECTS:implant_core>
        main.cpp)

# Define expected compiler features
# https://cmake.org/cmake/help/latest/command/target_compile_features.html
target_compile_features(${TARGET} PRIVATE cxx_std_17)

# Set library properties that affect how it is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)
# Set the executable to export symbols for loadable modules.
# https://cmake.org/cmake/help/latest/prop_tgt/ENABLE_EXPORTS.html
set_target_properties(${TARGET} PROPERTIES ENABLE_EXPORTS ON)
#set_target_properties(${TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Specify the include directories to use when compiling the targets.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(${TARGET} PRIVATE)

# Specify libraries or flags to use when linking the targets.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
#target_link_libraries(${TARGET} PRIVATE
target_link_dependencies(${TARGET} PRIVATE
        implant_core
        simple-web-server
        Catch2::Catch2)

# Specify compile time options when compiling the targets.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(${TARGET} PRIVATE)

# Add options to the link step for a target.
# https://cmake.org/cmake/help/latest/command/target_link_options.html
target_link_options(${TARGET} PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<PLATFORM_ID:MINGW>>>:-Wl,--unresolved-symbols=ignore-all>)

if(APPLE)
  # Add options to the link step for a target.
  # https://cmake.org/cmake/help/latest/command/target_link_options.html
  target_link_options(${TARGET} PRIVATE
          # https://stackoverflow.com/questions/48176641/linking-to-an-executable-under-osx-with-cmake
          -undefined dynamic_lookup)
endif()

# Add a dependency between top-level targets.
# https://cmake.org/cmake/help/latest/command/add_dependencies.html
#add_dependencies(${TARGET})

# Add a test to the project to be run by ctest
# https://cmake.org/cmake/help/latest/command/add_test.html
add_test(NAME ${TARGET} COMMAND ${TARGET} WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR})

# Get tests from a target
catch_discover_tests(${TARGET})
