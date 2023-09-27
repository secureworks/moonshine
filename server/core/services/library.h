#ifndef MOONSHINE_SERVER_SERVICE_LIBRARY_H_
#define MOONSHINE_SERVER_SERVICE_LIBRARY_H_

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/orm/Connection.hpp>
#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

#include "db/client.h"
#include "dto/library.h"
#include "dto/page.h"
#include "dto/status.h"

namespace services {

struct library {

 private:
  typedef oatpp::web::protocol::http::Status Status;
  OATPP_COMPONENT(std::shared_ptr<db::client>, _database); // Inject database component

  oatpp::Object<dto::library> getByRowId(v_int64 rowid);

 public:
  oatpp::Object<dto::library> create(oatpp::String &owner_id, const oatpp::Object<dto::library> &dto);
  oatpp::Vector<oatpp::Object<dto::library>> getAllByOwnerId(oatpp::String &listener_id);
  oatpp::Object<dto::status> deleteByOwnerId(oatpp::String &listener_id);
};

}

#endif //MOONSHINE_SERVER_SERVICE_LIBRARY_H_
