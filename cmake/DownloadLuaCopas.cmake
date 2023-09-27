include(cmake/CPM.cmake)

CPMAddPackage(
        NAME lua_copas
        GITHUB_REPOSITORY lunarmodules/copas
        GIT_TAG 4.3.2
        DOWNLOAD_ONLY YES
        EXCLUDE_FROM_ALL YES
)
