##############################################
##### Generate header files so that the module can be embedded

set(COXPCALL_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/coxpcall")
file(MAKE_DIRECTORY ${COXPCALL_HEADERS_DIR})

add_custom_command(
        OUTPUT ${COXPCALL_HEADERS_DIR}/coxpcall.lua.h
        COMMAND moonlight -c -z -m ${lua_coxpcall_SOURCE_DIR}/src/coxpcall.lua > ${COXPCALL_HEADERS_DIR}/coxpcall.lua.h
        DEPENDS moonlight
)

add_library(coxpcall_headers INTERFACE
        ${COXPCALL_HEADERS_DIR}/coxpcall.lua.h
        )
add_library(coxpcall::headers ALIAS coxpcall_headers)
target_include_directories(coxpcall_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
