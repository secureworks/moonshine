#include "messages.h"

#include <encryption.h>
#include <metadata.h>
#include <message.h>
#include <util/debug.hpp>
#include <util/string.hpp>

int messages::luaopen_object(lua_State *L) {
  lua_newtable(L);
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<messages, &messages::lua_submit>, 1);
  lua_setfield(L, -2, "submit");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<messages, &messages::lua_retrieve>, 1);
  lua_setfield(L, -2, "retrieve");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<messages, &messages::lua_retrieve_metadata>, 1);
  lua_setfield(L, -2, "retrieve_metadata");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<messages, &messages::lua_retrieve_if_messages>, 1);
  lua_setfield(L, -2, "retrieve_if_messages");
  return 1;
}

int messages::lua_submit(lua_State *L) {
  size_t length;
  const char *data = luaL_checklstring(L, 1, &length);

  bool success = false;

  try {
    if (length > 0) {
      std::vector<uint8_t> raw = {};
      if (encryption::decryptData(util::string::convert(data, length),
                                  raw,
                                  _state->keys.get_secret_key(),
                                  _state->callback_server_public_key)) {
        auto messages = shared::message::deserialize(raw);

#ifndef NDEBUG
        DEBUG_PRINT("Received %i messages\n", messages.size());
        //for (const auto& message : messages)
        //  DEBUG_PRINT("%s\n", message->to_string());
#endif

        _state->dispatcher.push_received("server", messages);
        success = true;
      }
      else {
        ERROR_PRINT("Error decrypting message data\n");
      }
    }
  }
  catch (const std::exception& e) {
    ERROR_PRINT("Error parsing messages, %s\n", e.what());
  }

  lua_pushboolean(L, success);
  return 1;
}

int messages::lua_retrieve(lua_State* L) {
  auto messages = _state->dispatcher.pull_send("server");
  return encrypt_and_return(L, messages);
}

int messages::lua_retrieve_metadata(lua_State* L) {
  auto public_key = _state->keys.get_public_key();
  auto metadata = std::make_shared<shared::metadata>(_state->id,
             std::vector<unsigned char> (public_key.begin(),public_key.end()));

  std::vector<uint8_t> metadata_blob = {};
  if (encryption::encryptMetadata(shared::metadata::serialize(metadata),
                                  metadata_blob,
                                  _state->callback_server_public_key)) {
    auto metadata_string = util::string::convert(metadata_blob.data(), metadata_blob.size());
    lua_pushlstring(L, metadata_string.data(), metadata_string.length());
    return 1;
  }

  ERROR_PRINT("Could not encrypt metadata");
  return luaL_error(L, "Could not encrypt metadata");
}

int messages::lua_retrieve_if_messages(lua_State* L) {
  auto messages = _state->dispatcher.pull_send("server");
  if (!messages.empty()) {
    return encrypt_and_return(L, messages);
  }
  else {
    lua_pushlstring(L, _state->callback_server().data(), _state->callback_server().length());
    lua_pushnil(L);
    lua_pushnil(L);
    return 3; // number of return values
  }
}

int messages::encrypt_and_return(lua_State* L, std::vector<std::shared_ptr<shared::message>> messages) {
  auto public_key = _state->keys.get_public_key();
  auto metadata = std::make_shared<shared::metadata>(_state->id,
             std::vector<unsigned char> (public_key.begin(),public_key.end()));

#ifndef NDEBUG
  if (!messages.empty()) {
    DEBUG_PRINT("Sending %i messages\n", messages.size());
    //DEBUG_PRINT("%s\n", metadata.to_string());
    //for (auto& message : messages)
    //  DEBUG_PRINT("%s\n", message->to_string());
  }
#endif

  std::vector<uint8_t> metadata_blob = {};
  if (encryption::encryptMetadata(shared::metadata::serialize(metadata),
                                  metadata_blob,
                                  _state->callback_server_public_key)) {
    auto metadata_string = util::string::convert(metadata_blob.data(), metadata_blob.size());

    lua_pushlstring(L, _state->callback_server().data(), _state->callback_server().length());
    lua_pushlstring(L, metadata_string.data(), metadata_string.length());

    if (!messages.empty()) {
      std::vector<std::uint8_t> raw = shared::message::serialize(messages);
      std::vector<uint8_t> blob = {};
      encryption::encryptData(raw, blob,
                              _state->keys.get_secret_key(),
                              _state->callback_server_public_key);
      auto blob_string = util::string::convert(blob.data(), blob.size());
      lua_pushlstring(L, blob_string.data(), blob_string.length());
    }
    else {
      lua_pushnil(L);
    }
    return 3; // number of return values
  }

  ERROR_PRINT("Could not encrypt metadata");
  return luaL_error(L, "Could not encrypt metadata");
}
