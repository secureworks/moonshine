#ifndef MOONSHINE_SERVER_CORE_WORKERS_JOBS_H_
#define MOONSHINE_SERVER_CORE_WORKERS_JOBS_H_

#include <future>

#include <signals.hpp>

#include <message.h>
#include <util/concurrency.hpp>

#include "handlers/dispatcher.h"
#include "services/job.h"

namespace handlers {

struct job {

  using queue_item = std::pair<const std::string, std::shared_ptr<shared::message>>;
  using result_queue = util::concurrency::queue<std::shared_ptr<queue_item>>;
  using queue_stopped = util::concurrency::queue_stopped;

  job();
  ~job();

 private:
  void service_responses();

  void on_response(const std::string& implant, std::shared_ptr<shared::message> message);

  handlers::dispatcher* _dispatcher = handlers::dispatcher::instance();
  services::job _job_service;
  result_queue job_queue;
  std::future<void> responses_thread;

  fteng::connection on_response_connection = _dispatcher->signal_message_received.connect<&job::on_response>(this);
};

}

#endif //MOONSHINE_SERVER_CORE_WORKERS_JOBS_H_
