#ifndef MOONSHINE_IMPLANT_CORE_HANDLERS_HELLO_H_
#define MOONSHINE_IMPLANT_CORE_HANDLERS_HELLO_H_

#include <future>

#include <util/concurrency.hpp>

#include "state.h"

namespace handler {

  struct hello {

    using queue_item = std::pair<const std::string, std::shared_ptr<shared::message>>;
    using mqueue = util::concurrency::queue<std::shared_ptr<queue_item>>;
    using queue_stopped = util::concurrency::queue_stopped;

    explicit hello(state* state);
    ~hello();

  private:

    void process();
    void on_message(const std::string& implant, std::shared_ptr<shared::message> message);

    state* _state;
    mqueue _message_queue;
    std::future<void> _worker_thread;
    fteng::connection on_message_connection;
  };

}

#endif //MOONSHINE_IMPLANT_CORE_HANDLERS_HELLO_H_
