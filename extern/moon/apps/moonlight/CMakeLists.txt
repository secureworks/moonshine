# Define a name that will be used for the executable
set(TARGET "moonlight")

include(CheckIncludeFile)
CHECK_INCLUDE_FILE("readline/readline.h" HAVE_READLINE_READLINE_H)

# Here we define the executable.
# https://cmake.org/cmake/help/latest/command/add_executable.html
add_executable(${TARGET})

if(MOON_BUILD_AS_CXX)
  SET_SOURCE_FILES_PROPERTIES(main.c PROPERTIES LANGUAGE CXX )
endif()

# Specify sources to use when building the executable.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PRIVATE
        $<TARGET_OBJECTS:moon_object>
        main.c)

# Define expected compiler features
# https://cmake.org/cmake/help/latest/command/target_compile_features.html
target_compile_features(${TARGET} PRIVATE)

# Set library properties that affect how it is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
if(MOON_BUILD_AS_CXX)
  set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)
else()
  set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE C)
endif()
set_target_properties(${TARGET} PROPERTIES ENABLE_EXPORTS ON)
#set_target_properties(${TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(TARGET_SYSTEM ${CMAKE_SYSTEM_NAME})
string(TOLOWER "${TARGET_SYSTEM}" TARGET_SYSTEM)
if (TARGET_SYSTEM STREQUAL "darwin")
  set(TARGET_SYSTEM "macos")
endif()
set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "${TARGET}.${TARGET_SYSTEM}.${ARCHITECTURE}")

# Specify the include directories to use when compiling the executable.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(${TARGET} PRIVATE)

# Specify libraries or flags to use when linking the executable.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_dependencies(${TARGET}
        PUBLIC moon_object)

if (HAVE_READLINE_READLINE_H)
  target_link_libraries(${TARGET}
          PUBLIC readline)
endif()

# Specify compile time options when compiling the executable.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(${TARGET} PRIVATE)

# Add compile definitions to the executable.
# https://cmake.org/cmake/help/latest/command/target_compile_definitions.html
if (HAVE_READLINE_READLINE_H)
  target_compile_definitions(${TARGET}
          PUBLIC LUA_USE_READLINE)
endif()

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

##############################################
##### Install

include(GNUInstallDirs)

# The INSTALL(TARGET) command specifies where to install specific targets.
install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
