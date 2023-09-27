#ifndef MOONSHINE_SERVER_CORE_LUA_JOB_H_
#define MOONSHINE_SERVER_CORE_LUA_JOB_H_

#include <signals.hpp>
#include <lua.hpp>
#include <util/concurrency.hpp>

#include "worker.h"
#include "handlers/dispatcher.h"

namespace lua {

  struct service : shared::abstract::worker {

    using queue_item = std::pair<const std::string, std::shared_ptr<shared::message>>;
    using mqueue = util::concurrency::queue<std::shared_ptr<queue_item>>;
    using queue_stopped = util::concurrency::queue_stopped;

    using worker::worker; // use base class constructors

    void stop(bool wait = false) override;

    int lua_recv(lua_State* L) override;
    int lua_send(lua_State* L) override;

   protected:

    void run_worker() override;

   private:

    void on_message(const std::string& implant, std::shared_ptr<shared::message> message);

    handlers::dispatcher* _messenger = handlers::dispatcher::instance();
    mqueue recv_queue;

    fteng::connection on_message_connection = _messenger->signal_message_received.connect<&service::on_message>(this);
  };

}


#endif //MOONSHINE_SERVER_CORE_LUA_JOB_H_
