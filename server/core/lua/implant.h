#ifndef MOONSHINE_SERVER_IMPLANTS_H_
#define MOONSHINE_SERVER_IMPLANTS_H_

#include <lua_object.hpp>

#include "services/implant.h"
#include "services/config.h"

namespace lua {

struct implant : lua_object {

  static implant *instance() {
    static implant instance;
    return &instance;
  }

  implant(const implant &) = delete; // Disable copy
  void operator=(const implant &) = delete; // Disable assignment

  int luaopen_object(lua_State *L) override;
  int lua_exists(lua_State *L);

 private:
  implant() = default;
  services::implant _implant_service; // Create implant service.
  services::config _config_service; // Create config service.
};

}

#endif //MOONSHINE_SERVER_IMPLANTS_H_
