include(cmake/CPM.cmake)

CPMAddPackage(
        NAME fmt
        GITHUB_REPOSITORY fmtlib/fmt
        GIT_TAG 8.0.1
)

if (fmt_ADDED)
endif()