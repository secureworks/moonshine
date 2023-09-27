include(cmake/CPM.cmake)

CPMAddPackage(
        NAME cxxopts
        GITHUB_REPOSITORY jarro2783/cxxopts
        VERSION 2.2.1
        OPTIONS
        "CXXOPTS_BUILD_EXAMPLES OFF"
        "CXXOPTS_BUILD_TESTS OFF"
        "CXXOPTS_ENABLE_INSTALL OFF"
)
if (cxxopts_ADDED)
endif()