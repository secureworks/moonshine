#include "library.h"

namespace services {

oatpp::Vector<oatpp::Object<dto::library>> library::getAllByOwnerId(oatpp::String &listener_id) {
  auto dbResult = _database->getLibrariesByOwnerId(listener_id);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  return dbResult->fetch<oatpp::Vector<oatpp::Object<dto::library>>>();
}

oatpp::Object<dto::status> library::deleteByOwnerId(oatpp::String &listener_id) {
  auto dbResult = _database->deleteLibrariesByOwnerId(listener_id);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Libraries were successfully deleted";
  return status;
}

oatpp::Object<dto::library> library::create(oatpp::String &owner_id, const oatpp::Object<dto::library> &dto) {
  auto dbResult = _database->createLibrary(owner_id, dto);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto rowid = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
  return getByRowId(rowid);
}

oatpp::Object<dto::library> library::getByRowId(v_int64 rowid) {
  auto dbResult = _database->getLibraryByRowId(rowid);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Argument not found");

  auto result = dbResult->template fetch<oatpp::Vector<oatpp::Object<dto::library>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
};
}
