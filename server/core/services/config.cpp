#include "config.h"

#include <util/string.hpp>

#include "dto/config.h"

static std::string hexString(const void *buf, size_t size) {
  auto hex = std::make_unique<char[]>(2*size + size/4 + 1);
  char *dst = hex.get();
  for (size_t i = 0; i < size; i++) {
    if (i > 0 && (i % 4) == 0) *dst++ = ' ';
    dst += sprintf(dst, "%02X", ((const uint8_t*)buf)[i]);
  }
  return {hex.get()};
}

monocypher::key_exchange::secret_key services::config::get_server_secret_key() {

  auto secret_key_b64 = get("secret_key");
  if (secret_key_b64.empty()) {
    throw std::runtime_error("secret_key does not exist in the database");
  }
  auto secret_key_raw = util::base64::decode(secret_key_b64);
  monocypher::key_exchange::secret_key secret_key;
  secret_key.clear();
  secret_key.fillWith(secret_key_raw.data(), secret_key_raw.size());
  return secret_key;
}

std::vector<unsigned char> services::config::get_server_public_key() {
  auto public_key = get("public_key");
  if (public_key.empty()) {
    throw std::runtime_error("public_key does not exist in the database");
  }
  return util::base64::decode(public_key);
}

std::string services::config::get(const std::string& key) {
  auto result = _database->getConfig(key);
  if (result->isSuccess() && result->hasMoreToFetch()) {
    auto dataset = result->fetch<oatpp::Vector<oatpp::Object<dto::config>>>();
    if (dataset->size() == 1) {
      return dataset[0]->value;
    }
  }
  return {};
}

void services::config::set(const std::string& key, const std::string& value) {
  _database->setConfig(key, value);
}
