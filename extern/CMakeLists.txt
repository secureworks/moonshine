#option(LUA_BUILD_AS_CXX "Build lua as C++" OFF)
#set(LUA_BUILD_AS_CXX ON)
#add_compile_definitions(LUA_USE_LONGJMP)
add_subdirectory(moon EXCLUDE_FROM_ALL)
