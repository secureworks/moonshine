include(cmake/CPM.cmake)

CPMAddPackage(
        NAME lua_binaryheap
        GITHUB_REPOSITORY Tieske/binaryheap.lua
        GIT_TAG version_0v4
        DOWNLOAD_ONLY YES
        EXCLUDE_FROM_ALL YES
)
