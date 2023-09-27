#ifndef LLWYTHWR_INCLUDE_LLWYTHWR_INIT_H_
#define LLWYTHWR_INCLUDE_LLWYTHWR_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "llwythwr.h"

#if defined(__APPLE__)

/*
 * *** Function Declarations ***
 *
 * NSObjectFileImageReturnCode NSCreateObjectFileImageFromMemory(const void*, size_t, NSObjectFileImage*);
 * NSModule NSLinkModule(NSObjectFileImage, const char*, uint32_t);
 * bool NSDestroyObjectFileImage(NSObjectFileImage);
 * bool NSUnLinkModule(NSModule, unsigned int);
 * NSSymbol NSLookupSymbolInModule(NSModule, const char*);
 * void* NSAddressOfSymbol(NSSymbol symbol);
 * void* dlopen(const char*, int);
 * int dlclose(void*);
 * void* dlsym(void*, const char*);
 * void* mmap(void*, size_t, int, int, int, off_t);
 * int munmap(void*, size_t);
 *
 * void* malloc(size_t);
 * void free(void*);
 * void* memcpy(void*, const void*, size_t);
 * void* memset(void*, int, size_t);
 * int memcmp(const void*, const void*, size_t);
 */

#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <sys/mman.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

/**
 * Auxiliary function to initialise an os_api struct for testing.
 * Very likely **NOT** what you want to use as OPSEC is poor!!!
 * @param os_api  The api struct to initialize.
 * @return  Zero on success
 */
int llwythwr_init(llwythwr_os_api* os_api)
{
  os_api->NSCreateObjectFileImageFromMemory = NSCreateObjectFileImageFromMemory;
  os_api->NSLinkModule = NSLinkModule;
  os_api->NSDestroyObjectFileImage = NSDestroyObjectFileImage;
  os_api->NSUnLinkModule = NSUnLinkModule;
  os_api->NSLookupSymbolInModule = NSLookupSymbolInModule;
  os_api->NSAddressOfSymbol = NSAddressOfSymbol;
  os_api->dlopen = dlopen;
  os_api->dlclose = dlclose;
  os_api->dlsym = dlsym;
  os_api->mmap = mmap;
  os_api->munmap = munmap;

  os_api->malloc = malloc;
  os_api->free = free;
  os_api->_memcpy = memcpy;
  os_api->asprintf = asprintf;

  return 0;
}

#elif defined(_WIN32)

/*
 * *** Function Declarations ***
 *
 * HMODULE GetModuleHandleA(LPCSTR);
 * PVOID VirtualAlloc(PVOID, SIZE_T, DWORD, DWORD);
 * BOOL VirtualFree(LPVOID, SIZE_T, DWORD);
 * BOOL VirtualProtect(LPVOID, SIZE_T, DWORD, PDWORD);
 * BOOL CloseHandle(HANDLE);
 * HANDLE CreateFileA(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
 * DWORD GetLastError(VOID);
 * VOID SetLastError(DWORD);
 * HLOCAL LocalAlloc(UINT, SIZE_T);
 * HLOCAL LocalFree(HLOCAL);
 * HLOCAL LocalReAlloc(HLOCAL, SIZE_T, UINT);
 * BOOL RtlAddFunctionTable(PRUNTIME_FUNCTION, DWORD, DWORD64);
 */

/**
 * Auxiliary function to initialise an os_api struct for testing.
 * Very likely **NOT** what you want to use as OPSEC is poor!!!
 * @param os_api  The api struct to initialize.
 * @return  Zero on success
 */
