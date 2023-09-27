#ifndef MOONSHINE_SHARED_UTIL_FILESYSTEM_H_
#define MOONSHINE_SHARED_UTIL_FILESYSTEM_H_

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>

#include <util/platform.hpp>

#ifdef OS_LINUX
#include <unistd.h>
#endif

#ifdef OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace util {

namespace filesystem {

#ifdef OS_WINDOWS
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

inline void write_file(const std::string &path, const std::vector<unsigned char> &data) {
  std::ofstream outfile(path, std::ofstream::binary);
  if (outfile.good()) {
    outfile.write(reinterpret_cast<const char *>(&data[0]), data.size());
    outfile.close();
    return;
  }
  outfile.close();
  throw std::runtime_error("Could not write to file " + path);
}

inline void write_file(const std::string &path, const void *data, size_t length) {
  std::ofstream outfile(path, std::ofstream::binary);
  if (outfile.good()) {
    outfile.write(reinterpret_cast<const char *>(data), length);
    outfile.close();
    return;
  }
  outfile.close();
  throw std::runtime_error("Could not write to file " + path);
}

inline std::vector<unsigned char> read_file(const std::string &path) {
  if (std::filesystem::exists(path)) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (file.good()) {
      std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);
      std::vector<unsigned char> buffer(size);
      file.read(reinterpret_cast<char *>(&buffer[0]), size);
      file.close();
      return buffer;
    }
    file.close();
  }
  throw std::runtime_error("Could not read from file " + path);
}

inline bool is_readable(const std::string &path)
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

inline std::string get_temp_filepath(const std::string &suffix = "") {
  std::string directory = std::filesystem::temp_directory_path().string();

  std::string str("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
  std::random_device rd;
  std::mt19937 generator(rd());
  std::shuffle(str.begin(), str.end(), generator);

  if (directory.find(PATH_SEPARATOR, directory.size() - std::string_view(PATH_SEPARATOR).size()) == directory.npos)
    directory.append(PATH_SEPARATOR);
  return directory.append(str.substr(0, 8)).append(suffix);
}

inline std::string get_temp_filename(const std::string &suffix = "") {
  std::string str("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
  std::random_device rd;
  std::mt19937 generator(rd());
  std::shuffle(str.begin(), str.end(), generator);

  return str.substr(0, 8).append(suffix);
}

#if defined(OS_WINDOWS)
#  include <stdlib.h>
#elif defined(OS_LINUX)
#  include <unistd.h>
#  include <limits.h>
#elif defined(OS_MACOS)
#  include <mach-o/dyld.h>
#endif

inline std::string get_executable_path() {
#if defined(OS_WINDOWS)
  /*char full_path[MAX_PATH];
  char *exePath;
  if (_get_pgmptr(&exePath) != 0)
    return {};
  return std::filesystem::canonical(exePath).string();*/

  char exePath[MAX_PATH] = {0};
  GetModuleFileNameA(nullptr, exePath, MAX_PATH);
  return std::filesystem::canonical(exePath).string();
#elif defined(OS_LINUX)
  char exePath[PATH_MAX];
  ssize_t len = ::readlink("/proc/self/exe", exePath, sizeof(exePath));
  if (len == -1 || len == sizeof(exePath))
      len = 0;
  exePath[len] = '\0';
  return std::filesystem::canonical(exePath).string();
#elif defined(OS_MACOS)
  char exePath[PATH_MAX];
  uint32_t len = sizeof(exePath);
  if (_NSGetExecutablePath(exePath, &len) != 0)
  {
      exePath[0] = '\0'; // buffer too small (!)
  } else
  {
      // resolve symlinks, ., .. if possible
      char *canonicalPath = realpath(exePath, nullptr);
      if (canonicalPath != nullptr)
      {
          strncpy(exePath,canonicalPath,len);
          free(canonicalPath);
      }
  }
  return std::filesystem::canonical(exePath).string();
#endif
}

} // filesystem

} // util

#endif //MOONSHINE_SHARED_UTIL_FILESYSTEM_H_
