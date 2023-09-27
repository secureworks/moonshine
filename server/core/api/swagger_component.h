#ifndef MOONSHINE_SERVER_SWAGGER_COMPONENT_H_
#define MOONSHINE_SERVER_SWAGGER_COMPONENT_H_

#include <spdlog/spdlog.h>
#include <oatpp-swagger/Model.hpp>
#include <oatpp-swagger/Resources.hpp>
#include <oatpp/core/macro/component.hpp>

#include "config.h"

/**
 *  Swagger ui is served at
 *  http://host:port/swagger/ui
 */
struct swagger_component {

 private:
  std::string address;
  unsigned short port;
  std::string resource_path;

 public:
  swagger_component(const std::string &address, const unsigned short port, const std::string &resource_path) : address(
      address), port(port), resource_path(resource_path) {};

  /**
   *  General API docs info
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, swaggerDocumentInfo)([this] {

    oatpp::swagger::DocumentInfo::Builder builder;

    builder
        .setTitle("Moonshine Server")
        .setDescription("Management REST API")
        .setVersion(PROJECT_VER)
        .setContactName("Contact Name")
        .setContactUrl("Contact Url")
        .setLicenseName("License Name")
        .setLicenseUrl("License Url")
        .addServer(std::string("https://127.0.0.1:").append(std::to_string(port)), "Moonshine Server Management REST API");

    return builder.build();
  }());

  /**
   *  Swagger-Ui Resources (<oatpp-examples>/lib/oatpp-swagger/res)
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, swaggerResources)([this] {
    // Make sure to specify correct full path to oatpp-swagger/res folder !!!
    return oatpp::swagger::Resources::loadResources(resource_path + "/swagger");
  }());
};

#endif //MOONSHINE_SERVER_SWAGGER_COMPONENT_H_
