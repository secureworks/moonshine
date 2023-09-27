#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <catch2/catch_test_macros.hpp>
#include "deinamig.h"

#if defined(_WIN32)
#define LIB_SUFFIX ".dll"
#define PATH_SEP "\\"
#elif defined(__linux__)
#define LIB_SUFFIX ".so"
#define PATH_SEP "/"
#else
#define LIB_SUFFIX ".dylib"
#define PATH_SEP "/"
#endif

#if _WIN32

#define ROR8(v) (v >> 8 | v << 24)

unsigned long hashit(const char* name, unsigned long key)
{
  unsigned long i = 0;
  unsigned long hash = key;

  while (name[i])
  {
    unsigned short PartialName = *(unsigned short*)((ULONG64)name + i++);
    hash ^= PartialName + ROR8(hash);
  }

  return hash;
}

TEST_CASE("by_name")
{
  auto handle = deinamig_get_peb_ldr_library_entry_by_name("ntdll.dll");
  REQUIRE(handle != nullptr);

  auto function = deinamig_get_export_address_by_name(handle, "NtCreateThread", FALSE, FALSE);
  REQUIRE(function != nullptr);
}

TEST_CASE("by_hash")
{
  // Must be lowercase and end with .dll suffix
  auto hash = hashit("ntdll.dll", 0x12345678);
  auto handle = deinamig_get_peb_ldr_library_entry_by_hash(hash, 0x12345678, hashit);
  REQUIRE(handle != nullptr);

  hash = hashit("NtCreateThread", 0x12345678);
  auto function = deinamig_get_export_address_by_hash(handle, hash, 0x12345678, hashit, FALSE, FALSE);
  REQUIRE(function != nullptr);
}

TEST_CASE("from_disk")
{
  auto handle = deinamig_load_library_from_disk((char *) "sample_dll" PATH_SEP "sample" LIB_SUFFIX);
  REQUIRE(handle != nullptr);

  typedef int(*addNumbers)(int, int);
  auto func_addNumbers = (addNumbers) deinamig_get_native_export_address(handle, "addNumbers");
  REQUIRE(func_addNumbers != nullptr);

  auto ret = func_addNumbers(1, 2);
  REQUIRE(ret == 3);

  deinamig_unload_library_from_disk(handle);
}

TEST_CASE("deinamig_get_library_address_by_name")
{
  auto function = deinamig_get_library_address_by_name("ntdll.dll", "NtCreateThread", FALSE, FALSE);
  REQUIRE(function != nullptr);
}

TEST_CASE("deinamig_get_library_address_by_hash")
{
  auto hash = hashit("addNumbers", 0x12345678);
  auto function = deinamig_get_library_address_by_hash((char *) "sample_dll" PATH_SEP "sample" LIB_SUFFIX,
                                                       hash, 0x12345678, hashit, TRUE, FALSE);
  REQUIRE(function != nullptr);
}

#include <winternl.h>

typedef NTSTATUS(NTAPI *Type_RtlGetVersion)(LPOSVERSIONINFOEXW lpVersionInformation);
typedef NTSTATUS(NTAPI *Type_RtlInitUnicodeString)(PUNICODE_STRING DestinationString, PCWSTR SourceString);
typedef NTSTATUS(NTAPI *Type_NtOpenSection)(PHANDLE SectionHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes);
typedef NTSTATUS(NTAPI *Type_NtCreateSection)(PHANDLE, ULONG, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
typedef NTSTATUS(NTAPI *Type_NtMapViewOfSection)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG);
typedef NTSTATUS(NTAPI *Type_NtUnmapViewOfSection)(HANDLE, PVOID);

