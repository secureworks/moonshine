#ifndef MOONSHINE_SERVER_API_ERROR_HANDLER_H_
#define MOONSHINE_SERVER_API_ERROR_HANDLER_H_

#include "dto/status.h"

#include "oatpp/web/server/handler/ErrorHandler.hpp"
#include "oatpp/web/protocol/http/outgoing/ResponseFactory.hpp"

namespace api {

  struct error_handler : public oatpp::web::server::handler::ErrorHandler {

  private:
    typedef oatpp::web::protocol::http::outgoing::Response OutgoingResponse;
    typedef oatpp::web::protocol::http::Status Status;
    typedef oatpp::web::protocol::http::outgoing::ResponseFactory ResponseFactory;

  public:
    explicit error_handler(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper);

    std::shared_ptr<OutgoingResponse> handleError(const Status& status,
      const oatpp::String& message,
      const Headers& headers) override;

  private:
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> _objectMapper;
  };

}

#endif //MOONSHINE_SERVER_API_ERROR_HANDLER_H_
