#ifndef MOONSHINE_SERVER_CONTROLLER_IMPLANT_H_
#define MOONSHINE_SERVER_CONTROLLER_IMPLANT_H_

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "services/implant.h"
#include "services/task.h"
#include "services/job.h"

namespace controllers {

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

struct implant : public oatpp::web::server::api::ApiController {

  explicit implant(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

  static std::shared_ptr<implant> createShared(
      OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                      objectMapper) // Inject objectMapper component here as default parameter
  ) {
    return std::make_shared<implant>(objectMapper);
  }

  ENDPOINT_INFO(getImplant) {
    info->summary = "Get Implant by Id";
    info->addResponse < Object < dto::implant >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Implant Identifier";
  }
  ENDPOINT("GET", "implants/{id}", getImplant, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _implantService.getById(const_cast<String &>(id)));
  }

  ENDPOINT_INFO(getImplants) {
    info->summary = "Get Implants";
    info->addResponse < Object < dto::implants_page >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
  }
  ENDPOINT("GET", "implants/offset/{offset}/limit/{limit}", getImplants, PATH(UInt32, offset), PATH(UInt32, limit)) {
    return createDtoResponse(Status::CODE_200, _implantService.getAll(offset, limit));
  }

  ENDPOINT_INFO(getTasksForImplant) {
    info->summary = "Get Tasks for Implant";
    info->addResponse < Object < dto::tasks_page >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Implant Identifier";
  }
  ENDPOINT("GET",
           "implants/{id}/tasks/offset/{offset}/limit/{limit}",
           getTasksForImplant,
           PATH(String, id),
           PATH(UInt32, offset),
           PATH(UInt32, limit)) {
    return createDtoResponse(Status::CODE_200, _taskService.getAllByImplant(const_cast<String &>(id), offset, limit));
  }

  ENDPOINT_INFO(getTasksForImplantWithStatus) {
    info->summary = "Get Tasks for Implant By Status";
    info->addResponse < Object < dto::tasks_page >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Implant Identifier";
    info->pathParams["status"].description = "Status Type";
  }
  ENDPOINT("GET",
           "implants/{id}/tasks/status/{status}/offset/{offset}/limit/{limit}",
           getTasksForImplantWithStatus,
           PATH(String, id),
           PATH(Enum<dto::task_status>::AsNumber, status),
           PATH(UInt32, offset),
           PATH(UInt32, limit)) {
    return createDtoResponse(Status::CODE_200,
                             _taskService.getAllByImplantWhereStatus(const_cast<String &>(id), status, offset, limit));
  }

  ENDPOINT_INFO(createTaskForImplant) {
    info->summary = "Create Task For Implant";

    info->addConsumes < Object < dto::task >> ("application/json");
    info->addResponse < Object < dto::task >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Implant Identifier";
  }
  ENDPOINT("POST",
           "implants/{id}/tasks",
           createTaskForImplant,
           PATH(String, id),
           BODY_DTO(Object < dto::task > , dto)) {
    dto->implant = id;
    return createDtoResponse(Status::CODE_200, _taskService.create(dto));
  }

  ENDPOINT_INFO(deleteImplant) {
    info->summary = "Delete Implant";
    info->addResponse < Object < dto::status >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Implant Identifier";
  }
  ENDPOINT("DELETE", "implants/{id}", deleteImplant, PATH(String, id)) {
    return createDtoResponse(Status::CODE_200, _implantService.deleteById(const_cast<String &>(id)));
  }

  ENDPOINT_INFO(getJobsForImplant) {
    info->summary = "Get Jobs for Implant";
    info->addResponse < Object < dto::jobs_page >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Implant Identifier";
  }
  ENDPOINT("GET",
           "implants/{id}/jobs/offset/{offset}/limit/{limit}",
           getJobsForImplant,
           PATH(String, id),
           PATH(UInt32, offset),
           PATH(UInt32, limit)) {
    return createDtoResponse(Status::CODE_200, _jobService.getAllWithChildrenByImplant(const_cast<String &>(id), offset, limit));
  }

  ENDPOINT_INFO(getJobsForImplantWithStatus) {
    info->summary = "Get Jobs for Implant By Status";
    info->addResponse < Object < dto::tasks_page >> (Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Implant Identifier";
    info->pathParams["status"].description = "Status Type";
  }
  ENDPOINT("GET",
           "implants/{id}/jobs/status/{status}/offset/{offset}/limit/{limit}",
           getJobsForImplantWithStatus,
           PATH(String, id),
           PATH(Enum<dto::job_status>::AsNumber, status),
           PATH(UInt32, offset),
           PATH(UInt32, limit)) {
    return createDtoResponse(Status::CODE_200, _jobService.getAllWithChildrenByImplantWhereStatus(const_cast<String &>(id), status, offset, limit));
  }

  ENDPOINT_INFO(createJobForImplant) {
    info->summary = "Create Job For Implant";

    info->addConsumes < Object < dto::job >>("application/json");
    info->addResponse < Object < dto::job >>(Status::CODE_200, "application/json");
    info->addResponse < Object < dto::status >>(Status::CODE_404, "application/json");
    info->addResponse < Object < dto::status >>(Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Implant Identifier";
  }
  ENDPOINT("POST",
           "implants/{id}/jobs",
           createJobForImplant,
           PATH(String, id),
           BODY_DTO(Object < dto::job >, dto)) {
    dto->implant = id;
    return createDtoResponse(Status::CODE_200, _jobService.createWithChildren(dto));
  }

 private:
  services::implant _implantService; // Create implant service.
  services::task _taskService; // Create task service.
  services::job _jobService; // Create task service.
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

}

#endif //MOONSHINE_SERVER_CONTROLLER_IMPLANT_H_