TEST_CASE("get_syscall")
{
  auto ntdll = deinamig_get_peb_ldr_library_entry_by_name("ntdll.dll");
  REQUIRE(ntdll != nullptr);

  auto fRtlGetVersion = (Type_RtlGetVersion) deinamig_get_export_address_by_name(ntdll, "RtlGetVersion", FALSE, FALSE);
  REQUIRE(fRtlGetVersion != nullptr);

  OSVERSIONINFOEXW osInfo = {0};
  osInfo.dwOSVersionInfoSize = sizeof(osInfo);
  fRtlGetVersion(&osInfo);

  DWORD syscall_number = 25;
  if (osInfo.dwBuildNumber <= 7601) { // Windows 7 and below
    syscall_number = 22;
  }
  else if (osInfo.dwBuildNumber == 9200) { // Windows 8
    syscall_number = 23;
  }
  else if (osInfo.dwBuildNumber == 9600) { // Windows 8.1
    syscall_number = 24;
  }

  unsigned long key = 0x12345678;
  auto hash = hashit("NtQueryInformationProcess", key);

  /*
   * Search NTDLL already loaded into the process...maybe hooked...
   */

  auto syscall_number1 = deinamig_get_syscall_number_syswhispers2_by_hash(ntdll, hash, key, hashit);
  REQUIRE(syscall_number1 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number1);

  auto function_address = deinamig_get_export_address_by_hash(ntdll, hash, key, hashit, FALSE, FALSE);
  REQUIRE(function_address != nullptr);
  auto syscall_number2 = deinamig_get_syscall_number_hellsgate(function_address, TRUE);
  REQUIRE(syscall_number2 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number2);

  auto syscall_number3 = deinamig_get_syscall_number_runtbl_by_hash(ntdll, hash, key, hashit);
  REQUIRE(syscall_number3 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number3);

  /*
   * Dual-load from Section
   */

  auto fRtlInitUnicodeString = (Type_RtlInitUnicodeString) deinamig_get_export_address_by_name(ntdll, "RtlInitUnicodeString", FALSE, FALSE);

  UNICODE_STRING KnownDllsNtDllName;
  fRtlInitUnicodeString(&KnownDllsNtDllName, L"\\KnownDlls\\ntdll.dll");

  OBJECT_ATTRIBUTES ObjectAttributes;
  InitializeObjectAttributes(&ObjectAttributes, &KnownDllsNtDllName, OBJ_CASE_INSENSITIVE, 0, 0);

  auto fNtOpenSection = (Type_NtOpenSection) deinamig_get_export_address_by_name(ntdll, "NtOpenSection", FALSE, FALSE);
  HANDLE section_handle = nullptr;
  NTSTATUS status = fNtOpenSection(&section_handle, SECTION_MAP_EXECUTE | SECTION_MAP_READ | SECTION_QUERY, &ObjectAttributes);
  REQUIRE(NT_SUCCESS(status));

  LARGE_INTEGER SectionOffset;
  SectionOffset.LowPart = 0;
  SectionOffset.HighPart = 0;

  auto fNtMapViewOfSection = (Type_NtMapViewOfSection) deinamig_get_export_address_by_name(ntdll, "NtMapViewOfSection", FALSE, FALSE);
  PVOID view_base = nullptr;
  size_t view_size = 0;
  status = fNtMapViewOfSection(section_handle, (HANDLE) -1, &view_base, 0, 0, &SectionOffset, &view_size, 0x1, 0, PAGE_EXECUTE_READ);
  REQUIRE(NT_SUCCESS(status));

  auto syscall_number4 = deinamig_get_syscall_number_syswhispers2_by_hash(view_base, hash, key, hashit);
  REQUIRE(syscall_number4 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number4);

  function_address = deinamig_get_export_address_by_hash(view_base, hash, key, hashit, FALSE, FALSE);
  REQUIRE(function_address != nullptr);
  auto syscall_number5 = deinamig_get_syscall_number_hellsgate(function_address, TRUE);
  REQUIRE(syscall_number5 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number5);

  auto syscall_number6 = deinamig_get_syscall_number_runtbl_by_hash(view_base, hash, key, hashit);
  REQUIRE(syscall_number6 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number6);

  auto fNtUnmapViewOfSection = (Type_NtUnmapViewOfSection) deinamig_get_export_address_by_name(ntdll, "NtUnmapViewOfSection", FALSE, FALSE);
  fNtUnmapViewOfSection((HANDLE) -1, view_base);
  CloseHandle(section_handle);

  /*
   * Dual-load from Disk (SEC_IMAGE)
   */

  // Hint, use 'llwythwr_map_library_from_disk_to_section' instead of this first part...

  auto filename = R"(C:\Windows\System32\ntdll.dll)";
  HANDLE file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
  REQUIRE(file_handle != nullptr);

  auto fNtCreateSection = (Type_NtCreateSection) deinamig_get_export_address_by_name(ntdll, "NtCreateSection", FALSE, FALSE);
  section_handle = nullptr;
  status = fNtCreateSection(&section_handle, SECTION_ALL_ACCESS, nullptr, nullptr, PAGE_READONLY, SEC_IMAGE, file_handle);
  REQUIRE(NT_SUCCESS(status));

  view_base = nullptr;
  view_size = 0;
  status = fNtMapViewOfSection(section_handle, (HANDLE) -1, &view_base, 0, 0, nullptr, &view_size, 0x2, 0, PAGE_READWRITE);
  REQUIRE(NT_SUCCESS(status));

  auto syscall_number7 = deinamig_get_syscall_number_syswhispers2_by_hash(view_base, hash, key, hashit);
  REQUIRE(syscall_number7 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number7);

  function_address = deinamig_get_export_address_by_hash(view_base, hash, key, hashit, FALSE, FALSE);
  REQUIRE(function_address != nullptr);
  auto syscall_number8 = deinamig_get_syscall_number_hellsgate(function_address, TRUE);
  REQUIRE(syscall_number8 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number8);

  auto syscall_number9 = deinamig_get_syscall_number_runtbl_by_hash(view_base, hash, key, hashit);
  REQUIRE(syscall_number9 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number9);

  fNtUnmapViewOfSection((HANDLE) -1, view_base);
  CloseHandle(section_handle);

  /*
   * Dual-load from Disk (SEC_COMMIT i.e. raw binary)
   */

  section_handle = nullptr;
  status = fNtCreateSection(&section_handle, SECTION_ALL_ACCESS, nullptr, nullptr, PAGE_READONLY, SEC_COMMIT, file_handle);
  REQUIRE(NT_SUCCESS(status));

  view_base = nullptr;
  view_size = 0;
  status = fNtMapViewOfSection(section_handle, (HANDLE) -1, &view_base, 0, 0, nullptr, &view_size, 0x2, 0, PAGE_READONLY);
  REQUIRE(NT_SUCCESS(status));

  function_address = deinamig_get_export_address_from_raw_by_hash(view_base, hash, key, hashit);
  REQUIRE(function_address != nullptr);
  auto syscall_number10 = deinamig_get_syscall_number_hellsgate(function_address, FALSE);
  REQUIRE(syscall_number10 != ULONG_MAX);
  REQUIRE(syscall_number == syscall_number10);

  fNtUnmapViewOfSection((HANDLE) -1, view_base);
  CloseHandle(section_handle);
}

