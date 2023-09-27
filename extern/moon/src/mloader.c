#define LUA_CORE

#include "mloader.h"

#include "lua.h"
#include "lauxlib.h"
#include "maux.h"
#ifndef MOON_USE_LUAJIT
#include "llimits.h"
#else
#include "mcompat.h"
#define UNUSED(x)	((void)(x))
#endif

#include "mcompress.h"
#include "miniz.h"
#include "mutil.h"
#ifdef WIN32
#include "mwinapi.h"
#endif

#include "llwythwr.h"
#ifndef WIN32
#include "llwythwr/init.h"
#endif

#define ERRLIB		1
#define ERRFUNC		2

#define LUA_POF		"luaopen_"
#define LUA_OFSEP	"_"
#define LUA_IGMARK	"-"

static const char *const MOONCLIBS = "_MOONCLIBS";
static const char *const MOONLLIBS = "_MOONLLIBS";

#ifdef WIN32
static char *hooks[] = {"lua54.dll", "lua53.dll", "lua52.dll", "lua51.dll", NULL};
#endif

static llwythwr_os_api* os_api = NULL;


int moon_addmeml(lua_State *L, const char* name, const void* module, size_t size, int compressed)
{
  if (name == NULL || module == NULL || size < 1) {
    return MOON_ARGUMENT_ERROR;
  }

  if (get_package(L, MOONLLIBS, name) != NULL)
    return MOON_ALREADY_LOADED; /* library already exists */

  add_package(L, MOONLLIBS, name, module, size, compressed);

  return LUA_OK;
}

int moon_addmemc(lua_State *L, const char* name, const void* module, size_t size, int compressed)
{
  if (name == NULL || module == NULL || size < 1) {
    return MOON_ARGUMENT_ERROR;
  }

  if (get_package(L, MOONCLIBS, name) != NULL)
    return MOON_ALREADY_LOADED; /* library already exists */

  add_package(L, MOONCLIBS, name, module, size, compressed);

  return LUA_OK;
}


int moon_addmemz(lua_State *L, const void *raw, size_t size)
{
  mz_zip_archive *zip = (mz_zip_archive*) malloc(sizeof(mz_zip_archive));
  mz_zip_zero_struct(zip);
  if (!mz_zip_reader_init_mem(zip, raw, size, 0)) {
    free(zip);
    return MOON_DECOMPRESSION_ERROR;
  }

  mz_uint i = 0;
  for (i = 0; i < mz_zip_reader_get_num_files(zip); i++) {
    if (mz_zip_reader_is_file_a_directory(zip, i)) continue; /* skip */

    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(zip, i, &file_stat)) {
      mz_zip_reader_end(zip);
      free(zip);
      return MOON_DECOMPRESSION_ERROR;
    }

    char name[_MAX_FNAME];
    char ext[_MAX_EXT];
    char dir[_MAX_DIR];
    split_path(file_stat.m_filename, NULL, dir, name, ext);

    if (strcmp(ext, ".lua") != 0 && strcmp(ext, LIB_SUFFIX) != 0) continue; /* skip non-lua related files */
    if (strcmp(file_stat.m_filename, "init.lua") == 0) continue; /* skip [root]init.lua */

    size_t buf_size;
    void *buf = NULL;

    buf = mz_zip_reader_extract_file_to_heap(zip, file_stat.m_filename, &buf_size, 0);
    if (buf == NULL) {
      mz_zip_reader_end(zip);
      free(zip);
      return MOON_DECOMPRESSION_ERROR;
    }

    const char* modname = lua_pushfstring(L, "%s%s", dir, name);
    modname = luaL_gsub(L, modname, "/", ".");

    if (strcmp(ext, ".lua") == 0) {
      moon_addmeml(L, modname, buf, buf_size, 0);
    }
    else {
      moon_addmemc(L, modname, buf, buf_size, 0);
    }

    lua_pop(L, 2); /* Remove results from lua_pushfstring and gsub */
    mz_free(buf);
  }
  mz_zip_reader_end(zip);
  free(zip);
  return LUA_OK;
}

