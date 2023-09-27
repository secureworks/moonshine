include(cmake/CPM.cmake)

CPMAddPackage(
        NAME lua_timerwheel
        GITHUB_REPOSITORY Tieske/timerwheel.lua
        GIT_TAG 68c468e687c2c17d357644c11290112e934760c7
        DOWNLOAD_ONLY YES
        EXCLUDE_FROM_ALL YES
)
