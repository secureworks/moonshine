#ifndef MOONSHINE_IMPLANT_STATE_H_
#define MOONSHINE_IMPLANT_STATE_H_

#include <atomic>
#include <random>
#include <string>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include <util/string.hpp>

#include <lua_object.hpp>

#include "encryption.h"
#include "config.h"
#include "dispatcher.h"

struct state {
  state() : id(util::string::random_alpha_numeric()), is_running(true), dwell_period{ DEFAULT_DWELL_PERIOD } {};

  state(const state&) = delete; // Disable copy

  const std::string id;
  std::atomic_bool is_running;
  std::atomic<float> dwell_period;
  monocypher::key_exchange::public_key callback_server_public_key{};
  monocypher::key_exchange keys{};
  class dispatcher dispatcher{};

  void callback_server(const std::string& value);
  const std::string& callback_server() const;
  void callback_package(const std::vector<unsigned char>& value);
  const std::vector<unsigned char>& callback_package() const;

 private:
  mutable std::shared_mutex _callback_server_mutex;
  std::string _callback_server{};
  mutable std::shared_mutex _callback_package_mutex;
  std::vector<unsigned char> _callback_package{};
};

namespace thread_storage {

extern thread_local std::vector<std::pair<std::string, lua_object *>> luaopen_objects;

}

#endif //MOONSHINE_IMPLANT_STATE_H_
