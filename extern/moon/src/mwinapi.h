#ifndef MOON_WINAPI_H_
#define MOON_WINAPI_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>
#include <iphlpapi.h>

typedef struct _PRTL_RB_TREE *PRTL_RB_TREE;
typedef struct _RTL_BALANCED_NODE *PRTL_BALANCED_NODE;

typedef PVOID (WINAPI *func_VirtualAlloc)(PVOID, SIZE_T, DWORD, DWORD);
typedef BOOL (WINAPI *func_VirtualFree)(LPVOID, SIZE_T, DWORD);
typedef BOOL (WINAPI *func_VirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD);
typedef FARPROC (WINAPI *func_GetProcAddress)(HMODULE, LPCSTR);
typedef HMODULE (WINAPI *func_GetModuleHandleA)(LPCSTR);
typedef BOOL (WINAPI *func_GetModuleHandleExA)(DWORD, LPCSTR, HMODULE*);
typedef HMODULE (WINAPI *func_LoadLibraryA)(LPCSTR);
typedef HMODULE (WINAPI *func_LoadLibraryExA)( LPCSTR, HANDLE, DWORD);
typedef BOOL (WINAPI *func_FreeLibrary)(HMODULE);
typedef DWORD (WINAPI *func_GetModuleFileNameA)(HMODULE, LPSTR, DWORD);
typedef DWORD (WINAPI *func_GetCurrentProcessId)();
typedef HANDLE (WINAPI *func_GetCurrentProcess)();
typedef BOOL (WINAPI *func_GetTokenInformation)(HANDLE, TOKEN_INFORMATION_CLASS, LPVOID, DWORD, PDWORD);
typedef BOOL (WINAPI *func_LookupAccountSidA)(LPCSTR, PSID, LPSTR, LPDWORD, LPSTR, LPDWORD, PSID_NAME_USE);
typedef BOOL (WINAPI *func_CloseHandle)(HANDLE);
typedef BOOL (WINAPI *func_QueryFullProcessImageNameA)(HANDLE, DWORD, LPSTR, PDWORD);
typedef BOOL (WINAPI *func_GetComputerNameA)(LPSTR, LPDWORD);
typedef BOOL (WINAPI *func_GetAdaptersInfo)(PIP_ADAPTER_INFO, PULONG);
typedef BOOL (WINAPI *func_OpenProcessToken)(HANDLE, DWORD, PHANDLE);
typedef BOOL (NTAPI* func_RtlAddFunctionTable)(PRUNTIME_FUNCTION, DWORD, DWORD64);
typedef VOID (NTAPI* func_RtlCaptureContext)(PCONTEXT);
typedef PRUNTIME_FUNCTION (NTAPI* func_RtlLookupFunctionEntry)(DWORD64, PDWORD64, PUNWIND_HISTORY_TABLE);
typedef PEXCEPTION_ROUTINE (NTAPI* func_RtlVirtualUnwind)(DWORD, DWORD64, DWORD64, PRUNTIME_FUNCTION, PCONTEXT, PVOID*, PDWORD64, PKNONVOLATILE_CONTEXT_POINTERS);
typedef SIZE_T (WINAPI *func_VirtualQuery)(LPCVOID, PMEMORY_BASIC_INFORMATION, SIZE_T);
typedef DWORD (WINAPI *func_GetLastError)(VOID);
typedef VOID (WINAPI *func_SetLastError)(DWORD);
typedef HLOCAL (WINAPI *func_LocalAlloc)(UINT, SIZE_T);
typedef HLOCAL (WINAPI *func_LocalFree)(HLOCAL);
typedef HLOCAL (WINAPI *func_LocalReAlloc)(HLOCAL, SIZE_T, UINT);
typedef int (WINAPI *func_MultiByteToWideChar)(UINT, DWORD, LPCCH, int, LPWSTR, int);
typedef NTSTATUS (NTAPI *func_RtlHashUnicodeString)(PCUNICODE_STRING, BOOLEAN, ULONG, PULONG);
typedef VOID (NTAPI *func_RtlRbInsertNodeEx)(PRTL_RB_TREE, PRTL_BALANCED_NODE, BOOLEAN, PRTL_BALANCED_NODE);
typedef int (NTAPI* func_RtlRbRemoveNode)(PRTL_RB_TREE, PRTL_BALANCED_NODE);
typedef NTSTATUS (NTAPI *func_NtOpenFile)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);
typedef NTSTATUS (NTAPI *func_NtCreateSection)(PHANDLE, ULONG, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
typedef NTSTATUS (NTAPI *func_NtMapViewOfSection)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG);
typedef NTSTATUS (NTAPI *func_NtUnmapViewOfSection)(HANDLE, PVOID);
typedef NTSTATUS (NTAPI *func_NtQuerySystemTime)(PLARGE_INTEGER);

typedef struct moon_win_api
{
  func_GetProcAddress GetProcAddress;
  func_GetModuleHandleA GetModuleHandleA;
  func_GetModuleHandleExA GetModuleHandleExA;
  func_LoadLibraryA LoadLibraryA;
  func_LoadLibraryExA LoadLibraryExA;
  func_FreeLibrary FreeLibrary;
  func_GetModuleFileNameA GetModuleFileNameA;
  func_GetCurrentProcessId GetCurrentProcessId;
  func_GetCurrentProcess GetCurrentProcess;
  func_GetTokenInformation GetTokenInformation;
  func_LookupAccountSidA LookupAccountSidA;
  func_QueryFullProcessImageNameA QueryFullProcessImageNameA;
  func_GetComputerNameA GetComputerNameA;
  func_GetAdaptersInfo GetAdaptersInfo;
  func_OpenProcessToken OpenProcessToken;
  func_RtlCaptureContext RtlCaptureContext;
  func_RtlLookupFunctionEntry RtlLookupFunctionEntry;
  func_RtlVirtualUnwind RtlVirtualUnwind;
  func_VirtualQuery VirtualQuery;
  func_VirtualAlloc VirtualAlloc;
  func_VirtualFree VirtualFree;
  func_VirtualProtect VirtualProtect;
  func_CloseHandle CloseHandle;
  func_GetLastError GetLastError;
  func_SetLastError SetLastError;
  func_LocalAlloc LocalAlloc;
  func_LocalFree LocalFree;
  func_LocalReAlloc LocalReAlloc;
  func_RtlAddFunctionTable RtlAddFunctionTable;
  func_RtlHashUnicodeString RtlHashUnicodeString;
  func_RtlRbInsertNodeEx RtlRbInsertNodeEx;
  func_RtlRbRemoveNode RtlRbRemoveNode;
  func_MultiByteToWideChar MultiByteToWideChar;
  func_NtOpenFile NtOpenFile;
  func_NtCreateSection NtCreateSection;
  func_NtMapViewOfSection NtMapViewOfSection;
  func_NtUnmapViewOfSection NtUnmapViewOfSection;
  func_NtQuerySystemTime NtQuerySystemTime;
} moon_win_api;

extern moon_win_api* win_api;

int moon_init_win_api();
HMODULE get_base_address();

HMODULE moon_GetModuleHandleA(LPCSTR lpModuleName);
FARPROC moon_GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
LPVOID moon_VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
BOOL moon_VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
BOOL moon_VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
HMODULE moon_LoadLibraryA(LPCSTR lpLibFileName);
HMODULE moon_LoadLibraryExA( LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
BOOL moon_FreeLibrary(HMODULE hLibModule);
DWORD moon_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
BOOL moon_GetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule);

#endif //MOON_WINAPI_H_
