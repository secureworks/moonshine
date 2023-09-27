#ifndef MOONSHINE_SERVER_SERVICE_ARGUMENT_H_
#define MOONSHINE_SERVER_SERVICE_ARGUMENT_H_

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/orm/Connection.hpp>
#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

#include "db/client.h"
#include "dto/argument.h"
#include "dto/page.h"
#include "dto/status.h"

namespace services {

struct argument {

 private:
  typedef oatpp::web::protocol::http::Status Status;
  OATPP_COMPONENT(std::shared_ptr<db::client>, _database); // Inject database component

 private:
  oatpp::Object<dto::argument> getByRowId(v_int64 rowid);

 public:
  oatpp::Object<dto::argument> create(oatpp::String &owner_id, v_int32 position, const oatpp::Object<dto::argument> &dto);
  oatpp::Vector<oatpp::Object<dto::argument>> getAllByOwnerId(oatpp::String &owner_id);
  oatpp::Object<dto::status> deleteByOwnerId(oatpp::String &owner_id);
};

}

#endif //MOONSHINE_SERVER_SERVICE_ARGUMENT_H_
