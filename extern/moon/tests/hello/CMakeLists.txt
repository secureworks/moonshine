##############################################
##### Build

# Define a name that will be used for the library
set(TARGET "hello")

# Here we define the executable.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(${TARGET} MODULE)

# Specify sources to use when building the library.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PRIVATE hello.c)

# Set library properties that affect how it is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE C)
set_target_properties(${TARGET} PROPERTIES PREFIX "")

# Specify libraries or flags to use when linking the library.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
if (WIN32)
  if(MOON_USE_LUAJIT)
    target_link_dependencies(${TARGET} PRIVATE liblua51)
  else()
    target_link_dependencies(${TARGET} PRIVATE liblua54)
  endif()
endif()

# Specify the include directories to use when compiling the library.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
if(MOON_USE_LUAJIT)
  target_include_directories(${TARGET} PRIVATE $<TARGET_PROPERTY:liblua51,INCLUDE_DIRECTORIES>)
else()
  target_include_directories(${TARGET} PRIVATE $<TARGET_PROPERTY:liblua54,INCLUDE_DIRECTORIES>)
endif()

# Specify compile time options when compiling the target.
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
