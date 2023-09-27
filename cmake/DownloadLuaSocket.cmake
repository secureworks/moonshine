include(cmake/CPM.cmake)

CPMAddPackage(
        NAME luasocket
        GITHUB_REPOSITORY lunarmodules/luasocket
        GIT_TAG v3.1.0
        DOWNLOAD_ONLY YES
        EXCLUDE_FROM_ALL YES
)
if (luasocket_ADDED)
    # socket.core
    set(luasocket-core_HEADERS
            luasocket.h socket.h timeout.h buffer.c io.h auxiliar.h
            compat.h
            options.h inet.h except.h select.h tcp.h udp.h)
    set(luasocket-core_SOURCES
            luasocket.c timeout.c buffer.c io.c auxiliar.c
            compat.c
            options.c inet.c except.c select.c tcp.c udp.c)
    if(WIN32)
        set(luasocket-core_HEADERS ${luasocket-core_HEADERS} wsocket.h)
        set(luasocket-core_SOURCES ${luasocket-core_SOURCES} wsocket.c)
    else()
        set(luasocket-core_HEADERS ${luasocket-core_HEADERS} usocket.h)
        set(luasocket-core_SOURCES ${luasocket-core_SOURCES} usocket.c)
    endif()

    list(TRANSFORM luasocket-core_HEADERS PREPEND ${luasocket_SOURCE_DIR}/src/)
    list(TRANSFORM luasocket-core_SOURCES PREPEND ${luasocket_SOURCE_DIR}/src/)

    add_library(luasocket-core INTERFACE ${luasocket-core_HEADERS})
    target_sources(luasocket-core INTERFACE ${luasocket-core_SOURCES})
    target_include_directories(luasocket-core INTERFACE $<BUILD_INTERFACE:${luasocket_SOURCE_DIR}/src>)
    set_target_properties(luasocket-core PROPERTIES LINKER_LANGUAGE C)
    set_target_properties(luasocket-core PROPERTIES POSITION_INDEPENDENT_CODE ON)

    if(WIN32)
        target_compile_definitions(luasocket-core INTERFACE "LUASOCKET_API=__declspec(dllexport)")
        target_link_libraries(luasocket-core INTERFACE ws2_32 wsock32)
    endif()

    # socket.mime
    set(luasocket-mime_HEADERS mime.h
            compat.h
            )
    list(TRANSFORM luasocket-mime_HEADERS PREPEND ${luasocket_SOURCE_DIR}/src/)
    set(luasocket-mime_SOURCES mime.c
            compat.c
            )
    list(TRANSFORM luasocket-mime_SOURCES PREPEND ${luasocket_SOURCE_DIR}/src/)

    add_library(luasocket-mime INTERFACE ${luasocket-mime_HEADERS})
    target_sources(luasocket-mime INTERFACE ${luasocket-mime_SOURCES})
    target_include_directories(luasocket-mime INTERFACE $<BUILD_INTERFACE:${luasocket_SOURCE_DIR}/src>)
    set_target_properties(luasocket-mime PROPERTIES LINKER_LANGUAGE C)
    set_target_properties(luasocket-mime PROPERTIES POSITION_INDEPENDENT_CODE ON)

    if(WIN32)
      target_compile_definitions(luasocket-mime INTERFACE "MIME_API=__declspec(dllexport)")
    endif()
endif()
