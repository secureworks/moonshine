#ifndef MOONSHINE_SERVER_CORE_SERVICES_JOB_H_
#define MOONSHINE_SERVER_CORE_SERVICES_JOB_H_

#include <unordered_map>
#include <mutex>

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/orm/Connection.hpp>
#include <oatpp/web/protocol/http/Http.hpp>
#include <oatpp/core/macro/component.hpp>

#include "handlers/dispatcher.h"
#include "db/client.h"
#include "dto/job.h"
#include "dto/page.h"
#include "dto/status.h"
#include "services/argument.h"

namespace services {

struct job {

 private:

  typedef oatpp::web::protocol::http::Status Status;

  handlers::dispatcher* _dispatcher = handlers::dispatcher::instance();
  OATPP_COMPONENT(std::shared_ptr<db::client>, _database); // Inject database component

  argument _argument_service;

  oatpp::Object<dto::job> getByRowId(v_int64 rowid);
  oatpp::Object<dto::job> getById(oatpp::String &id);
  oatpp::Object<dto::page<oatpp::Object<dto::job>>> getAllByImplant(const oatpp::String &implant,
                                                                    const oatpp::UInt32 &offset,
                                                                    const oatpp::UInt32 &limit);
  oatpp::Object<dto::page<oatpp::Object<dto::job>>> getAllByImplantWhereStatus(const oatpp::String &implant,
                                                                               const oatpp::Enum<dto::job_status>::AsNumber& status,
                                                                               const oatpp::UInt32 &offset,
                                                                               const oatpp::UInt32 &limit);

 public:

  oatpp::Object<dto::job> createWithChildren(const oatpp::Object<dto::job> &dto);
  oatpp::Object<dto::page<oatpp::Object<dto::job>>> getAllWithChildrenByImplant(const oatpp::String &implant,
                                                                                const oatpp::UInt32 &offset,
                                                                                const oatpp::UInt32 &limit);
  oatpp::Object<dto::page<oatpp::Object<dto::job>>> getAllWithChildrenByImplantWhereStatus(const oatpp::String &implant,
                                                                                           const oatpp::Enum<dto::job_status>::AsNumber& status,
                                                                                           const oatpp::UInt32 &offset,
                                                                                           const oatpp::UInt32 &limit);
  oatpp::Object<dto::job> getByIdWithChildren(oatpp::String id);
  oatpp::Object<dto::status> stopById(oatpp::String id);
  oatpp::Object<dto::status> commandById(oatpp::String id, oatpp::String command, oatpp::String data);

  bool existsById(oatpp::String id);
  bool update(oatpp::String id, dto::job_status status, dto::job_success success, const std::string& output);
  bool update_status(oatpp::String id, dto::job_status task_status, oatpp::String implant);
  bool update_output(oatpp::String id, const std::string &output);

};

} // services

#endif //MOONSHINE_SERVER_CORE_SERVICES_JOB_H_