int moon_loader_lua(lua_State *L)
{
  const mem_package* p;
  const char *name = luaL_checkstring(L, 1);

  mem_package *package = get_package(L, MOONLLIBS, name);
  if (package == NULL) {
    const char* initname = lua_pushfstring(L, "%s.init.lua", name);
    lua_pop(L, 1);
    package = get_package(L, MOONLLIBS, initname);
    if (package == NULL) {
      lua_pushfstring(L, "no field package.moon.lua['%s']", name);
      return 1; /* library has not been added */
    }
  }

  if (package->loaded != 0) { /* library has already been loaded */
    lua_pushfstring(L, "library '%s' has already been loaded", name);
    return 1;
  }

  if (package->compressed == 0) {
    if (luaL_loadbufferx(L, (const char *) package->module, package->size, name, "t") != LUA_OK) {
      return luaL_error(L, "error loading module '%s' :\n\t%s", name, lua_tostring(L, -1));
    }
    package->loaded = 1;
  }
  else {
    void *decompressed = NULL;
    size_t decompressed_size = 0;
    if (decompress_memory(package->module, package->size, &decompressed, &decompressed_size)) {
      if (luaL_loadbufferx(L, (const char *) decompressed, decompressed_size, name, "t") != LUA_OK) {
        memzero(decompressed, decompressed_size);
        free(decompressed);
        return luaL_error(L, "error loading module '%s' :\n\t%s", name, lua_tostring(L, -1));
      }
      package->loaded = 1;
      memzero(decompressed, decompressed_size);
      free(decompressed);
    }
    else {
      return luaL_error(L, "failed to decompress module '%s'", name);
    }
  }

  //luaL_loadbufferx has aleady pushed the lua bytecode onto the stack
#ifndef MOON_USE_LUAJIT
  lua_pushliteral(L, ":moon_loader_lua:");
  return 2;
#else
  return 1;
#endif
}

int moon_loader_c(lua_State *L)
{
  const mem_package* p;
  const char *name = luaL_checkstring(L, 1);

  mem_package *package = get_package(L, MOONCLIBS, name);
  if (package == NULL) { /* library has not been added */
    lua_pushfstring(L, "no field package.moon.c['%s']", name);
    return 1; /* library has not been added */
  }

  if (package->loaded != 0) { /* library has already been loaded */
    lua_pushcfunction(L, (lua_CFunction)package->handle);
#ifndef MOON_USE_LUAJIT
    lua_pushliteral(L, ":moon_loader_c:");
    return 2;
#else
    return 1;
#endif
  }

  if (package->compressed == 0) {
    void* handle;
    if (memory_loadfunc(L, name, package->module, package->size, &handle, 0) != LUA_OK) {
      return luaL_error(L, "error loading module '%s' :\n\t%s", name, lua_tostring(L, -1));
    }
    package->loaded = 1;
    package->handle = handle;
  }
  else {
    void *decompressed = NULL;
    size_t decompressed_size = 0;
    if (decompress_memory(package->module, package->size, &decompressed, &decompressed_size)) {
      void* handle;
      int fail = memory_loadfunc(L, name, decompressed, decompressed_size, &handle, 0);
      memzero(decompressed, decompressed_size);
      free(decompressed);
      if (fail) return luaL_error(L, "error loading module '%s' :\n\t%s", name, lua_tostring(L, -1));
      package->loaded = 1;
      package->handle = handle;
    }
    else {
      return luaL_error(L, "failed to decompress module '%s'", name);
    }
  }

  //memory_loadfunc has already pushed the c function onto the stack
#ifndef MOON_USE_LUAJIT
  lua_pushliteral(L, ":moon_loader_c:");
  return 2;
#else
  return 1;
#endif
}

void* moon_loaded(lua_State *L, const char *name, const char *suffix)
{
  if (strcmp(suffix, LIB_SUFFIX) == 0) {
    mem_package* package = get_package(L, MOONCLIBS, name);
    if (package != NULL) return package->handle;
  }
  return NULL;
}

