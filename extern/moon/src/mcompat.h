#ifndef MOON_SRC_MCOMPAT_H_
#define MOON_SRC_MCOMPAT_H_

#include <lua.h>

#ifdef MOON_USE_LUAJIT

lua_Integer luaL_len (lua_State *L, int i);
void lua_len (lua_State *L, int i);
int luaL_getsubtable (lua_State *L, int i, const char *name);
int lua_absindex (lua_State *L, int i);
const char *luaL_tolstring (lua_State *L, int idx, size_t *len);

#if !defined(lua_pushglobaltable)
#define lua_pushglobaltable(L) \
      lua_pushvalue((L), LUA_GLOBALSINDEX)
#endif

#if !defined(lua_writestring)
#define lua_writestring(s,l) \
      fwrite((s), sizeof(char), (l), stdout)
#endif

/* print a newline and flush the output */
#if !defined(lua_writeline)
#define lua_writeline() \
      (lua_writestring("\n", 1), fflush(stdout))
#endif

/* print an error message */
#if !defined(lua_writestringerror)
#define lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

#else // MOON_USE_LUAJIT

const char *luaL_findtable(lua_State *L, int idx, const char *fname, int szhint);

#endif // MOON_USE_LUAJIT

#endif //MOON_SRC_MCOMPAT_H_
