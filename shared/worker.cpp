#include "worker.h"

#include <yas/serialize.hpp>
#include <yas/std_types.hpp>

#include <moon.hpp>
extern "C" {
#include "mutil.h"
}
#include <util/debug.hpp>
#include <util/zip.hpp>
#include <util/debug.hpp>
#include <util/string.hpp>

namespace shared::abstract {

#ifdef _WIN32
#define LIB_SUFFIX ".dll"
#else
#define LIB_SUFFIX ".so"
#endif
#define LUA_SUFFIX ".lua"

#define INIT_LUA "init" LUA_SUFFIX

bool worker::start() {
  std::unique_lock<std::shared_mutex> guard(mutex_);
  if (running_)
    return running_;

  running_ = true;
  thread_ = std::async(std::launch::async, [this] { run_worker(); });
  auto status = thread_.wait_for(std::chrono::milliseconds(250));
  if (status == std::future_status::ready)
    running_ = false;
  return running_;
}

void worker::stop(bool wait) {
  std::unique_lock<std::shared_mutex> guard(mutex_);
  running_ = false;
  if (wait) thread_.wait();
}

void worker::run_worker() {
  std::string script;
  runtime = luaL_newstate();

  luaL_openlibs(runtime);
  register_lua_object(runtime, "worker", this);

  try {
    util::zip::zip_file zip(_package);

    if (!zip.has_file(INIT_LUA))
      throw std::runtime_error("No '" INIT_LUA "' script found in package");

    script = zip.read(INIT_LUA);

    auto infolist = zip.infolist();
    for (const auto& info : infolist) {
      if (info.is_dir) continue; // skip
      if (!util::string::ends_with(info.filename, LUA_SUFFIX) && !util::string::ends_with(info.filename, LIB_SUFFIX)) continue;
      if (info.filename == INIT_LUA) continue;

      char dir[_MAX_DIR];
      char name[_MAX_FNAME];
      char ext[_MAX_EXT];
      split_path(info.filename.c_str(), NULL, dir, name, ext);

      auto modname = std::string(dir);
      if (!modname.empty()) modname = modname.append("/");

      modname = modname.append(ext);
      std::replace(modname.begin(), modname.end(), '/', '.');

      auto bytes = zip.read(info.filename);

      if (strcmp(ext, LUA_SUFFIX) == 0) {
        moon_addmeml(runtime, modname.c_str(), bytes.data(), bytes.size(), 0);
      }
      else {
        moon_addmemc(runtime, modname.c_str(), bytes.data(), bytes.size(), 0);
      }
    }

    moon_output_cb(runtime, [](lua_State *LS, const void* data, size_t length, void *userdata) {
      auto s = (worker*) userdata;
      auto output = std::string((char*)data, length);
      s->signal_worker_output(output);
      s->output_.append(output); // TODO do we need this!?
      //ERROR_PRINT("%s", output);
    }, this);

    std::vector<const char *> argv(_arguments.size());
    std::transform(_arguments.begin(), _arguments.end(), argv.begin(), [](const std::string& str){return str.c_str();});
    moon_doscript(runtime, argv.size(), const_cast<char **>(argv.data()), script.data(), script.size());
  }
  catch (const std::runtime_error &e) {
    output_.append(e.what());
  }

  lua_close(runtime);
  running_ = false;
  signal_worker_stopped(_id);
}

bool worker::is_running() {
  return running_;
}

std::string worker::output() {
  return output_;
}

int worker::luaopen_object(lua_State *L) {
  lua_newtable(L);
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<worker, &worker::lua_run>, 1);
  lua_setfield(L, -2, "run");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<worker, &worker::lua_recv>, 1);
  lua_setfield(L, -2, "recv");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<worker, &worker::lua_send>, 1);
  lua_setfield(L, -2, "send");
  return 1;
}

int worker::lua_recv(lua_State *L) {
  return 0;
}

int worker::lua_send(lua_State *L) {
  return 0;
}

int worker::lua_run(lua_State *L) {
  lua_pushboolean(L, is_running() ? 1 : 0); // push result
  return 1; // number of return values
}

std::string worker::serialize(shared::abstract::worker& object) {
  try {
    auto buf = yas::save<yas::mem | yas::binary | yas::no_header>(
        YAS_OBJECT_NVP("worker", ("id", object._id), ("name", object._name), ("package", object._package), ("arguments", object._arguments))
    );
    return { buf.data.get(), buf.size };
  }
  catch (const std::exception &ex) {
    throw std::invalid_argument(ex.what());
  }
}

worker worker::deserialize(std::string &raw) {
  try {
    shared::abstract::worker object;
    yas::shared_buffer buf{raw.data(), raw.size()};
    yas::load<yas::mem | yas::binary | yas::no_header>(
        buf, YAS_OBJECT_NVP("worker", ("id", object._id), ("name", object._name), ("package", object._package), ("arguments", object._arguments))
    );
    return object;
  }
  catch (const std::exception &ex) {
    throw std::invalid_argument(ex.what());
  }
}

bool worker::operator ==(const worker& o) const
{
  return this->_id == o.id() &&
      this->_name == o.name() &&
      this->_package == o.package() &&
      this->_arguments == o.arguments();
}

}