static int memory_loadfunc(lua_State *L, const char *name, const void *module, size_t size, void** handle, int r)
{
  void *reg = moon_loaded(L, name, LIB_SUFFIX);
  if (reg == NULL) {
    reg = memory_load(L, name, module, size, (*name == '*'));
  }
  *handle = reg;

  if (reg == NULL) {
    return ERRLIB;  /* Unable to load library. */
  }

  // TODO, add as an option
  //wchar_t* w_name = StringToWideString(name, (int)strlen(name));
  //LinkModuleToPEB(reg, w_name, w_name);

  // =====

  if (*name == '*') {  /* Only load library into global namespace. */
    lua_pushboolean(L, 1);
    return LUA_OK;
  }
  else {
    /*
    ** Try to find a load function for module 'modname' at file 'filename'.
    ** First, change '.' to '_' in 'modname'; then, if 'modname' has
    ** the form X-Y (that is, it has an "ignore mark"), build a function
    ** name "luaopen_X" and look for it. (For compatibility, if that
    ** fails, it also tries "luaopen_Y".) If there is no ignore mark,
    ** look for a function named "luaopen_modname".
    */
    const char *openfunc;
    const char *mark;
    name = luaL_gsub(L, name, ".", LUA_OFSEP);
    mark = strchr(name, *LUA_IGMARK);
    if (mark) {
      lua_pushlstring(L, (mark - name) > 0 ? name : "", mark - name);
      openfunc = lua_tostring(L, -1);
      openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
      lua_CFunction f = memory_sym(L, reg, openfunc);
      if (f) {
        lua_pushcfunction(L, f);
        return LUA_OK;
      }
      name = mark + 1;  /* else go ahead and try old-style name */
    }
    openfunc = lua_pushfstring(L, LUA_POF"%s", name);
    lua_CFunction f = memory_sym(L, reg, openfunc);
    if (f) {
      lua_pushcfunction(L, f);
      return LUA_OK;
    }
    memory_unloadlib(reg, size);
    return ERRFUNC;  /* Unable to find function. */
  }
}

#if defined(LUA_USE_MACOSX)

static void *memory_load(lua_State *L, const char *name, const void * module, size_t size, int gl)
{
  if (os_api == NULL) {
    os_api = (llwythwr_os_api*) malloc(sizeof(llwythwr_os_api));
    llwythwr_init(os_api);
  }

  void* lib = llwythwr_load_library_from_memory(os_api, name, module, size, gl);
  if (lib == NULL) lua_pushfstring(L, "failed to load module '%s'", name);
  return lib;
}

static void memory_unloadlib(void *lib, size_t size)
{
  llwythwr_unload_library_from_memory(lib);
}

static lua_CFunction memory_sym(lua_State *L, void *lib, const char *sym)
{
  lua_CFunction f = (lua_CFunction) llwythwr_get_symbol_address_memory(lib, sym);
  if (f == NULL) lua_pushfstring(L, "failed to find function with name '%s'", sym);
  return f;
}

#elif defined(LUA_USE_LINUX)

static void *memory_load(lua_State *L, const char *name, const void * module, size_t size, int gl)
{
  if (os_api == NULL) {
    os_api = (llwythwr_os_api*) malloc(sizeof(llwythwr_os_api));
    llwythwr_init(os_api);
  }

  void* lib = llwythwr_load_library_from_memory(os_api, name, module, size, gl);
  if (lib == NULL) lua_pushfstring(L, "failed to load module '%s'", name);
  return lib;
}

static void memory_unloadlib(void *lib, size_t size)
{
  llwythwr_unload_library_from_memory(lib);
}

static lua_CFunction memory_sym(lua_State *L, void *lib, const char *sym)
{
  lua_CFunction f = (lua_CFunction) llwythwr_get_symbol_address_memory(lib, sym);
  if (f == NULL) lua_pushfstring(L, "failed to find function with name '%s'", sym);
  return f;
}

#elif defined(WIN32)

/*
 * Function called to load a dependant module during IAT processing
 */
void* x_get_lib_address_function(const char* name, void* user_data) {
  lua_State* L = (lua_State*) user_data;
  void* dll = NULL;

  int h = 0;
  while(hooks[h]) {
    if(strcmp(hooks[h], name) == 0) {
      dll = get_base_address();
      break;
    }
    h++;
  }

  if (dll == NULL)
    dll = moon_loaded(L, name, LIB_SUFFIX);

  if (dll == NULL)
    dll = moon_LoadLibraryA(name);

  return dll;
}

