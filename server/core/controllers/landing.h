#ifndef MOONSHINE_SERVER_CONTROLLER_LANDING_H_
#define MOONSHINE_SERVER_CONTROLLER_LANDING_H_

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

namespace controllers {

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

struct landing : public oatpp::web::server::api::ApiController {

  explicit landing(const std::shared_ptr<ObjectMapper> &objectMapper)
      : oatpp::web::server::api::ApiController(objectMapper) {}

  static std::shared_ptr<landing> createShared(
      OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                      objectMapper) // Inject objectMapper component here as default parameter
  ) {
    return std::make_shared<landing>(objectMapper);
  }

  ENDPOINT("GET", "/", root) {
    const char *html =
        "<html lang='en'>"
        "  <head>"
        "    <meta charset=utf-8/>"
        "  </head>"
        "  <body>"
        "    <a href='swagger/ui?url=http://localhost:9000/api-docs/oas-3.0.0.json'>Swagger-UI</a>"
        "  </body>"
        "</html>";
    auto response = createResponse(Status::CODE_200, html);
    response->putHeader(Header::CONTENT_TYPE, "text/html");
    return response;
  }
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

}

#endif //MOONSHINE_SERVER_CONTROLLER_LANDING_H_