typedef NTSTATUS (NTAPI *Type_NtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

TEST_CASE("get_syscall_number_ldrp_thunk_signature")
{
  auto fRtlGetVersion = (Type_RtlGetVersion) deinamig_get_export_address_by_name(deinamig_get_peb_ldr_library_entry_by_name("ntdll.dll"),
                                                                                 "RtlGetVersion", FALSE, FALSE);
  REQUIRE(fRtlGetVersion != nullptr);

  OSVERSIONINFOEXW osInfo = {0};
  osInfo.dwOSVersionInfoSize = sizeof(osInfo);
  fRtlGetVersion(&osInfo);

  DWORD t_NtOpenFile = 51;
  DWORD t_NtOpenSection = 55;
  DWORD t_NtCreateSection = 74;
  DWORD t_NtMapViewOfSection = 40;
  DWORD t_NtQueryAttributesFile = 61;

  DWORD syscall_NtOpenFile = 0;
  DWORD syscall_NtOpenSection = 0;
  DWORD syscall_NtCreateSection = 0;
  DWORD syscall_NtMapViewOfSection = 0;
  DWORD syscall_NtQueryAttributesFile = 0;

  int ret = deinamig_get_syscall_number_ldrp_thunk_signature(&syscall_NtOpenFile,
                                                             &syscall_NtOpenSection,
                                                             &syscall_NtCreateSection,
                                                             &syscall_NtMapViewOfSection,
                                                             &syscall_NtQueryAttributesFile);

  REQUIRE(ret);

  REQUIRE(syscall_NtOpenFile == t_NtOpenFile);
  REQUIRE(syscall_NtOpenSection == t_NtOpenSection);
  REQUIRE(syscall_NtCreateSection == t_NtCreateSection);
  REQUIRE(syscall_NtMapViewOfSection == t_NtMapViewOfSection);
  REQUIRE(syscall_NtQueryAttributesFile == t_NtQueryAttributesFile);
}

TEST_CASE("build_direct_syscall_stub")
{
  // Create result to compare against
  auto ntdll = deinamig_get_peb_ldr_library_entry_by_name("ntdll.dll");
  REQUIRE(ntdll != nullptr);

  HANDLE hProcess = GetCurrentProcess();
  PROCESS_BASIC_INFORMATION pbi = {0};
  DWORD dwSize = sizeof(PROCESS_BASIC_INFORMATION);
  DWORD dwSizeNeeded = 0;

  auto fNtQueryInformationProcess1 = (Type_NtQueryInformationProcess) deinamig_get_export_address_by_name(ntdll, "NtQueryInformationProcess", FALSE, FALSE);
  REQUIRE(fNtQueryInformationProcess1 != nullptr);

  NTSTATUS status = fNtQueryInformationProcess1(hProcess, ProcessBasicInformation, &pbi, dwSize, &dwSizeNeeded);
  REQUIRE(NT_SUCCESS(status));
  REQUIRE(pbi.UniqueProcessId);
  auto UniqueProcessId = pbi.UniqueProcessId;

  auto fRtlGetVersion = (Type_RtlGetVersion) deinamig_get_export_address_by_name(ntdll, "RtlGetVersion", FALSE, FALSE);
  REQUIRE(fRtlGetVersion != nullptr);

  OSVERSIONINFOEXW osInfo = {0};
  osInfo.dwOSVersionInfoSize = sizeof(osInfo);
  fRtlGetVersion(&osInfo);

  DWORD syscall_number = 25;
  if (osInfo.dwBuildNumber <= 7601) { // Windows 7 and below
    syscall_number = 22;
  }
  else if (osInfo.dwBuildNumber == 9200) { // Windows 8
    syscall_number = 23;
  }
  else if (osInfo.dwBuildNumber == 9600) { // Windows 8.1
    syscall_number = 24;
  }

  // Actual test ...

  unsigned char syscall_stub[64] = {0};
  //auto syscall_stub = (unsigned char*) VirtualAlloc(nullptr, 64, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  auto fNtQueryInformationProcess2 = (Type_NtQueryInformationProcess) deinamig_build_direct_syscall_stub(syscall_stub,
                                                                                                         syscall_number,
                                                                                                         nullptr);
  DWORD oldProtect;
  VirtualProtect(syscall_stub, sizeof(syscall_stub), PAGE_EXECUTE_READWRITE, &oldProtect);
  //VirtualProtect(syscall_stub, sizeof(syscall_stub), PAGE_EXECUTE_READ, &oldProtect);

  REQUIRE(fNtQueryInformationProcess2 != nullptr);

  pbi = {0};
  dwSize = sizeof(PROCESS_BASIC_INFORMATION);
  dwSizeNeeded = 0;

  status = fNtQueryInformationProcess2(hProcess, ProcessBasicInformation, &pbi, dwSize, &dwSizeNeeded);
  REQUIRE(NT_SUCCESS(status));
  REQUIRE(pbi.UniqueProcessId);
  REQUIRE(UniqueProcessId == pbi.UniqueProcessId);

  //REQUIRE(VirtualFree(syscall_stub, 0, MEM_RELEASE) != 0);
}