/*
 * Function called to resolve the address of a function during IAT processing
 */
void* x_get_proc_address_function(const char* module_name, void* module_handle, const char* func_name, int func_ordinal, void* user_data) {
  lua_State* L = (lua_State*) user_data;

  if (moon_loaded(L, module_name, LIB_SUFFIX) != NULL)
    return llwythwr_get_export_address_by_name(module_handle, func_name);

  if (func_name != NULL)
    return moon_GetProcAddress((HMODULE) module_handle, func_name);
  else
    return moon_GetProcAddress((HMODULE) module_handle, (LPCSTR) func_ordinal);
}

/*
 * Function called to unload a dependant module when unloading the module
 */
void x_free_library_function(const char* name, void* module, void* user_data) {
  lua_State* L = (lua_State*) user_data;

  int h = 0;
  while(hooks[h]) {
    if(strcmp(hooks[h], name) == 0) {
      return;
    }
    h++;
  }

  if (moon_loaded(L, name, LIB_SUFFIX) != NULL) {
    llwythwr_unload_library(module);
    return;
  }

  moon_FreeLibrary((HMODULE)module);
}

int llwythwr_init_custom(llwythwr_os_api* api)
{
  moon_init_win_api();
  api->VirtualAlloc = win_api->VirtualAlloc;
  api->VirtualFree = win_api->VirtualFree;
  api->VirtualProtect = win_api->VirtualProtect;
  api->CloseHandle = win_api->CloseHandle;
  api->GetLastError = win_api->GetLastError;
  api->SetLastError = win_api->SetLastError;
  api->LocalAlloc = win_api->LocalAlloc;
  api->LocalFree = win_api->LocalFree;
  api->LocalReAlloc = win_api->LocalReAlloc;
  api->RtlAddFunctionTable = win_api->RtlAddFunctionTable;
  api->RtlHashUnicodeString = win_api->RtlHashUnicodeString;
  api->RtlRbInsertNodeEx = win_api->RtlRbInsertNodeEx;
  api->RtlRbRemoveNode = win_api->RtlRbRemoveNode;
  api->MultiByteToWideChar = win_api->MultiByteToWideChar;
  api->NtOpenFile = win_api->NtOpenFile;
  api->NtCreateSection = win_api->NtCreateSection;
  api->NtMapViewOfSection = win_api->NtMapViewOfSection;
  api->NtUnmapViewOfSection = win_api->NtUnmapViewOfSection;
  api->NtQuerySystemTime = win_api->NtQuerySystemTime;
  return 0;
}

static void *memory_load(lua_State *L, const char *name, const void * module, size_t size, int gl)
{
  if (os_api == NULL) {
    os_api = (llwythwr_os_api*) malloc(sizeof(llwythwr_os_api));
    llwythwr_init_custom(os_api);
  }

  void* lib = llwythwr_alloc_library(os_api,
                                     module,
                                     x_get_lib_address_function,
                                     x_get_proc_address_function,
                                     x_free_library_function,
                                     L);
  if (lib != NULL)
    llwythwr_call_entry_point(lib);
  else
    lua_pushfstring(L, "failed to load module '%s'", name);

  return lib;
}

static lua_CFunction memory_sym(lua_State *L, void *lib, const char *sym)
{
  lua_CFunction f = (lua_CFunction) llwythwr_get_export_address_by_name(lib, sym);
  if (f == NULL) lua_pushfstring(L, "failed to find function with name '%s'", sym);
  return f;
}

static void memory_unloadlib(void *lib, size_t size)
{
  llwythwr_unload_library(lib);
}

#else

#define DLMSG "memory loader not enabled; no support for target OS"

static void *memory_load(lua_State *L, const char *name, const void * module, size_t size, int gl)
{
  UNUSED(name); UNUSED(module); UNUSED(size); UNUSED(gl);
  lua_pushliteral(L, DLMSG);
  return NULL;
}

