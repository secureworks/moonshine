#include "mzloader.h"

#include "mloader.h"

/*
static const char *registry_ziploader_uuid = "45e834bd-6bc2-412a-90df-218f5bd604b4";

int zip_loader(lua_State *L)
{
  const zip_package* p;
  const char *name = luaL_checkstring(L, 1);

  lua_pushlightuserdata(L, (void *)&registry_ziploader_uuid);
  lua_gettable(L, LUA_REGISTRYINDEX);

  zip_package *package = (zip_package*) lua_touserdata(L, -1);

  if (package == NULL) return 1;  // no zip has been added

  for (; package; package = package->next) {
    int file_index;
    do {
      // Find entry matching name in zip...
      // First search for files with .lua extensions
      const char* luaname = lua_pushfstring(L, "%s.lua", name);
      file_index = mz_zip_reader_locate_file(package->zip, luaname, NULL, 0);
      lua_pop(L, 1);  // remove 'lua_pushfstring' result
      if (file_index >= 0 && mz_zip_reader_is_file_a_directory(package->zip, file_index) != MZ_TRUE)
        break;

      // Then for files with a .dll/.so extension
      const char* cname = lua_pushfstring(L, "%s%s", name, LIB_SUFFIX);
      file_index = mz_zip_reader_locate_file(package->zip, cname, NULL, 0);
      lua_pop(L, 1);  // remove 'lua_pushfstring' result
      if (file_index >= 0 && mz_zip_reader_is_file_a_directory(package->zip, file_index) != MZ_TRUE)
        break;

      // Finally, if the name contains a '.' then also check inside paths by substituting '.' with '/'
      const char* luapname = luaL_gsub(L, name, ".", "_");
      luapname = lua_pushfstring(L, "%s.lua", luapname);
      file_index = mz_zip_reader_locate_file(package->zip, luapname, NULL, 0);
      lua_pop(L, 1);
      lua_pop(L, 1);
      if (file_index >= 0 && mz_zip_reader_is_file_a_directory(package->zip, file_index) != MZ_TRUE)
        break;

      const char* cpname = luaL_gsub(L, name, ".", "/");
      cpname = lua_pushfstring(L, "%s%s", cpname, LIB_SUFFIX);
      file_index = mz_zip_reader_locate_file(package->zip, cpname, NULL, 0);
      lua_pop(L, 1);
      lua_pop(L, 1);
    } while(0);

    // if found, extract entry and use memloader to inject it into memory
    if (file_index >= 0 && mz_zip_reader_is_file_a_directory(package->zip, file_index) != MZ_TRUE) {
      unsigned char *buf = NULL;
      size_t size = 0;
      buf = mz_zip_reader_extract_to_heap(package->zip, file_index, &size, 0);
      if (buf != NULL && size > 0) {
        lua_CFunction f;
        int st = luaL_loadlib(L, name, buf, size, 0, mksymname(L, name, SYMPREFIX_CF), (void **) &f);
        free(buf);
        if (st == LUA_OK)
          lua_pushcfunction(L, f);
        else {
          lua_pushstring(L, (st == PACKAGE_ERR_LIB) ?  PACKAGE_LIB_FAIL : "init");
          return luaL_error(L, "error %s module " LUA_QS " :\n\t%s", name, lua_tostring(L, -1), lua_tostring(L, -2));
        }
      }
      //else {
      //  return luaL_error(L, "failed to decompress zip " LUA_QS, package->name);
      //}
      break;
    }
  }
  return 1;
}

void zip_loader_destroy(lua_State *L)
{
  lua_pushlightuserdata(L, (void *)&registry_ziploader_uuid);
  lua_gettable(L, LUA_REGISTRYINDEX);
  zip_package *packages = (zip_package*) lua_touserdata(L, -1);
  if (packages != NULL) {
    zip_package_free(&packages->refcount);
    lua_pushlightuserdata(L, (void*) &registry_ziploader_uuid);
    lua_pushlightuserdata(L, NULL);
    lua_settable(L, LUA_REGISTRYINDEX);
  }
}

int luaL_addzip(lua_State *L, const char *name, const void *raw, size_t size)
{
  if (name == NULL || raw == NULL || size < 1) {
    return MOON_ARGUMENT_ERROR;
  }

  global_State *g = G(L);
  L = mainthread(g);

  lua_pushlightuserdata(L, (void*) &registry_ziploader_uuid);
  lua_gettable(L, LUA_REGISTRYINDEX);
  zip_package *packages = (zip_package*) lua_touserdata(L, -1);

  if (zip_package_find(packages, name) != NULL) return MOON_ALREADY_LOADED; // zip already exists

  if (zip_package_push(&packages, name, raw, size) == NULL) return MOON_LOADING_ERROR;

  lua_pushlightuserdata(L, (void*) &registry_ziploader_uuid);
  lua_pushlightuserdata(L, packages);
  lua_settable(L, LUA_REGISTRYINDEX);

  return LUA_OK;
}

int luaopen_ziploader(lua_State *L)
{
  return 1;
}

static zip_package *zip_package_find(zip_package *package, const char *name)
{
  for (; package; package = package->next)
    if (strcmp(package->name, name) == 0) return package;
  return NULL;
}

static void zip_package_free(const struct ref *ref)
{
  zip_package *node = container_of(ref, zip_package, refcount);
  zip_package *child = node->next;
  mz_zip_reader_end(node->zip);
  free(node->name);
  free(node);
  if (child)
    ref_dec(&child->refcount);
}

static zip_package *zip_package_create(const char *name, const void * raw, size_t size)
{
  zip_package *node = (zip_package*) malloc(sizeof(*node));
  node->name = (char*) malloc(strlen(name) + 1);
  snprintf(node->name, strlen(name) + 1, "%s", name); // strncpy() is broken, https://randomascii.wordpress.com/2013/04/03/stop-using-strncpy-already/
  mz_zip_zero_struct(node->zip);
  if (!mz_zip_reader_init_mem(node->zip, raw, size, 0))
    return NULL;
  node->next = NULL;
  node->refcount = (struct ref){zip_package_free, 1};
  return node;
}

static zip_package *zip_package_push(zip_package **nodes, const char *name, const void * raw, size_t size)
{
  zip_package *node = zip_package_create(name, raw, size);
  if (node != NULL) {
    node->next = *nodes;
    *nodes = node;
  }
  return node;
}

static struct zip_package * zip_package_pop(zip_package **nodes)
{
  zip_package *node = *nodes;
  *nodes = (*nodes)->next;
  if (*nodes)
    ref_inc(&(*nodes)->refcount);
  return node;
}
*/
