#include "lua.h"
#include "lauxlib.h"

#define MESSAGE "hello from inside a module!"

int say_hello(lua_State *L) {
  lua_pushstring(L, MESSAGE);
  return 1;
}

int check_string(lua_State *L) {
  luaL_checkstring(L, 1);
  return 0;
}

#if defined(_WIN32)
int __declspec(dllexport) luaopen_hello(lua_State *L) {
#else
int luaopen_hello(lua_State *L) {
#endif
  lua_newtable(L);
  lua_pushcfunction (L, say_hello);
  lua_setfield (L, -2, "say_hello");
  lua_pushcfunction (L, check_string);
  lua_setfield (L, -2, "check_string");
  return 1;
}
