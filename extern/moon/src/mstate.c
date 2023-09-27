#include "mstate.h"

void moon_newstate_cb(newstate_callback callback, void *userdata) {
  if (callback != NULL) {
    newstate_cb = callback;
    newstate_userdata = userdata;
  }
}

void moon_newstate_call_cb(lua_State *L) {
  if (newstate_cb != NULL)
    newstate_cb(L, newstate_userdata);
}
