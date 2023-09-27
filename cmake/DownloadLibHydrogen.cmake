include(CPM)

CPMAddPackage(
        NAME libhydrogen
        GITHUB_REPOSITORY jedisct1/libhydrogen
        GIT_TAG 01c32862f6f6e864c113efc3f0142e05ea3531fe
        EXCLUDE_FROM_ALL YES
)
if (libhydrogen_ADDED)
  set_target_properties("hydrogen-tests"
          PROPERTIES
          EXCLUDE_FROM_ALL 1
          EXCLUDE_FROM_DEFAULT_BUILD 1)
endif()
