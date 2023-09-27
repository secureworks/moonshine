#ifndef MOONSHINE_SERVER_CONTROLLER_TASK_H_
#define MOONSHINE_SERVER_CONTROLLER_TASK_H_

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "services/task.h"

namespace controllers {

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

  struct task : public oatpp::web::server::api::ApiController {

    explicit task(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

    static std::shared_ptr<task> createShared(
      OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
        objectMapper) // Inject objectMapper component here as default parameter
    ) {
      return std::make_shared<task>(objectMapper);
    }

    ENDPOINT_INFO(getTask) {
      info->summary = "Get Task by Id";
      info->addResponse < Object < dto::task >>(Status::CODE_200, "application/json");
      info->addResponse < Object < dto::status >>(Status::CODE_404, "application/json");
      info->addResponse < Object < dto::status >>(Status::CODE_500, "application/json");
      info->pathParams["id"].description = "Task Identifier";
    }
    ENDPOINT("GET", "tasks/{id}", getTask, PATH(String, id)) {
      return createDtoResponse(Status::CODE_200, _taskService.getById(const_cast<String&>(id)));
    }

  private:
    services::task _taskService; // Create task service.
  };

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

}

#endif // MOONSHINE_SERVER_CONTROLLER_TASK_H_
