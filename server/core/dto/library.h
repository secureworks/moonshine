#ifndef MOONSHINE_SERVER_DTO_LIBRARY_H_
#define MOONSHINE_SERVER_DTO_LIBRARY_H_

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>
#include <oatpp-sqlite/Types.hpp>

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(library_type, v_int32,
     VALUE(LUA_SCRIPT, 1, "lua_script", "Lua Script Module"),
     VALUE(LUA_C, 2, "lua_c", "Lua C Module")
);

struct library : public oatpp::DTO {
  DTO_INIT(library, DTO);

  DTO_FIELD(Enum<library_type>::AsNumber, type) = library_type::LUA_SCRIPT;
  DTO_FIELD(String, name);
  DTO_FIELD(String, data);

  DTO_FIELD_INFO(type) { info->required = true; };
  DTO_FIELD_INFO(name) { info->required = true; };
  DTO_FIELD_INFO(data) { info->required = true; };
};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_DTO_LIBRARY_H_
