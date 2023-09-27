#ifndef MOON_LUA_OBJECT_HPP_
#define MOON_LUA_OBJECT_HPP_

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <string>

struct lua_object {
  virtual ~lua_object() = default;

  virtual int luaopen_object(lua_State *L) = 0;
};

template<class T>
using member_func = int (T::*)(lua_State *L);

template<class T, member_func<T> func>
int dispatch_to_member(lua_State *L) {
  T *ptr = (T *) lua_topointer(L, lua_upvalueindex(1));
  return ((*ptr).*func)(L);
}

template<class T, member_func<T> Y>
inline void lua_register_member(lua_State *L, const std::string &func_name, void *object) {
  lua_pushlightuserdata(L, object);
  lua_pushcclosure(L, dispatch_to_member<T, Y>, 1);
  lua_setglobal(L, func_name.c_str());
}

inline void register_lua_object(lua_State *L, const std::string &name, lua_object *obj_ptr) {
  // Register luaopen compatible objects
  lua_getglobal(L, "package"); // Get the package global table.
  lua_getfield(L, -1, "preload"); // Get the list of preloaded modules in the package table.
  lua_pushlightuserdata(L, obj_ptr);
  lua_pushcclosure(L, &dispatch_to_member<lua_object, &lua_object::luaopen_object>, 1);
  lua_setfield(L, -2, name.c_str());
  lua_pop(L, 2);  // Remove the preloaded module and the package tables from the stack
}

#endif //MOON_LUA_OBJECT_HPP_
