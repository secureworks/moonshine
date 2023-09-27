include(cmake/CPM.cmake)

find_package(Catch2 QUIET)
if(NOT Catch2_FOUND)
    CPMAddPackage(
            NAME Catch2
            GITHUB_REPOSITORY catchorg/Catch2
            VERSION 2.13.8
    )
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
endif()
