#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <catch2/catch_test_macros.hpp>

#include "llwythwr.h"
#include "llwythwr/init.h"

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

static int read_file(const char* path, void** contents, size_t *size)
{
  FILE* in_file = fopen(path, "rb");
  if (!in_file) return 0;

#if _WIN32
  struct _stat sb{};
  if (_stat( path, &sb ) == -1) return 0;
#else
  struct stat sb{};
  if (stat(path, &sb) == -1) return 0;
#endif

  *size = sb.st_size;
  *contents = (void*) malloc(sb.st_size);
  fread(*contents, sb.st_size, 1, in_file);

  fclose(in_file);
  return 1;
}

#if _WIN32

#define ROR8(v) (v >> 8 | v << 24)

unsigned long hashit(const char* name, unsigned long key, void* user_data)
{
  unsigned long i = 0;
  unsigned long hash = key;

  while (name[i]) {
    unsigned short PartialName = *(unsigned short*)((ULONG64)name + i++);
    hash ^= PartialName + ROR8(hash);
  }

  return hash;
}

void* x_get_lib_address_function(const char* name, void* user_data) {
  return LoadLibraryA(name);
}

void* x_get_proc_address_function(const char* lib, void* handle, const char* func, int ordinal, void* user_data) {
  if (func != nullptr)
    return (void*) GetProcAddress((HMODULE) handle, func);
  else
    return (void*) GetProcAddress((HMODULE) handle, reinterpret_cast<LPCSTR>(ordinal));
}

void x_free_library_function(const char* name, void* handle, void* userdata) {
  FreeLibrary((HMODULE)handle);
}

TEST_CASE("map_library")
{
  static char *paths[] = {
      (char*) "sample_dll" PATH_SEP "Debug" PATH_SEP "sample" LIB_SUFFIX,
      (char*) "sample_dll" PATH_SEP "sample" LIB_SUFFIX,
      nullptr
  };

  void *module_sample = nullptr;
  size_t size_a = 0;
  int i = 0;
  while (paths[i]) {
    if (read_file(paths[i], &module_sample, &size_a))
      break;
    i++;
  }
  REQUIRE(module_sample);

  llwythwr_os_api os_api{};
  llwythwr_init(&os_api);

  auto library = llwythwr_map_library(&os_api,
                                      module_sample,
                                      x_get_lib_address_function,
                                      x_get_proc_address_function,
                                      x_free_library_function,
                                      nullptr);
  REQUIRE(library);

  int success = llwythwr_call_entry_point(library);
  REQUIRE(success);

  typedef int(*addNumbers)(int, int);
  auto func_addNumbers = (addNumbers) llwythwr_get_export_address_by_name(library, "addNumbers");
  REQUIRE(func_addNumbers != nullptr);

  auto ret = func_addNumbers(1, 2);
  REQUIRE(ret == 3);

  unsigned long key = 0x12345678;
  unsigned long hash = hashit("addNumbers", key, nullptr);
  func_addNumbers = (addNumbers) llwythwr_get_export_address_by_hash(library, hash, key, hashit);
  REQUIRE(func_addNumbers != nullptr);

  ret = func_addNumbers(2, 3);
  REQUIRE(ret == 5);

  llwythwr_unload_library(library);
}

TEST_CASE("alloc_library")
{
  static char *paths[] = {
      (char*) "sample_dll" PATH_SEP "Debug" PATH_SEP "sample" LIB_SUFFIX,
      (char*) "sample_dll" PATH_SEP "sample" LIB_SUFFIX,
      nullptr
  };

  void *module_sample = nullptr;
  size_t size_a = 0;
  int i = 0;
  while (paths[i]) {
    if (read_file(paths[i], &module_sample, &size_a))
      break;
    i++;
  }
  REQUIRE(module_sample);

  llwythwr_os_api os_api{};
  llwythwr_init(&os_api);

  auto library = llwythwr_alloc_library(&os_api,
                                        module_sample,
                                        x_get_lib_address_function,
                                        x_get_proc_address_function,
                                        x_free_library_function,
                                        nullptr);
  REQUIRE(library);

  int success = llwythwr_call_entry_point(library);
  REQUIRE(success);

  typedef int(*addNumbers)(int, int);
  auto func_addNumbers = (addNumbers) llwythwr_get_export_address_by_name(library, "addNumbers");
  REQUIRE(func_addNumbers != nullptr);

  auto ret = func_addNumbers(1, 2);
  REQUIRE(ret == 3);

  unsigned long key = 0x12345678;
  unsigned long hash = hashit("addNumbers", key, nullptr);
  func_addNumbers = (addNumbers) llwythwr_get_export_address_by_hash(library, hash, key, hashit);
  REQUIRE(func_addNumbers != nullptr);

  ret = func_addNumbers(2, 3);
  REQUIRE(ret == 5);

  llwythwr_wipe_headers(library);

  //printf("<tests:pause>\n");
  //getchar();

  llwythwr_unload_library(library);
}

