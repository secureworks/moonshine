#include "mwinapi.h"

//#include <stdlib.h>
#include "mutil.h"
#include "deinamig.h"

moon_win_api* win_api = NULL;

int moon_init_win_api() {
  if (win_api != NULL) return 0;

  win_api = (moon_win_api*) malloc(sizeof(moon_win_api));

  char ntdll_dll[] = {'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l', '\0'};
  HMODULE hNtdll = moon_GetModuleHandleA(ntdll_dll);
  char kernel32_dll[] = {'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', '\0'};
  HMODULE hKernel32 = moon_GetModuleHandleA(kernel32_dll);

  win_api->GetProcAddress = (func_GetProcAddress) moon_GetProcAddress;
  win_api->GetModuleHandleA = (func_GetModuleHandleA) moon_GetModuleHandleA;

  char cLoadLibraryA[] = { 'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'A', '\0' };
  win_api->LoadLibraryA = (func_LoadLibraryA) moon_GetProcAddress(hKernel32, cLoadLibraryA);

  char advapi32_dll[] = {'a','d','v','a','p','i','3','2','.','d','l','l','\0'};
  HMODULE hAdvapi32 = moon_GetModuleHandleA(advapi32_dll);
  if (hAdvapi32 == NULL) hAdvapi32 = win_api->LoadLibraryA(advapi32_dll);

  char iphlpapi_dll[] = {'i','p','h','l','p','a','p','i','.','d','l','l','\0'};
  HMODULE hIphlpapi = moon_GetModuleHandleA(iphlpapi_dll);
  if (hIphlpapi == NULL) hIphlpapi = win_api->LoadLibraryA(iphlpapi_dll);

  char cGetModuleHandleExA[] = { 'G', 'e', 't', 'M', 'o', 'd', 'u', 'l', 'e', 'H', 'a', 'n', 'd', 'l', 'e', 'E', 'x', 'A', 0 };
  win_api->GetModuleHandleExA = (func_GetModuleHandleExA) moon_GetProcAddress(hKernel32, cGetModuleHandleExA);
  char cLoadLibraryExA[] = { 'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'E', 'x', 'A', 0 };
  win_api->LoadLibraryExA = (func_LoadLibraryExA) moon_GetProcAddress(hKernel32, cLoadLibraryExA);
  char cFreeLibrary[] = { 'F', 'r', 'e', 'e', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 0 };
  win_api->FreeLibrary = (func_FreeLibrary) moon_GetProcAddress(hKernel32, cFreeLibrary);
  char cGetModuleFileNameA[] = { 'G', 'e', 't', 'M', 'o', 'd', 'u', 'l', 'e', 'F', 'i', 'l', 'e', 'N', 'a', 'm', 'e', 'A', 0 };
  win_api->GetModuleFileNameA = (func_GetModuleFileNameA)moon_GetProcAddress(hKernel32, cGetModuleFileNameA);
  char cGetCurrentProcessId[] = { 'G', 'e', 't', 'C', 'u', 'r', 'r', 'e', 'n', 't', 'P', 'r', 'o', 'c', 'e', 's', 's', 'I', 'd', 0 };
  win_api->GetCurrentProcessId = (func_GetCurrentProcessId)moon_GetProcAddress(hKernel32, cGetCurrentProcessId);
  char cGetCurrentProcess[] = { 'G', 'e', 't', 'C', 'u', 'r', 'r', 'e', 'n', 't', 'P', 'r', 'o', 'c', 'e', 's', 's', 0 };
  win_api->GetCurrentProcess = (func_GetCurrentProcess)moon_GetProcAddress(hKernel32, cGetCurrentProcess);
  char cGetTokenInformation[] = { 'G', 'e', 't', 'T', 'o', 'k', 'e', 'n', 'I', 'n', 'f', 'o', 'r', 'm', 'a', 't', 'i', 'o', 'n', 0 };
  win_api->GetTokenInformation = (func_GetTokenInformation)moon_GetProcAddress(hAdvapi32, cGetTokenInformation);
  char cLookupAccountSidA[] = { 'L', 'o', 'o', 'k', 'u', 'p', 'A', 'c', 'c', 'o', 'u', 'n', 't', 'S', 'i', 'd', 'A', 0 };
  win_api->LookupAccountSidA = (func_LookupAccountSidA)moon_GetProcAddress(hAdvapi32, cLookupAccountSidA);
  char cQueryFullProcessImageNameA[] = { 'Q', 'u', 'e', 'r', 'y', 'F', 'u', 'l', 'l', 'P', 'r', 'o', 'c', 'e', 's', 's', 'I', 'm', 'a', 'g', 'e', 'N', 'a', 'm', 'e', 'A', 0 };
  win_api->QueryFullProcessImageNameA = (func_QueryFullProcessImageNameA)moon_GetProcAddress(hKernel32, cQueryFullProcessImageNameA);
  char cGetComputerNameA[] = { 'G', 'e', 't', 'C', 'o', 'm', 'p', 'u', 't', 'e', 'r', 'N', 'a', 'm', 'e', 'A', 0 };
  win_api->GetComputerNameA = (func_GetComputerNameA)moon_GetProcAddress(hKernel32, cGetComputerNameA);
  char cGetAdaptersInfo[] = { 'G', 'e', 't', 'A', 'd', 'a', 'p', 't', 'e', 'r', 's', 'I', 'n', 'f', 'o', 0 };
  win_api->GetAdaptersInfo = (func_GetAdaptersInfo)moon_GetProcAddress(hIphlpapi, cGetAdaptersInfo);
  char cOpenProcessToken[] = { 'O', 'p', 'e', 'n', 'P', 'r', 'o', 'c', 'e', 's', 's', 'T', 'o', 'k', 'e', 'n', 0 };
  win_api->OpenProcessToken = (func_OpenProcessToken)moon_GetProcAddress(hAdvapi32, cOpenProcessToken);
  char cRtlCaptureContext[] = { 'R', 't', 'l', 'C', 'a', 'p', 't', 'u', 'r', 'e', 'C', 'o', 'n', 't', 'e', 'x', 't', 0 };
  win_api->RtlCaptureContext = (func_RtlCaptureContext)moon_GetProcAddress(hKernel32, cRtlCaptureContext);
  char cRtlLookupFunctionEntry[] = { 'R', 't', 'l', 'L', 'o', 'o', 'k', 'u', 'p', 'F', 'u', 'n', 'c', 't', 'i', 'o', 'n', 'E', 'n', 't', 'r', 'y', 0 };
  win_api->RtlLookupFunctionEntry = (func_RtlLookupFunctionEntry)moon_GetProcAddress(hKernel32, cRtlLookupFunctionEntry);
  char cRtlVirtualUnwind[] = { 'R', 't', 'l', 'V', 'i', 'r', 't', 'u', 'a', 'l', 'U', 'n', 'w', 'i', 'n', 'd', 0 };
  win_api->RtlVirtualUnwind = (func_RtlVirtualUnwind)moon_GetProcAddress(hKernel32, cRtlVirtualUnwind);
  char cVirtualQuery[] = { 'V', 'i', 'r', 't', 'u', 'a', 'l', 'Q', 'u', 'e', 'r', 'y', 0 };
  win_api->VirtualQuery = (func_VirtualQuery)moon_GetProcAddress(hKernel32, cVirtualQuery);
  char cVirtualAlloc[] = {'V', 'i', 'r', 't', 'u', 'a', 'l', 'A', 'l', 'l', 'o', 'c', '\0'};
  win_api->VirtualAlloc = (func_VirtualAlloc) moon_GetProcAddress(hKernel32, cVirtualAlloc);
  char cVirtualFree[] = {'V', 'i', 'r', 't', 'u', 'a', 'l', 'F', 'r', 'e', 'e', '\0'};
  win_api->VirtualFree = (func_VirtualFree) moon_GetProcAddress(hKernel32, cVirtualFree);
  char cVirtualProtect[] = {'V', 'i', 'r', 't', 'u', 'a', 'l', 'P', 'r', 'o', 't', 'e', 'c', 't', '\0'};
  win_api->VirtualProtect = (func_VirtualProtect) moon_GetProcAddress(hKernel32, cVirtualProtect);
  char cCloseHandle[] = {'C', 'l', 'o', 's', 'e', 'H','a','n','d','l','e','\0'};
  win_api->CloseHandle = (func_CloseHandle) moon_GetProcAddress(hKernel32, cCloseHandle);
  char cGetLastError[] = {'G', 'e', 't', 'L', 'a', 's', 't', 'E', 'r', 'r', 'o', 'r', '\0'};
  win_api->GetLastError = (func_GetLastError) moon_GetProcAddress(hKernel32, cGetLastError);
  char cSetLastError[] = {'S', 'e', 't', 'L', 'a', 's', 't', 'E', 'r', 'r', 'o', 'r', '\0'};
  win_api->SetLastError = (func_SetLastError) moon_GetProcAddress(hKernel32, cSetLastError);
  char cLocalAlloc[] = {'L', 'o', 'c', 'a', 'l', 'A', 'l', 'l', 'o', 'c', '\0'};
  win_api->LocalAlloc = (func_LocalAlloc) moon_GetProcAddress(hKernel32, cLocalAlloc);
  char cLocalFree[] = {'L', 'o', 'c', 'a', 'l', 'F', 'r', 'e', 'e', '\0'};
  win_api->LocalFree = (func_LocalFree) moon_GetProcAddress(hKernel32, cLocalFree);
  char cLocalReAlloc[] = {'L', 'o', 'c', 'a', 'l', 'R', 'e', 'A', 'l', 'l', 'o', 'c', '\0'};
  win_api->LocalReAlloc = (func_LocalReAlloc) moon_GetProcAddress(hKernel32, cLocalReAlloc);
  char cRtlAddFunctionTable[] = {'R', 't', 'l', 'A', 'd', 'd', 'F', 'u', 'n', 'c', 't', 'i', 'o', 'n', 'T', 'a', 'b', 'l', 'e', '\0'};
  win_api->RtlAddFunctionTable = (func_RtlAddFunctionTable) moon_GetProcAddress(hKernel32, cRtlAddFunctionTable);
  char cRtlHashUnicodeString[] = {'R', 't', 'l', 'H', 'a', 's', 'h', 'U', 'n', 'i', 'c', 'o', 'd', 'e', 'S', 't', 'r', 'i', 'n', 'g', '\0'};
  win_api->RtlHashUnicodeString = (func_RtlHashUnicodeString) moon_GetProcAddress(hNtdll, cRtlHashUnicodeString);
  char cRtlRbInsertNodeEx[] = {'R', 't', 'l', 'R', 'b', 'I', 'n', 's', 'e', 'r', 't', 'N', 'o', 'd', 'e', 'E', 'x', '\0'};
  win_api->RtlRbInsertNodeEx = (func_RtlRbInsertNodeEx) moon_GetProcAddress(hNtdll, cRtlRbInsertNodeEx);
  char cRtlRbRemoveNode[] = {'R', 't', 'l', 'R', 'b', 'R', 'e', 'm', 'o', 'v', 'e', 'N', 'o', 'd', 'e', '\0'};
  win_api->RtlRbRemoveNode = (func_RtlRbRemoveNode) moon_GetProcAddress(hNtdll, cRtlRbRemoveNode);
  char cMultiByteToWideChar[] = {'M','u','l','t','i','B','y','t','e','T','o','W','i','d','e','C','h','a','r','\0'};
  win_api->MultiByteToWideChar = (func_MultiByteToWideChar) moon_GetProcAddress(hKernel32, cMultiByteToWideChar);
  char cNtOpenFile[] = {'N', 't', 'O', 'p', 'e', 'n', 'F', 'i', 'l', 'e', '\0'};
  win_api->NtOpenFile = (func_NtOpenFile) moon_GetProcAddress(hNtdll, cNtOpenFile);
  char cNtCreateSection[] = {'N', 't', 'C', 'r', 'e', 'a', 't', 'e', 'S', 'e', 'c', 't', 'i', 'o', 'n', '\0'};
  win_api->NtCreateSection = (func_NtCreateSection) moon_GetProcAddress(hNtdll, cNtCreateSection);
  char cNtMapViewOfSection[] = {'N', 't', 'M', 'a', 'p', 'V', 'i', 'e', 'w', 'O', 'f', 'S', 'e', 'c', 't', 'i', 'o', 'n', '\0'};
  win_api->NtMapViewOfSection = (func_NtMapViewOfSection) moon_GetProcAddress(hNtdll, cNtMapViewOfSection);
  char cNtUnmapViewOfSection[] = {'N', 't', 'U', 'n', 'm', 'a', 'p', 'V', 'i', 'e', 'w', 'O', 'f', 'S', 'e', 'c', 't', 'i', 'o', 'n', '\0'};
  win_api->NtUnmapViewOfSection = (func_NtUnmapViewOfSection) moon_GetProcAddress(hNtdll, cNtUnmapViewOfSection);
  char cNtQuerySystemTime[] = {'N', 't', 'Q', 'u', 'e', 'r', 'y', 'S', 'y', 's', 't', 'e', 'm', 'T', 'i', 'm', 'e', '\0'};
  win_api->NtQuerySystemTime = (func_NtQuerySystemTime) moon_GetProcAddress(hNtdll, cNtQuerySystemTime);
  return 0;
}

FARPROC moon_GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
#ifdef SYSTEM_GETPROCADDRESS
  return GetProcAddress(hModule, lpProcName);
#else
  return deinamig_get_export_address_by_name(hModule, lpProcName, TRUE, TRUE);
#endif
}

HMODULE moon_GetModuleHandleA(LPCSTR lpModuleName) {
#ifdef SYSTEM_GETMODULEHANDLE
  return GetModuleHandleA(lpModuleName);
#else
  if (lpModuleName == NULL) {
    return get_base_address();
  }
  return deinamig_get_peb_ldr_library_entry_by_name(lpModuleName);
#endif
}

LPVOID moon_VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
  moon_init_win_api();
  return win_api->VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
}

