include(CPM)

CPMAddPackage(
        NAME monocyphercpp
        GITHUB_REPOSITORY snej/monocypher-cpp
        GIT_TAG f7e82c287a87cb49008fd11e1e8c67678646f57a
        DOWNLOAD_ONLY YES
)
if (monocyphercpp_ADDED)
  #add_library(monocyphercpp INTERFACE)
  #add_library(monocyphercpp::monocyphercpp ALIAS monocyphercpp)
  #target_include_directories(monocyphercpp INTERFACE
  #        ${monocyphercpp_SOURCE_DIR}/include
  #        ${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src
  #        ${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src/optional)
  #target_include_directories(monocyphercpp INTERFACE
  #        $<BUILD_INTERFACE:${monocyphercpp_SOURCE_DIR}/include>
  #        $<BUILD_INTERFACE:${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src>
  #        $<BUILD_INTERFACE:${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src/optional>)
  #target_sources(monocyphercpp INTERFACE
  #        ${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src/monocypher.c
  #        ${monocyphercpp_SOURCE_DIR}/vendor/monocypher/src/optional/monocypher-ed25519.c)
endif()
