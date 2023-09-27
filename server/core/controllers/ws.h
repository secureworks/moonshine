#ifndef MOONSHINE_SERVER_CORE_CONTROLLERS_WS_H_
#define MOONSHINE_SERVER_CORE_CONTROLLERS_WS_H_

#include <oatpp-websocket/Handshaker.hpp>
#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/network/ConnectionHandler.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>

namespace controllers {

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

/**
 * Controller with WebSocket-connect endpoint.
 */
class ws : public oatpp::web::server::api::ApiController {
 private:
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler, "websocket");

 public:

  explicit ws(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) : oatpp::web::server::api::ApiController(
      objectMapper) {}

  ENDPOINT("GET", "ws", websocket, REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    return oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), websocketConnectionHandler);
  };
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end

}

#endif //MOONSHINE_SERVER_CORE_CONTROLLERS_WS_H_
