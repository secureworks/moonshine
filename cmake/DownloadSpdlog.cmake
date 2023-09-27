include(CPM)

CPMAddPackage(
        NAME spdlog
        GITHUB_REPOSITORY gabime/spdlog
        VERSION 1.9.2
        OPTIONS "SPDLOG_BUILD_EXAMPLE OFF" "SPDLOG_INSTALL OFF" "SPDLOG_FMT_EXTERNAL ON"
)
