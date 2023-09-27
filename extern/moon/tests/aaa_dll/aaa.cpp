#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <ostream>
#include <sstream>

#include "aaa.h"

using namespace std;

extern "C" __declspec(dllexport) char* FunctionA()
{
  ostringstream *stringStream = new ostringstream();
  ostream *outputStream = stringStream;
  (*outputStream) << "Hello from function A";
  string output = (*stringStream).str();
  const char* outputStr = output.c_str();
  char* out = new char[output.size()+1];
  strcpy(out, outputStr);
  out[output.size()] = '\0';
  return out;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID)
{
  switch (dwReason)
  {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}
