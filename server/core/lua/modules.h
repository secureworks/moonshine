#ifndef MOONSHINE_SERVER_CORE_LUA_MODULES_H_
#define MOONSHINE_SERVER_CORE_LUA_MODULES_H_

#include <vector>
#include <string>

#include <lua_object.hpp>

#include "luasocket/socket.lua.h" // depends: socket.core
#include "luasocket/socket.core.c.h"
#include "luasocket/socket.http.lua.h" // depends: socket, socket.url, ltn12, mime, socket.headers
#include "luasocket/socket.url.lua.h" // depends: socket
#include "luasocket/ltn12.lua.h"
#include "luasocket/mime.lua.h" // depends: ltn12, mime.core
#include "luasocket/mime.core.c.h"
#include "luasocket/socket.headers.lua.h" // depends: socket

#include "luasec/ssl.lua.h"
#include "luasec/ssl.c.h"
#include "luasec/ssl.https.lua.h"

#include "copas/copas.lua.h"
#include "binaryheap/binaryheap.lua.h"
#include "coxpcall/coxpcall.lua.h"
#include "timerwheel/timerwheel.lua.h"

//#include "luv/luv.c.h"
#include "struct/struct.c.h"

namespace thread_storage {

extern thread_local std::vector<std::pair<std::string, lua_object *>> luaopen_objects;

}

namespace lua {

void on_newstate(lua_State *L, void *userdata);

}


#endif //MOONSHINE_SERVER_CORE_LUA_MODULES_H_
