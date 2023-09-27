#ifndef MOONSHINE_SERVER_DTO_STATUS_H_
#define MOONSHINE_SERVER_DTO_STATUS_H_

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"
#include <oatpp-sqlite/Types.hpp>

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

struct status : public oatpp::DTO {

  DTO_INIT(status, DTO);

  DTO_FIELD_INFO(text) { info->description = "Short status text"; };
  DTO_FIELD(String, text);
  DTO_FIELD_INFO(code) { info->description = "Status code"; };
  DTO_FIELD(Int32, code);
  DTO_FIELD_INFO(message) { info->description = "Verbose message"; };
  DTO_FIELD(String, message);

};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_DTO_STATUS_H_
