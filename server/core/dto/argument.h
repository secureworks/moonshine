#ifndef MOONSHINE_SERVER_DTO_ARGUMENT_H_
#define MOONSHINE_SERVER_DTO_ARGUMENT_H_

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>
#include <oatpp-sqlite/Types.hpp>

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

struct argument : public oatpp::DTO {
  DTO_INIT(argument, DTO);

  DTO_FIELD(String, data);

  DTO_FIELD_INFO(data) { info->required = true; };
};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_DTO_ARGUMENT_H_
