include(cmake/CPM.cmake)

find_package(oatpp-sqlite QUIET)
if(NOT oatpp-sqlite)
    CPMAddPackage(
            NAME oatpp-sqlite
            #GITHUB_REPOSITORY oatpp/oatpp-sqlite
            #GIT_TAG 95488343163a9111f03709f68e4944aa46217962 # v1.3.0
            GITHUB_REPOSITORY sidaf/oatpp-sqlite
            GIT_TAG fix_windows # v1.3.0
            OPTIONS "BUILD_SHARED_LIBS OFF" "OATPP_INSTALL OFF" "OATPP_BUILD_TESTS OFF" "OATPP_SQLITE_AMALGAMATION ON"
    )
    add_dependencies(oatpp-sqlite oatpp)
    add_library(oatpp::oatpp-sqlite ALIAS oatpp-sqlite)
endif()
