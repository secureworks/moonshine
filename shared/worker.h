#ifndef MOONSHINE_SHARED_LUA_SERVICE_H_
#define MOONSHINE_SHARED_LUA_SERVICE_H_

#include <future>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <string>
#include <vector>

#include <signals.hpp>
#include <lua_object.hpp>

namespace shared::abstract {

  struct worker : lua_object {

    worker() = default;
    worker(std::string id, std::string name, std::vector<unsigned char> package) : _id(std::move(id)), _name(std::move(name)), _package(std::move(package)) {};
    worker(std::string id, std::string name, std::vector<unsigned char> package, std::vector<std::string> arguments) : _id(std::move(id)), _name(std::move(name)), _package(std::move(package)), _arguments(std::move(arguments)) {};
    worker(const worker& w) : _id(w.id()), _name(w.name()), _package(w.package()), _arguments(w.arguments()) {};

    bool start();
    virtual void stop(bool wait = false);
    bool is_running();

    std::string id() const { return _id; };
    std::string name() const { return _name; };
    std::vector<unsigned char> package() const { return _package; };
    std::vector<std::string> arguments() const { return _arguments; };

    std::string output();

    int luaopen_object(lua_State* L) override;
    int lua_run(lua_State* L);
    virtual int lua_recv(lua_State* L);
    virtual int lua_send(lua_State* L);

    static std::string serialize(shared::abstract::worker& worker);
    static worker deserialize(std::string& raw);

    bool operator ==(const worker& o) const;

    fteng::signal<void(const std::string& id)> signal_worker_output;
    fteng::signal<void(const std::string& output)> signal_worker_stopped;

   private:

    std::future<void> thread_;
    std::atomic_bool running_ = false;
    std::shared_mutex mutex_;

   protected:

    virtual void run_worker();

    lua_State *runtime = nullptr;
    std::string output_;
    std::string _id;
    std::string _name;
    std::vector<unsigned char> _package{};
    std::vector<std::string> _arguments{};
  };

}

#endif //MOONSHINE_SHARED_LUA_SERVICE_H_
