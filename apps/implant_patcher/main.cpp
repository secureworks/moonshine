#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <algorithm>

#include <cxxopts.hpp>

#include <util/filesystem.hpp>
#include <util/string.hpp>

#include "config.h"

int main(int argc, char *argv[]) {
  try {
    cxxopts::Options options("implant_patcher", "");

    options.add_options()
        ("i,in",      "Implant template", cxxopts::value<std::string>())
        ("o,out",     "Implant binary",  cxxopts::value<std::string>())
        ("c,server",  "Server connection string",  cxxopts::value<std::string>())
        ("p,pkey",    "Server public key",  cxxopts::value<std::string>())
        ("s,script",  "Path to communication script",  cxxopts::value<std::string>())
        ("v,version", "Print version")
        ("h,help",    "Print usage")
        ;

    auto result = options.parse(argc, argv);

    if (result.count("version")) {
      std::cout << "Implant Patcher v" PROJECT_VER << std::endl;
      exit(0);
    }

    if (result.count("help") ||
        result.count("in") != 1 ||
        result.count("out") != 1 ||
        result.count("server") != 1 ||
        result.count("pkey") != 1 ||
        result.count("script") != 1) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    std::cout << "Reading template " << result["in"].as<std::string>() << std::endl;
    std::string input = util::string::convert(util::filesystem::read_file(result["in"].as<std::string>()));
    std::string output = result["out"].as<std::string>();
    std::string server = result["server"].as<std::string>();
    std::string pkey = result["pkey"].as<std::string>();
    std::string script = util::string::convert(util::filesystem::read_file(result["script"].as<std::string>()));

    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.emplace_back(std::string(45, 'A'), server);
    pairs.emplace_back(std::string(45, 'B'), pkey);
    pairs.emplace_back(std::string(45, 'C'), script);

    for (auto & pair : pairs) {
      auto needle = pair.first;
      auto substitute = pair.second;

      // Ideally we would use std::boyer_moore_searcher here, but it's not available on macOS :-/
      auto wordPosition = search(input.begin(), input.end(),std::default_searcher(needle.begin(), needle.end()));
      if (wordPosition != input.end()) {
        auto start = std::distance(begin(input), wordPosition);
        auto end = input.find_first_not_of(needle[0], start);
        //auto length = std::distance(begin(input), wordPosition.second) - start;
        auto length = end - start;
        std::cout << "Found '" << needle.substr(0, 4) << "...' at offset " << start << ", length " << length << std::endl;
        substitute.insert(substitute.end(), length - substitute.size(), '\0');
        input.replace(start, length, substitute);
      } else {
        std::cerr << "Error: Could not find '" << needle.substr(0, 4) << "'" << std::endl;
        exit(1);
      }
    }

    std::cout << "Saving patched binary to " << output << std::endl;
    util::filesystem::write_file(output, util::string::convert(input));
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    exit(1);
  }
}
