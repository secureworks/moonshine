include(CPM)

CPMAddPackage(
        NAME yas
        GITHUB_REPOSITORY niXman/yas
        GIT_TAG 7.1.0
        EXCLUDE_FROM_ALL YES
)
if (yas_ADDED)
  add_library(yas INTERFACE)
  target_include_directories(yas INTERFACE ${yas_SOURCE_DIR}/include)
  add_library(yas::yas ALIAS yas)
endif()
