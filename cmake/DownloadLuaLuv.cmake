include(CPM)

CPMAddPackage(
        NAME luv
        GITHUB_REPOSITORY luvit/luv
        GIT_TAG e2fbfba499f9481ebef6a8510b526b183233fd63 # 1.43.0-0
        DOWNLOAD_ONLY YES
        EXCLUDE_FROM_ALL YES
)
if (luv_ADDED)
  add_library(luv INTERFACE)
  target_sources(luv INTERFACE ${luv_SOURCE_DIR}/src/luv.c)
  set_target_properties(luv PROPERTIES POSITION_INDEPENDENT_CODE ON)

  if(WIN32)
    target_compile_definitions(luv INTERFACE -DLUA_BUILD_AS_DLL)
    target_compile_definitions(luv INTERFACE -DLUA_LIB)
  endif()

  add_subdirectory(${luv_SOURCE_DIR}/deps/libuv EXCLUDE_FROM_ALL)
  target_link_libraries(luv INTERFACE uv_a) # link in static libuv library
  target_compile_definitions(uv_a PRIVATE -DBUILDING_UV_SHARED) # set up dll export decorators
  set_target_properties(uv_a PROPERTIES POSITION_INDEPENDENT_CODE ON)

  target_include_directories(luv INTERFACE $<BUILD_INTERFACE:${luv_SOURCE_DIR}/src>)
  target_include_directories(luv INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/libuv/include>)
  target_include_directories(luv INTERFACE ${luv_SOURCE_DIR}/deps/lua-compat-5.3/c-api)
endif()
