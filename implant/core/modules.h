#ifndef MOONSHINE_IMPLANT_CORE_MODULES_H_
#define MOONSHINE_IMPLANT_CORE_MODULES_H_

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

#endif //MOONSHINE_IMPLANT_CORE_MODULES_H_
