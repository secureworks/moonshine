#include "listener.h"

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/orm/Connection.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include <util/string.hpp>
#include "services/ws.h"

namespace services {

oatpp::Object<dto::listener> listener::createWithChildren(const oatpp::Object<dto::listener> &dto) {
  auto dbResult = _database->createListener(dto->name, dto->status, dto->package, dto->implant_package, dto->implant_connection_string);
  if (!dbResult->isSuccess()) spdlog::error("Could not create listener with name '{}'", dto->name->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto rowid = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
  auto listener_dto = getByRowId(rowid);
  dto->id = listener_dto->id;

  int i = 1;
  for (auto &argument_dto : *dto->arguments) {
    _argument_service.create(dto->id, i, argument_dto);
    i++;
  }

  spdlog::info("Created listener with id '{}'", dto->id->c_str());

  websocket::broadcast(nlohmann::json{
      {"source", "listener"},
      {"id", dto->id->c_str()},
      {"event", "created"},
  }.dump());

  return dto;
}

oatpp::Object<dto::page<oatpp::Object<dto::listener>>> listener::getAll(const oatpp::UInt32 &offset,
                                                                        const oatpp::UInt32 &limit) {
  auto dbResult = _database->getListeners(offset, limit);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve listeners");
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::listener>>>();

  auto page = dto::page<oatpp::Object<dto::listener>>::createShared();
  page->offset = offset;
  page->limit = limit;
  page->count = items->size();
  page->items = items;

  return page;
};

oatpp::Object<dto::listener> listener::getByIdWithChildren(oatpp::String &id) {
  auto dto = getById(id);
  dto->arguments = _argument_service.getAllByOwnerId(id);
  return dto;
}

oatpp::Object<dto::page<oatpp::Object<dto::listener>>> listener::getAllWithChildren(const oatpp::UInt32 &offset,
                                                                                    const oatpp::UInt32 &limit) {
  auto page = getAll(offset, limit);
  for (auto &listener_dto : *page->items) {
    listener_dto->arguments = _argument_service.getAllByOwnerId(listener_dto->id);
  }
  return page;
}

oatpp::Object<dto::status> listener::deleteByIdWithChildren(oatpp::String &id) {
  try {
    stopById(id);
  }
  catch (const oatpp::web::protocol::http::HttpError &e) {}
  _argument_service.deleteByOwnerId(id);
  return deleteById(id);
}

oatpp::Object<dto::status> listener::deleteById(oatpp::String &id) {
  auto dbResult = _database->deleteListenerById(id);
  if (!dbResult->isSuccess()) spdlog::error("Could not delete listener with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  spdlog::info("Deleted listener with id '{}'", id->c_str());

  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Listener was deleted";

  websocket::broadcast(nlohmann::json{
      {"source", "listener"},
      {"id", id->c_str()},
      {"event", "deleted"},
  }.dump());

  return status;
};

oatpp::Object<dto::status> listener::startById(oatpp::String &id) {
  if (auto it = _listeners.find(std::string(id)); it == _listeners.end()) {
    auto listener_dto = getByIdWithChildren(id);

    auto package = util::base64::decode(listener_dto->package->c_str());

    std::vector<std::string> arguments{};
    for (auto &argument_dto : *listener_dto->arguments) {
      const std::vector<unsigned char> arg_bytes = util::base64::decode(argument_dto->data->c_str());
      const std::string arg{reinterpret_cast<const char *>(&arg_bytes[0]), arg_bytes.size()};
      arguments.push_back(arg);
    }

    auto lstner =
        std::make_shared<lua::listener>(listener_dto->id->c_str(),
                                        listener_dto->name->c_str(),
                                        package,
                                        arguments);

    spdlog::trace("Attempting to start listener with id '{}'", listener_dto->id->c_str());
    bool success = lstner->start();
    if (success)
      spdlog::info("Started listener with id '{}'", listener_dto->id->c_str());
    else
      spdlog::error("Could not start listener with id '{}'", listener_dto->id->c_str());//, lstner->output());
    OATPP_ASSERT_HTTP(success, Status::CODE_500, lstner->output());

    _listeners[lstner->id] = lstner;

    _database->updateListenerWithStatus(listener_dto->id, (v_uint32) 1);
  }

  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Listener started";

  websocket::broadcast(nlohmann::json{
      {"source", "listener"},
      {"id", id->c_str()},
      {"event", "started"},
  }.dump());

  return status;
}

oatpp::Object<dto::status> listener::stopById(oatpp::String &id, bool wait) {
  if (auto it = _listeners.find(std::string(id)); it != _listeners.end()) {
    if (it->second->is_running()) {
      spdlog::trace("Attempting to stop listener with id '{}'", it->second->id);
      it->second->stop(wait);
    }

    if (!it->second->is_running()) {
      spdlog::info("Stopped listener with id '{}'", it->second->id);
      _database->updateListenerWithStatus(id, (v_uint32) 0);
    }
    else {
      spdlog::error("Could not stop listener with id '{}', {}", it->second->id, it->second->output());
      OATPP_ASSERT_HTTP(false, Status::CODE_500, fmt::format("Could not stop listener with id '{}', {}", it->second->id, it->second->output()));
    }
  }
  else {
    OATPP_ASSERT_HTTP(false, Status::CODE_404, "Listener not found");
  }

  _listeners.erase(std::string(id));

  auto status = dto::status::createShared();
  status->text = "OK";
  status->code = 200;
  status->message = "Listener stopped";

  websocket::broadcast(nlohmann::json{
      {"source", "listener"},
      {"id", id->c_str()},
      {"event", "stopped"},
  }.dump());

  return status;
}

oatpp::Object<dto::listener> listener::getById(oatpp::String &id) {
  auto dbResult = _database->getListenerById(id);
  if (!dbResult->isSuccess()) spdlog::error("Could not retrieve listener with id = {}", id->c_str());
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Listener not found");

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::listener>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
}

oatpp::Object<dto::listener> listener::getByRowId(v_int64 rowid) {
  auto dbResult = _database->getListenerByRowId(rowid);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Listener not found");

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::listener>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return result[0];
}

bool listener::is_running(oatpp::String &id) {
  if (auto it = _listeners.find(std::string(id)); it != _listeners.end()) {
    if (it->second->is_running()) {
      return true;
    }
  }
  return false;
}

void listener::init() {
  auto dbResult = _database->getListeners((v_uint32)0, 1000);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::listener>>>();
  spdlog::info("{} listeners registered", items->size());

  dbResult = _database->getStartedListeners();
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  items = dbResult->fetch<oatpp::Vector<oatpp::Object<dto::listener>>>();

  if (!items->empty())
    spdlog::info("Restarting {} previously running listeners...", items->size());
  for (auto &dto : *items) {
    try {
      startById(dto->id);
    }
    catch (std::exception &e) {
      _database->updateListenerWithStatus(dto->id, (v_uint32) 0);
    }
  }
}

}
