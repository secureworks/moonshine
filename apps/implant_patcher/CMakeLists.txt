# Define a name that will be used for the executable
set(TARGET "implant_patcher")

# Here we define the executable.
# https://cmake.org/cmake/help/latest/command/add_executable.html
add_executable(${TARGET})

# Specify sources to use when building the executable.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(${TARGET} PRIVATE
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
target_link_dependencies(${TARGET} PRIVATE
        cxxopts
        shared)

# Specify compile time options when compiling the executable.
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

##############################################
##### Install

include(GNUInstallDirs)

# The INSTALL(TARGET) command specifies where to install specific targets.
install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