TEST_CASE("map_library_from_disk_to_section")
{
  llwythwr_os_api os_api{};
  llwythwr_init(&os_api);
  auto library = llwythwr_map_library_from_disk_to_section(&os_api, R"(C:\Windows\System32\ntdll.dll)");
  REQUIRE(library);
  llwythwr_unload_library(library);
}

TEST_CASE("map_hollow_library")
{
  static char *paths[] = {
      (char*) "sample_dll" PATH_SEP "Debug" PATH_SEP "sample" LIB_SUFFIX,
      (char*) "sample_dll" PATH_SEP "sample" LIB_SUFFIX,
      nullptr
  };

  void *module_sample = nullptr;
  size_t size_a = 0;
  int i = 0;
  while (paths[i]) {
    if (read_file(paths[i], &module_sample, &size_a))
      break;
    i++;
  }
  REQUIRE(module_sample);

  llwythwr_os_api os_api{};
  llwythwr_init(&os_api);

  auto library =
      llwythwr_map_hollow_library(&os_api, module_sample, R"(C:\Windows\system32\crypt32.dll)", FALSE,
                                  x_get_lib_address_function,
                                  x_get_proc_address_function,
                                  x_free_library_function,
                                  nullptr);
  REQUIRE(library);

  int success = llwythwr_call_entry_point(library);
  REQUIRE(success);

  typedef int(*addNumbers)(int, int);
  auto func_addNumbers = (addNumbers) llwythwr_get_export_address_by_name(library, "addNumbers");
  REQUIRE(func_addNumbers != nullptr);

  auto ret = func_addNumbers(1, 2);
  REQUIRE(ret == 3);

  unsigned long key = 0x12345678;
  unsigned long hash = hashit("addNumbers", key, nullptr);
  func_addNumbers = (addNumbers) llwythwr_get_export_address_by_hash(library, hash, key, hashit);
  REQUIRE(func_addNumbers != nullptr);

  ret = func_addNumbers(2, 3);
  REQUIRE(ret == 5);

  llwythwr_unload_library(library);
}

TEST_CASE("load_hollow_library")
{
  static char *paths[] = {
      (char*) "sample_dll" PATH_SEP "Debug" PATH_SEP "sample" LIB_SUFFIX,
      (char*) "sample_dll" PATH_SEP "sample" LIB_SUFFIX,
      nullptr
  };

  void *module_sample = nullptr;
  size_t size_a = 0;
  int i = 0;
  while (paths[i]) {
    if (read_file(paths[i], &module_sample, &size_a))
      break;
    i++;
  }
  REQUIRE(module_sample);

  llwythwr_os_api os_api{};
  llwythwr_init(&os_api);

  HMODULE decoy = LoadLibraryExA(R"(C:\Windows\system32\crypt32.dll)", NULL, DONT_RESOLVE_DLL_REFERENCES);
  REQUIRE(decoy);

  auto library =
      llwythwr_load_hollow_library(&os_api, module_sample, decoy, FALSE,
                                  x_get_lib_address_function,
                                  x_get_proc_address_function,
                                  x_free_library_function,
                                  nullptr);
  REQUIRE(library);

  int success = llwythwr_call_entry_point(library);
  REQUIRE(success);

  typedef int(*addNumbers)(int, int);
  auto func_addNumbers = (addNumbers) llwythwr_get_export_address_by_name(library, "addNumbers");
  REQUIRE(func_addNumbers != nullptr);

  auto ret = func_addNumbers(1, 2);
  REQUIRE(ret == 3);

  unsigned long key = 0x12345678;
  unsigned long hash = hashit("addNumbers", key, nullptr);
  func_addNumbers = (addNumbers) llwythwr_get_export_address_by_hash(library, hash, key, hashit);
  REQUIRE(func_addNumbers != nullptr);

  ret = func_addNumbers(2, 3);
  REQUIRE(ret == 5);

  llwythwr_unload_library(library);

  FreeLibrary(decoy);
}

