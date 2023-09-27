##############################################
##### Generate header files so that the module can be embedded

set(COPAS_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/copas")
file(MAKE_DIRECTORY ${COPAS_HEADERS_DIR})

add_custom_command(
        OUTPUT ${COPAS_HEADERS_DIR}/copas.lua.h
        COMMAND moonlight -c -z -m ${lua_copas_SOURCE_DIR}/src/copas.lua > ${COPAS_HEADERS_DIR}/copas.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${COPAS_HEADERS_DIR}/copas.ftp.lua.h
        COMMAND moonlight -c -z -p 'copas_' -m ${lua_copas_SOURCE_DIR}/src/copas/ftp.lua > ${COPAS_HEADERS_DIR}/copas.ftp.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${COPAS_HEADERS_DIR}/copas.http.lua.h
        COMMAND moonlight -c -z -p 'copas_' -m ${lua_copas_SOURCE_DIR}/src/copas/http.lua > ${COPAS_HEADERS_DIR}/copas.http.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${COPAS_HEADERS_DIR}/copas.lock.lua.h
        COMMAND moonlight -c -z -p 'copas_' -m ${lua_copas_SOURCE_DIR}/src/copas/lock.lua > ${COPAS_HEADERS_DIR}/copas.lock.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${COPAS_HEADERS_DIR}/copas.queue.lua.h
        COMMAND moonlight -c -z -p 'copas_' -m ${lua_copas_SOURCE_DIR}/src/copas/queue.lua > ${COPAS_HEADERS_DIR}/copas.queue.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${COPAS_HEADERS_DIR}/copas.semaphore.lua.h
        COMMAND moonlight -c -z -p 'copas_' -m ${lua_copas_SOURCE_DIR}/src/copas/semaphore.lua > ${COPAS_HEADERS_DIR}/copas.semaphore.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${COPAS_HEADERS_DIR}/copas.smtp.lua.h
        COMMAND moonlight -c -z -p 'copas_' -m ${lua_copas_SOURCE_DIR}/src/copas/smtp.lua > ${COPAS_HEADERS_DIR}/copas.smtp.lua.h
        DEPENDS moonlight
)

add_custom_command(
        OUTPUT ${COPAS_HEADERS_DIR}/copas.timer.lua.h
        COMMAND moonlight -c -z -p 'copas_' -m ${lua_copas_SOURCE_DIR}/src/copas/timer.lua > ${COPAS_HEADERS_DIR}/copas.timer.lua.h
        DEPENDS moonlight
)

add_library(copas_headers INTERFACE
        ${COPAS_HEADERS_DIR}/copas.lua.h
        ${COPAS_HEADERS_DIR}/copas.ftp.lua.h
        ${COPAS_HEADERS_DIR}/copas.http.lua.h
        ${COPAS_HEADERS_DIR}/copas.lock.lua.h
        ${COPAS_HEADERS_DIR}/copas.semaphore.lua.h
        ${COPAS_HEADERS_DIR}/copas.queue.lua.h
        ${COPAS_HEADERS_DIR}/copas.smtp.lua.h
        ${COPAS_HEADERS_DIR}/copas.timer.lua.h
        )
add_library(copas::headers ALIAS copas_headers)
target_include_directories(copas_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
