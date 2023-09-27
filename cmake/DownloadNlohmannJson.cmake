include(CPM)

find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    CPMAddPackage(
            NAME nlohmann_json
            GITHUB_REPOSITORY ArthurSonzogni/nlohmann_json_cmake_fetchcontent
            GIT_TAG 6aebf09233951e4ce30a63919186a70b2b195756 # v3.10.5
            OPTIONS "JSON_BuildTests OFF" "JSON_Install OFF"
    )
endif()
