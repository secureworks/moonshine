#include "modules.h"

#include <spdlog/spdlog.h>

#include <moon.hpp>

namespace thread_storage {

thread_local std::vector<std::pair<std::string, lua_object *>> luaopen_objects{};

}

namespace lua {

void on_newstate(lua_State *L, void *userdata) {
  // Callback which is executed when a new lua state is created - this will
  // apply globally for all lua states created in the running process.

  // Open default lua libraries
  luaL_openlibs(L);

  // Preload any thread local lua objects that have been registered
  for (const auto &pair : thread_storage::luaopen_objects) {
    register_lua_object(L, pair.first, pair.second);
  }

  // Preload a standard set of modules
#if defined(_WIN32)
  moon_addmemc(L, "socket.core", socket_core_dll_array, socket_core_dll_size, 1);
  moon_addmemc(L, "mime.core", mime_core_dll_array, mime_core_dll_size, 1);
  moon_addmemc(L, "ssl", ssl_dll_array, ssl_dll_size, 1);
  //moon_addmemc(L, "luv", luv_dll_array, luv_dll_size, 1);
  moon_addmemc(L, "struct", struct_dll_array, struct_dll_size, 1);
#else
  moon_addmemc(L, "socket.core", socket_core_so_array, socket_core_so_size, 1);
  moon_addmemc(L, "mime.core", mime_core_so_array, mime_core_so_size, 1);
  moon_addmemc(L, "ssl", ssl_so_array, ssl_so_size, 1);
  //moon_addmemc(L, "luv", luv_so_array, luv_so_size, 1);
  moon_addmemc(L, "struct", struct_so_array, struct_so_size, 1);
#endif
  moon_addmeml(L, "socket", socket_lua_array, socket_lua_size, 1);
  moon_addmeml(L, "socket.http", socket_http_lua_array, socket_http_lua_size, 1);
  moon_addmeml(L, "socket.url", socket_url_lua_array, socket_url_lua_size, 1);
  moon_addmeml(L, "ltn12", ltn12_lua_array, ltn12_lua_size, 1);
  moon_addmeml(L, "mime", mime_lua_array, mime_lua_size, 1);
  moon_addmeml(L, "socket.headers", socket_headers_lua_array, socket_headers_lua_size, 1);
  moon_addmeml(L, "ssl", ssl_lua_array, ssl_lua_size, 1);
  moon_addmeml(L, "ssl.https", ssl_https_lua_array, ssl_https_lua_size, 1);

  moon_addmeml(L, "copas", copas_lua_array, copas_lua_size, 1);
  moon_addmeml(L, "binaryheap", binaryheap_lua_array, binaryheap_lua_size, 1);
  moon_addmeml(L, "coxpcall", coxpcall_lua_array, coxpcall_lua_size, 1);
  moon_addmeml(L, "timerwheel", timerwheel_lua_array, timerwheel_lua_size, 1);

  // Setup default callback for output produced by a script
  //moon_output_cb(L, [](lua_State *LS, const void *data, size_t length, void *userdata) {
  //  spdlog::error("%s", std::string((char*)data, length));
  //}, nullptr);
}

}
