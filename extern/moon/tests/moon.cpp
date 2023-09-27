#include <ostream>
#include <vector>

#include <catch2/catch.hpp>

#define LUA_LIB
#include "lua.hpp"
#include "moon.hpp"
#include "lua_object.hpp"

extern "C" {
#include "mcompress.h"
#include <sys/stat.h>
#include "miniz.h"
}


#ifdef WIN32
#define LIB_SUFFIX ".dll"
#define PATH_SEP "\\"
#else
#define LIB_SUFFIX ".so"
#define PATH_SEP "/"
#endif


std::string output{};
void store_output(lua_State *L, const void* data, size_t length, void *userdata) {
  output.append((char*)data, length);
}

static int read_file(const char* path, void** contents, size_t *size)
{
  FILE* in_file = fopen(path, "rb");
  if (!in_file) return 0;

#if _WIN32
  struct _stat sb{};
  if (_stat( path, &sb ) == -1) return 0;
#else
  struct stat sb{};
  if (stat(path, &sb) == -1) return 0;
#endif

  *size = sb.st_size;
  *contents = (void*) malloc(sb.st_size);
  fread(*contents, sb.st_size, 1, in_file);

  fclose(in_file);
  return 1;
}

TEST_CASE("fail_require")
{
  output.clear();
  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  int fail = luaL_dostring(L, "require 'idonotexist'");
  REQUIRE(fail);
  lua_close(L);
}

bool callback_called = false;

TEST_CASE("newstate_callback")
{
  output.clear();
  moon_newstate_cb([](lua_State *L, void *userdata) {
    callback_called = true;
  }, nullptr);
  lua_State *L = luaL_newstate();
  REQUIRE(L);
  REQUIRE(callback_called);
  moon_newstate_cb(nullptr, nullptr);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  int fail = luaL_dostring(L, "print('hello1'); return 'hello2'");
  REQUIRE(!fail);
  REQUIRE(output == "hello1\n");
  auto ret = std::string(lua_tostring(L, -1));
  REQUIRE(ret == "hello2");
  lua_close(L);
}

TEST_CASE("doscript")
{
  output.clear();
  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  std::vector<const char *> args;
  args.push_back("one");
  args.push_back("two");
  args.push_back("three");
  char** argv = const_cast<char**>(&args[0]);
  int argn = args.size();
  std::string script = "print(#arg); print(arg[1] .. arg[2] .. arg[3]); print(...); return 'hello2'";
  int fail = moon_doscript(L, argn, argv, script.data(), script.size());
  if (fail) printf("Error: %s", output.c_str());
  REQUIRE(!fail);
  REQUIRE(output == "3\nonetwothree\none\ttwo\tthree\n");
  auto ret = std::string(lua_tostring(L, -1));
  REQUIRE(ret == "hello2");
  lua_close(L);
}

TEST_CASE("doscript_no_args")
{
  output.clear();
  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  std::string script = "print('hello')";
  int fail = moon_doscript(L, 0, NULL, script.data(), script.size());
  REQUIRE(!fail);
  REQUIRE(output == "hello\n");
  lua_close(L);
}

TEST_CASE("doscript_uncaught_error")
{
  output.clear();
  const char* name = "hello";
  std::vector<std::string> paths;
  paths.push_back("hello" PATH_SEP "Debug" PATH_SEP "hello" LIB_SUFFIX);
  paths.push_back("hello" PATH_SEP "hello" LIB_SUFFIX);

  void* module = NULL;
  size_t size = 0;
  for (const auto& path : paths) {
    if (read_file(path.c_str(), &module, &size))
      break;
  }
  REQUIRE(module);

  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  REQUIRE(moon_addmemc(L, name, module, size, 0) == 0);
  std::string script = "hello = require('hello'); hello.check_string(nil)";
  int error = moon_doscript(L, 0, NULL, script.data(), script.size());
  REQUIRE(error);
  REQUIRE(!output.empty());
  lua_close(L);
  free(module);
}

TEST_CASE("load_lua_module")
{
  output.clear();
  std::string name = "world";
  std::string module = R"(
  -- Table to be returned by the world module.
  local world = {}

  -- Add the hello() function to the table returned by this module.
  function world.hello()
    return 'Hello from World!'
  end

  -- Return the hello_world table to make it accessible as a module.
  return world
  )";

  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  REQUIRE(moon_addmeml(L, name.c_str(), module.data(), module.size(), 0) == 0);
  REQUIRE(moon_addmeml(L, name.c_str(), module.data(), module.size(), 0) != 0);
  int error = luaL_dostring(L, "world = require('world'); print(world.hello())");
  if (error) printf("Error: %s", lua_tostring(L, -1));
  REQUIRE(!error);
  REQUIRE(output == "Hello from World!\n");
  lua_close(L);
}

