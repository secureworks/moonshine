#include "task.h"

#include <thread>

#include <spdlog/spdlog.h>
#include <oatpp/core/Types.hpp>
#include <util/string.hpp>

namespace handlers {

  task::task() : results_thread {std::async(std::launch::async, [this] { service_results(); }) } {}

  task::~task() {
    results_queue.stop();
  }

  void task::service_results() {

    while (true) {
      try {
        auto item = results_queue.wait_and_pop();
        auto id = oatpp::String(item->second->id);
        try {
          if (_task_service.exists(id)) {
            spdlog::debug("Received task result '{}' from '{}'", item->second->id, item->first);
            _task_service.update(id, dto::task_status::COMPLETED,
                                 static_cast<dto::task_success>(item->second->success),
                                 item->second->data);
          }
          else {
            spdlog::error("Received result from '{}' for unknown task '{}'", item->first, id->c_str());
          }
        }
        catch (const std::exception& e) {
          spdlog::error("Error retrieving task '{}' from database, {}", id->c_str(), e.what());
        }
      }
      catch (const queue_stopped& e) {
        break;
      }
    }
  }

  void task::on_result(const std::string& implant, std::shared_ptr<shared::message> message)
  {
    if (util::string::compare_insensitive(message->type, "task")) {
      results_queue.push(std::make_shared<queue_item>(implant , message));
    }
  }

}
