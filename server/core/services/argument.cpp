#include "argument.h"

namespace services {

oatpp::Vector<oatpp::Object<dto::argument>> argument::getAllByOwnerId(oatpp::String &owner_id) {
  auto dbResult = _database->getArgumentsByOwnerId(owner_id);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  return dbResult->fetch<oatpp::Vector<oatpp::Object<dto::argument>>>();
}

oatpp::Object<dto::status> argument::deleteByOwnerId(oatpp::String &owner_id) {
  auto dbResult = _database->deleteArgumentsByOwnerId(owner_id);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Arguments were successfully deleted";
  return status;
}

oatpp::Object<dto::argument> argument::create(oatpp::String &owner_id, v_int32 position, const oatpp::Object<dto::argument> &dto) {
  auto dbResult = _database->createArgument(owner_id, position, dto);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto rowid = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
  return getByRowId(rowid);
}

oatpp::Object<dto::argument> argument::getByRowId(v_int64 rowid) {
  auto dbResult = _database->getArgumentByRowId(rowid);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Argument not found");

  auto result = dbResult->template fetch<oatpp::Vector<oatpp::Object<dto::argument>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
};
}