TEST_CASE("load_lua_module_compressed")
{
  output.clear();
  std::string name = "world";
  std::string module = R"(
  -- Table to be returned by the world module.
  local world = {}

  -- Add the hello() function to the table returned by this module.
  function world.hello()
    return 'Hello from World!'
  end

  -- Return the hello_world table to make it accessible as a module.
  return world
  )";

  void *compressed = NULL;
  size_t compressed_size = 0;
  REQUIRE(compress_memory(module.data(), module.size(), &compressed, &compressed_size));

  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  REQUIRE(moon_addmeml(L, name.c_str(), compressed, compressed_size, 1) == 0);
  int error = luaL_dostring(L, "world = require('world'); print(world.hello())");
  if (error) printf("Error: %s", lua_tostring(L, -1));
  REQUIRE(!error);
  REQUIRE(output == "Hello from World!\n");
  lua_close(L);
  free(compressed);
}

TEST_CASE("load_no_lua_module")
{
  output.clear();
  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  int error = luaL_dostring(L, "world = require('world'); print(world.hello())");
  REQUIRE(error);
  lua_close(L);
  output.clear();
}

TEST_CASE("lua_c_module")
{
  output.clear();
  const char* name = "hello";
  std::vector<std::string> paths;
  paths.push_back("hello" PATH_SEP "Debug" PATH_SEP "hello" LIB_SUFFIX);
  paths.push_back("hello" PATH_SEP "hello" LIB_SUFFIX);

  void* module = NULL;
  size_t size = 0;
  for (const auto& path : paths) {
    if (read_file(path.c_str(), &module, &size))
      break;
  }
  REQUIRE(module);

  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  REQUIRE(moon_addmemc(L, name, module, size, 0) == 0);
  REQUIRE(moon_addmemc(L, name, module, size, 0) != 0);
  int error = luaL_dostring(L, "hello = require('hello'); print(hello.say_hello())");
  if (error) printf("Error: %s", lua_tostring(L, -1));
  REQUIRE(!error);
  REQUIRE(output == "hello from inside a module!\n");
  lua_close(L);
  free(module);
}

TEST_CASE("lua_c_module_compressed")
{
  output.clear();
  const char* name = "hello";
  std::vector<std::string> paths;
  paths.push_back("hello" PATH_SEP "Debug" PATH_SEP "hello" LIB_SUFFIX);
  paths.push_back("hello" PATH_SEP "hello" LIB_SUFFIX);

  void* module = NULL;
  size_t size = 0;
  for (const auto& path : paths) {
    if (read_file(path.c_str(), &module, &size))
      break;
  }
  REQUIRE(module);

  void *compressed = NULL;
  size_t compressed_size = 0;
  REQUIRE(compress_memory(module, size, &compressed, &compressed_size));

  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  REQUIRE(moon_addmemc(L, name, compressed, compressed_size, 1) == 0);
  int error = luaL_dostring(L, "hello = require('hello'); print(hello.say_hello())");
  if (error) printf("Error: %s", lua_tostring(L, -1));
  REQUIRE(!error);
  REQUIRE(output == "hello from inside a module!\n");
  lua_close(L);
  free(module);
  free(compressed);
}

TEST_CASE("lua_package")
{
  output.clear();
  std::vector<std::string> paths;
  paths.push_back("hello" PATH_SEP "Debug" PATH_SEP "hello" LIB_SUFFIX);
  paths.push_back("hello" PATH_SEP "hello" LIB_SUFFIX);

  void* module = NULL;
  size_t size = 0;
  for (const auto& path : paths) {
    if (read_file(path.c_str(), &module, &size))
      break;
  }
  REQUIRE(module);

  auto *za = (mz_zip_archive*) malloc(sizeof(mz_zip_archive));
  mz_zip_zero_struct(za);
  mz_zip_writer_init_heap(za, size, size);
  REQUIRE(mz_zip_writer_add_mem_ex(za, ("hello" LIB_SUFFIX), module, size, "", (mz_uint16)0, MZ_DEFAULT_LEVEL, 0, 0));
  size_t zip_size = 0;
  void* zip = NULL;
  REQUIRE(mz_zip_writer_finalize_heap_archive(za, &zip, &zip_size));
  mz_zip_writer_end(za);

  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  REQUIRE(moon_addmemz(L, zip, zip_size) == 0);
  int error = luaL_dostring(L, "hello = require('hello'); print(hello.say_hello())");
  if (error) printf("Error: %s", lua_tostring(L, -1));
  REQUIRE(!error);
  REQUIRE(output == "hello from inside a module!\n");
  lua_close(L);
  free(module);
  free(zip);
}

struct object : lua_object {

  #define HELLO_STRING "hello from inside an object!"

  int say_hello(lua_State *L) {
    lua_pushstring(L, HELLO_STRING);
    return 1;
  };

  int check_string(lua_State *L) {
    luaL_checkstring(L, 1);
    return 0;
  };

