#include "task.h"

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <util/string.hpp>

#include "services/ws.h"

namespace services {

oatpp::Object<dto::page<oatpp::Object<dto::task>>> task::getAllByImplant(const oatpp::String &implant,
                                                                         const oatpp::UInt32 &offset,
                                                                         const oatpp::UInt32 &limit) {

  auto dbResult = _database->getTasksByImplant(implant, offset, limit);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve tasks");
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::task>>>();

  auto page = dto::page<oatpp::Object<dto::task>>::createShared();
  page->offset = offset;
  page->limit = limit;
  page->count = items->size();
  page->items = items;

  return page;
}

oatpp::Object<dto::page<oatpp::Object<dto::task>>> task::getAllByImplantWhereStatus(const oatpp::String &implant,
                                                                                    const oatpp::Enum<dto::task_status>::AsNumber& status,
                                                                                    const oatpp::UInt32 &offset,
                                                                                    const oatpp::UInt32 &limit) {

  auto dbResult = _database->getTasksByImplantWhereStatus(implant, status, offset, limit);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve tasks");
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::task>>>();

  auto page = dto::page<oatpp::Object<dto::task>>::createShared();
  page->offset = offset;
  page->limit = limit;
  page->count = items->size();
  page->items = items;

  return page;
}

oatpp::Object<dto::task> task::create(const oatpp::Object<dto::task> &dto) {
  auto dbResult = _database->createTask(dto->implant, dto::task_status::SUBMITTED, dto->script, dto->success, dto->output);
  if (!dbResult->isSuccess()) spdlog::error("Could not create task");
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto rowid = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
  auto task = getByRowId(oatpp::Int64(rowid));
  spdlog::debug("Task '{}' created for '{}'", task->id->c_str(), task->implant->c_str());

  auto t = std::make_shared<shared::message>(task->id->c_str(), "task", "", util::string::convert(util::base64::decode(task->script->c_str())));
  _dispatcher->send(task->implant->c_str(), t);

  websocket::broadcast(nlohmann::json{
      {"source", "task"},
      {"id", task->id->c_str()},
      {"implant", task->implant->c_str()},
      {"event", oatpp::Enum<dto::task_status>::getEntryByValue(task->status).name.std_str()},
  }.dump());

  return task;
}

oatpp::Object<dto::task> task::getByRowId(v_int64 rowid) {
  auto dbResult = _database->getTaskByRowId(rowid);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Object not found");

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::task>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
}

oatpp::Object<dto::task> task::getById(oatpp::String &id) {
  auto dbResult = _database->getTaskById(id);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve task with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Object not found");

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::task>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
}

bool task::exists(oatpp::String id) {
  auto dbResult = _database->getTaskById(id);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve task with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  if (!dbResult->hasMoreToFetch())
    return false;

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::task>>>();
  if (result->size() != 1)
    return false;

  return true;
}

bool task::update_status(oatpp::String id, dto::task_status task_status, oatpp::Object<dto::implant> implant) {
  auto dbResult = _database->updateTaskWithStatus(id, task_status);

  websocket::broadcast(nlohmann::json{
      {"source", "task"},
      {"id", id->c_str()},
      {"implant", implant->id->c_str()},
      {"event", oatpp::Enum<dto::task_status>::getEntryByValue(task_status).name.std_str()},
  }.dump());

  return dbResult->isSuccess();
}

bool task::update(oatpp::String id, dto::task_status status, dto::task_success success, const std::string &output) {
  auto dbResult = _database->updateTask(id, status, success, util::base64::encode(output));

  oatpp::Object<dto::task> task = getById(id);

  websocket::broadcast(nlohmann::json{
      {"source", "task"},
      {"id", id->c_str()},
      {"implant", task->implant->c_str()},
      {"event", oatpp::Enum<dto::task_status>::getEntryByValue(status).name.std_str()},
  }.dump());

  return dbResult->isSuccess();
}

}
