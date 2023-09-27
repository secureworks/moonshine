include(cmake/CPM.cmake)

find_package(simple-web-server QUIET)
if(NOT simple-web-server_FOUND)
    CPMAddPackage(
            NAME simple-web-server
            GITLAB_REPOSITORY eidheim/Simple-Web-Server
            GIT_TAG v3.1.1
            OPTIONS "USE_STANDALONE_ASIO ON" "USE_OPENSSL ON" "BUILD_TESTING OFF" "BUILD_FUZZING OFF"
    )
    if(simple-web-server_ADDED)
        add_dependencies(simple-web-server asio)
    endif()
endif()