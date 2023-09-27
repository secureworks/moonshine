#define LUA_CORE

#include "moutput.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lauxlib.h"

#ifdef MOON_USE_LUAJIT
#include "mcompat.h"
#endif

static const char *registry_output_cb_uuid = "c2642416-ff3e-47a4-84cd-64247949f30c";
static const char *registry_output_userdata_uuid = "ef58462f-f3da-446f-a833-28266ff24418";

/*LUA_API*/ void moon_output_cb(lua_State *L, output_callback callback, void *userdata)
{
  if (L != NULL && callback != NULL) {
    // set registry key
    lua_pushlightuserdata(L, (void *)&registry_output_cb_uuid);
    // push pointer
    lua_pushlightuserdata(L, (void *)callback);
    // push to registry
    lua_settable(L, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L, (void *)&registry_output_userdata_uuid);
    lua_pushlightuserdata(L, userdata);
    lua_settable(L, LUA_REGISTRYINDEX);
  }
}

void push_output(lua_State *L, const char *str)
{
  // set the registry key
  lua_pushlightuserdata(L, (void *)&registry_output_cb_uuid);
  // retrieve value
  lua_gettable(L, LUA_REGISTRYINDEX);
  // cast value
  output_callback func_output_cb = (output_callback)lua_touserdata(L, -1);
  if (func_output_cb != NULL) {
    lua_pushlightuserdata(L, (void *)&registry_output_userdata_uuid);
    lua_gettable(L, LUA_REGISTRYINDEX);
    void* userdata = lua_touserdata(L, -1);
    size_t len = strlen(str);
    func_output_cb(L, str, len, userdata);
  }
}

void push_raw_output(lua_State *L, const void *data, size_t len)
{
  lua_pushlightuserdata(L, (void *)&registry_output_cb_uuid);
  lua_gettable(L, LUA_REGISTRYINDEX);
  output_callback func_output_cb = (output_callback)lua_touserdata(L, -1);
  if (func_output_cb != NULL) {
    lua_pushlightuserdata(L, (void *)&registry_output_userdata_uuid);
    lua_gettable(L, LUA_REGISTRYINDEX);
    void* userdata = lua_touserdata(L, -1);
    func_output_cb(L, data, len, userdata);
  }
}

void push_char_output(lua_State *L, int i)
{
  lua_pushlightuserdata(L, (void *)&registry_output_cb_uuid);
  lua_gettable(L, LUA_REGISTRYINDEX);
  output_callback func_output_cb = (output_callback)lua_touserdata(L, -1);
  if (func_output_cb != NULL) {
    lua_pushlightuserdata(L, (void *)&registry_output_userdata_uuid);
    lua_gettable(L, LUA_REGISTRYINDEX);
    void* userdata = lua_touserdata(L, -1);
    char str = (char)i;
    func_output_cb(L, &str, 1, userdata);
  }
}

void push_printf_output(lua_State *L, char *fmt, ...)
{
  lua_pushlightuserdata(L, (void *)&registry_output_cb_uuid);
  lua_gettable(L, LUA_REGISTRYINDEX);
  output_callback func_output_cb = (output_callback)lua_touserdata(L, -1);
  if (func_output_cb != NULL) {
    lua_pushlightuserdata(L, (void *)&registry_output_userdata_uuid);
    lua_gettable(L, LUA_REGISTRYINDEX);
    void* userdata = lua_touserdata(L, -1);


    va_list va;
    va_start(va, fmt);
    char sbuf[1024];
    int size = vsnprintf(sbuf, sizeof(sbuf), fmt, va);
    va_end(va);

    if (size < sizeof(sbuf)) {
      func_output_cb(L, sbuf, size, userdata);
      return;
    }

    char *lbuf = (char*) malloc(size + 1);
    va_start(va, fmt);
    vsnprintf(lbuf, size + 1, fmt, va);
    va_end(va);
    func_output_cb(L, lbuf, 1, userdata);
    free(lbuf);
  }
}

int moon_print (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */

  if (n > 0) {
    char *output = NULL;
    size_t output_len;
    const char *s = luaL_tolstring(L, 1, &output_len);  /* convert it to string */
    output = (char*) malloc(output_len + 2); /* +1 for terminating NULL, +1 for newline */
    memcpy(output, s, output_len);
    output[output_len] = '\n';
    output[output_len + 1] = '\0';
    lua_pop(L, 1);  /* pop result */

    int i;
    size_t string_len;
    for (i = 2; i <= n; i++) {  /* for each argument */
      s = luaL_tolstring(L, i, &string_len);  /* convert it to string */
      output = (char*) realloc(output, output_len + string_len + 3); /* +1 for terminating NULL, +1 for tab, +1 for newline */
      output[output_len] = '\t';
      memcpy(output + output_len + 1, s, string_len);
      output_len = output_len + string_len + 1; /* +1 for tab */
      output[output_len] = '\n';
      output[output_len + 1] = '\0';
      lua_pop(L, 1);  /* pop result */
    }

    moon_writestringlength(L, output, output_len + 1); /* +1 for newline */
    free(output);
  }
  else {
    moon_writeline(L);
  }
  return 0;
}
