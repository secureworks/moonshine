#include "dispatcher.h"

namespace handlers {

  void dispatcher::send(const std::string& implant, std::shared_ptr<shared::message> message)
  {
    std::scoped_lock<std::mutex> lock{ send_mutex };

    if (auto it = send_map.find(implant); it != send_map.end()) {
      it->second.push_back(message);
    }
    else {
      std::vector<std::shared_ptr<shared::message>> local_messages;
      local_messages.push_back(message);
      send_map[implant] = local_messages;
    }
  }

  std::vector<std::shared_ptr<shared::message>> dispatcher::pull_send(const std::string& implant)
  {
    std::scoped_lock<std::mutex> lock{ send_mutex };

    if (auto it = send_map.find(implant); it != send_map.end()) {
      std::vector<std::shared_ptr<shared::message>> local_messages;
      it->second.swap(local_messages);
      return local_messages;
    }
    else {
      return {};
    }
  }

  void dispatcher::push_received(const std::string& implant, std::vector<std::shared_ptr<shared::message>> messages)
  {
    std::scoped_lock<std::mutex> lock{ received_mutex };

    for (const auto& message : messages) {
      signal_message_received(implant, message);
    }
  }

  void dispatcher::push_received(const std::string& implant, std::shared_ptr<shared::message> message) {
    std::scoped_lock<std::mutex> lock{ received_mutex };
    signal_message_received(implant, message);
  }

}
