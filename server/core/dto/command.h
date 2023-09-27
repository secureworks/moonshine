#ifndef MOONSHINE_SERVER_CORE_DTO_COMMAND_H_
#define MOONSHINE_SERVER_CORE_DTO_COMMAND_H_

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

struct command : public oatpp::DTO {
  DTO_INIT(command, DTO);

  DTO_FIELD(String, command);
  DTO_FIELD(String, data);

  DTO_FIELD_INFO(command) { info->required = true; };
};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_CORE_DTO_COMMAND_H_