int llwythwr_init(llwythwr_os_api* os_api)
{
  char ntdll_dll[] = {'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l', '\0'};
  HMODULE ntdll = GetModuleHandleA(ntdll_dll);
  char kernel32_dll[] = {'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', '\0'};
  HMODULE kernel32 = GetModuleHandleA(kernel32_dll);
  char cVirtualAlloc[] = {'V', 'i', 'r', 't', 'u', 'a', 'l', 'A', 'l', 'l', 'o', 'c', '\0'};
  os_api->VirtualAlloc = (Type_VirtualAlloc) GetProcAddress(kernel32, cVirtualAlloc);
  char cVirtualFree[] = {'V', 'i', 'r', 't', 'u', 'a', 'l', 'F', 'r', 'e', 'e', '\0'};
  os_api->VirtualFree = (Type_VirtualFree) GetProcAddress(kernel32, cVirtualFree);
  char cVirtualProtect[] = {'V', 'i', 'r', 't', 'u', 'a', 'l', 'P', 'r', 'o', 't', 'e', 'c', 't', '\0'};
  os_api->VirtualProtect = (Type_VirtualProtect) GetProcAddress(kernel32, cVirtualProtect);
  char cCloseHandle[] = {'C', 'l', 'o', 's', 'e', 'H','a','n','d','l','e','\0'};
  os_api->CloseHandle = (Type_CloseHandle) GetProcAddress(kernel32, cCloseHandle);
  char cGetLastError[] = {'G', 'e', 't', 'L', 'a', 's', 't', 'E', 'r', 'r', 'o', 'r', '\0'};
  os_api->GetLastError = (Type_GetLastError) GetProcAddress(kernel32, cGetLastError);
  char cSetLastError[] = {'S', 'e', 't', 'L', 'a', 's', 't', 'E', 'r', 'r', 'o', 'r', '\0'};
  os_api->SetLastError = (Type_SetLastError) GetProcAddress(kernel32, cSetLastError);
  char cLocalAlloc[] = {'L', 'o', 'c', 'a', 'l', 'A', 'l', 'l', 'o', 'c', '\0'};
  os_api->LocalAlloc = (Type_LocalAlloc) GetProcAddress(kernel32, cLocalAlloc);
  char cLocalFree[] = {'L', 'o', 'c', 'a', 'l', 'F', 'r', 'e', 'e', '\0'};
  os_api->LocalFree = (Type_LocalFree) GetProcAddress(kernel32, cLocalFree);
  char cLocalReAlloc[] = {'L', 'o', 'c', 'a', 'l', 'R', 'e', 'A', 'l', 'l', 'o', 'c', '\0'};
  os_api->LocalReAlloc = (Type_LocalReAlloc) GetProcAddress(kernel32, cLocalReAlloc);
  char cRtlAddFunctionTable[] = {'R', 't', 'l', 'A', 'd', 'd', 'F', 'u', 'n', 'c', 't', 'i', 'o', 'n', 'T', 'a', 'b', 'l', 'e', '\0'};
  os_api->RtlAddFunctionTable = (Type_RtlAddFunctionTable) GetProcAddress(kernel32, cRtlAddFunctionTable);
  char cRtlHashUnicodeString[] = {'R', 't', 'l', 'H', 'a', 's', 'h', 'U', 'n', 'i', 'c', 'o', 'd', 'e', 'S', 't', 'r', 'i', 'n', 'g', '\0'};
  os_api->RtlHashUnicodeString = (Type_RtlHashUnicodeString) GetProcAddress(ntdll, cRtlHashUnicodeString);
  char cRtlRbInsertNodeEx[] = {'R', 't', 'l', 'R', 'b', 'I', 'n', 's', 'e', 'r', 't', 'N', 'o', 'd', 'e', 'E', 'x', '\0'};
  os_api->RtlRbInsertNodeEx = (Type_RtlRbInsertNodeEx) GetProcAddress(ntdll, cRtlRbInsertNodeEx);
  char cRtlRbRemoveNode[] = {'R', 't', 'l', 'R', 'b', 'R', 'e', 'm', 'o', 'v', 'e', 'N', 'o', 'd', 'e', '\0'};
  os_api->RtlRbRemoveNode = (Type_RtlRbRemoveNode) GetProcAddress(ntdll, cRtlRbRemoveNode);
  char cMultiByteToWideChar[] = {'M','u','l','t','i','B','y','t','e','T','o','W','i','d','e','C','h','a','r','\0'};
  os_api->MultiByteToWideChar = (Type_MultiByteToWideChar) GetProcAddress(kernel32, cMultiByteToWideChar);
  char cNtOpenFile[] = {'N', 't', 'O', 'p', 'e', 'n', 'F', 'i', 'l', 'e', '\0'};
  os_api->NtOpenFile = (Type_NtOpenFile) GetProcAddress(ntdll, cNtOpenFile);
  char cNtCreateSection[] = {'N', 't', 'C', 'r', 'e', 'a', 't', 'e', 'S', 'e', 'c', 't', 'i', 'o', 'n', '\0'};
  os_api->NtCreateSection = (Type_NtCreateSection) GetProcAddress(ntdll, cNtCreateSection);
  char cNtMapViewOfSection[] = {'N', 't', 'M', 'a', 'p', 'V', 'i', 'e', 'w', 'O', 'f', 'S', 'e', 'c', 't', 'i', 'o', 'n', '\0'};
  os_api->NtMapViewOfSection = (Type_NtMapViewOfSection) GetProcAddress(ntdll, cNtMapViewOfSection);
  char cNtUnmapViewOfSection[] = {'N', 't', 'U', 'n', 'm', 'a', 'p', 'V', 'i', 'e', 'w', 'O', 'f', 'S', 'e', 'c', 't', 'i', 'o', 'n', '\0'};
  os_api->NtUnmapViewOfSection = (Type_NtUnmapViewOfSection) GetProcAddress(ntdll, cNtUnmapViewOfSection);
  char cNtQuerySystemTime[] = {'N', 't', 'Q', 'u', 'e', 'r', 'y', 'S', 'y', 's', 't', 'e', 'm', 'T', 'i', 'm', 'e', '\0'};
  os_api->NtQuerySystemTime = (Type_NtQuerySystemTime) GetProcAddress(ntdll, cNtQuerySystemTime);
  return 0;
}

#else

/*
 * void* dlopen(const char*, int);
 * int dlclose(void*);
 * void* dlsym(void*, const char*);
 * char* dlerror(void);
 * pid_t getpid(void);
 * int memfd_create(const char*, unsigned int);
 * int snprintf(char*, size_t, const char *, ...);
 * ssize_t write(int, const void *, size_t);
 * void* malloc(size_t);
 * void free(void*);
 */

#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/syscall.h>

static int memfd_create_wrapper(const char *name, unsigned int flags) {
    return syscall(SYS_memfd_create, name, 1);
}

/**
 * Auxiliary function to initialise an os_api struct for testing.
 * Very likely **NOT** what you want to use as OPSEC is poor!!!
 * @param os_api  The api struct to initialize.
 * @return  Zero on success
 */
int llwythwr_init(llwythwr_os_api* os_api)
{
  os_api->dlopen = dlopen;
  os_api->dlclose = dlclose;
  os_api->dlsym = dlsym;
  os_api->dlerror = dlerror;
  os_api->getpid = getpid;
  os_api->memfd_create = memfd_create_wrapper;
  os_api->snprintf = snprintf;
  os_api->write = write;
  os_api->malloc = malloc;
  os_api->free = free;

  return 0;
}

#endif

#ifdef __cplusplus
}
#endif

#endif //LLWYTHWR_INCLUDE_LLWYTHWR_INIT_H_
