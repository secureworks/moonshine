include(initialize_submodule)
include(patch_project)

if(MOON_USE_LUAJIT)
  initialize_submodule(luajit)
  patch_project(luajit luajit.patch)
else()
  initialize_submodule(lua)
  patch_project(lua lua.patch)
endif()

add_subdirectory(llwythwr EXCLUDE_FROM_ALL)
add_subdirectory(deinamig EXCLUDE_FROM_ALL)