static void memory_unloadlib(void *lib)
{
  UNUSED(lib);
}

static lua_CFunction memory_load(lua_State *L, void *lib, const char *sym)
{
  UNUSED(lib); UNUSED(sym);
  lua_pushliteral(L, DLMSG);
  return NULL;
}

#endif

static int lua_loaded(lua_State* L) {
  lua_getfield(L, LUA_REGISTRYINDEX, MOONLLIBS);
  lua_Integer l_len = luaL_len(L, 1);
  mem_package **l_packages = (mem_package**) malloc(l_len * sizeof(mem_package*));

  for (int i = 1; i <= l_len; i++) {  /* for each package */
    lua_rawgeti(L, 1, i);  /* get package MOONLIBS[n] */
    l_packages[i - 1] = (mem_package*) lua_touserdata(L, -1);
    lua_pop(L, 1);  /* pop package */
  }
  lua_pop(L, 1);  /* pop table */

  lua_getfield(L, LUA_REGISTRYINDEX, MOONCLIBS);
  lua_Integer c_len = luaL_len(L, 1);
  mem_package **c_packages = (mem_package**) malloc(c_len * sizeof(mem_package*));

  for (int i = 1; i <= c_len; i++) {  /* for each package, in reverse order */
    lua_rawgeti(L, 1, i);  /* get package MOONLIBS[n] */
    c_packages[i - 1] = (mem_package*) lua_touserdata(L, -1);
    lua_pop(L, 1);  /* pop package */
  }
  lua_pop(L, 1);  /* pop table */


  lua_newtable(L);

  for (int i = 0; i < l_len; i++) {
    if (l_packages[i]->loaded != 0) {
      lua_pushnumber(L, i);
      lua_pushfstring(L, "%s%s", l_packages[i]->name, LIB_SUFFIX);
      lua_settable(L, -3);
    }
  }

  free(l_packages);

  for (int i = 0; i < c_len; i++) {
    if (c_packages[i]->loaded != 0) {
      lua_pushnumber(L, i);
      lua_pushfstring(L, "%s%s", c_packages[i]->name, LIB_SUFFIX);
      lua_settable(L, -3);
    }
  }

  free(c_packages);
  return 1;
}

static int lua_preloaded(lua_State* L)
{
  lua_getfield(L, LUA_REGISTRYINDEX, MOONLLIBS);
  lua_Integer l_len = luaL_len(L, -1);
  mem_package **l_packages = (mem_package**) malloc(l_len * sizeof(mem_package*));

  for (int i = 1; i <= l_len; i++) {  /* for each package */
    lua_rawgeti(L, 1, i);  /* get package MOONLIBS[n] */
    l_packages[i - 1] = (mem_package*) lua_touserdata(L, -1);
    lua_pop(L, 1);  /* pop package */
  }
  lua_pop(L, 1);  /* pop table */

  lua_getfield(L, LUA_REGISTRYINDEX, MOONCLIBS);
  lua_Integer c_len = luaL_len(L, -1);
  mem_package **c_packages = (mem_package**) malloc(c_len * sizeof(mem_package*));

  for (int i = 1; i <= c_len; i++) {  /* for each package, in reverse order */
    lua_rawgeti(L, 1, i);  /* get package MOONLIBS[n] */
    c_packages[i - 1] = (mem_package*) lua_touserdata(L, -1);
    lua_pop(L, 1);  /* pop package */
  }
  lua_pop(L, 1);  /* pop table */


  lua_newtable(L);
  int index = 1;

  for (int i = 0; i < l_len; i++) {
    lua_pushnumber(L, index++);
    lua_pushfstring(L, "%s%s", l_packages[i]->name, ".lua");
    lua_settable(L, -3);
  }

  free(l_packages);

  for (int i = 0; i < c_len; i++) {
    lua_pushnumber(L, index++);
    lua_pushfstring(L, "%s%s", c_packages[i]->name, LIB_SUFFIX);
    lua_settable(L, -3);
  }

  free(c_packages);
  return 1;
}

