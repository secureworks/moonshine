##############################################
##### Build - socket.core

# Here we define the target.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(socket.core MODULE)
add_library(luasocket::core ALIAS socket.core)

# Specify sources to use when building the target.
# https://cmake.org/cmake/help/latest/command/target_sources.html
#target_sources(socket.core PRIVATE $<TARGET_OBJECTS:luasocket-core>)

# Set library properties that affect how the target is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(socket.core PROPERTIES LINKER_LANGUAGE C)
set_target_properties(socket.core PROPERTIES PREFIX "")

# Specify libraries or flags to use when linking the target.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_libraries(socket.core PRIVATE luasocket-core)
if (WIN32)
    target_link_libraries(socket.core PRIVATE liblua51)
endif()

# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(socket.core PRIVATE $<TARGET_PROPERTY:liblua51,INCLUDE_DIRECTORIES>)

# Specify compile time options when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(socket.core PRIVATE)

# Add options to the link step for a target.
# https://cmake.org/cmake/help/latest/command/target_link_options.html
target_link_options(socket.core PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<PLATFORM_ID:MINGW>>>:-Wl,--unresolved-symbols=ignore-all>)

if(APPLE)
  # Add options to the link step for a target.
  # https://cmake.org/cmake/help/latest/command/target_link_options.html
  target_link_options(socket.core PRIVATE
          # https://stackoverflow.com/questions/48176641/linking-to-an-executable-under-osx-with-cmake
          -undefined dynamic_lookup)
endif()

##############################################
##### Build - mime.core

# Here we define the target.
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(mime.core MODULE)
add_library(luasocket::mime ALIAS mime.core)

# Specify sources to use when building the target.
# https://cmake.org/cmake/help/latest/command/target_sources.html
#target_sources(mime.core PRIVATE $<TARGET_OBJECTS:luasocket-mime>)

# Set library properties that affect how the target is built
# https://cmake.org/cmake/help/latest/command/set_target_properties.html
set_target_properties(mime.core PROPERTIES LINKER_LANGUAGE C)
set_target_properties(mime.core PROPERTIES PREFIX "")

# Specify libraries or flags to use when linking the target.
# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_libraries(mime.core PRIVATE luasocket-mime)
if (WIN32)
    target_link_libraries(mime.core PRIVATE liblua51)
endif()

# Specify the include directories to use when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(mime.core PRIVATE $<TARGET_PROPERTY:liblua51,INCLUDE_DIRECTORIES>)

# Specify compile time options when compiling the target.
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
target_compile_options(mime.core PRIVATE)

# Add options to the link step for a target.
# https://cmake.org/cmake/help/latest/command/target_link_options.html
target_link_options(mime.core PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<PLATFORM_ID:MINGW>>>:-Wl,--unresolved-symbols=ignore-all>)

if(APPLE)
  # Add options to the link step for a target.
  # https://cmake.org/cmake/help/latest/command/target_link_options.html
  target_link_options(mime.core PRIVATE
          # https://stackoverflow.com/questions/48176641/linking-to-an-executable-under-osx-with-cmake
          -undefined dynamic_lookup)
endif()

##############################################
##### Generate header files so that the module can be embedded

set(LUASOCKET_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/luasocket")
file(MAKE_DIRECTORY ${LUASOCKET_HEADERS_DIR})

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/socket.lua.h
        COMMAND moonlight -c -z -m ${luasocket_SOURCE_DIR}/src/socket.lua > ${LUASOCKET_HEADERS_DIR}/socket.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/mime.lua.h
        COMMAND moonlight -c -z -m ${luasocket_SOURCE_DIR}/src/mime.lua > ${LUASOCKET_HEADERS_DIR}/mime.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/ltn12.lua.h
        COMMAND moonlight -c -z -m ${luasocket_SOURCE_DIR}/src/ltn12.lua > ${LUASOCKET_HEADERS_DIR}/ltn12.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/socket.http.lua.h
        COMMAND moonlight -c -z -p socket_ -m ${luasocket_SOURCE_DIR}/src/http.lua > ${LUASOCKET_HEADERS_DIR}/socket.http.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/socket.url.lua.h
        COMMAND moonlight -c -z -p socket_ -m ${luasocket_SOURCE_DIR}/src/url.lua > ${LUASOCKET_HEADERS_DIR}/socket.url.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/socket.headers.lua.h
        COMMAND moonlight -c -z -p socket_ -m ${luasocket_SOURCE_DIR}/src/headers.lua > ${LUASOCKET_HEADERS_DIR}/socket.headers.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/socket.tp.lua.h
        COMMAND moonlight -c -z -p socket_ -m ${luasocket_SOURCE_DIR}/src/tp.lua > ${LUASOCKET_HEADERS_DIR}/socket.tp.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/socket.ftp.lua.h
        COMMAND moonlight -c -z -p socket_ -m ${luasocket_SOURCE_DIR}/src/ftp.lua > ${LUASOCKET_HEADERS_DIR}/socket.ftp.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/socket.smtp.lua.h
        COMMAND moonlight -c -z -p socket_ -m ${luasocket_SOURCE_DIR}/src/smtp.lua > ${LUASOCKET_HEADERS_DIR}/socket.smtp.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/socket.core.c.h
        COMMAND moonlight -c -z -m $<TARGET_FILE_DIR:socket.core>/socket.core${CMAKE_SHARED_MODULE_SUFFIX} > ${LUASOCKET_HEADERS_DIR}/socket.core.c.h
        DEPENDS moonlight socket.core
)

add_custom_command(
        OUTPUT ${LUASOCKET_HEADERS_DIR}/mime.core.c.h
        COMMAND moonlight -c -z -m $<TARGET_FILE_DIR:mime.core>/mime.core${CMAKE_SHARED_MODULE_SUFFIX} > ${LUASOCKET_HEADERS_DIR}/mime.core.c.h
        DEPENDS moonlight mime.core
)

add_library(luasocket_headers INTERFACE
        ${LUASOCKET_HEADERS_DIR}/socket.lua.h
        ${LUASOCKET_HEADERS_DIR}/mime.lua.h
        ${LUASOCKET_HEADERS_DIR}/ltn12.lua.h
        ${LUASOCKET_HEADERS_DIR}/socket.http.lua.h
        ${LUASOCKET_HEADERS_DIR}/socket.url.lua.h
        ${LUASOCKET_HEADERS_DIR}/socket.headers.lua.h
        ${LUASOCKET_HEADERS_DIR}/socket.tp.lua.h
        ${LUASOCKET_HEADERS_DIR}/socket.ftp.lua.h
        ${LUASOCKET_HEADERS_DIR}/socket.smtp.lua.h
        ${LUASOCKET_HEADERS_DIR}/socket.core.c.h
        ${LUASOCKET_HEADERS_DIR}/mime.core.c.h
        )
add_library(luasocket::headers ALIAS luasocket_headers)
target_include_directories(luasocket_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

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
