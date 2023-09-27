include(cmake/CPM.cmake)

find_package(oatpp-websocket QUIET)
if(NOT oatpp-websocket)
    CPMAddPackage(
            NAME oatpp-websocket
            #GITHUB_REPOSITORY oatpp/oatpp-websocket
            #GIT_TAG 62cc5df6b761699ced70f9a719ce701db1bb0a07 # v1.3.0
            GITHUB_REPOSITORY sidaf/oatpp-websocket
            GIT_TAG fix_windows # v1.3.0
            OPTIONS "BUILD_SHARED_LIBS OFF" "OATPP_INSTALL OFF" "OATPP_BUILD_TESTS OFF" "OATPP_MSVC_LINK_STATIC_RUNTIME ON"
    )
    add_dependencies(oatpp-websocket oatpp)
    add_library(oatpp::oatpp-websocket ALIAS oatpp-websocket)
endif()