static int lua_preload(lua_State* L)
{
  const size_t type = luaL_checkinteger(L, 1);
  const char* name = luaL_checkstring(L, 2);
  size_t length;
  const char* module = luaL_checklstring(L, 3, &length);
  const int compressed = lua_toboolean(L, 4);

  int ret = 1;
  if (type == 0) {
    ret = moon_addmeml(L, name, module, length, compressed);
  }
  else if (type == 1) {
    ret = moon_addmemc(L, name, module, length, compressed);
  }
  else if (type == 2) {
    ret = moon_addmemz(L, module, length);
  }
  else {
    return luaL_error(L, "invalid module type provided, accepted values are 0, 1, or 2");
  }

  if (ret == LUA_OK) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else if (ret == MOON_ALREADY_LOADED) {
    lua_pushboolean(L, 0);
    lua_pushstring(L, "module already preloaded");
    return 2;
  }
  else {
    lua_pushboolean(L, 0);
    lua_pushstring(L, "could not preload module, unknown error");
    return 2;
  }
}

int moon_loadlib(lua_State *L, const char *name, const void *module, size_t size, int compressed, const char* sym, void** func)
{
  void *reg = NULL;
  *func = NULL;
  reg = moon_loaded(L, name, LIB_SUFFIX);
  if (reg == NULL) {
    if (compressed == 0) {
      reg = memory_load(L, name, module, size, (*name == '*'));
    }
    else {
      void *decompressed = NULL;
      size_t decompressed_size = 0;

      if (decompress_memory(module, size, &decompressed, &decompressed_size)) {
        reg = memory_load(L, name, decompressed, decompressed_size, (*name == '*'));
        memzero(decompressed, decompressed_size);
        free(decompressed);
      }
      else {
        lua_pushfstring(L, "failed to decompress module '%s'", name);
      }
    }
  }

  if (reg == NULL) { return ERRLIB; } /* Unable to load library. */

  if (*name != '*' && sym != NULL && strlen(sym) != 0) {
    *func = memory_sym(L, reg, sym);
    if (*func == NULL) {
      memory_unloadlib(reg, size);
      return ERRFUNC;  /* Unable to find function. */
    }
  }

  mem_package* package = add_package(L, MOONCLIBS, name, module, size, compressed);
  package->loaded = 1;
  package->handle = reg;

  return LUA_OK;
}

static int lua_loadlibm(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  size_t size;
  const char* module = luaL_checklstring(L, 2, &size);
  const int compressed = lua_toboolean(L, 3);
  const char* sym = luaL_optstring(L, 4, NULL);

  void* func;
  int st = moon_loadlib(L, name, module, size, compressed, sym, &func);
  if (st != LUA_OK) {
    /* error; error message is on stack top */
    lua_pushnil(L);
    lua_insert(L, -2);
    lua_pushstring(L, (st == ERRLIB) ?  "open" : "init");
    return 3;  /* return nil, error message, and where */
  }
  /* no errors */
  if (sym == NULL || strlen(sym) == 0) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushcfunction(L, (lua_CFunction) func);
    return 1;
  }
}

static int lua_unrequire(lua_State* L)
{
  const char* module = luaL_checkstring(L, 1);

  mem_package *package = get_package(L, MOONLLIBS, module);
  if (package == NULL)
    package = get_package(L, MOONCLIBS, module);

  if (package != NULL) {
    /* package.loaded['module'] = nil */
    lua_pushnil(L);
    lua_getglobal(L, "package");
    //lua_getfield(L,LUA_ENVIRONINDEX,"package");
    lua_pushliteral(L, "loaded");
    lua_gettable(L, -2);
    lua_remove(L, -2);
    lua_insert(L, -2);
    lua_pushstring(L, module);
    lua_insert(L, -2);
    lua_settable(L, -3);
    lua_pop(L, 1);

    /* _G['module'] = nil */
    lua_pushnil(L);
    lua_getglobal(L, "_G");
    //lua_getfield(L,LUA_ENVIRONINDEX,"_G");
    lua_insert(L, -2);
    lua_pushstring(L, module);
    lua_insert(L, -2);
    lua_settable(L, -3);
    lua_pop(L, 1);

    if (package->handle) {
      memory_unloadlib(package->handle, package->size);
      package->handle = NULL;
    }
    package->loaded = 0;
    return 0;
  }

  return luaL_error(L, "module '%s' is not loaded", module);
}