typedef struct _LDR_DATA_TABLE_ENTRY1 {
  LIST_ENTRY InLoadOrderLinks;
  LIST_ENTRY InMemoryOrderLinks;
  LIST_ENTRY InInitializationOrderLinks;
  void *DllBase;
  void *EntryPoint;
  ULONG SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
  ULONG Flags;
  SHORT LoadCount;
  SHORT TlsIndex;
  HANDLE SectionHandle;
  ULONG CheckSum;
  ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY1, *PLDR_DATA_TABLE_ENTRY1;

// PEB defined by rewolf
// http://blog.rewolf.pl/blog/?p=573
typedef struct _PEB_LDR_DATA1 {
  ULONG Length;
  BOOL Initialized;
  LPVOID SsHandle;
  LIST_ENTRY InLoadOrderModuleList;
  LIST_ENTRY InMemoryOrderModuleList;
  LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA1, *PPEB_LDR_DATA1;

#define RVA2VA(type, base, rva) (type)((ULONG_PTR) base + rva)

void* get_module_by_walking_peb(const WCHAR* module_name) {
  auto ldr = (PPEB_LDR_DATA1) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;

  LIST_ENTRY *head = &ldr->InMemoryOrderModuleList;
  for(LIST_ENTRY *current = head->Flink; current != head; current = current->Flink){
    LDR_DATA_TABLE_ENTRY1* entry = CONTAINING_RECORD(current, LDR_DATA_TABLE_ENTRY1, InMemoryOrderLinks);
    if (!entry || !entry->DllBase) break;

    WCHAR* curr_name = entry->BaseDllName.Buffer;
    if (!curr_name) continue;

    // both of the strings finished, and so far they were identical:
    if (wcscmp(module_name, curr_name) == 0) {
      return entry->DllBase;
    }
  }

  return nullptr;
}

TEST_CASE("link_ldr_tables")
{
  static char *paths[] = {
      (char*) "sample_dll" PATH_SEP "Debug" PATH_SEP "sample" LIB_SUFFIX,
      (char*) "sample_dll" PATH_SEP "sample" LIB_SUFFIX,
      nullptr
  };

  void *module_sample = nullptr;
  size_t size_a = 0;
  int i = 0;
  while (paths[i]) {
    if (read_file(paths[i], &module_sample, &size_a))
      break;
    i++;
  }
  REQUIRE(module_sample);

  llwythwr_os_api os_api{};
  llwythwr_init(&os_api);

  auto library =
      llwythwr_map_hollow_library(&os_api, module_sample, R"(C:\Windows\system32\crypt32.dll)", TRUE,
                                  x_get_lib_address_function,
                                  x_get_proc_address_function,
                                  x_free_library_function,
                                  nullptr);
  REQUIRE(library);

  typedef int(*addNumbers)(int, int);
  auto func_addNumbers = (addNumbers) llwythwr_get_export_address_by_name(library, "addNumbers");
  REQUIRE(func_addNumbers != nullptr);
  auto ret = func_addNumbers(1, 2);
  REQUIRE(ret == 3);

  REQUIRE(llwythwr_link_library(library, "crypt32.dll", "C:\\Windows\\System32\\crypt32.dll"));

  ret = func_addNumbers(1, 2);
  REQUIRE(ret == 3);

  auto handle1 = GetModuleHandleA("crypt32.dll");
  REQUIRE(handle1);

  auto handle2 = LoadLibraryA("crypt32.dll");
  REQUIRE(handle2);
  REQUIRE(handle1 == handle2);

  auto handle3 = (HMODULE) get_module_by_walking_peb(L"crypt32.dll");
  REQUIRE(handle3);
  REQUIRE(handle2 == handle3);

  //printf("<tests:pause>\n");
  //getchar();

  REQUIRE(llwythwr_unlink_library(library));

  handle3 = (HMODULE) get_module_by_walking_peb(L"crypt32.dll");
  REQUIRE(handle3 == nullptr);

  handle1 = GetModuleHandleA("crypt32.dll");
  REQUIRE(handle1 == nullptr);
}

/*
TEST_CASE("testing")
{
  static char *paths[] = {
      (char*) "sample_dll" PATH_SEP "Debug" PATH_SEP "sample" LIB_SUFFIX,
      (char*) "sample_dll" PATH_SEP "sample" LIB_SUFFIX,
      nullptr
  };

  void *module_sample = nullptr;
  size_t size_a = 0;
  int i = 0;
  while (paths[i]) {
    if (read_file(paths[i], &module_sample, &size_a))
      break;
    i++;
  }
  REQUIRE(module_sample);

  llwythwr_os_api os_api{};
  llwythwr_init(&os_api);

  llwythwr_windows_library *decoy = llwythwr_map_library_from_disk_to_section(&os_api, R"(C:\Windows\System32\crypt32.dll)");
  REQUIRE(decoy);

  auto dos = (PIMAGE_DOS_HEADER) decoy->base_address;
  auto nt = RVA2VA(PIMAGE_NT_HEADERS, decoy->base_address, dos->e_lfanew);
  PIMAGE_SECTION_HEADER sh = IMAGE_FIRST_SECTION(nt);
  ULONG_PTR data_section_address = 0;
  DWORD data_section_size = 0;

  for (i = 0; i < nt->FileHeader.NumberOfSections; i++) {
    if (!strcmp((char*)sh[i].Name, ".text")) {
      data_section_address = (ULONG_PTR) decoy->base_address + sh[i].VirtualAddress;
      data_section_size = sh[i].Misc.VirtualSize;
      break;
    }
  }

  DWORD old_protect;
  REQUIRE(os_api.VirtualProtect((PVOID)data_section_address, data_section_size, PAGE_READWRITE, &old_protect));
  memset((PVOID)data_section_address, 0, data_section_size);
  REQUIRE(os_api.VirtualProtect((PVOID)data_section_address, data_section_size, old_protect, &old_protect));

  printf("<tests:pause>\n");
  getchar();
}
*/

#else

#ifdef __linux__

TEST_CASE("llwythwr_load_module_from_memory")
{
  typedef int(*addNumbers)(int, int);

  static char *paths[] = {
      (char *) "sample_dll" PATH_SEP "Debug" PATH_SEP "sample" LIB_SUFFIX,
      (char*) "sample_dll" PATH_SEP "sample" LIB_SUFFIX,
      nullptr
  };

  void *module_sample = nullptr;
  size_t size_a = 0;
  int i = 0;
  while (paths[i]) {
    if (read_file(paths[i], &module_sample, &size_a))
      break;
    i++;
  }
  REQUIRE(module_sample);

  llwythwr_os_api os_api{};
  llwythwr_init(&os_api);

  llwythwr_linux_library* library = llwythwr_load_library_from_memory(&os_api, "sample", module_sample, size_a, 0);
  REQUIRE(library != nullptr);

  auto func_addNumbers = (addNumbers) llwythwr_get_symbol_address_memory(library, "addNumbers");
  REQUIRE(func_addNumbers != nullptr);
  int ret = func_addNumbers(1, 2);
  REQUIRE(ret == 3);
  llwythwr_unload_library_from_memory(library);
}

#else

TEST_CASE("llwythwr_load_module_from_memory")
{
  typedef int(*addNumbers)(int, int);

  static char *paths[] = {
      (char *) "sample_dll" PATH_SEP "Debug" PATH_SEP "sample" LIB_SUFFIX,
      (char*) "sample_dll" PATH_SEP "sample" LIB_SUFFIX,
      nullptr
  };

  void *module_sample = nullptr;
  size_t size_a = 0;
  int i = 0;
  while (paths[i]) {
    if (read_file(paths[i], &module_sample, &size_a))
      break;
    i++;
  }
  REQUIRE(module_sample);

  llwythwr_os_api os_api{};
  llwythwr_init(&os_api);

  llwythwr_mac_library* library = llwythwr_load_library_from_memory(&os_api, "sample", module_sample, size_a, 0);
  REQUIRE(library != nullptr);

  auto func_addNumbers = (addNumbers) llwythwr_get_symbol_address_memory(library, "addNumbers");
  REQUIRE(func_addNumbers != nullptr);
  int ret = func_addNumbers(1, 2);
  REQUIRE(ret == 3);
  llwythwr_unload_library_from_memory(library);
}

#endif

#endif
