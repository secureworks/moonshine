#include "implant.h"

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "services/ws.h"

namespace services {

  oatpp::Object<dto::implant> implant::getByRowId(v_int64 rowid) {
    auto dbResult = _database->getImplantByRowId(rowid);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Implant not found");

    auto result = dbResult->template fetch<oatpp::Vector<oatpp::Object<dto::implant>>>();
    OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

    return result[0];
  }

  oatpp::Object<dto::implant> implant::getById(const oatpp::String& id) {
    auto dbResult = _database->getImplantById(id);
    if (!dbResult->isSuccess()) spdlog::error("Could not retrieve implant with id = {}", id->c_str());
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Implant not found");

    auto result = dbResult->template fetch<oatpp::Vector<oatpp::Object<dto::implant>>>();
    OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

    return result[0];
  }

  oatpp::Object<dto::page<oatpp::Object<dto::implant>>> implant::getAll(const oatpp::UInt32& offset, const oatpp::UInt32& limit) {
    auto dbResult = _database->getImplants(offset, limit);
    if (!dbResult->isSuccess()) spdlog::error("Could not retrieve implants");
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto items = dbResult->template fetch<oatpp::Vector<oatpp::Object<dto::implant>>>();

    auto page = dto::page<oatpp::Object<dto::implant>>::createShared();
    page->offset = offset;
    page->limit = limit;
    page->count = items->size();
    page->items = items;

    return page;
  }

  bool implant::exists(oatpp::String id) {
    auto dbResult = _database->getImplantById(id);

    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    if (!dbResult->hasMoreToFetch())
      return false;

    auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::implant>>>();
    if (result->size() != 1)
      return false;

    return true;
  }

  oatpp::Object<dto::implant> implant::create(const oatpp::Object<dto::implant>& dto) {
    auto dbResult = _database->createImplant(dto);
    if (!dbResult->isSuccess()) spdlog::error("Could not create implant with id = {}", dto->id->c_str());
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto rowid = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    auto implant = getByRowId(rowid);
    spdlog::info("Implant with id '{}' has been created", implant->id->c_str());

    websocket::broadcast(nlohmann::json{
        {"source", "implant"},
        {"id", implant->id->c_str()},
        {"event", "created"},
    }.dump());

    return implant;
  }

oatpp::Object<dto::status> implant::deleteById(oatpp::String &id) {
  auto dbResult = _database->deleteImplantById(id);
  if (!dbResult->isSuccess()) spdlog::error("Could not delete implant with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  spdlog::info("Implant with id '{}' has been deleted", id->c_str());

  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Implant was deleted";

  websocket::broadcast(nlohmann::json{
      {"source", "implant"},
      {"id", id->c_str()},
      {"event", "deleted"},
  }.dump());

  return status;
}

void implant::updateById(oatpp::String &id, const oatpp::Int32 &architecture, const oatpp::Int32 &operating_system, const oatpp::Int32 &process_id,
                         oatpp::String &process_user, oatpp::String &process_path, oatpp::String &system_name, oatpp::String &system_addrs) {
  auto dbResult = _database->updateImplant(id, architecture, operating_system, process_id, process_user, process_path, system_name, system_addrs);
  if (!dbResult->isSuccess()) spdlog::error("Could not update implant with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  spdlog::info("Implant with id '{}' has been updated", id->c_str());
}

} // service
