#ifndef MOONSHINE_IMPLANT_CORE_MESSAGES_H_
#define MOONSHINE_IMPLANT_CORE_MESSAGES_H_

#include <lua_object.hpp>

#include "state.h"

struct messages : lua_object {

  explicit messages(state* state) : _state(state) {};

  int luaopen_object(lua_State* L) override;
  int lua_submit(lua_State *L);
  int lua_retrieve(lua_State* L);
  int lua_retrieve_metadata(lua_State* L);
  int lua_retrieve_if_messages(lua_State* L);
  int encrypt_and_return(lua_State* L, std::vector<std::shared_ptr<shared::message>> messages);

 private:

  state* _state;
};

#endif //MOONSHINE_IMPLANT_CORE_MESSAGES_H_
