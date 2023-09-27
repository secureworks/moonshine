# Define a name that will be used for the executable
set(TARGET "moon_tests")

# Here we define the executable.
# https://cmake.org/cmake/help/latest/command/add_executable.html
add_executable(${TARGET})

# Specify sources to use when building the executable.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PRIVATE main.cpp moon.cpp $<TARGET_OBJECTS:moon_object>)

# Define expected compiler features
# https://cmake.org/cmake/help/latest/command/target_compile_features.html
#target_compile_features(${TARGET} PRIVATE cxx_std_17)

# Set library properties that affect how it is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)
# Set the executable to export symbols for loadable modules.
# https://cmake.org/cmake/help/latest/prop_tgt/ENABLE_EXPORTS.html
set_target_properties(${TARGET} PROPERTIES ENABLE_EXPORTS ON)
#set_target_properties(${TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Specify the include directories to use when compiling the executable.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(${TARGET} PRIVATE)

# Specify libraries or flags to use when linking the executable.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
#target_link_libraries(${TARGET} PRIVATE
target_link_dependencies(${TARGET} PRIVATE
        moon_object
        Catch2::Catch2
        miniz)

# Specify compile time options when compiling the executable.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(${TARGET} PRIVATE)

# Specify options to the link step for an executable.
# https://cmake.org/cmake/help/git-stage/command/target_link_options.html
target_link_options(${TARGET} PRIVATE)

# Include dynamic library for use in testing
add_subdirectory(hello)
add_dependencies(${TARGET} hello)
if(WIN32)
    add_subdirectory(aaa_dll)
    add_dependencies(${TARGET} aaa_dll)
    add_subdirectory(bbb_dll)
    add_dependencies(${TARGET} bbb_dll)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${TARGET})
