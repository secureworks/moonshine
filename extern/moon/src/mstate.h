#ifndef MOON_NEWSTATE_H_
#define MOON_NEWSTATE_H_

#include "lua.h"

typedef void (*newstate_callback)(lua_State *L, void *userdata);
static newstate_callback newstate_cb;
static void* newstate_userdata;
void moon_newstate_cb(newstate_callback callback, void *userdata);
void moon_newstate_call_cb(lua_State *L);

#endif //MOON_NEWSTATE_H_
