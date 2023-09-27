#include "state.h"

void state::callback_server(const std::string& value)
{
  std::shared_lock lock(_callback_server_mutex);
  _callback_server = value;
}

const std::string& state::callback_server() const
{
  std::unique_lock lock(_callback_server_mutex);
  return _callback_server;
}

void state::callback_package(const std::vector<unsigned char>& value)
{
  std::shared_lock lock(_callback_package_mutex);
  _callback_package = value;
}

const std::vector<unsigned char>& state::callback_package() const
{
  std::unique_lock lock(_callback_package_mutex);
  return _callback_package;
}

namespace thread_storage {

thread_local std::vector<std::pair<std::string, lua_object*>> luaopen_objects{};

}
