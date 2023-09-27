include(cmake/CPM.cmake)

CPMAddPackage(
        NAME OpenSSL
        GITHUB_REPOSITORY janbar/openssl-cmake
        GIT_TAG 1.1.1n-20220327
        OPTIONS "DWITH_APPS OFF" "BUILD_SHARED_LIBS OFF"
        EXCLUDE_FROM_ALL YES
)
if (OpenSSL_ADDED)
    add_library(OpenSSL::SSL ALIAS ssl)
    target_include_directories(ssl PUBLIC ${OpenSSL_SOURCE_DIR}/include)
    set_target_properties(ssl PROPERTIES POSITION_INDEPENDENT_CODE ON)

    add_library(OpenSSL::Crypto ALIAS crypto)
    target_include_directories(crypto PUBLIC ${OpenSSL_SOURCE_DIR}/include)
    set_target_properties(crypto PROPERTIES POSITION_INDEPENDENT_CODE ON)

    set(OPENSSL_INCLUDE_DIR "${OpenSSL_SOURCE_DIR}/include" CACHE INTERNAL "")
    if(UNIX OR MINGW)
        set(OPENSSL_SSL_LIBRARY "${OpenSSL_BINARY_DIR}/ssl/libssl.a" CACHE INTERNAL "")
        set(OPENSSL_CRYPTO_LIBRARY "${OpenSSL_BINARY_DIR}/crypto/libcrypto.a" CACHE INTERNAL "")
    elseif(WIN32)
        set(OPENSSL_SSL_LIBRARY "${OpenSSL_BINARY_DIR}/ssl/libssl.lib" CACHE INTERNAL "")
        set(OPENSSL_CRYPTO_LIBRARY "${OpenSSL_BINARY_DIR}/crypto/libcrypto.lib" CACHE INTERNAL "")
    endif()
    set(OPENSSL_SSL_LIBRARIES "${OPENSSL_SSL_LIBRARY}" CACHE INTERNAL "")
    set(OPENSSL_CRYPTO_LIBRARIES "${OPENSSL_CRYPTO_LIBRARY}" CACHE INTERNAL "")
    set(OPENSSL_LIBRARIES "${OPENSSL_SSL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARIES}" CACHE INTERNAL "")
endif()
