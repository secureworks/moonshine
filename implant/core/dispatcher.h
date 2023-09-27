#ifndef MOONSHINE_IMPLANT_CORE_DISPATCHER_H_
#define MOONSHINE_IMPLANT_CORE_DISPATCHER_H_

#include <map>
#include <mutex>
#include <unordered_map>

#include <signals.hpp>

#include <message.h>

struct dispatcher {

  dispatcher() = default;

  dispatcher(const dispatcher&) = delete; // Disable copy

  void send(const std::string& implant, std::shared_ptr<shared::message> message);
  std::vector<std::shared_ptr<shared::message>> pull_send(const std::string& implant);
  void push_received(const std::string& implant, std::vector<std::shared_ptr<shared::message>> messages);
  void push_received(const std::string& implant, const std::vector<shared::message>& messages);
  void push_received(const std::string& implant, std::shared_ptr<shared::message> message);

  fteng::signal<void(const std::string& implant, std::shared_ptr<shared::message>)> signal_message_received;

private:
  std::mutex send_mutex, received_mutex;
  std::unordered_map<std::string, std::vector<std::shared_ptr<shared::message>>> send_map;
};

#endif //MOONSHINE_IMPLANT_CORE_DISPATCHER_H_
