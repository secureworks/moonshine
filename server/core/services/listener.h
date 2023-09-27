#ifndef MOONSHINE_SERVER_SERVICE_LISTENER_H_
#define MOONSHINE_SERVER_SERVICE_LISTENER_H_

#include <unordered_map>
#include <mutex>

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/orm/Connection.hpp>
#include <oatpp/web/protocol/http/Http.hpp>
#include <oatpp/core/macro/component.hpp>

#include "db/client.h"
#include "dto/listener.h"
#include "dto/page.h"
#include "dto/status.h"
#include "services/argument.h"
#include "lua/listener.h"

namespace services {

struct listener {

 private:

  typedef oatpp::web::protocol::http::Status Status;
  OATPP_COMPONENT(std::shared_ptr<db::client>, _database); // Inject database component

  listener() = default;
  ~listener() = default;

  argument _argument_service;

  std::mutex _listeners_mutex;
  std::unordered_map<std::string, std::shared_ptr<lua::listener>> _listeners;

  oatpp::Object<dto::listener> getByRowId(v_int64 rowid);
  oatpp::Object<dto::status> deleteById(oatpp::String &id);
  oatpp::Object<dto::listener> getById(oatpp::String &id);

 public:

  static listener *instance() {
    static listener instance;
    return &instance;
  }

  listener(const listener &) = delete; // Disable copy
  void operator=(const listener &) = delete; // Disable assignment

  oatpp::Object<dto::page<oatpp::Object<dto::listener>>> getAll(const oatpp::UInt32 &offset,
                                                                const oatpp::UInt32 &limit);
  oatpp::Object<dto::listener> createWithChildren(const oatpp::Object<dto::listener> &dto);
  oatpp::Object<dto::listener> getByIdWithChildren(oatpp::String &id);
  oatpp::Object<dto::page<oatpp::Object<dto::listener>>> getAllWithChildren(const oatpp::UInt32 &offset,
                                                                            const oatpp::UInt32 &limit);
  oatpp::Object<dto::status> deleteByIdWithChildren(oatpp::String &id);
  oatpp::Object<dto::status> startById(oatpp::String &id);
  oatpp::Object<dto::status> stopById(oatpp::String &id, bool wait = true);

  bool is_running(oatpp::String &id);
  void init();
};

}

#endif //MOONSHINE_SERVER_SERVICE_LISTENER_H_
