#ifndef MOONSHINE_SERVER_CONTROLLER_SERVICE_H_
#define MOONSHINE_SERVER_CONTROLLER_SERVICE_H_

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "services/service.h"

namespace controllers {

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

struct service : public oatpp::web::server::api::ApiController {

  explicit service(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

  static std::shared_ptr<service> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
    return std::make_shared<service>(objectMapper);
  }

  ENDPOINT_INFO(createService) {
    info->summary = "Create Service";

    info->addConsumes < Object < dto::service >> ("application/json");
    info->addResponse < Object < dto::service >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
  }
  ENDPOINT("POST", "services", createService, BODY_DTO(Object < dto::service > , dto)) {
    return createDtoResponse(Status::CODE_200, _serviceService->createWithChildren(dto));
  }

  ENDPOINT_INFO(getService) {
    info->summary = "Get Service by Id";
    info->addResponse < Object < dto::service >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Service Identifier";
  }
  ENDPOINT("GET", "services/{id}", getService, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _serviceService->getByIdWithChildren(const_cast<String &>(id)));
  }

  ENDPOINT_INFO(getServices) {
    info->summary = "Get Services";
    info->addResponse < Object < dto::services_page >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
  }
  ENDPOINT("GET", "services/offset/{offset}/limit/{limit}", getServices, PATH(UInt32, offset), PATH(UInt32, limit)) {
    return createDtoResponse(Status::CODE_200, _serviceService->getAllWithChildren(offset, limit));
  }

  ENDPOINT_INFO(deleteService) {
    info->summary = "Delete Service";
    info->addResponse < Object < dto::status >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Service Identifier";
  }
  ENDPOINT("DELETE", "services/{id}", deleteService, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _serviceService->deleteByIdWithChildren(const_cast<String &>(id)));
  }

  ENDPOINT_INFO(startService) {
    info->summary = "Start Service by Id";
    info->addResponse < Object < dto::status >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Service Identifier";
  }
  ENDPOINT("GET", "services/{id}/start", startService, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _serviceService->startById(const_cast<String &>(id)));
  }

  ENDPOINT_INFO(stopService) {
    info->summary = "Stop Service by Id";
    info->addResponse < Object < dto::status >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Service Identifier";
  }
  ENDPOINT("GET", "services/{id}/stop", stopService, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _serviceService->stopById(const_cast<String &>(id)));
  }

 private:
  services::service *_serviceService = services::service::instance(); // Create service service.
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

}

#endif // MOONSHINE_SERVER_CONTROLLER_SERVICE_H_
