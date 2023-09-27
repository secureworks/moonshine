#include "server.h"

#include <spdlog/spdlog.h>
#include <moon.hpp>
#include <encryption.h>
#include <util/string.hpp>

#include "lua/modules.h"
#include "services/listener.h"
#include "services/service.h"

server::server(const std::string &address,
               unsigned short port,
               const std::string &resource_dir,
               const std::string &implants_dir,
               const std::string &database_path,
               const std::string &cert_pem_path,
               const std::string &cert_crt_path,
               const std::string &cert_dhparams_path)
  : _api_server(address, port, resource_dir, database_path, cert_pem_path, cert_crt_path, cert_dhparams_path), _task_worker(), _jobs_worker(), _hello_worker() {

  encryption::init();
  // Ensure new lua states have an output callback set and a set of standard modules preloaded
  moon_newstate_cb(lua::on_newstate, this);
  services::listener::instance()->init();
  services::service::instance()->init();

  // generate server public and private key pair (if needed)
  auto public_key_b64 = _config_service.get("public_key");
  if (public_key_b64.empty()) {
    monocypher::key_exchange key_exchange;
    auto secret_key = key_exchange.get_secret_key();
    auto public_key = key_exchange.get_public_key();

    auto secret_key_b64 = util::base64::encode(secret_key.data(), secret_key.size());
    _config_service.set("secret_key", secret_key_b64);
    public_key_b64 = util::base64::encode(public_key.data(), public_key.size());
    _config_service.set("public_key", public_key_b64);
  }
  spdlog::info("Server public key @ {}", public_key_b64);

  _config_service.set("resource_dir", resource_dir);
  spdlog::info("Resource directory @ {}", resource_dir);

  _config_service.set("implants_dir", implants_dir);
  spdlog::info("Implants directory @ {}", implants_dir);
}

void server::start() {
  _api_server.run();
}

void server::stop() {
  _api_server.stop();
}