BOOL moon_VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
  moon_init_win_api();
  return win_api->VirtualFree(lpAddress, dwSize, dwFreeType);
}

BOOL moon_VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect) {
  moon_init_win_api();
  return win_api->VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect);
}

HMODULE moon_LoadLibraryA(LPCSTR lpLibFileName) {
  moon_init_win_api();
  return win_api->LoadLibraryA(lpLibFileName);
}

HMODULE moon_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
  moon_init_win_api();
  return win_api->LoadLibraryExA(lpLibFileName, hFile, dwFlags);
}

BOOL moon_FreeLibrary(HMODULE hLibModule) {
  moon_init_win_api();
  return win_api->FreeLibrary(hLibModule);
}

DWORD moon_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) {
  moon_init_win_api();
  return win_api->GetModuleFileNameA(hModule, lpFilename, nSize);
}

BOOL moon_GetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule) {
  moon_init_win_api();
  return win_api->GetModuleHandleExA(dwFlags, lpModuleName, phModule);
}

static int capture_stack_backtrace(int FramesToSkip, int nFrames, PVOID* BackTrace)
{
#ifdef _WIN64
  CONTEXT ContextRecord;
  RtlCaptureContext(&ContextRecord);

  UINT iFrame;
  for (iFrame = 0; iFrame < (UINT)nFrames; iFrame++)
  {
    DWORD64 ImageBase;
    PRUNTIME_FUNCTION pFunctionEntry = RtlLookupFunctionEntry(ContextRecord.Rip, &ImageBase, NULL);

    if (pFunctionEntry == NULL)
    {
      if (iFrame != -1)
        iFrame--;           // Eat last as it's not valid.
      break;
    }

    PVOID HandlerData;
    DWORD64 EstablisherFrame;
    RtlVirtualUnwind(0 /*UNW_FLAG_NHANDLER*/,
                      ImageBase,
                      ContextRecord.Rip,
                      pFunctionEntry,
                      &ContextRecord,
                      &HandlerData,
                      &EstablisherFrame,
                      NULL);

    if(FramesToSkip > (int)iFrame)
      continue;

    BackTrace[iFrame - FramesToSkip] = (PVOID)ContextRecord.Rip;
  }
#else
  //
  //  This approach was taken from StackInfoManager.cpp / FillStackInfo
  //  http://www.codeproject.com/Articles/11221/Easy-Detection-of-Memory-Leaks
  //  - slightly simplified the function itself.
  //
  int regEBP;
  __asm mov regEBP, ebp;

  long *pFrame = (long*)regEBP;               // pointer to current function frame
  void* pNextInstruction;
  int iFrame = 0;

  //
  // Using __try/_catch is faster than using ReadProcessMemory or VirtualProtect.
  // We return whatever frames we have collected so far after exception was encountered.
  //
  __try {
      for (; iFrame < nFrames; iFrame++)
      {
          pNextInstruction = (void*)(*(pFrame + 1));

          if (!pNextInstruction)     // Last frame
              break;

          if (FramesToSkip > iFrame)
              continue;

          BackTrace[iFrame - FramesToSkip] = pNextInstruction;
          pFrame = (long*)(*pFrame);
      }
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
  }

#endif //_WIN64
  iFrame -= FramesToSkip;
  if(iFrame < 0)
    iFrame = 0;

  return iFrame;
} //capture_stack_backtrace

HMODULE get_base_address() {
  void *pfunc = &capture_stack_backtrace;
  capture_stack_backtrace(1, 2, &pfunc);
  MEMORY_BASIC_INFORMATION info;
  VirtualQuery(pfunc, &info, sizeof(MEMORY_BASIC_INFORMATION));
  return (HMODULE)(info.AllocationBase);
}
