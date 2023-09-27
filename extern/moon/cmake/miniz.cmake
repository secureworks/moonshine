include(cpm)

CPMAddPackage(
        NAME miniz
        GITHUB_REPOSITORY richgel999/miniz
        GIT_TAG 5a38f5564a092462817b97014f4477c8f0783f8f # pre 3.0.0
        OPTIONS "MINIZ_STANDALONE_PROJECT OFF" "BUILD_HEADER_ONLY OFF" "BUILD_SHARED_LIBS OFF"
        EXCLUDE_FROM_ALL YES
        DOWNLOAD_ONLY YES
)
if (miniz_ADDED)

  include(GenerateExportHeader)
  add_library(miniz STATIC
          ${miniz_SOURCE_DIR}/miniz.c
          ${miniz_SOURCE_DIR}/miniz_zip.c
          ${miniz_SOURCE_DIR}/miniz_tinfl.c
          ${miniz_SOURCE_DIR}/miniz_tdef.c)
  generate_export_header(miniz)

  string(TOUPPER miniz TARGET_UPPER)
  set_target_properties(miniz PROPERTIES INTERFACE_COMPILE_DEFINITIONS ${TARGET_UPPER}_STATIC_DEFINE)

  target_include_directories(miniz PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
          $<BUILD_INTERFACE:${miniz_SOURCE_DIR}>
          $<INSTALL_INTERFACE:include>
          )

  target_compile_definitions(miniz PUBLIC MINIZ_NO_STDIO MINIZ_NO_TIME MINIZ_NO_ZLIB_APISs) # MINIZ_NO_DEFLATE_APIS)
endif()
