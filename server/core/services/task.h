#ifndef MOONSHINE_SERVER_SERVICE_TASK_H_
#define MOONSHINE_SERVER_SERVICE_TASK_H_

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/orm/Connection.hpp>
#include <oatpp/web/protocol/http/Http.hpp>
#include <oatpp/core/macro/component.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>

#include "handlers/dispatcher.h"
#include "db/client.h"
#include "dto/task.h"
#include "dto/page.h"
#include "dto/status.h"

namespace services {

struct task {

 private:

  typedef oatpp::web::protocol::http::Status Status;

  handlers::dispatcher* _dispatcher = handlers::dispatcher::instance();
  OATPP_COMPONENT(std::shared_ptr<db::client>, _database); // Inject database component
  std::shared_ptr<oatpp::parser::json::mapping::ObjectMapper>
      _jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

 private:

  oatpp::Object<dto::task> getByRowId(v_int64 rowid);

 public:

  oatpp::Object<dto::task> create(const oatpp::Object<dto::task> &dto);
  oatpp::Object<dto::page<oatpp::Object<dto::task>>> getAllByImplant(const oatpp::String &implant,
                                                                     const oatpp::UInt32 &offset,
                                                                     const oatpp::UInt32 &limit);
  oatpp::Object<dto::page<oatpp::Object<dto::task>>> getAllByImplantWhereStatus(const oatpp::String &implant,
                                                                                const oatpp::Enum<dto::task_status>::AsNumber& status,
                                                                                const oatpp::UInt32 &offset,
                                                                                const oatpp::UInt32 &limit);
  oatpp::Object<dto::task> getById(oatpp::String &id);
  bool exists(oatpp::String id);
  bool update_status(oatpp::String id, dto::task_status task_status, oatpp::Object<dto::implant> implant);
  bool update(oatpp::String id, dto::task_status status, dto::task_success success, const std::string& output);
};

}

#endif //MOONSHINE_SERVER_SERVICE_TASK_H_
