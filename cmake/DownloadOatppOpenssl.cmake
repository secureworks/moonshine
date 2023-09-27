include(CPM)

find_package(oatpp-openssl QUIET)
if(NOT oatpp-openssl)
    CPMAddPackage(
            NAME oatpp-openssl
            GITHUB_REPOSITORY oatpp/oatpp-openssl
            GIT_TAG 10a9745f9d28d0a031b263054698fdf63d95548b # v1.3.0
            OPTIONS "BUILD_SHARED_LIBS OFF" "OATPP_INSTALL OFF" "OATPP_BUILD_TESTS OFF"
    )
    add_library(oatpp::oatpp-openssl ALIAS oatpp-openssl)
    add_dependencies(oatpp-openssl OpenSSL::SSL OpenSSL::Crypto)
endif()
