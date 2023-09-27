#ifndef MOONSHINE_SERVER_CONTROLLER_LISTENER_H_
#define MOONSHINE_SERVER_CONTROLLER_LISTENER_H_

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "services/listener.h"

namespace controllers {

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

struct listener : public oatpp::web::server::api::ApiController {

  explicit listener(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

  static std::shared_ptr<listener> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
    return std::make_shared<listener>(objectMapper);
  }

  ENDPOINT_INFO(createListener) {
    info->summary = "Create Listener";

    info->addConsumes < Object < dto::listener >> ("application/json");
    info->addResponse < Object < dto::listener >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
  }
  ENDPOINT("POST", "listeners", createListener, BODY_DTO(Object < dto::listener > , dto)) {
    return createDtoResponse(Status::CODE_200, _listenerService->createWithChildren(dto));
  }

  ENDPOINT_INFO(getListener) {
    info->summary = "Get Listener by Id";
    info->addResponse < Object < dto::listener >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Listener Identifier";
  }
  ENDPOINT("GET", "listeners/{id}", getListener, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _listenerService->getByIdWithChildren(const_cast<String &>(id)));
  }

  ENDPOINT_INFO(getListeners) {
    info->summary = "Get Listeners";
    info->addResponse < Object < dto::listeners_page >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
  }
  ENDPOINT("GET", "listeners/offset/{offset}/limit/{limit}", getListeners, PATH(UInt32, offset), PATH(UInt32, limit)) {
    return createDtoResponse(Status::CODE_200, _listenerService->getAllWithChildren(offset, limit));
  }

  ENDPOINT_INFO(deleteListener) {
    info->summary = "Delete Listener";
    info->addResponse < Object < dto::status >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Listener Identifier";
  }
  ENDPOINT("DELETE", "listeners/{id}", deleteListener, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _listenerService->deleteByIdWithChildren(const_cast<String &>(id)));
  }

  ENDPOINT_INFO(startListener) {
    info->summary = "Start Listener by Id";
    info->addResponse < Object < dto::status >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Listener Identifier";
  }
  ENDPOINT("GET", "listeners/{id}/start", startListener, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _listenerService->startById(const_cast<String &>(id)));
  }

  ENDPOINT_INFO(stopListener) {
    info->summary = "Stop Listener by Id";
    info->addResponse < Object < dto::status >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Listener Identifier";
  }
  ENDPOINT("GET", "listeners/{id}/stop", stopListener, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _listenerService->stopById(const_cast<String &>(id)));
  }

 private:
  services::listener *_listenerService = services::listener::instance(); // Create listener service.
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

}

#endif // MOONSHINE_SERVER_CONTROLLER_LISTENER_H_
