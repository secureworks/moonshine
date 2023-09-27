#ifndef MOONSHINE_SERVER_API_COMPONENT_H_
#define MOONSHINE_SERVER_API_COMPONENT_H_

#include <utility>

#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/web/server/HttpRouter.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp-openssl/server/ConnectionProvider.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/core/macro/component.hpp>
#include <oatpp-websocket/ConnectionHandler.hpp>

#include "swagger_component.h"
#include "database_component.h"
#include "error_handler.h"

#include "services/ws.h"

namespace api {

  /**
   *  Class which creates and holds API components and registers components in oatpp::base::Environment
   *  Order of components initialization is from top to bottom
   */
  struct components {

  private:
    std::string address;
    unsigned short port;
    std::string cert_pem_path;
    std::string cert_crt_path;
    std::string cert_dhparams_path;

  public:

    components(const std::string& address,
      const unsigned short port,
      const std::string& resource_path,
      const std::string& database_path,
      const std::string& cert_pem_path,
      const std::string& cert_crt_path,
      const std::string& cert_dhparams_path)
      : address(address), port(port), cert_pem_path(cert_pem_path), cert_crt_path(cert_crt_path), cert_dhparams_path(cert_dhparams_path), swaggerComponent(address, port, resource_path), databaseComponent(database_path, resource_path) {
      spdlog::info("Management API @ https://{}:{}", address, port);
      spdlog::info("Swagger/OpenAPI @ https://{}:{}/swagger/ui?url=https://{}:{}/api-docs/oas-3.0.0.json", address, port, address, port);
    };

    /**
     *  Swagger component
     */
    swagger_component swaggerComponent;

    /**
     * Database component
     */
    database_component databaseComponent;

    /**
     * Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
      auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      objectMapper->getDeserializer()->getConfig()->allowUnknownFields = false;
      objectMapper->getDeserializer()->getConfig()->enabledInterpretations = { "sqlite" };
      objectMapper->getSerializer()->getConfig()->alwaysIncludeNullCollectionElements = true;
      objectMapper->getSerializer()->getConfig()->includeNullFields = true;
      objectMapper->getSerializer()->getConfig()->enabledInterpretations = { "sqlite" };
      return objectMapper;
    }());

    /**
     *  Create ConnectionProvider component which listens on the port
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([this] {
      //return oatpp::network::tcp::server::ConnectionProvider::createShared({address, port, oatpp::network::Address::IP_4});

      std::shared_ptr<oatpp::network::ServerConnectionProvider> streamProvider =
        oatpp::network::tcp::server::ConnectionProvider::createShared({ address, port, oatpp::network::Address::IP_4 });

      auto config = oatpp::openssl::Config::createDefaultServerConfigShared(cert_pem_path, cert_crt_path);
      //config->addContextConfigurer(std::make_shared<oatpp::openssl::configurer::TemporaryDhParamsFile>(cert_dhparams_path));

      return oatpp::openssl::server::ConnectionProvider::createShared(config, streamProvider);
    }());

    /**
     *  Create Router component
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
      return oatpp::web::server::HttpRouter::createShared();
    }());

    /**
     *  Create ConnectionHandler component which uses Router component to route requests
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)("http",[] {
      OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
      OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper); // get ObjectMapper component

      auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
      connectionHandler->setErrorHandler(std::make_shared<api::error_handler>(objectMapper));
      return connectionHandler;
    }());

    /**
     *  Create wesocet listener component
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<services::wslistener>, websocketListener)([] {
      return std::make_shared<services::wslistener>();
    }());

    /**
     *  Create websocket connection handler
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler)("websocket" /* qualifier */, [] {
      OATPP_COMPONENT(std::shared_ptr<services::wslistener>, listener); // get wslistener component

      auto connectionHandler = oatpp::websocket::ConnectionHandler::createShared();
      connectionHandler->setSocketInstanceListener(listener);
      return connectionHandler;
    }());
  };

}

#endif //MOONSHINE_SERVER_API_COMPONENT_H_
