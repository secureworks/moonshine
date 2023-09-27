#include "job.h"

#include <nlohmann/json.hpp>

#include "services/ws.h"
#include "worker.h"

namespace services {

oatpp::Object<dto::page<oatpp::Object<dto::job>>> job::getAllByImplant(const oatpp::String &implant,
                                                                       const oatpp::UInt32 &offset,
                                                                       const oatpp::UInt32 &limit) {

  auto dbResult = _database->getJobsByImplant(implant, offset, limit);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve jobs");
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::job>>>();

  auto page = dto::page<oatpp::Object<dto::job>>::createShared();
  page->offset = offset;
  page->limit = limit;
  page->count = items->size();
  page->items = items;

  return page;
}

oatpp::Object<dto::page<oatpp::Object<dto::job>>> job::getAllByImplantWhereStatus(const oatpp::String &implant,
                                                                                  const oatpp::Enum<dto::job_status>::AsNumber& status,
                                                                                  const oatpp::UInt32 &offset,
                                                                                  const oatpp::UInt32 &limit) {

  auto dbResult = _database->getJobsByImplantWhereStatus(implant, status, offset, limit);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve jobs");
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::job>>>();

  auto page = dto::page<oatpp::Object<dto::job>>::createShared();
  page->offset = offset;
  page->limit = limit;
  page->count = items->size();
  page->items = items;

  return page;
}

oatpp::Object<dto::job> job::createWithChildren(const oatpp::Object<dto::job> &dto) {
  auto dbResult = _database->createJob(dto->name, dto->implant, dto::job_status::SUBMITTED, dto->package, dto->success, dto->output);
  if (!dbResult->isSuccess()) spdlog::error("Could not create task");
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto rowid = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
  auto job = getByRowId(oatpp::Int64(rowid));

  std::vector<std::string> arguments;
  int i = 1;
  for (auto &argument_dto : *dto->arguments) {
    _argument_service.create(dto->id, i, argument_dto);
    const std::string arg = util::string::convert(util::base64::decode(argument_dto->data->c_str()));
    arguments.push_back(arg);
    i++;
  }

  spdlog::debug("Job '{}' created for '{}'", job->id->c_str(), job->implant->c_str());

  shared::abstract::worker desc(job->id->c_str(), job->name->c_str(), util::base64::decode(job->package->c_str()), arguments);
  auto t = std::make_shared<shared::message>(job->id->c_str(), "job_handler", "execute", shared::abstract::worker::serialize(desc));
  _dispatcher->send(job->implant->c_str(), t);

  websocket::broadcast(nlohmann::json{
      {"source", "job"},
      {"id", job->id->c_str()},
      {"implant", job->implant->c_str()},
      {"event", oatpp::Enum<dto::job_status>::getEntryByValue(job->status).name.std_str()},
  }.dump());

  return job;
}

oatpp::Object<dto::job> job::getByRowId(v_int64 rowid) {
  auto dbResult = _database->getJobByRowId(rowid);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Object not found");

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::job>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
}

oatpp::Object<dto::job> job::getById(oatpp::String &id) {
  auto dbResult = _database->getJobById(id);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve job with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Object not found");

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::job>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
}

bool job::existsById(oatpp::String id) {
  auto dbResult = _database->getJobById(id);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve job with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  if (!dbResult->hasMoreToFetch())
    return false;

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::job>>>();
  if (result->size() != 1)
    return false;

  return true;
}

bool job::update_status(oatpp::String id, dto::job_status job_status, oatpp::String implant) {
  auto dbResult = _database->updateJobWithStatus(id, job_status);

  websocket::broadcast(nlohmann::json{
      {"source", "job"},
      {"id", id->c_str()},
      {"implant", implant->c_str()},
      {"event", oatpp::Enum<dto::job_status>::getEntryByValue(job_status).name.std_str()},
  }.dump());

  return dbResult->isSuccess();
}

bool job::update(oatpp::String id, dto::job_status job_status, dto::job_success success, const std::string &output) {
  auto dbResult = _database->updateJob(id, job_status, success, util::base64::encode(output));

  oatpp::Object<dto::job> job = getById(id);

  websocket::broadcast(nlohmann::json{
      {"source", "job"},
      {"id", id->c_str()},
      {"implant", job->implant->c_str()},
      {"event", oatpp::Enum<dto::job_status>::getEntryByValue(job_status).name.std_str()},
  }.dump());

  return dbResult->isSuccess();
}

bool job::update_output(oatpp::String id, const std::string &output) {
  oatpp::Object<dto::job> job = getById(id);

  auto appended = util::base64::encode(util::string::convert(util::base64::decode(job->output)).append(output));

  auto dbResult = _database->updateJobOutput(id, appended);

  websocket::broadcast(nlohmann::json{
      {"source", "job"},
      {"id", id->c_str()},
      {"implant", job->implant->c_str()},
      {"event", oatpp::Enum<dto::job_status>::getEntryByValue(job->status).name.std_str()},
  }.dump());

  return dbResult->isSuccess();
}

oatpp::Object<dto::page<oatpp::Object<dto::job>>> job::getAllWithChildrenByImplant(const oatpp::String &implant,
                                                                                   const oatpp::UInt32 &offset,
                                                                                   const oatpp::UInt32 &limit) {
  auto page = getAllByImplant(implant, offset, limit);
  for (auto &dto : *page->items) {
    dto->arguments = _argument_service.getAllByOwnerId(dto->id);
  }
  return page;
}

oatpp::Object<dto::page<oatpp::Object<dto::job>>> job::getAllWithChildrenByImplantWhereStatus(const oatpp::String &implant,
                                                                                              const oatpp::Enum<dto::job_status>::AsNumber& status,
                                                                                              const oatpp::UInt32 &offset,
                                                                                              const oatpp::UInt32 &limit) {
  auto page = getAllByImplantWhereStatus(implant, status, offset, limit);
  for (auto &dto : *page->items) {
    dto->arguments = _argument_service.getAllByOwnerId(dto->id);
  }
  return page;
}

oatpp::Object<dto::job> job::getByIdWithChildren(oatpp::String id) {
  auto dto = getById(id);
  dto->arguments = _argument_service.getAllByOwnerId(id);
  return dto;
}

oatpp::Object<dto::status> job::stopById(oatpp::String id) {
  oatpp::Object<dto::job> job = getById(id);

  auto message = std::make_shared<shared::message>(job->id->c_str(), "job_handler", "stop", job->id->c_str());
  _dispatcher->send(job->implant->c_str(), message);

  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Job stop request has been sent";

  return status;
}

oatpp::Object<dto::status> job::commandById(oatpp::String id, oatpp::String command, oatpp::String data) {
  oatpp::Object<dto::job> job = getById(id);

  auto message = std::make_shared<shared::message>(job->id->c_str(), "server", job->id->c_str(), "job", command->c_str(), data->c_str());
  _dispatcher->send(job->implant->c_str(), message);

  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Job command has been sent";

  return status;
}

} // services