  int luaopen_object(lua_State *L) override {
    lua_newtable(L);
    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, &dispatch_to_member<object, &object::say_hello>, 1);
    lua_setfield (L, -2, "say_hello");
    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, &dispatch_to_member<object, &object::check_string>, 1);
    lua_setfield (L, -2, "check_string");
    return 1;
  };
};

TEST_CASE("lua_object")
{
  output.clear();
  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);

  object obj{};
  register_lua_object(L, "object", &obj);

  int error = luaL_dostring(L, "object = require('object'); print(object.say_hello())");
  if (error) printf("Error: %s", lua_tostring(L, -1));
  REQUIRE(!error);
  REQUIRE(output == "hello from inside an object!\n");
  lua_close(L);
}

TEST_CASE("lua_object_uncaught_error")
{
  output.clear();
  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);

  object obj{};
  register_lua_object(L, "object", &obj);

  std::string script = "object = require('object'); object.check_string(nil)";
  int error = moon_doscript(L, 0, NULL, script.data(), script.size());
  REQUIRE(error);
  REQUIRE(!output.empty());
  lua_close(L);
}

TEST_CASE("global_functions")
{
  output.clear();
  const char* name = "hello";
  std::vector<std::string> paths;
  paths.push_back("hello" PATH_SEP "Debug" PATH_SEP "hello" LIB_SUFFIX);
  paths.push_back("hello" PATH_SEP "hello" LIB_SUFFIX);

  void* module = NULL;
  size_t size = 0;
  for (const auto& path : paths) {
    if (read_file(path.c_str(), &module, &size))
      break;
  }
  REQUIRE(module);

  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);
  REQUIRE(moon_addmemc(L, name, module, size, 0) == 0);

  std::string script = R"(
  for index, data in pairs(preloaded()) do
    print(data)
  end
  )";

  int error = luaL_dostring(L, script.c_str());
  if (error) printf("Error: %s", lua_tostring(L, -1));
  REQUIRE(!error);
  REQUIRE(output == "hello" LIB_SUFFIX "\n");
  output.clear();

  script = R"(
  local hello = require('hello');
  for index, data in pairs(loaded()) do
    print(data)
  end
  )";

  error = luaL_dostring(L, script.c_str());
  if (error) printf("Error: %s", lua_tostring(L, -1));
  REQUIRE(!error);
  REQUIRE(output == "hello" LIB_SUFFIX "\n");
  output.clear();

  script = R"(
  unrequire('hello');
  for index, data in pairs(loaded()) do
    print(data)
  end
  )";

  error = luaL_dostring(L, script.c_str());
  if (error) printf("Error: %s", lua_tostring(L, -1));
  REQUIRE(!error);
  REQUIRE(output == "");

  lua_close(L);
  free(module);
}

#ifdef _WIN32

TEST_CASE("library_with_dependency")
{
  output.clear();
  lua_State *L = luaL_newstate();
  REQUIRE(L);
  luaL_openlibs(L);
  moon_output_cb(L, store_output, nullptr);

  typedef char* (__cdecl *invoke)(void);

  static char *paths_a[] = {
      (char*) "aaa_dll" PATH_SEP "Debug" PATH_SEP "aaa" LIB_SUFFIX,
      (char*) "aaa_dll" PATH_SEP "aaa" LIB_SUFFIX,
      NULL};

  void* module_a = NULL;
  size_t size_a = 0;
  int i = 0;
  while(paths_a[i]) {
    if (read_file(paths_a[i], &module_a, &size_a))
      break;
    i++;
  }
  REQUIRE(module_a);

  void* function_a;
  REQUIRE(moon_loadlib(L, "aaa.dll", module_a, size_a, 0, "FunctionA", &function_a) == LUA_OK);
  REQUIRE(function_a);
  char* rep_a = ((invoke)function_a)();
  REQUIRE(strcmp(rep_a, "Hello from function A") == 0);

  static char *paths_b[] = {
      (char*) "bbb_dll" PATH_SEP "Debug" PATH_SEP "bbb" LIB_SUFFIX,
      (char*) "bbb_dll" PATH_SEP "bbb" LIB_SUFFIX,
      NULL};

  void* module_b = NULL;
  size_t size_b = 0;
  i = 0;
  while(paths_b[i]) {
    if (read_file(paths_b[i], &module_b, &size_b))
      break;
    i++;
  }
  REQUIRE(module_b);

  void* function_b;
  REQUIRE(moon_loadlib(L, "bbb.dll", module_b, size_b, 0, "FunctionB", &function_b) == LUA_OK);
  REQUIRE(function_b);
  char* rep_b = ((invoke)function_b)();
  REQUIRE(strcmp(rep_b, "Hello from function A and from function B") == 0);

  lua_close(L);
  free(module_a);
  free(module_b);
}
#endif
