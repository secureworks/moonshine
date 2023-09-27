#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <ostream>
#include <sstream>
#include <iostream>

#include "aaa.h"

using namespace std;

extern "C" __declspec(dllexport) char* FunctionB()
{
  ostream *outputStream = NULL;
  // If you want to output to cout, simply set outputStream to &cout.
  // This allows you to write a program that can switch between outputting to string or to cout.
  //outputStream = &cout;
  ostringstream *stringStream = new ostringstream();
  outputStream = stringStream;

  //(*outputStream) << "Hello from test function B";
  (*outputStream) << FunctionA() << " and from function B";

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