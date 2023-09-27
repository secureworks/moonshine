#include "error_handler.h"

namespace api {

  error_handler::error_handler(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper)
    : _objectMapper(objectMapper) {}

  std::shared_ptr<error_handler::OutgoingResponse> error_handler::handleError(const Status& status,
    const oatpp::String& message,
    const Headers& headers) {
    auto error = dto::status::createShared();
    error->text = "ERROR";
    error->code = status.code;
    error->message = message;

    auto response = ResponseFactory::createResponse(status, error, _objectMapper);

    for (const auto& pair : headers.getAll()) {
      response->putHeader(pair.first.toString(), pair.second.toString());
    }

    return response;
  }

}
