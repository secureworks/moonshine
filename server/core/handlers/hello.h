#ifndef MOONSHINE_SERVER_CORE_HANDLERS_HELLO_H_
#define MOONSHINE_SERVER_CORE_HANDLERS_HELLO_H_

#include <future>

#include <signals.hpp>

#include <message.h>
#include <util/concurrency.hpp>

#include "handlers/dispatcher.h"
#include "services/implant.h"

namespace handlers {

struct hello {

  using queue_item = std::pair<const std::string, std::shared_ptr<shared::message>>;
  using result_queue = util::concurrency::queue<std::shared_ptr<queue_item>>;
  using queue_stopped = util::concurrency::queue_stopped;

  hello();
  ~hello();

 private:
  void service_results();

  void on_result(const std::string& implant, std::shared_ptr<shared::message> message);

  handlers::dispatcher* _dispatcher = handlers::dispatcher::instance();
  services::implant _implant_service;
  result_queue results_queue;
  std::future<void> results_thread;

  fteng::connection on_result_connection = _dispatcher->signal_message_received.connect<&hello::on_result>(this);
};

}

#endif //MOONSHINE_SERVER_CORE_HANDLERS_HELLO_H_
