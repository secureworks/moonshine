include(cmake/CPM.cmake)

CPMAddPackage(
        NAME luasec
        GITHUB_REPOSITORY brunoos/luasec
        GIT_TAG v1.2.0
        DOWNLOAD_ONLY YES
        EXCLUDE_FROM_ALL YES
)
if (luasec_ADDED)
    set(luasec_HEADERS options.h x509.h context.h ssl.h ec.h compat.h luasocket/buffer.h luasocket/timeout.h luasocket/io.h luasocket/socket.h)
    set(luasec_SOURCES options.c x509.c context.c ssl.c ec.c config.c luasocket/buffer.c luasocket/timeout.c luasocket/io.c)

    if(WIN32)
      set(luasec_HEADERS ${luasec_HEADERS} luasocket/wsocket.h)
      set(luasec_SOURCES ${luasec_SOURCES} luasocket/wsocket.c)
    else()
      set(luasec_HEADERS ${luasec_HEADERS} luasocket/usocket.h)
      set(luasec_HEADERS ${luasec_SOURCES} luasocket/usocket.c)
    endif()

    list(TRANSFORM luasec_HEADERS PREPEND ${luasec_SOURCE_DIR}/src/)
    list(TRANSFORM luasec_SOURCES PREPEND ${luasec_SOURCE_DIR}/src/)

    add_library(luasec INTERFACE ${luasec_HEADERS})
    target_sources(luasec INTERFACE ${luasec_SOURCES})
    target_include_directories(luasec INTERFACE $<BUILD_INTERFACE:${luasec_SOURCE_DIR}/src>)
    set_target_properties(luasec PROPERTIES LINKER_LANGUAGE C)
    set_target_properties(luasec PROPERTIES POSITION_INDEPENDENT_CODE ON)

    if(WIN32)
      target_link_libraries(luasec INTERFACE ws2_32)
    endif()
endif()
