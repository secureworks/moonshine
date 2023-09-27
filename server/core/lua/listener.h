#ifndef MOONSHINE_SERVER_LUA_LISTENER_H_
#define MOONSHINE_SERVER_LUA_LISTENER_H_

#include <future>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <vector>

#include <lua_object.hpp>

#include "handlers/dispatcher.h"
#include "services/task.h"
#include "services/job.h"
#include "services/implant.h"
#include "services/config.h"
#include "dto/implant.h"

namespace lua {

struct listener : lua_object {

  listener(std::string id,
           std::string name,
           std::vector<unsigned char> package,
           std::vector<std::string> arguments);

  bool start();
  void stop(bool wait = false);
  bool is_running();

  const std::string id;
  const std::string name;
  const std::vector<unsigned char> package;
  const std::vector<std::string> arguments;

  std::string output();

  int luaopen_object(lua_State *L) override;
  int lua_run(lua_State *L);
  int lua_submit(lua_State *L);
  int lua_retrieve(lua_State *L);
  int lua_retrieve_by_token(lua_State *L);

 private:

  void run_listener();
  oatpp::Object<dto::implant> authenticate(std::vector<unsigned char>& blob);
  std::vector<uint8_t> encrypt_messages(lua_State* L, oatpp::Object<dto::implant> implant, std::vector<std::shared_ptr<shared::message>>& messages);

  std::future<void> thread_;
  std::atomic_bool running_ = false;
  std::shared_mutex mutex_;
  std::string output_;

  services::task _task_service; // Create task service.
  services::job _job_service; // Create job service.
  services::implant _implant_service; // Create implant service.
  services::config _config_service; // Create config service.
  handlers::dispatcher* _dispatcher = handlers::dispatcher::instance();
};

}

#endif //MOONSHINE_SERVER_LUA_LISTENER_H_
