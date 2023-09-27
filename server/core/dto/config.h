#ifndef MOONSHINE_SERVER_DTO_CONFIG_H_
#define MOONSHINE_SERVER_DTO_CONFIG_H_

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

struct config : public oatpp::DTO {
  DTO_INIT(config, DTO);

  DTO_FIELD(String, key);
  DTO_FIELD(String, value);

  DTO_FIELD_INFO(key) { info->required = true; };
  DTO_FIELD_INFO(value) { info->required = true; };
};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_DTO_CONFIG_H_
