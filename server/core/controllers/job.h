#ifndef MOONSHINE_SERVER_CONTROLLER_JOBS_H_
#define MOONSHINE_SERVER_CONTROLLER_JOBS_H_

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "services/job.h"
#include "dto/command.h"

namespace controllers {

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

  struct job : public oatpp::web::server::api::ApiController {

    explicit job(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

    static std::shared_ptr<job> createShared(
      OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
        objectMapper) // Inject objectMapper component here as default parameter
    ) {
      return std::make_shared<job>(objectMapper);
    }

    ENDPOINT_INFO(getJob) {
      info->summary = "Get Job by Id";
      info->addResponse < Object < dto::job >>(Status::CODE_200, "application/json");
      info->addResponse < Object < dto::status >>(Status::CODE_404, "application/json");
      info->addResponse < Object < dto::status >>(Status::CODE_500, "application/json");
      info->pathParams["id"].description = "Job Identifier";
    }
    ENDPOINT("GET", "jobs/{id}", getJob, PATH(String, id)) {
      return createDtoResponse(Status::CODE_200, _jobService.getByIdWithChildren(const_cast<String&>(id)));
    }

    ENDPOINT_INFO(stopJob) {
      info->summary = "Stop Job by Id";
      info->addResponse < Object < dto::status >> (Status::CODE_200, "application/json");
      info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
      info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
      info->pathParams["id"].description = "Job Identifier";
    }
    ENDPOINT("DELETE", "jobs/{id}", stopJob, PATH(String, id)) {
      return createDtoResponse(Status::CODE_200, _jobService.stopById(const_cast<String&>(id)));
    }

    ENDPOINT_INFO(commandJob) {
      info->summary = "Send a Command to a Job";
      info->addConsumes < Object < dto::command >>("application/json");
      info->addResponse < Object < dto::status >> (Status::CODE_200, "application/json");
      info->addResponse < Object < dto::status >> (Status::CODE_404, "application/json");
      info->addResponse < Object < dto::status >> (Status::CODE_500, "application/json");
      info->pathParams["id"].description = "Job Identifier";
    }
    ENDPOINT("PUT", "jobs/{id}", commandJob, PATH(String, id), BODY_DTO(Object < dto::command >, dto)) {
      return createDtoResponse(Status::CODE_200, _jobService.commandById(const_cast<String&>(id), dto->command, dto->data));
    }

  private:
    services::job _jobService; // Create jobs service.
  };

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

}

#endif // MOONSHINE_SERVER_CONTROLLER_JOBS_H_
