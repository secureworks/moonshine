include(cmake/CPM.cmake)

find_package(oatpp QUIET)
if(NOT oatpp)
    CPMAddPackage(
            NAME oatpp
            #GITHUB_REPOSITORY oatpp/oatpp
            #GIT_TAG 349d96114ae03ae6fd44dd2a398e5cdd9529fb48 # v1.3.0
            GITHUB_REPOSITORY sidaf/oatpp
            GIT_TAG fix_windows # v1.3.0
            OPTIONS "BUILD_SHARED_LIBS OFF" "OATPP_INSTALL OFF" "OATPP_BUILD_TESTS OFF" "OATPP_MSVC_LINK_STATIC_RUNTIME ON" "OATPP_DISABLE_ENV_OBJECT_COUNTERS ON"
    )
    add_library(oatpp::oatpp ALIAS oatpp)
    add_library(oatpp::oatpp-test ALIAS oatpp-test)
endif()
