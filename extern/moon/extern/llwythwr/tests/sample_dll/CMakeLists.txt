##############################################
##### Build

# Define a name that will be used for the library
set(TARGET "sample_dll")

# Here we define the executable.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(${TARGET} SHARED)

# Specify sources to use when building the library.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PRIVATE sample.cpp)

# Set library properties that affect how it is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${TARGET} PROPERTIES PREFIX "")
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "sample")

# Specify libraries or flags to use when linking the library.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_libraries(${TARGET} PRIVATE)

# Specify the include directories to use when compiling the library.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(${TARGET} PUBLIC .)
