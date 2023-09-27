#ifndef MOONSHINE_SERVER_CORE_LUA_JOB_H_
#define MOONSHINE_SERVER_CORE_LUA_JOB_H_

#include "worker.h"

#include <util/concurrency.hpp>

struct tservice : shared::abstract::worker {

  using queue_item = std::pair<const std::string, std::shared_ptr<shared::message>>;
  using mqueue = util::concurrency::queue<std::shared_ptr<queue_item>>;
  using queue_stopped = util::concurrency::queue_stopped;

  mqueue send_queue;
  mqueue recv_queue;

  using worker::worker;

  void stop(bool wait = false) override {
    send_queue.stop();
    recv_queue.stop();
    worker::stop();
  }

  int lua_recv(lua_State* L) override {
    try {
      auto item = recv_queue.wait_and_pop();
      lua_pushlstring(L, item->first.data(), item->first.size()); // implant
      lua_pushlstring(L, item->second->src.data(), item->second->src.size());
      lua_pushlstring(L, item->second->command.data(), item->second->command.size());
      lua_pushlstring(L, item->second->data.data(), item->second->data.size());
      return 4;
    }
    catch (const queue_stopped& e) {
      lua_pushnil(L);
      return 1;
    }
  };

  int lua_send(lua_State* L) override {
    const char* implant = luaL_checkstring(L, 1);
    const char* dst = luaL_checkstring(L, 2);
    const char* command = luaL_checkstring(L, 3);
    size_t length;
    const char* data = luaL_checklstring(L, 4, &length);

    auto message = std::make_shared<shared::message>();
    message->src = id();
    message->dst = dst;
    message->type = "job";
    message->command = command;
    message->data = std::string(data, length);

    auto item = std::make_shared<queue_item>(implant, message);
    send_queue.push(item);

    return 0;
  };

};


#endif //MOONSHINE_SERVER_CORE_LUA_JOB_H_
