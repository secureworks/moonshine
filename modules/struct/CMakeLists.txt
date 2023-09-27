##############################################
##### Build - struct

# Here we define the target.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(struct_module MODULE)

# Specify sources to use when building the target.
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(struct_module PRIVATE struct.c)

# Set library properties that affect how the target is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html=
set_target_properties(struct_module PROPERTIES PREFIX "")
set_target_properties(struct_module PROPERTIES OUTPUT_NAME "struct")

# Specify libraries or flags to use when linking the target.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_libraries(struct_module PRIVATE)
if (WIN32)
    target_link_libraries(struct_module PRIVATE liblua51)
endif()

# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(struct_module PRIVATE $<TARGET_PROPERTY:liblua51,INCLUDE_DIRECTORIES>)

# Specify compile time options when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(struct_module PRIVATE)

# Specifies compile definitions to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_definitions.html
target_compile_definitions(struct_module PRIVATE -DLUA_LIB)
if (WIN32)
  target_compile_definitions(struct_module PRIVATE -DLUA_BUILD_AS_DLL)
endif()

# Add options to the link step for a target.
# https://cmake.org/cmake/help/latest/command/target_link_options.html
target_link_options(struct_module PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<PLATFORM_ID:MINGW>>>:-Wl,--unresolved-symbols=ignore-all>)

if(APPLE)
  # Add options to the link step for a target.
  # https://cmake.org/cmake/help/latest/command/target_link_options.html
  target_link_options(struct_module PRIVATE
          # https://stackoverflow.com/questions/48176641/linking-to-an-executable-under-osx-with-cmake
          -undefined dynamic_lookup)
endif()

##############################################
##### Generate header files so that the module can be embedded

set(STRUCT_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/struct")
file(MAKE_DIRECTORY ${STRUCT_HEADERS_DIR})

add_custom_command(
        OUTPUT ${STRUCT_HEADERS_DIR}/struct.c.h
        COMMAND moonlight -c -z -m $<TARGET_FILE_DIR:struct_module>/struct${CMAKE_SHARED_MODULE_SUFFIX} > ${STRUCT_HEADERS_DIR}/struct.c.h
        DEPENDS moonlight struct_module
)

add_library(struct_headers INTERFACE ${STRUCT_HEADERS_DIR}/struct.c.h)
add_library(struct::headers ALIAS struct_headers)
target_include_directories(struct_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

##############################################
##### Install

include(GNUInstallDirs)

set(TARGET_SYSTEM ${CMAKE_SYSTEM_NAME})
string(TOLOWER "${TARGET_SYSTEM}" TARGET_SYSTEM)
if (TARGET_SYSTEM STREQUAL "darwin")
  set(TARGET_SYSTEM "macos")
endif()

# The INSTALL(TARGET) command specifies where to install specific targets.
#install(TARGETS socket.core LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${TARGET_SYSTEM}/${ARCHITECTURE} COMPONENT ${PROJECT_NAME})
#install(TARGETS mime.core LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${TARGET_SYSTEM}/${ARCHITECTURE} COMPONENT ${PROJECT_NAME})
