##############################################
##### Generate header files so that the module can be embedded

set(BINARYHEAP_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/binaryheap")
file(MAKE_DIRECTORY ${BINARYHEAP_HEADERS_DIR})

add_custom_command(
        OUTPUT ${BINARYHEAP_HEADERS_DIR}/binaryheap.lua.h
        COMMAND moonlight -c -z -m ${lua_binaryheap_SOURCE_DIR}/src/binaryheap.lua > ${BINARYHEAP_HEADERS_DIR}/binaryheap.lua.h
        DEPENDS moonlight
)

add_library(binaryheap_headers INTERFACE
        ${BINARYHEAP_HEADERS_DIR}/binaryheap.lua.h
        )
add_library(binaryheap::headers ALIAS binaryheap_headers)
target_include_directories(binaryheap_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
