#ifndef MOON_LOADER_H_
#define MOON_LOADER_H_

#include <stdlib.h>
#include <stdio.h>

#include "lua.h"

typedef struct mem_package {
  char *name;
  void *module;
  size_t size;
  int compressed;
  int loaded;
  void* handle;
} mem_package;

#define LUA_MLOADERLIBNAME	"mloader"
int luaopen_mloader(lua_State *L);
int moon_loader_lua(lua_State *L);
int moon_loader_c(lua_State *L);

int moon_addmeml(lua_State *L, const char *name, const void *module, size_t size, int compressed);
int moon_addmemc(lua_State *L, const char *name, const void *module, size_t size, int compressed);
int moon_addmemz(lua_State *L, const void *zip, size_t size);
int moon_loadlib(lua_State *L, const char *name, const void *module, size_t size, int compressed, const char* sym, void** func);
void* moon_loaded(lua_State *L, const char *name, const char *suffix);

static int memory_loadfunc(lua_State *L, const char *name, const void *module, size_t size, void** handle, int r);
static void* memory_load(lua_State *L, const char *name, const void *module, size_t size, int gl);
static void memory_unloadlib(void *lib, size_t size);
static lua_CFunction memory_sym(lua_State *L, void *lib, const char *sym);

//mem_package *clone_packages(lua_State *L, const char* uuid);

static mem_package *get_package(lua_State *L, const char *table, const char *name);
static mem_package *add_package(lua_State *L, const char *table, const char *name, const void * module, size_t size, int compressed);
static void free_package(mem_package * package);

/* luaL_addmodule status; 0 is OK */
#define MOON_ARGUMENT_ERROR       1
#define MOON_ALREADY_LOADED       2
#define MOON_LOADING_ERROR        3
#define MOON_DECOMPRESSION_ERROR  4

#define MODULE_LUA            0
#define MODULE_C              1
#define MODULE_ZIP            2

#endif //MOON_LOADER_H_
