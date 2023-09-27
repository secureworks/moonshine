#ifndef MOONSHINE_SERVER_DTO_JOB_H_
#define MOONSHINE_SERVER_DTO_JOB_H_

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>
#include <oatpp-sqlite/Types.hpp>

#include "library.h"
#include "argument.h"

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(job_status, v_int32,
     VALUE(LOCAL, 0, "local", "Local"),
     VALUE(SUBMITTED, 1, "submitted", "Submitted"),
     VALUE(RETRIEVED, 2, "retrieved", "Retrieved"),
     VALUE(EXECUTING, 3, "executing", "Executing"),
     VALUE(STOPPING,  4, "stopping",  "Stopping"),
     VALUE(COMPLETED, 5, "completed", "Completed")
);

ENUM(job_success, v_int32,
     VALUE(UNKNOWN, -1, "unknown", "Unknown"),
     VALUE(FAILED, 0, "failed", "Failed"),
     VALUE(SUCCESSFUL, 1, "successful", "Successful")
)

struct job : public oatpp::DTO {
  DTO_INIT(job, DTO);

  DTO_FIELD(String, id);
  DTO_FIELD(String, name);
  DTO_FIELD(Enum<job_status>::AsNumber, status) = job_status::LOCAL;
  DTO_FIELD(String, implant);
  DTO_FIELD(String, package);
  DTO_FIELD(Vector<Object<dto::argument>>, arguments) = oatpp::Vector<dto::argument::Wrapper>::createShared();
  DTO_FIELD(Enum<job_success>::AsNumber, success) = job_success::UNKNOWN;
  DTO_FIELD(String, output) = "";

  DTO_FIELD_INFO(name) { info->required = true; };
  DTO_FIELD_INFO(package) { info->required = true; };
};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_DTO_JOB_H_