TEST_CASE("build_indirect_syscall_stub")
{
  // Create result to compare against
  auto ntdll = deinamig_get_peb_ldr_library_entry_by_name("ntdll.dll");
  REQUIRE(ntdll != nullptr);

  HANDLE hProcess = GetCurrentProcess();
  PROCESS_BASIC_INFORMATION pbi = {0};
  DWORD dwSize = sizeof(PROCESS_BASIC_INFORMATION);
  DWORD dwSizeNeeded = 0;

  auto fNtQueryInformationProcess1 = (Type_NtQueryInformationProcess) deinamig_get_export_address_by_name(ntdll, "NtQueryInformationProcess", FALSE, FALSE);
  REQUIRE(fNtQueryInformationProcess1 != nullptr);

  NTSTATUS status = fNtQueryInformationProcess1(hProcess, ProcessBasicInformation, &pbi, dwSize, &dwSizeNeeded);
  REQUIRE(NT_SUCCESS(status));
  REQUIRE(pbi.UniqueProcessId);
  auto UniqueProcessId = pbi.UniqueProcessId;

  auto fRtlGetVersion = (Type_RtlGetVersion) deinamig_get_export_address_by_name(ntdll, "RtlGetVersion", FALSE, FALSE);
  REQUIRE(fRtlGetVersion != nullptr);

  OSVERSIONINFOEXW osInfo = {0};
  osInfo.dwOSVersionInfoSize = sizeof(osInfo);
  fRtlGetVersion(&osInfo);

  DWORD syscall_number = 25;
  if (osInfo.dwBuildNumber <= 7601) { // Windows 7 and below
    syscall_number = 22;
  }
  else if (osInfo.dwBuildNumber == 9200) { // Windows 8
    syscall_number = 23;
  }
  else if (osInfo.dwBuildNumber == 9600) { // Windows 8.1
    syscall_number = 24;
  }

  // Actual test ...
  unsigned long key = 0x12345678;
  auto hash = hashit("NtQueryInformationProcess", key);


  auto function_address = deinamig_get_export_address_by_hash(ntdll, hash, key, hashit, FALSE, FALSE);
  REQUIRE(function_address != NULL);
  auto syscall_address = deinamig_get_syscall_address(function_address, TRUE);
  REQUIRE(syscall_address != NULL);

  unsigned char syscall_stub[64] = {0};
  //auto syscall_stub = (unsigned char*) VirtualAlloc(nullptr, 64, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  auto fNtQueryInformationProcess2 = (Type_NtQueryInformationProcess) deinamig_build_indirect_syscall_stub(syscall_stub,
                                                                                                           syscall_number,
                                                                                                           syscall_address,
                                                                                                           nullptr);
  DWORD oldProtect;
  VirtualProtect(syscall_stub, sizeof(syscall_stub), PAGE_EXECUTE_READWRITE, &oldProtect);
  //VirtualProtect(syscall_stub, sizeof(syscall_stub), PAGE_EXECUTE_READ, &oldProtect);

  REQUIRE(fNtQueryInformationProcess2 != nullptr);

  pbi = {0};
  dwSize = sizeof(PROCESS_BASIC_INFORMATION);
  dwSizeNeeded = 0;

  status = fNtQueryInformationProcess2(hProcess, ProcessBasicInformation, &pbi, dwSize, &dwSizeNeeded);
  REQUIRE(NT_SUCCESS(status));
  REQUIRE(pbi.UniqueProcessId);
  REQUIRE(UniqueProcessId == pbi.UniqueProcessId);

  //REQUIRE(VirtualFree(syscall_stub, 0, MEM_RELEASE) != 0);
}

#endif