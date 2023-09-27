#ifndef MOONSHINE_SERVER_DTO_IMPLANT_H_
#define MOONSHINE_SERVER_DTO_IMPLANT_H_

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

struct implant : public oatpp::DTO {
  DTO_INIT(implant, DTO);

  DTO_FIELD(String, id);
  DTO_FIELD(String, listener);
  DTO_FIELD(String, name);
  DTO_FIELD(String, public_key);
  DTO_FIELD(Int32,  architecture) = (v_int32) 0;
  DTO_FIELD(Int32,  operating_system) = (v_int32) 0;
  DTO_FIELD(Int32,  process_id) = (v_int32) 0;
  DTO_FIELD(String, process_user);
  DTO_FIELD(String, process_path);
  DTO_FIELD(String, system_name);
  DTO_FIELD(String, system_addrs);
};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_DTO_IMPLANT_H_
