##############################################
##### Build - socket.core

# Here we define the target.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(luasec_lib MODULE)
add_library(luasec::lib ALIAS luasec_lib)

# Specify sources to use when building the target.
# https://cmake.org/cmake/help/latest/command/target_sources.html
#target_sources(luasec_lib PRIVATE $<TARGET_OBJECTS:luasec>)

# Set library properties that affect how the target is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(luasec_lib PROPERTIES LINKER_LANGUAGE C)
set_target_properties(luasec_lib PROPERTIES PREFIX "")
set_target_properties(luasec_lib PROPERTIES OUTPUT_NAME "ssl")

# Specify libraries or flags to use when linking the target.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_libraries(luasec_lib PRIVATE luasec OpenSSL::Crypto OpenSSL::SSL)
if (WIN32)
    target_link_libraries(luasec_lib PRIVATE liblua51)
endif()

# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(luasec_lib PRIVATE $<TARGET_PROPERTY:liblua51,INCLUDE_DIRECTORIES>)

# Specify compile time options when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(luasec_lib PRIVATE)

# Add options to the link step for a target.
# https://cmake.org/cmake/help/latest/command/target_link_options.html
target_link_options(luasec_lib PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<PLATFORM_ID:MINGW>>>:-Wl,--unresolved-symbols=ignore-all>)

if(APPLE)
  # Add options to the link step for a target.
  # https://cmake.org/cmake/help/latest/command/target_link_options.html
  target_link_options(luasec_lib PRIVATE
          # https://stackoverflow.com/questions/48176641/linking-to-an-executable-under-osx-with-cmake
          -undefined dynamic_lookup)
endif()

##############################################
##### Generate header files so that the module can be embedded

set(LUASEC_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/luasec")
file(MAKE_DIRECTORY ${LUASEC_HEADERS_DIR})

add_custom_command(
        OUTPUT ${LUASEC_HEADERS_DIR}/ssl.lua.h
        COMMAND moonlight -c -z -m ${luasec_SOURCE_DIR}/src/ssl.lua > ${LUASEC_HEADERS_DIR}/ssl.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASEC_HEADERS_DIR}/ssl.https.lua.h
        COMMAND moonlight -c -z -p ssl_ -m ${luasec_SOURCE_DIR}/src/https.lua > ${LUASEC_HEADERS_DIR}/ssl.https.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASEC_HEADERS_DIR}/ssl.c.h
        COMMAND moonlight -c -z -m $<TARGET_FILE_DIR:luasec_lib>/ssl${CMAKE_SHARED_MODULE_SUFFIX} > ${LUASEC_HEADERS_DIR}/ssl.c.h
        DEPENDS moonlight luasec_lib
)

add_library(luasec_headers INTERFACE
        ${LUASEC_HEADERS_DIR}/ssl.lua.h
        ${LUASEC_HEADERS_DIR}/ssl.https.lua.h
        ${LUASEC_HEADERS_DIR}/ssl.c.h
        )
add_library(luasec::headers ALIAS luasec_headers)
target_include_directories(luasec_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

##############################################
##### Install

include(GNUInstallDirs)

set(TARGET_SYSTEM ${CMAKE_SYSTEM_NAME})
string(TOLOWER "${TARGET_SYSTEM}" TARGET_SYSTEM)
if (TARGET_SYSTEM STREQUAL "darwin")
  set(TARGET_SYSTEM "macos")
endif()

# The INSTALL(TARGET) command specifies where to install specific targets.
#install(TARGETS luasec_lib LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${TARGET_SYSTEM}/${ARCHITECTURE} COMPONENT ${PROJECT_NAME})
