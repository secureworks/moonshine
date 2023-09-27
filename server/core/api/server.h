#ifndef MOONSHINE_SERVER_API_H_
#define MOONSHINE_SERVER_API_H_

#include <string>

#include <oatpp-swagger/Controller.hpp>
#include <oatpp/network/Server.hpp>

#include "components.h"
#include "controllers/implant.h"
#include "controllers/listener.h"
#include "controllers/task.h"
#include "controllers/landing.h"
#include "controllers/artifact.h"
#include "controllers/service.h"
#include "controllers/job.h"
#include "controllers/ws.h"

namespace api {

  struct server {

  private:
    std::shared_ptr<api::components> _api_components;
    std::shared_ptr<oatpp::network::Server> _server;
    std::shared_ptr<oatpp::network::ConnectionHandler> _connectionHandler;
    std::shared_ptr<oatpp::network::ServerConnectionProvider> _connectionProvider;

  public:

    server(const std::string& address,
      const unsigned short port,
      const std::string& resource_path,
      const std::string& database_path,
      const std::string& cert_pem_path,
      const std::string& cert_crt_path,
      const std::string& cert_dhparams_path) {
      oatpp::base::Environment::init();

      _api_components = std::make_shared<api::components>(address,
        port,
        resource_path,
        database_path,
        cert_pem_path,
        cert_crt_path,
        cert_dhparams_path); // Create scope Environment components
    };

    ~server() {
      //oatpp::base::Environment::destroy();
    };

    void run() {

      /* Get router component */
      OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

      oatpp::web::server::api::Endpoints swaggerEndpoints;
      swaggerEndpoints.append(router->addController(controllers::listener::createShared())->getEndpoints());
      swaggerEndpoints.append(router->addController(controllers::implant::createShared())->getEndpoints());
      swaggerEndpoints.append(router->addController(controllers::task::createShared())->getEndpoints());
      swaggerEndpoints.append(router->addController(controllers::artifact::createShared())->getEndpoints());
      swaggerEndpoints.append(router->addController(controllers::service::createShared())->getEndpoints());
      swaggerEndpoints.append(router->addController(controllers::job::createShared())->getEndpoints());
      swaggerEndpoints.append(router->addController(std::make_shared<controllers::ws>())->getEndpoints());

      router->addController(oatpp::swagger::Controller::createShared(swaggerEndpoints));
      router->addController(controllers::landing::createShared());

      /* Get connection handler component */
      OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");

      /* Get connection provider component */
      OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

      /* create server */
      _server = oatpp::network::Server::createShared(connectionProvider, connectionHandler);
      _server->run();

      /* Stop connections */
      //connectionProvider->stop();
      //connectionHandler->stop();

      /* stop db connection pool */
      //OATPP_COMPONENT(std::shared_ptr<oatpp::provider::Provider<oatpp::sqlite::Connection>>, dbConnectionProvider);
      //dbConnectionProvider->stop();
    };

    void stop() {
      _server->stop();
    };
  };

}

#endif //MOONSHINE_SERVER_API_H_
