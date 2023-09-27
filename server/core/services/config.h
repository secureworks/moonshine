#ifndef MOONSHINE_SERVER_CORE_SERVICE_CONFIG_H_
#define MOONSHINE_SERVER_CORE_SERVICE_CONFIG_H_

#include <string>
#include <oatpp/core/macro/component.hpp>

#include <encryption.h>

#include "db/client.h"

namespace services {

struct config {

 private:
  OATPP_COMPONENT(std::shared_ptr<db::client>, _database); // Inject database component

 public:

  monocypher::key_exchange::secret_key get_server_secret_key();
  std::vector<unsigned char> get_server_public_key();

  std::string get(const std::string& key);
  void set(const std::string& key, const std::string& value);
};

}

#endif //MOONSHINE_SERVER_CORE_SERVICE_CONFIG_H_
