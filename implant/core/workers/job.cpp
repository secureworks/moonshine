#include "job.h"

#include <utility>

#include <util/debug.hpp>

namespace worker {

job::job(const shared::abstract::worker& desc, state* state) : worker(desc), _state(state) {
  on_message_connection = _state->dispatcher.signal_message_received.connect<&job::on_message>(this);
}

void job::stop(bool wait) {
  _recv_queue.stop();
  worker::stop(wait);
}

int job::lua_recv(lua_State* L)
{
  try {
    //auto item = _recv_queue.wait_and_pop();
    std::shared_ptr<queue_item> item;
    if (_recv_queue.try_pop(item)) {
      lua_pushlstring(L, item->first.data(), item->first.size()); // implant
      lua_pushlstring(L, item->second->src.data(), item->second->src.size()); // internal src (e.g. another job/service)
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

int job::lua_send(lua_State* L)
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

  _state->dispatcher.send(implant, message);

  return 0;
}

void job::on_message(const std::string& implant, std::shared_ptr<shared::message> message)
{
  if (util::string::compare_insensitive(message->type, "job") && message->dst == id()) {
    _recv_queue.push(std::make_shared<queue_item>(implant, message));
  }
}

void job::run_worker() {
  thread_storage::luaopen_objects.emplace_back("job", this);
  worker::run_worker();
}

} // worker
