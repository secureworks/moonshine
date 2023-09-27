#ifndef MOONSHINE_SERVER_DTO_TASK_H_
#define MOONSHINE_SERVER_DTO_TASK_H_

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>
#include <oatpp-sqlite/Types.hpp>

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(task_status, v_int32,
     VALUE(LOCAL, 0, "local", "Local"),
     VALUE(SUBMITTED, 1, "submitted", "Submitted"),
     VALUE(RETRIEVED, 2, "retrieved", "Retrieved"),
     VALUE(COMPLETED, 3, "completed", "Completed")
);

ENUM(task_success, v_int32,
     VALUE(UNKNOWN, -1, "unknown", "Unknown"),
     VALUE(FAILED, 0, "failed", "Failed"),
     VALUE(SUCCESSFUL, 1, "successful", "Successful")
)

struct task : public oatpp::DTO {
  DTO_INIT(task, DTO);

  DTO_FIELD(String, id);
  DTO_FIELD(Enum<task_status>::AsNumber, status) = task_status::LOCAL;
  DTO_FIELD(String, implant);
  DTO_FIELD(String, script) = "";
  DTO_FIELD(Enum<task_success>::AsNumber, success) = task_success::UNKNOWN;
  DTO_FIELD(String, output)= "";

  DTO_FIELD_INFO(implant) {
    info->required = true;
    info->pattern = "^[a-z0-9]{10}$";
  };
  DTO_FIELD_INFO(script) { info->required = true; };
};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_DTO_TASK_H_
