#ifndef MOONSHINE_IMPLANT_CORE_WORKERS_JOB_H_
#define MOONSHINE_IMPLANT_CORE_WORKERS_JOB_H_

#include <signals.hpp>
#include <util/concurrency.hpp>

#include "state.h"
#include "worker.h"

namespace worker {

struct job : shared::abstract::worker {

  using queue_item = std::pair<const std::string, std::shared_ptr<shared::message>>;
  using mqueue = util::concurrency::queue<std::shared_ptr<queue_item>>;
  using queue_stopped = util::concurrency::queue_stopped;

  job(const shared::abstract::worker& description, state* state);

  void stop(bool wait = false) override;

 protected:

  void run_worker() override;

 private:

  int lua_recv(lua_State* L) override;
  int lua_send(lua_State* L) override;

  void on_message(const std::string& implant, std::shared_ptr<shared::message> message);

  state* _state;
  mqueue _recv_queue;
  fteng::connection on_message_connection;
};

} // worker

#endif //MOONSHINE_IMPLANT_CORE_WORKERS_JOB_H_
