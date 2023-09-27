include(CPM)

CPMAddPackage(
        NAME signals
        GITHUB_REPOSITORY TheWisp/signals
        GIT_TAG 7ff59d1bc6387337ef34856a450be34cfa2c9e51
        EXCLUDE_FROM_ALL YES
)
if (signals_ADDED)
  add_library(signals INTERFACE)
  target_include_directories(signals INTERFACE ${signals_SOURCE_DIR})
  add_library(thewisp::signals ALIAS signals)
endif()
