#ifndef MOONSHINE_SERVER_SERVICE_IMPLANT_H_
#define MOONSHINE_SERVER_SERVICE_IMPLANT_H_

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/orm/Connection.hpp>
#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

#include "db/client.h"
#include "dto/implant.h"
#include "dto/page.h"
#include "dto/status.h"

namespace services {

struct implant {

 private:
  typedef oatpp::web::protocol::http::Status Status;
  OATPP_COMPONENT(std::shared_ptr<db::client>, _database); // Inject database component

 private:
  oatpp::Object<dto::implant> getByRowId(v_int64 rowid);

 public:
  oatpp::Object<dto::implant> create(const oatpp::Object<dto::implant> &dto);
  oatpp::Object<dto::implant> getById(const oatpp::String &id);
  oatpp::Object<dto::page<oatpp::Object<dto::implant>>> getAll(const oatpp::UInt32 &offset, const oatpp::UInt32 &limit);
  bool exists(oatpp::String id);
  oatpp::Object<dto::status> deleteById(oatpp::String &id);
  void updateById(oatpp::String &id, const oatpp::Int32 &architecture, const oatpp::Int32 &operating_system, const oatpp::Int32 &process_id,
                  oatpp::String &process_user, oatpp::String &process_path, oatpp::String &system_name, oatpp::String &system_addrs);
};

}

#endif //MOONSHINE_SERVER_SERVICE_IMPLANT_H_
