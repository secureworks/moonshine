include(cpm)

CPMAddPackage(
        NAME apultra
        GITHUB_REPOSITORY emmanuel-marty/apultra
        GIT_TAG 85e019447213805881fbcee97d2597971247ef46 # 1.4.4
        DOWNLOAD_ONLY YES
        EXCLUDE_FROM_ALL YES
)
if (apultra_ADDED)
  add_library(apultra OBJECT
          ${apultra_SOURCE_DIR}/src/expand.c
          ${apultra_SOURCE_DIR}/src/matchfinder.c
          ${apultra_SOURCE_DIR}/src/shrink.c
          ${apultra_SOURCE_DIR}/src/libdivsufsort/lib/divsufsort.c
          ${apultra_SOURCE_DIR}/src/libdivsufsort/lib/divsufsort_utils.c
          ${apultra_SOURCE_DIR}/src/libdivsufsort/lib/sssort.c
          ${apultra_SOURCE_DIR}/src/libdivsufsort/lib/trsort.c)

  target_include_directories(apultra PUBLIC
          $<BUILD_INTERFACE:${apultra_SOURCE_DIR}/src>
          $<BUILD_INTERFACE:${apultra_SOURCE_DIR}/src/libdivsufsort/include>)

  set_target_properties(apultra PROPERTIES POSITION_INDEPENDENT_CODE ON)
  set_target_properties(apultra PROPERTIES LINKER_LANGUAGE C)
endif()
