#ifndef MOON_AUXILIARY_H_
#define MOON_AUXILIARY_H_

#include <lua.h>

int moon_doscript(lua_State *L, int argc, char *argv[], const char *script, size_t size);
void moon_printstack(lua_State *L);

#endif //MOON_AUXILIARY_H_
