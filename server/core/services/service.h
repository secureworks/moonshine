#ifndef MOONSHINE_SERVER_SERVICE_SERVICE_H_
#define MOONSHINE_SERVER_SERVICE_SERVICE_H_

#include <map>
#include <mutex>

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/orm/Connection.hpp>
#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

#include "db/client.h"
#include "dto/service.h"
#include "dto/page.h"
#include "dto/status.h"
#include "services/library.h"
#include "services/argument.h"
#include "lua/service.h"

namespace services {

struct service {

 private:

  typedef oatpp::web::protocol::http::Status Status;
  OATPP_COMPONENT(std::shared_ptr<db::client>, _database); // Inject database component

  service() = default;
  ~service() = default;

  library _library_service;
  argument _argument_service;

  std::mutex _mutex;
  std::unordered_map<std::string, std::shared_ptr<lua::service>> _services;

 private:

  oatpp::Object<dto::service> getByRowId(v_int64 rowid);
  oatpp::Object<dto::status> deleteById(oatpp::String &id);
  oatpp::Object<dto::service> getById(oatpp::String &id);

 public:

  static service *instance() {
    static service instance;
    return &instance;
  }

  service(const service &) = delete; // Disable copy
  void operator=(const service &) = delete; // Disable assignment

  oatpp::Object<dto::page<oatpp::Object<dto::service>>> getAll(const oatpp::UInt32 &offset,
                                                                const oatpp::UInt32 &limit);
  oatpp::Object<dto::service> createWithChildren(const oatpp::Object<dto::service> &dto);
  oatpp::Object<dto::service> getByIdWithChildren(oatpp::String &id);
  oatpp::Object<dto::page<oatpp::Object<dto::service>>> getAllWithChildren(const oatpp::UInt32 &offset,
                                                                            const oatpp::UInt32 &limit);
  oatpp::Object<dto::status> deleteByIdWithChildren(oatpp::String &id);
  oatpp::Object<dto::status> startById(oatpp::String &id);
  oatpp::Object<dto::status> stopById(oatpp::String &id, bool wait = true);

  bool is_running(oatpp::String &id);
  void init();
};

}

#endif //MOONSHINE_SERVER_SERVICE_SERVICE_H_
