#include "implant.h"

#include <moon.hpp>
#include <encryption.h>
#include <metadata.h>
#include <util/string.hpp>

namespace lua {

int implant::luaopen_object(lua_State *L) {
  lua_newtable(L);
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<implant, &implant::lua_exists>, 1);
  lua_setfield(L, -2, "exists");
  return 1;
}

int implant::lua_exists(lua_State *L) {
  // check and fetch the arguments
  size_t length;
  const char* a1 = luaL_checklstring(L, 1, &length);
  auto blob = util::string::convert(a1, length);

  bool success = false;
  std::vector<uint8_t> metadata = {};

  if (encryption::decryptMetadata(blob, metadata,
    _config_service.get_server_secret_key(),
    _config_service.get_server_public_key())) {

    try {
      auto m = shared::metadata::deserialize(metadata);
      auto implant_id = oatpp::String(m->id);
      auto public_key_b64 = oatpp::String(util::base64::encode(m->public_key));

      if (_implant_service.exists(implant_id)) {
        auto dto = _implant_service.getById(implant_id);
        if (dto->public_key == public_key_b64)
          success = true;
      }
    }
    catch (const std::exception& e) {
      spdlog::error("Error parsing metadata {}", e.what());
    }
  }

  lua_pushboolean(L, success ? 1 : 0); // push result
  return 1; // number of contents
}

}