/*
** return registry.MOONLIBS[name]
*/
static mem_package *get_package(lua_State *L, const char *table, const char *name) {
  lua_getfield(L, LUA_REGISTRYINDEX, table);
  lua_getfield(L, -1, name);
  mem_package *package = (mem_package*) lua_touserdata(L, -1);  /* package = table[name] */
  lua_pop(L, 2);  /* pop table and 'package' */
  return package;
}

/*
** registry.MOONLIBS[name] = package           -- for queries
** registry.MOONLIBS[#MOONLIBS + 1] = package  -- also keep a list of all libraries
*/
static mem_package* add_package(lua_State *L, const char *table, const char *name, const void * module, size_t size, int compressed) {
  mem_package *package = (mem_package*) malloc(sizeof(mem_package));
  package->name = (char*) malloc(strlen(name) + 1);
  snprintf(package->name, strlen(name) + 1, "%s", name); // strncpy() is broken, https://randomascii.wordpress.com/2013/04/03/stop-using-strncpy-already/
  package->module = malloc(sizeof(unsigned char *) * size);
  memcpy(package->module, module, size);
  package->size = size;
  package->compressed = compressed;
  package->loaded = 0;
  package->handle = NULL;

  lua_getfield(L, LUA_REGISTRYINDEX, table);
  lua_pushlightuserdata(L, package);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, name);  /* table[name] = plib */
  lua_rawseti(L, -2, luaL_len(L, -2) + 1);  /* table[#table + 1] = plib */
  lua_pop(L, 1);  /* pop table */

  return package;
}

static void free_package(mem_package * package)
{
  if (package->handle != NULL) {
    memory_unloadlib(package->handle, package->size);
    package->handle = NULL;
  }

  memzero(package->name, strlen(package->name));
  free(package->name);
  package->name = NULL;

  memzero(package->module, package->size);
  free(package->module);
  package->module = NULL;

  free(package);
  package = NULL;
}

static int gc_libs(lua_State *L) {
  lua_Integer n = luaL_len(L, 1);
  for (; n >= 1; n--) {  /* for each handle, in reverse order */
    lua_rawgeti(L, 1, n);  /* get handle MOONLIBS[n] */
    mem_package *package = (mem_package*) lua_touserdata(L, -1);
    free_package(package);
    lua_pop(L, 1);  /* pop handle */
  }
  return 0;
}

static const luaL_Reg memloader_global[] = {
    // Returns a lists of all modules loaded into memory
    {"loaded", lua_loaded },
    // Returns a lists of all available modules
    {"preloaded", lua_preloaded },
    // Preload a module
    {"preload", lua_preload },
    // 'unrequire' a module
    {"unrequire", lua_unrequire },
    // Replicate loadlib, but for loading from memory
    {"loadlibm", lua_loadlibm },
    { NULL, NULL }
};

int luaopen_mloader(lua_State *L)
{
  /*
  ** create table MOONLLIBS to keep track of all Lua libraries,
  ** setting a finalizer to clean up all libraries when closing state.
  */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, MOONLLIBS);  /* create MOONLLIBS table */
  lua_createtable(L, 0, 1);  /* create metatable for MOONLLIBS */
  lua_pushcfunction(L, gc_libs);
  lua_setfield(L, -2, "__gc");  /* set finalizer for MOONLLIBS table */
  lua_setmetatable(L, -2);

  /*
  ** create table MOONCLIBS to keep track of all C libraries,
  ** setting a finalizer to clean up all libraries when closing state.
  */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, MOONCLIBS);  /* create MOONCLIBS table */
  lua_createtable(L, 0, 1);  /* create metatable for MOONCLIBS */
  lua_pushcfunction(L, gc_libs);
  lua_setfield(L, -2, "__gc");  /* set finalizer for MOONCLIBS table */
  lua_setmetatable(L, -2);

  lua_pushglobaltable(L);
  luaL_setfuncs(L, memloader_global, 0);
  return 1;
}
