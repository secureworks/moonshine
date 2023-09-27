##############################################
##### Generate header files so that the module can be embedded

set(TIMERWHEEL_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/timerwheel")
file(MAKE_DIRECTORY ${TIMERWHEEL_HEADERS_DIR})

add_custom_command(
        OUTPUT ${TIMERWHEEL_HEADERS_DIR}/timerwheel.lua.h
        COMMAND moonlight -c -z -m ${lua_timerwheel_SOURCE_DIR}/src/timerwheel.lua > ${TIMERWHEEL_HEADERS_DIR}/timerwheel.lua.h
        DEPENDS moonlight
)

add_library(timerwheel_headers INTERFACE
        ${TIMERWHEEL_HEADERS_DIR}/timerwheel.lua.h
        )
add_library(timerwheel::headers ALIAS timerwheel_headers)
target_include_directories(timerwheel_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
