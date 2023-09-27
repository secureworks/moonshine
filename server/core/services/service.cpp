#include "service.h"

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/orm/Connection.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "services/ws.h"
#include "worker.h"

namespace services {

oatpp::Object<dto::service> service::createWithChildren(const oatpp::Object<dto::service> &dto) {
  auto dbResult = _database->createService(dto->name, dto->status,dto->package);
  if (!dbResult->isSuccess()) spdlog::error("Could not create service with name '{}'", dto->name->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto rowid = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
  auto service_dto = getByRowId(rowid);
  dto->id = service_dto->id;

  int i = 1;
  for (auto &argument_dto : *dto->arguments) {
    _argument_service.create(dto->id, i, argument_dto);
    i++;
  }

  spdlog::info("Created service with id '{}'", dto->id->c_str());

  websocket::broadcast(nlohmann::json{
      {"source", "service"},
      {"id", dto->id->c_str()},
      {"event", "created"},
  }.dump());

  return dto;
}

oatpp::Object<dto::page<oatpp::Object<dto::service>>> service::getAll(const oatpp::UInt32 &offset,
                                                                        const oatpp::UInt32 &limit) {
  auto dbResult = _database->getServices(offset, limit);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve services");
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::service>>>();

  auto page = dto::page<oatpp::Object<dto::service>>::createShared();
  page->offset = offset;
  page->limit = limit;
  page->count = items->size();
  page->items = items;

  return page;
};

oatpp::Object<dto::service> service::getByIdWithChildren(oatpp::String &id) {
  auto dto = getById(id);
  dto->arguments = _argument_service.getAllByOwnerId(id);
  return dto;
}

oatpp::Object<dto::page<oatpp::Object<dto::service>>> service::getAllWithChildren(const oatpp::UInt32 &offset,
                                                                                    const oatpp::UInt32 &limit) {
  auto page = getAll(offset, limit);
  for (auto &service_dto : *page->items) {
    service_dto->arguments = _argument_service.getAllByOwnerId(service_dto->id);
  }
  return page;
}

oatpp::Object<dto::status> service::deleteByIdWithChildren(oatpp::String &id) {
  try {
    stopById(id);
  }
  catch (const oatpp::web::protocol::http::HttpError &e) {}
  _library_service.deleteByOwnerId(id);
  _argument_service.deleteByOwnerId(id);
  return deleteById(id);
}

oatpp::Object<dto::status> service::deleteById(oatpp::String &id) {
  auto dbResult = _database->deleteServiceById(id);
  if (!dbResult->isSuccess()) spdlog::error("Could not delete service with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  spdlog::info("Deleted service with id '{}'", id->c_str());

  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Service was deleted";

  websocket::broadcast(nlohmann::json{
      {"source", "service"},
      {"id", id->c_str()},
      {"event", "deleted"},
  }.dump());

  return status;
};

oatpp::Object<dto::status> service::startById(oatpp::String &id) {

  auto service_dto = getByIdWithChildren(id);

  std::vector<std::string> arguments{};
  for (auto &argument_dto : *service_dto->arguments) {
    const std::vector<unsigned char> arg_bytes = util::base64::decode(argument_dto->data->c_str());
    const std::string arg{reinterpret_cast<const char *>(&arg_bytes[0]), arg_bytes.size()};
    arguments.push_back(arg);
  }

  auto s = std::make_shared<lua::service>(
    service_dto->id->c_str(),
    service_dto->name->c_str(),
    util::base64::decode(service_dto->package->c_str()),
    arguments
  );

  spdlog::trace("Attempting to start service with id '{}'", service_dto->id->c_str());
  bool success = s->start();
  if (success)
    spdlog::info("Started service with id '{}'", service_dto->id->c_str());
  else
    spdlog::error("Could not start service with id '{}', {}", service_dto->id->c_str(), s->output());
  OATPP_ASSERT_HTTP(success, Status::CODE_500, s->output());

  _services[s->id()] = s;

  _database->updateServiceWithStatus(service_dto->id, (v_uint32) 1);

  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Service started";

  websocket::broadcast(nlohmann::json{
      {"source", "service"},
      {"id", s->id()},
      {"event", "started"},
  }.dump());

  return status;
}

oatpp::Object<dto::status> service::stopById(oatpp::String &id, bool wait) {

  if (auto it = _services.find(std::string(id)); it != _services.end()) {
    if (it->second->is_running()) {
      spdlog::trace("Attempting to stop service with id '{}'", it->second->id());
      it->second->stop(wait);
    }

    if (!it->second->is_running()) {
      spdlog::info("Stopped service with id '{}'", it->second->id());
      _database->updateListenerWithStatus(id, (v_uint32) 0);
    }
    else {
      spdlog::error("Could not stop service with id '{}', {}", it->second->id(), it->second->output());
      OATPP_ASSERT_HTTP(false, Status::CODE_500, fmt::format("Could not stop service with id '{}', {}", it->second->id(), it->second->output()));
    }
  }
  else {
    OATPP_ASSERT_HTTP(false, Status::CODE_404, "Service not found");
  }

  _services.erase(std::string(id));

  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Service stopped";

  websocket::broadcast(nlohmann::json{
      {"source", "service"},
      {"id", id->c_str()},
      {"event", "stopped"},
  }.dump());

  return status;
}

oatpp::Object<dto::service> service::getById(oatpp::String &id) {
  auto dbResult = _database->getServiceById(id);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve service with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Service not found");

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::service>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
};

oatpp::Object<dto::service> service::getByRowId(v_int64 rowid) {
  auto dbResult = _database->getServiceByRowId(rowid);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Service not found");

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::service>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
};

bool service::is_running(oatpp::String &id) {
  if (auto it = _services.find(std::string(id)); it != _services.end()) {
    if (it->second->is_running()) {
      return true;
    }
  }
  return false;
}

void service::init() {
  auto dbResult = _database->getStartedServices();
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::service>>>();

  if (!items->empty())
    spdlog::info("Restarting {} previously running services...", items->size());
  for (auto &dto : *items) {
    try {
      startById(dto->id);
    }
    catch (std::exception &e) {
      _database->updateServiceWithStatus(dto->id, (v_uint32) 0);
    }
  }
}

}
