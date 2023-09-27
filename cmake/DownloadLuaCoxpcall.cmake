include(cmake/CPM.cmake)

CPMAddPackage(
        NAME lua_coxpcall
        GITHUB_REPOSITORY keplerproject/coxpcall
        GIT_TAG 50ccf2651b50a1ad882722984ea049cedd4ec811
        DOWNLOAD_ONLY YES
        EXCLUDE_FROM_ALL YES
)
