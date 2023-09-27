#include "job.h"

#include <thread>

#include <spdlog/spdlog.h>
#include <oatpp/core/Types.hpp>
#include <util/string.hpp>

namespace handlers {

job::job() : responses_thread {std::async(std::launch::async, [this] { service_responses(); }) } {}

job::~job() {
  job_queue.stop();
}

void job::service_responses() {

  while (true) {
    try {
      auto item = job_queue.wait_and_pop();
      auto implant = item->first;
      auto message = item->second;
      try {
        if (_job_service.existsById(message->src)) {
          auto job = _job_service.getByIdWithChildren(oatpp::String(message->src));
          spdlog::debug("Received job result '{}' from '{}', {}", job->id->c_str(), implant, message->command);
          if (util::string::compare_insensitive(message->command, "executing")) {
            _job_service.update(job->id, dto::job_status::EXECUTING,
                                static_cast<dto::job_success>(message->success),job->output->append(message->data));
          }
          else if (util::string::compare_insensitive(message->command, "stopping")) {
            _job_service.update_status(job->id, dto::job_status::STOPPING, oatpp::String(implant));
          }
          else if (util::string::compare_insensitive(message->command, "completed")) {
            _job_service.update(job->id, dto::job_status::COMPLETED,
                                static_cast<dto::job_success>(message->success),
                                message->data);
          }
          else {
            spdlog::error("Received result '{}' from '{}' for unknown job command '{}'", job->id->c_str(), implant, message->command);
          }
        }
        else {
          spdlog::error("Received result from '{}' for unknown job '{}'", implant, message->src);
        }
      }
      catch (const std::exception& e) {
        spdlog::error("Error retrieving job '{}' from database, {}", message->src, e.what());
      }
    }
    catch (const queue_stopped& e) {
      break;
    }
  }
}

void job::on_response(const std::string& implant, std::shared_ptr<shared::message> message)
{
  if (util::string::compare_insensitive(message->type, "job_handler")) {
    job_queue.push(std::make_shared<queue_item>(implant , message));
  }
}

}
