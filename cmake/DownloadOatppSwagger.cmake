include(cmake/CPM.cmake)

find_package(oatpp-swagger QUIET)
if(NOT oatpp-swagger)
    CPMAddPackage(
            NAME oatpp-swagger
            GITHUB_REPOSITORY oatpp/oatpp-swagger
            GIT_TAG b0bf4c0e71ed77e3e52040f0da30032de56b7840 # v1.3.0
            OPTIONS "BUILD_SHARED_LIBS OFF" "OATPP_INSTALL OFF" "OATPP_BUILD_TESTS OFF" "OATPP_MSVC_LINK_STATIC_RUNTIME ON"
    )
    add_dependencies(oatpp-swagger oatpp)
    add_library(oatpp::oatpp-swagger ALIAS oatpp-swagger)
endif()