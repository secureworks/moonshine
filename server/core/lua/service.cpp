#include "service.h"

#include <spdlog/spdlog.h>

#include "modules.h"

namespace lua {

  void service::stop(bool wait) {
    recv_queue.stop();
    shared::abstract::worker::stop(wait);
  }

  int service::lua_recv(lua_State* L)
  {
    try {
      //auto item = recv_queue.wait_and_pop();
      std::shared_ptr<queue_item> item;
      if (recv_queue.try_pop(item)) {
        lua_pushlstring(L, item->first.data(), item->first.size()); // implant
        lua_pushlstring(L, item->second->src.data(), item->second->src.size());
        lua_pushlstring(L, item->second->command.data(), item->second->command.size());
        lua_pushlstring(L, item->second->data.data(), item->second->data.size());
        return 4;
      }
      else {
        lua_pushnil(L);
        return 1;
      }
    }
    catch (const queue_stopped& e) {
      lua_pushnil(L);
      return 1;
    }
  }

  int service::lua_send(lua_State* L)
  {
    const char* implant = luaL_checkstring(L, 1);
    const char* dst = luaL_checkstring(L, 2);
    const char* command = luaL_checkstring(L, 3);
    size_t length;
    const char* data = luaL_checklstring(L, 4, &length);

    auto message = std::make_shared<shared::message>();
    message->type = "job";
    message->src = id();
    message->dst = dst;
    message->command = command;
    message->data = std::string(data, length);

    _messenger->send(implant, message);

    return 0;
  }

  void service::on_message(const std::string& implant, std::shared_ptr<shared::message> message)
  {
    if (util::string::compare_insensitive(message->type, "job") && message->dst == id()) {
      recv_queue.push(std::make_shared<queue_item>(implant, message));
    }
  }

  void service::run_worker() {
    thread_storage::luaopen_objects.emplace_back("service", this);
    this->signal_worker_output.connect([&](const std::string &output) {
      std::string::size_type pos = output.find_last_not_of("\n \t");
      if (pos != std::string::npos)
        spdlog::debug("'{}' output...\n{}", name(), output.substr(0, pos + 1));
      else
        spdlog::debug("'{}' output...\n{}", name(), output);
    });
    worker::run_worker();
  }

}
