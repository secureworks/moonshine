#ifndef MOONSHINE_SERVER_DTO_SERVICE_H_
#define MOONSHINE_SERVER_DTO_SERVICE_H_

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>
#include <oatpp-sqlite/Types.hpp>

#include "library.h"
#include "argument.h"

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

struct service : public oatpp::DTO {
  DTO_INIT(service, DTO);

  DTO_FIELD(String, id);
  DTO_FIELD(UInt32, status) = (v_uint32) 0;
  DTO_FIELD(String, name);
  DTO_FIELD(String, package);
  DTO_FIELD(Vector<Object<dto::argument>>, arguments) = oatpp::Vector<dto::argument::Wrapper>::createShared();

  DTO_FIELD_INFO(name) { info->required = true; };
  DTO_FIELD_INFO(package) { info->required = true; };
};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_DTO_SERVICE_H_
