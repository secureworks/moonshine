#ifndef MOON_ZIPLOADER_H_
#define MOON_ZIPLOADER_H_
/*
#include <stdlib.h>
#include <stdio.h>

#include <miniz.h>

#include "lua.h"
#include "ref.h"

typedef struct zip_package {
  char *name;
  mz_zip_archive *zip;
  struct zip_package *next;
  struct ref refcount;
} zip_package;

#define LUA_ZIPLOADERLIBNAME	"ziploader"
int luaopen_ziploader(lua_State *L);

int zip_loader(lua_State *L);
void zip_loader_destroy(lua_State *L);

int luaL_addzip(lua_State *L, const char *name, const void *raw, size_t size);

static zip_package *zip_package_find(zip_package *package, const char *name);
static void zip_package_free(const struct ref *ref);
static zip_package *zip_package_create(const char *name, const void * raw, size_t size);
static zip_package *zip_package_push(zip_package **nodes, const char *name, const void * raw, size_t size);
static struct zip_package * zip_package_pop(zip_package **nodes);
*/
#endif //MOON_ZIPLOADER_H_
