#include <filesystem>

#include <fmt/color.h>
#include <cxxopts.hpp>
#include <util/filesystem.hpp>

#include "config.h"
#include "server.h"
#include "x509.h"

bool is_readable(const std::string &path)
{
  if (std::filesystem::exists(path)) {
    std::error_code ec; // For noexcept overload usage.
    auto perms = std::filesystem::status(path, ec).permissions();
    if ((perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none &&
        (perms & std::filesystem::perms::group_read) != std::filesystem::perms::none &&
        (perms & std::filesystem::perms::others_read) != std::filesystem::perms::none
        ) {
      return true;
    }
  }
  return false;
}

std::string find_resource_dir(const std::string& base_path) {
  std::vector<std::string> paths{
      base_path + "/../share/moonshine/resources",
#if DEBUG
      SOURCE_DIR "/server/resources",
      base_path + "/../share/moonshine/resources",
      base_path + "/../../../../server/resources",
      base_path + "/../../../server/resources",
#endif
  };

  for (auto &attempt : paths) {
    try {
      return std::filesystem::canonical(attempt).string();
    }
    catch (std::filesystem::filesystem_error &e) {}
  }
  return {};
}

std::string find_implants_dir(const std::string& base_path) {
  std::vector<std::string> paths{
      base_path + "/../share/moonshine/implants",
#if DEBUG
      BUILD_DIR "/implant/executable",
      SOURCE_DIR "/debug/share/moonshine/implants",
      base_path + "/../../implant/executable",
      base_path + "/../implant/executable",
#endif
  };

  for (auto &attempt : paths) {
    try {
      return std::filesystem::canonical(attempt).string();
    }
    catch (std::filesystem::filesystem_error &e) {}
  }
  return {};
}

int main(int argc, char *argv[]) {
  try {
    std::string
        executable_path = std::filesystem::canonical(std::filesystem::path(util::filesystem::get_executable_path()).remove_filename()).string();

    cxxopts::Options options("server", "Moonshine Server");

    options.add_options()
        ("a,address",  "Management interface bind address.", cxxopts::value<std::string>()->default_value("127.0.0.1"))
        ("p,port",     "Management interface listening port.", cxxopts::value<unsigned short>()->default_value("9000"))
        ("r,resource", "Resource root directory.", cxxopts::value<std::string>()->default_value(find_resource_dir(executable_path)))
        ("i,implants", "Implants root directory.", cxxopts::value<std::string>()->default_value(find_implants_dir(executable_path)))
        ("d,database", "Database path.", cxxopts::value<std::string>()->default_value(executable_path +  PATH_SEPARATOR + "moonshine.sqlite"))
        ("c,certificate", "Certificate, private key, and dhparam root directory.", cxxopts::value<std::string>()->default_value(executable_path))
        ("l,loglevel", "Log level (>=).", cxxopts::value<unsigned short>()->default_value("2"))
        ("v,version",  "Print version.")
        ("h,help",     "Print usage.");

    auto result = options.parse(argc, argv);

    if (result.count("version")) {
      fmt::print("Moonshine Server v" PROJECT_VER "\n");
      exit(0);
    }

    if (result.count("help")) {
      fmt::print(options.help());
      exit(0);
    }

    std::string address = result["address"].as<std::string>();

    unsigned short port = result["port"].as<unsigned short>();

    std::string resource_dir = result["resource"].as<std::string>();
    try {
      if (resource_dir.empty())
        throw std::runtime_error("");
      resource_dir = std::filesystem::canonical(resource_dir).string();
    }
    catch (std::runtime_error &e) {
      fmt::print(stderr, "Error: Could not find resource root directory, set manually using '-r' or '--resource'\n");
      exit(1);
    }

    std::string implants_dir = result["implants"].as<std::string>();
    try {
      if (implants_dir.empty())
        throw std::runtime_error("");
      implants_dir = std::filesystem::canonical(implants_dir).string();
    }
    catch (std::runtime_error &e) {
      fmt::print(stderr, "Error: Could not find implants root directory, set manually using '-i' or '--implants'\n");
      exit(1);
    }

    std::string database_path = result["database"].as<std::string>();

    unsigned short loglevel = result["loglevel"].as<unsigned short>();
    if (loglevel > 6) {
      fmt::print(stderr, "Error: Log level must be between 0 and 6\n");
      exit(1);
    }

    fmt::print("\n");
    fmt::print("                 ___---___                 \n");
    fmt::print("              .--         --.              \n");
    fmt::print("            ./   ()      .-. \\.           \n");
    fmt::print("           /   o    .   (   )  \\          \n");
    fmt::print("          / .            '-'    \\         \n");
    fmt::print("         | ()    .  O         .  |         \n");
    fmt::print("        |                         |        \n");
    fmt::print("        |    o           ()       |        \n");
    fmt::print("        |       .--.          O   |        \n");
    fmt::print("         | .   |    |            |         \n");
    fmt::print("          \\    `.__.'    o   .  /         \n");
    fmt::print("           \\                   /          \n");
    fmt::print("            `\\  o    ()      /' Moonshine \n");
    fmt::print("              `--___   ___--'      v" PROJECT_VER "  \n");
    fmt::print("                    ---                    \n");
    fmt::print("\n");

    auto loglevel_string = spdlog::level::to_string_view(static_cast<spdlog::level::level_enum>(loglevel));
    spdlog::set_level(spdlog::level::from_str(loglevel_string.data()));
    spdlog::info("Logging set to show '{}' severity and above", loglevel_string);

    std::string certs_path = result["certificate"].as<std::string>();
    auto cert_pem_path = certs_path + PATH_SEPARATOR + "moonshine.crt";
    auto cert_crt_path = certs_path + PATH_SEPARATOR + "moonshine.pem";
    if (!is_readable(cert_pem_path) || !is_readable(cert_crt_path)) {
      //spdlog::warn("Unable to read 'moonshine.crt' and/or 'moonshine.pem' in {}", certs_path + PATH_SEPARATOR);
      x509::generate_self_signed_certificate("moonshine", certs_path, "US", "None", "localhost");
    }
    spdlog::info("Management API certificate @ {}", cert_pem_path);
    spdlog::info("Management API private key @ {}", cert_crt_path);

    auto cert_dhparams_path = certs_path + PATH_SEPARATOR + "dhparams.pem";
    /*
    if (!is_readable(cert_dhparams_path)) {
      spdlog::info("Unable to read 'dhparams.pem' file in {}", certs_path);
      x509::generate_dhparam(cert_dhparams_path);
    }
    spdlog::info("Management REST API DH parameters @ {}", cert_dhparams_path);
    */

    server server(address, port, resource_dir, implants_dir, database_path, cert_pem_path, cert_crt_path, cert_dhparams_path);
    server.start();
    exit(0);

  } catch (const std::exception &e) {
    spdlog::critical("Error: {}", e.what());
    exit(1);
  }
}
