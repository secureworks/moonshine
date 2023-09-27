#ifndef MOONSHINE_IMPLANT_CORE_HANDLERS_JOB_H_
#define MOONSHINE_IMPLANT_CORE_HANDLERS_JOB_H_

#include <thread>
#include <map>
#include <string>
#include <vector>
#include <mutex>

#include <signals.hpp>
#include <util/concurrency.hpp>

#include "workers/job.h"
#include "state.h"

namespace handler {

  struct job {

    using queue_item = std::pair<const std::string, std::shared_ptr<shared::message>>;
    using mqueue = util::concurrency::queue<std::shared_ptr<queue_item>>;
    using queue_stopped = util::concurrency::queue_stopped;

    explicit job(state* state);
    ~job();

    std::vector<std::pair<std::string, std::string>> list();

  private:

    void process();
    void on_message(const std::string& implant, std::shared_ptr<shared::message> message);

    state* _state;
    std::mutex _jobs_mutex;
    std::unordered_map<std::string, std::shared_ptr<worker::job>> _jobs;
    mqueue _message_queue;
    std::future<void> _worker_thread;
    fteng::connection on_message_connection;
  };

}

#endif //MOONSHINE_IMPLANT_CORE_HANDLERS_JOB_H_
