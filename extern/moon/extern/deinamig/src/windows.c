#include "deinamig/windows.h"

#include <windows.h>
#include <winternl.h>

#include "debug.h"

/*
 * Typedefs
 */

typedef HMODULE (WINAPI *Type_LoadLibraryA)(LPCSTR);              // LdrLoadDll
typedef BOOL (WINAPI *Type_FreeLibrary)(HMODULE);                 // LdrUnloadDll
typedef FARPROC (WINAPI *Type_GetProcAddress)(HMODULE, LPCSTR);   // LdrGetProcedureAddress
typedef BOOL (WINAPI *Type_IsWow64Process) (HANDLE, PBOOL);

/*
 * Defines
 */

// Relative Virtual Address to Virtual Address
#define RVA2VA(type, base, rva) (type)((ULONG_PTR) base + rva)

#ifndef deinamig_tolower
#define deinamig_tolower(c) ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))
#endif

#define DEINAMIG_MAX_ENTRIES 512
#define DEINAMIG_HALO_UP -32
#define DEINAMIG_HALO_DOWN 32

/*
 * Structures
 */

// enhanced version of LDR_DATA_TABLE_ENTRY
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

typedef enum _SECTION_INHERIT {
  ViewShare = 1,
  ViewUnmap = 2
} SECTION_INHERIT, *PSECTION_INHERIT;

typedef struct _IMAGE_RELOC {
  WORD offset: 12;
  WORD type: 4;
} IMAGE_RELOC, *PIMAGE_RELOC;

typedef struct _DEINAMIG_SYSCALL_ENTRY {
  DWORD hash;
  DWORD address;
} DEINAMIG_SYSCALL_ENTRY, *PDEINAMIG_SYSCALL_ENTRY;

typedef struct _DEINAMIG_SYSCALL_LIST {
  DWORD count;
  DEINAMIG_SYSCALL_ENTRY entries[DEINAMIG_MAX_ENTRIES];
} DEINAMIG_SYSCALL_LIST, *PDEINAMIG_SYSCALL_LIST;

/*
 * Utility functions
 */

static int deinamig_strcmp(const char *s1, const char *s2) {
  while (*s1 == *s2++) {
    if (*s1++ == '\0') return (0);
  }
  return (*(const unsigned char *) s1 - *(const unsigned char *) (s2 - 1));
}

static int deinamig_strcasecmp(const char *s1, const char *s2) {
  const u_char *us1 = (const u_char *) s1, *us2 = (const u_char *) s2;
  while (deinamig_tolower(*us1) == deinamig_tolower(*us2)) {
    if (*us1++ == '\0') return (0);
    us2++;
  }
  return (deinamig_tolower(*us1) - deinamig_tolower(*us2));
}

size_t deinamig_strlcpy(char *dst, const char *src, size_t siz) {
  char *d = dst;
  const char *s = src;
  size_t n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0) {
    while (--n != 0) {
      if ((*d++ = *s++) == '\0')
        break;
    }
  }
  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (siz != 0)
      *d = '\0';        /* NUL-terminate dst */
    while (*s++);
  }

  return (s - src - 1);    /* count does not include NUL */
}

char *deinamig_strlwr(char *string) {
  char *bufp = string;
  while (*bufp) {
    *bufp = deinamig_tolower((unsigned char) *bufp);
    ++bufp;
  }
  return string;
}

static void* deinamig_memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size) {
  unsigned char *dst = (unsigned char *) dstptr;
  const unsigned char *src = (const unsigned char *) srcptr;
  for (size_t i = 0; i < size; i++)
    dst[i] = src[i];
  return dstptr;
}

static int deinamig_memcmp(const void *s1, const void *s2, size_t n) {
  if (n != 0) {
    const unsigned char *p1 = s1, *p2 = s2;

    do {
      if (*p1++ != *p2++)
        return (*--p1 - *--p2);
    } while (--n != 0);
  }
  return (0);
}

/*
static size_t deinamig_strlen(const char *str) {
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

static size_t deinamig_wcslen(const wchar_t *s) {
  const wchar_t *p;
  p = s;
  while (*p)
    p++;
  return p - s;
}

static void *deinamig_memset(void *ptr, int value, size_t num) {
#ifdef _MSC_VER
  __stosb(ptr, value, num);
#else
  unsigned char *p = (unsigned char *) ptr;
  while (num--) {
    *p = (unsigned char) value;
    p++;
  }
#endif
  return ptr;
}

static char *deinamig_strcat(char *s, char *append) {
  char *save = s;
  for (; *s; ++s);
  while ((*s++ = *append++) != 0);
  return (save);
}
*/

static int deinamig_is_wow32() {
  BOOL is_wow32 = FALSE;

  char kernel32_dll[] = {'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', '\0'};
  char function[] = {'I', 's', 'W', 'o', 'w', '6', '4', 'P', 'r', 'o', 'c', 'e', 's', 's', '\0'};
  Type_IsWow64Process fIsWow64Process = (Type_IsWow64Process) deinamig_get_library_address_by_name(kernel32_dll,
                                                                                                   function,
                                                                                                   FALSE,
                                                                                                   FALSE);
  if (fIsWow64Process == NULL) {
    return FALSE;
  }

  if (!fIsWow64Process(GetCurrentProcess(),&is_wow32)) {
    DPRINT("IsWow64Process failed, \n");
    return FALSE;
  }

  return is_wow32;
}

/*
 * Main functions
 */

void *deinamig_load_library_from_disk(const char *filename) {
  char kernel32_dll[] = {'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', '\0'};
  char function[] = {'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'A', '\0'};
  Type_LoadLibraryA fLoadLibraryA = (Type_LoadLibraryA) deinamig_get_library_address_by_name(kernel32_dll,
                                                                                             function,
                                                                                             FALSE,
                                                                                             FALSE);
  return fLoadLibraryA(filename);
}

void deinamig_unload_library_from_disk(void *handle) {
  char kernel32_dll[] = {'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', '\0'};
  char function[] = {'F', 'r', 'e', 'e', 'L', 'i', 'b', 'r', 'a', 'r', 'y', '\0'};
  Type_FreeLibrary fFreeLibrary = (Type_FreeLibrary) deinamig_get_library_address_by_name(kernel32_dll,
                                                                                          function,
                                                                                          FALSE,
                                                                                          FALSE);
  fFreeLibrary(handle);
}

void *deinamig_get_native_export_address(void *module_base, const char *export_name) {
  char kernel32_dll[] = {'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', '\0'};
  char function[] = {'G', 'e', 't', 'P', 'r', 'o', 'c', 'A', 'd', 'd', 'r', 'e', 's', 's', '\0'};
  Type_GetProcAddress fGetProcAddress = (Type_GetProcAddress) deinamig_get_library_address_by_name(kernel32_dll,
                                                                                                   function,
                                                                                                   FALSE,
                                                                                                   FALSE);
  return fGetProcAddress(module_base, export_name);
}

void *deinamig_get_library_address_by_name(const char *filename,
                                           const char *function_name,
                                           int load_from_disk,
                                           int resolve_forwards) {
  void *handle = deinamig_get_peb_ldr_library_entry_by_name(filename);

  if (handle == NULL && load_from_disk) {
    DPRINT("Using LoadLibrary in an attempt to load library %s.\n", filename);
    handle = deinamig_load_library_from_disk(filename);
    if (handle == NULL) {
      return NULL;
    }
    DPRINT("LoadLibrary loaded library %s at 0x%p.\n", filename, handle);
  } else if (handle == NULL) {
    return NULL;
  }

  return deinamig_get_export_address_by_name(handle, function_name, resolve_forwards, load_from_disk);
}

void *deinamig_get_library_address_by_hash(const char *filename,
                                           unsigned long function_hash,
                                           unsigned long key,
                                           deinamig_hash_function hash_function,
                                           int load_from_disk,
                                           int resolve_forwards) {
  void *handle = deinamig_get_peb_ldr_library_entry_by_name(filename);

  if (handle == NULL && load_from_disk) {
    handle = deinamig_load_library_from_disk(filename);
    if (handle == NULL) {
      return NULL;
    }
  } else if (handle == NULL) {
    return NULL;
  }

  return deinamig_get_export_address_by_hash(handle,
                                             function_hash,
                                             key,
                                             hash_function,
                                             resolve_forwards,
                                             load_from_disk);
}

void *deinamig_get_peb_ldr_library_entry_by_name(const char *filename) {
  DPRINT("Searching for library %s by walking entries in PPEB_LDR_DATA.\n", filename);

  DWORD i;
  CHAR dll_name[64] = {0};
  for (i = 0; filename[i] != 0 && i < 64; i++) {
    dll_name[i] = filename[i];
  }
  dll_name[i] = 0;
  // make sure the name ends with '.dll'
  if (dll_name[i - 4] != '.') {
    dll_name[i++] = '.';
    dll_name[i++] = 'd';
    dll_name[i++] = 'l';
    dll_name[i++] = 'l';
    dll_name[i++] = 0;
  }

  //PEB* peb = (PPEB)NtCurrentTeb()->ProcessEnvironmentBlock;

#if defined(_WIN64)
  PEB *peb = (PPEB) __readgsqword(0x60);
#else
  PEB* peb = (PPEB)__readfsdword(0x30);
#endif
  PEB_LDR_DATA1 *ldr = (PPEB_LDR_DATA1) peb->Ldr;

  // for each DLL loaded
  PLDR_DATA_TABLE_ENTRY1 dte;

  // for each DLL loaded
  for (dte = (PLDR_DATA_TABLE_ENTRY1) ldr->InLoadOrderModuleList.Flink;
       dte->DllBase != NULL;
       dte = (PLDR_DATA_TABLE_ENTRY1) dte->InLoadOrderLinks.Flink) {
    LPVOID base = dte->DllBase;
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) base;
    PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, base, dos->e_lfanew);
    DWORD rva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (rva == 0) continue;

    PIMAGE_EXPORT_DIRECTORY exp = RVA2VA(PIMAGE_EXPORT_DIRECTORY, base, rva);
    PCHAR name = RVA2VA(PCHAR, base, exp->Name);

    if (deinamig_strcasecmp(dll_name, name) == 0) {
      DPRINT("Found library %s at address 0x%p.\n", filename, base);
      return base;
    }
  }

  DPRINT("Could not find library %s.\n", filename);
  return NULL;
}

void *deinamig_get_peb_ldr_library_entry_by_hash(unsigned long library_hash,
                                                 unsigned long key,
                                                 deinamig_hash_function hash_function) {
  DPRINT("Searching for library with hash %lu by walking entries in PPEB_LDR_DATA.\n", library_hash);

  //PEB* peb = (PPEB)NtCurrentTeb()->ProcessEnvironmentBlock;

#if defined(_WIN64)
  PEB *peb = (PPEB) __readgsqword(0x60);
#else
  PEB* peb = (PPEB)__readfsdword(0x30);
#endif
  PEB_LDR_DATA1 *ldr = (PPEB_LDR_DATA1) peb->Ldr;

  // for each DLL loaded
  PLDR_DATA_TABLE_ENTRY1 dte;

  // for each DLL loaded
  for (dte = (PLDR_DATA_TABLE_ENTRY1) ldr->InLoadOrderModuleList.Flink;
       dte->DllBase != NULL;
       dte = (PLDR_DATA_TABLE_ENTRY1) dte->InLoadOrderLinks.Flink) {
    LPVOID base = dte->DllBase;
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) base;
    PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, base, dos->e_lfanew);
    DWORD rva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (rva == 0) continue;

    PIMAGE_EXPORT_DIRECTORY exp = RVA2VA(PIMAGE_EXPORT_DIRECTORY, base, rva);
    PCHAR name = RVA2VA(PCHAR, base, exp->Name);

    CHAR buf[64] = {0};
    deinamig_strlcpy(buf, name, 64);
    deinamig_strlwr(buf);

    if (hash_function(buf, key) == library_hash) {
      DPRINT("Found library with hash %lu at address 0x%p.\n", library_hash, base);
      return base;
    }
  }

  DPRINT("Could not find library with hash %lu.\n", library_hash);
  return NULL;
}

void *deinamig_get_export_address_by_name(void *module_base,
                                          const char *export_name,
                                          int resolve_forwards,
                                          int load_from_disk) {
  DPRINT("Searching for function %s in library with base address of 0x%p.\n", export_name, module_base);

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);
  PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY) nt->OptionalHeader.DataDirectory;
  DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

  // if no export table, return NULL
  if (rva == 0) return NULL;

  PIMAGE_EXPORT_DIRECTORY exp = RVA2VA(PIMAGE_EXPORT_DIRECTORY, module_base, rva);
  DWORD cnt = exp->NumberOfNames;

  // if no api names, return NULL
  if (cnt == 0) return NULL;

  PDWORD adr = RVA2VA(PDWORD, module_base, exp->AddressOfFunctions);
  PDWORD sym = RVA2VA(PDWORD, module_base, exp->AddressOfNames);
  PWORD ord = RVA2VA(PWORD, module_base, exp->AddressOfNameOrdinals);
  PCHAR dll = RVA2VA(PCHAR, module_base, exp->Name);

  LPVOID addr = NULL;

  if (HIWORD(export_name) == 0) {
    // load function by ordinal value
    if (LOWORD(export_name) >= exp->Base) {
      DWORD index = LOWORD(export_name) - exp->Base;
      //addr = (FARPROC)(LPVOID)(handle + (*(DWORD *) (handle + exp->AddressOfFunctions + (index * 4))));
      //addr = RVA2VA(LPVOID, handle, (PDWORD) adr + (index * 4));
      addr = RVA2VA(PVOID, module_base, adr[index]);
    }
  } else {
    do {
      // search function name in list of exported names
      PCHAR api = RVA2VA(PCHAR, module_base, sym[cnt - 1]);

      if (deinamig_strcmp(export_name, api) == 0) {
        addr = RVA2VA(LPVOID, module_base, adr[ord[cnt - 1]]);
        break;
      }
    } while (--cnt);
  }

  // is this a forward reference?
  if (addr != NULL && resolve_forwards
      && ((PBYTE) addr >= (PBYTE) exp && (PBYTE) addr < (PBYTE) exp + dir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)) {
    DPRINT("Function %s is forwarded, resolving.\n", export_name);
    return deinamig_get_forward_address(addr, load_from_disk);
  }

  if (addr != NULL)
    DPRINT("Found function %s at 0x%p.\n", export_name, addr);
  else
    DPRINT("Could not find function %s.\n", export_name);

  return addr;
}

void *deinamig_get_export_address_by_hash(void *module_base,
                                          unsigned long function_hash,
                                          unsigned long key,
                                          deinamig_hash_function hash_function,
                                          int resolve_forwards,
                                          int load_from_disk) {
  DPRINT("Searching for function with hash 0x%lu in library with base address of 0x%p.\n", function_hash, module_base);

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);
  PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY) nt->OptionalHeader.DataDirectory;
  DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

  // if no export table, return NULL
  if (rva == 0) return NULL;

  PIMAGE_EXPORT_DIRECTORY exp = RVA2VA(PIMAGE_EXPORT_DIRECTORY, module_base, rva);
  DWORD cnt = exp->NumberOfNames;

  // if no api names, return NULL
  if (cnt == 0) return NULL;

  PDWORD adr = RVA2VA(PDWORD, module_base, exp->AddressOfFunctions);
  PDWORD sym = RVA2VA(PDWORD, module_base, exp->AddressOfNames);
  PWORD ord = RVA2VA(PWORD, module_base, exp->AddressOfNameOrdinals);
  PCHAR dll = RVA2VA(PCHAR, module_base, exp->Name);

  LPVOID addr = NULL;

  do {
    PCHAR api = RVA2VA(PCHAR, module_base, sym[cnt - 1]);
    if (hash_function(api, key) == function_hash) {
      addr = RVA2VA(LPVOID, module_base, adr[ord[cnt - 1]]);
      break;
    }
  } while (--cnt);

  // is this a forward reference?
  if (addr != NULL && resolve_forwards
      && ((PBYTE) addr >= (PBYTE) exp && (PBYTE) addr < (PBYTE) exp + dir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)) {
    DPRINT("Function with hash 0x%lu is forwarded, resolving.\n", function_hash);
    return deinamig_get_forward_address(addr, load_from_disk);
  }

  if (addr != NULL)
    DPRINT("Found function with hash 0x%lu at 0x%p.\n", function_hash, addr);
  else
    DPRINT("Could not find function with hash 0x%lu.\n", function_hash);

  return addr;
}

void *deinamig_get_forward_address(void *export_address, int load_from_disk) {
  // copy DLL name to buffer
  PCHAR p = (char *) export_address;

  DWORD i;
  CHAR dll_name[64] = {0};
  for (i = 0; p[i] != 0 && i < sizeof(dll_name) - 4; i++) {
    dll_name[i] = p[i];
    if (p[i] == '.') break;
  }

  dll_name[i + 1] = 'd';
  dll_name[i + 2] = 'l';
  dll_name[i + 3] = 'l';
  dll_name[i + 4] = 0;

  p += i + 1;

  // copy API name to buffer
  CHAR api_name[128];
  for (i = 0; p[i] != 0 && i < sizeof(api_name) - 1; i++) {
    api_name[i] = p[i];
  }
  api_name[i] = 0;

  void *handle = deinamig_get_peb_ldr_library_entry_by_name(dll_name);

  if (handle == NULL && load_from_disk) {
    handle = deinamig_load_library_from_disk(dll_name);
  }

  if (handle != NULL) {
    return deinamig_get_export_address_by_name(handle, api_name, TRUE, load_from_disk);
  }

  return NULL;
}

/*
 * https://github.com/jthuraisamy/SysWhispers2
 * https://github.com/crummie5/FreshyCalls
 */

unsigned long deinamig_get_syscall_number_syswhispers2_by_hash(void* ntdll,
                                                               unsigned long function_hash,
                                                               unsigned long key,
                                                               deinamig_hash_function hash_function) {

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) ntdll;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, ntdll, dos->e_lfanew);
  PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY) nt->OptionalHeader.DataDirectory;
  DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  PIMAGE_EXPORT_DIRECTORY exp = RVA2VA(PIMAGE_EXPORT_DIRECTORY, ntdll, rva);

  DWORD cnt = exp->NumberOfNames;
  PDWORD adr = RVA2VA(PDWORD, ntdll, exp->AddressOfFunctions);
  PDWORD sym = RVA2VA(PDWORD, ntdll, exp->AddressOfNames);
  PWORD ord = RVA2VA(PWORD, ntdll, exp->AddressOfNameOrdinals);

  DEINAMIG_SYSCALL_LIST syscall_list = {0, 1};
  DWORD i = 0;
  PDEINAMIG_SYSCALL_ENTRY entries = syscall_list.entries;

  // Populate syscall_list with unsorted Zw* entries.
  do {
    PCHAR function_name = RVA2VA(PCHAR, ntdll, sym[cnt - 1]);

    // Is this a system call?
    if (*(USHORT *) function_name == 0x775a) { // 'wZ'
      // duplicate string and replace wZ with tN
      CHAR buf[256] = {0};
      deinamig_strlcpy(buf, function_name, 256);
      buf[0] = 'N';
      buf[1] = 't';

      entries[i].hash = hash_function(buf, key);
      entries[i].address = adr[ord[cnt - 1]];
      i++;
      if (i == DEINAMIG_MAX_ENTRIES) break;
    }
  } while (--cnt);

  // Save total number of system calls found.
  syscall_list.count = i;

  // Sort the list by address in ascending order.
  for (i = 0; i < syscall_list.count - 1; i++) {
    for (DWORD j = 0; j < syscall_list.count - i - 1; j++) {
      if (entries[j].address > entries[j + 1].address) {
        // Swap entries.
        DEINAMIG_SYSCALL_ENTRY temp_entry;

        temp_entry.hash = entries[j].hash;
        temp_entry.address = entries[j].address;

        entries[j].hash = entries[j + 1].hash;
        entries[j].address = entries[j + 1].address;

        entries[j + 1].hash = temp_entry.hash;
        entries[j + 1].address = temp_entry.address;
      }
    }
  }

  // Get syscall number
  for (i = 0; i < syscall_list.count; i++) {
    if (function_hash == entries[i].hash) {
      DPRINT("Found syscall number %lu.\n", i);
      return i;
    }
  }

  DPRINT("Syscall number not found.\n");
  return ULONG_MAX;
}

/*
 * https://gist.github.com/wbenny/b08ef73b35782a1f57069dff2327ee4d
 * https://chromium.googlesource.com/external/github.com/DynamoRIO/dynamorio/+/release_6_0_0/core/win32/callback.c#1912
 */

static unsigned long extract_syscall_hellsgate(void* address) {
#ifdef _WIN64
  // Opcodes should be :
  //    MOV R10, RCX
  //    MOV EAX, <syscall>
  //    syscall
  if (*((PBYTE) address) == 0x4c
      && *((PBYTE) address + 1) == 0x8b
      && *((PBYTE) address + 2) == 0xd1
      && *((PBYTE) address + 3) == 0xb8
      && *((PBYTE) address + 6) == 0x00
      && *((PBYTE) address + 7) == 0x00) {
    BYTE high = *((PBYTE) address + 5);
    BYTE low = *((PBYTE) address + 4);
    return (high << 8) | low;
  }
#else
  if (!deinamig_is_wow32()) {
      // Windows 8, Windows 8.1, Windows 10, opcodes should be :
      //    MOV  EAX, <syscall>
      //    CALL ??
      //    RETN
      //    MOV  EDX, ESP
      //    sysenter
      if (*((PBYTE) address) == 0xb8
          && *((PBYTE) address + 3) == 0x00
          && *((PBYTE) address + 4) == 0x00
          && *((PBYTE) address + 5) == 0xe8
          && *((PBYTE) address + 7) == 0x00
          && *((PBYTE) address + 8) == 0x00
          && *((PBYTE) address + 9) == 0x00
          && *((PBYTE) address + 10) == 0xc3
          && *((PBYTE) address + 11) == 0x8b
          && *((PBYTE) address + 12) == 0xd4) {
          BYTE high = *((PBYTE) address + 2);
          BYTE low = *((PBYTE) address + 1);
          return (high << 8) | low;
      }

      // Windows 8, Windows 8.1, Windows 10, opcodes should be :
      //    MOV  EAX, <syscall>
      //    CALL ??
      //    RETN [??]
      //    MOV  EDX, ESP
      //    sysenter
      if (*((PBYTE) address) == 0xb8
          && *((PBYTE) address + 3) == 0x00
          && *((PBYTE) address + 4) == 0x00
          && *((PBYTE) address + 5) == 0xe8
          && *((PBYTE) address + 7) == 0x00
          && *((PBYTE) address + 8) == 0x00
          && *((PBYTE) address + 9) == 0x00
          && *((PBYTE) address + 10) == 0xc2
          && *((PBYTE) address + 13) == 0x8b
          && *((PBYTE) address + 14) == 0xd4) {
          BYTE high = *((PBYTE) address + 2);
          BYTE low = *((PBYTE) address + 1);
          return (high << 8) | low;
      }

      // Windows XP (SP3), Windows 7, Windows 7 (SP1), opcodes should be :
      //    MOV  EAX, <syscall>
      //    MOV  EDX, 7FFE0300h
      //    CALL dword ptr [EDX]
      if (*((PBYTE) address) == 0xb8
          && *((PBYTE) address + 3) == 0x00
          && *((PBYTE) address + 4) == 0x00
          && *((PBYTE) address + 5) == 0xba
          && *((PBYTE) address + 7) == 0x00
          && *((PBYTE) address + 8) == 0x03
          && *((PBYTE) address + 9) == 0xfe
          && *((PBYTE) address + 9) == 0x7f
          && *((PBYTE) address + 10) == 0xff
          && *((PBYTE) address + 11) == 0x12) {
          BYTE high = *((PBYTE) function_address + 2);
          BYTE low = *((PBYTE) function_address + 1);
          return (high << 8) | low;
      }
  }
  else { // deinamig_is_wow32()
      // Windows 10, opcodes should be :
      //    MOV  EAX, <syscall>
      //    MOV  EDX, ??
      //    CALL EDX
      if (*((PBYTE) address) == 0xb8
          && *((PBYTE) address + 3) == 0x00
          && *((PBYTE) address + 4) == 0x00
          && *((PBYTE) address + 5) == 0xba
          && *((PBYTE) address + 10) == 0xff
          && *((PBYTE) address + 11) == 0xd2) {
          BYTE high = *((PBYTE) address + 2);
          BYTE low = *((PBYTE) address + 1);
          return (high << 8) | low;
      }

      // Windows 8, Windows 8.1, opcodes should be :
      //    MOV  EAX, <syscall>
      //    CALL large dword ptr fs:0C0h
      if (*((PBYTE) address) == 0xb8
          && *((PBYTE) address + 3) == 0x00
          && *((PBYTE) address + 4) == 0x00
          && *((PBYTE) address + 5) == 0x64
          && *((PBYTE) address + 6) == 0xff
          && *((PBYTE) address + 7) == 0x15
          && *((PBYTE) address + 8) == 0xc0
          && *((PBYTE) address + 9) == 0x00
          && *((PBYTE) address + 10) == 0x00
          && *((PBYTE) address + 11) == 0x00) {
          BYTE high = *((PBYTE) address + 2);
          BYTE low = *((PBYTE) address + 1);
          return (high << 8) | low;
      }

      // Windows 7, Windows 7 (SP1), opcodes should be :
      //    MOV  EAX, <syscall>
      //    XOR ECX, ECX
      //    LEA  EDX, [ESP + 4]
      //    CALL LARGE dword ptr fs : 0C0h
      if (*((PBYTE) address) == 0xb8
          && *((PBYTE) address + 3) == 0x00
          && *((PBYTE) address + 4) == 0x00
          && *((PBYTE) address + 5) == 0x33
          && *((PBYTE) address + 6) == 0xc9
          && *((PBYTE) address + 7) == 0x8d
          && *((PBYTE) address + 8) == 0x54
          && *((PBYTE) address + 9) == 0x24
          && *((PBYTE) address + 10) == 0x04
          && *((PBYTE) address + 11) == 0x64
          && *((PBYTE) address + 12) == 0xff
          && *((PBYTE) address + 13) == 0x15
          && *((PBYTE) address + 14) == 0xc0
          && *((PBYTE) address + 15) == 0x00
          && *((PBYTE) address + 16) == 0x00
          && *((PBYTE) address + 17) == 0x00) {
          BYTE high = *((PBYTE) address + 2);
          BYTE low = *((PBYTE) address + 1);
          return (high << 8) | low;
      }

      // Windows 7, Windows 7 (SP1), opcodes should be :
      //    MOV  EAX, <syscall>
      //    MOV ECX, ??
      //    LEA  EDX, [ESP + 4]
      //    CALL LARGE dword ptr fs : 0C0h
      if (*((PBYTE) address) == 0xb8
          && *((PBYTE) address + 3) == 0x00
          && *((PBYTE) address + 4) == 0x00
          && *((PBYTE) address + 5) == 0xb9
          && *((PBYTE) address + 10) == 0x8d
          && *((PBYTE) address + 11) == 0x54
          && *((PBYTE) address + 12) == 0x24
          && *((PBYTE) address + 13) == 0x04
          && *((PBYTE) address + 14) == 0x64
          && *((PBYTE) address + 15) == 0xff
          && *((PBYTE) address + 16) == 0x15
          && *((PBYTE) address + 17) == 0xc0
          && *((PBYTE) address + 18) == 0x00
          && *((PBYTE) address + 19) == 0x00
          && *((PBYTE) address + 20) == 0x00) {
          BYTE high = *((PBYTE) address + 2);
          BYTE low = *((PBYTE) address + 1);
          return (high << 8) | low;
      }
  }
#endif

  return ULONG_MAX;
}

/*
 * https://github.com/am0nsec/HellsGate
 * https://blog.sektor7.net/#!res/2021/halosgate.md
 */

unsigned long deinamig_get_syscall_number_hellsgate(void* function_address, int try_halo) {

  if (function_address == NULL) {
    return ULONG_MAX;
  }

  // Quick and dirty fix in case the function has been hooked
  DWORD index = 0;
  while (TRUE) {
#ifdef _WIN64
    // check for syscall, in this case we are too far
    if (*((PBYTE) function_address + index) == 0x0f && *((PBYTE) function_address + index + 1) == 0x05)
      break;
#else
    // check for sysenter, in this case we are too far
    if (*((PBYTE) function_address + index) == 0x0f && *((PBYTE) function_address + index + 1) == 0x34)
        break;
#endif

    // check if ret or retn, in this case we are also probably too far
    if (*((PBYTE) function_address + index) == 0xc3 || *((PBYTE) function_address + index) == 0xc2)
      break;

    DWORD hell = extract_syscall_hellsgate(function_address + index);
    if (hell != ULONG_MAX) {
      DPRINT("Found syscall number %lu.\n", hell);
      return hell;
    }

    index++;
  }

  if(!try_halo) {
    DPRINT("Syscall number not found.\n");
    return ULONG_MAX;
  }

  // Not found, try a method similar to Halo's Gate by deducing from neighboring functions
  DPRINT("Syscall number not found, trying Halo's Gate method.\n");

  for (index = 1; index <= DEINAMIG_MAX_ENTRIES; index++) {
    // check neighboring syscall down
    DWORD hell = extract_syscall_hellsgate(function_address + index * DEINAMIG_HALO_DOWN);
    if (hell != ULONG_MAX) {
      DPRINT("Found syscall number %lu.\n", hell - index);
      return hell - index;
    }
    
    // check neighboring syscall up
    hell = extract_syscall_hellsgate(function_address + index * DEINAMIG_HALO_UP);
    if (hell != ULONG_MAX) {
      DPRINT("Found syscall number %lu.\n", hell + index);
      return hell + index;
    }
  }

  DPRINT("Syscall number not found.\n");
  return ULONG_MAX;
}

/*
 * https://www.mdsec.co.uk/2022/04/resolving-system-service-numbers-using-the-exception-directory/
 */

unsigned long deinamig_get_syscall_number_runtbl_by_hash(void* ntdll,
                                                         unsigned long function_hash,
                                                         unsigned long key,
                                                         deinamig_hash_function hash_function) {

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) ntdll;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, ntdll, dos->e_lfanew);
  PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY) nt->OptionalHeader.DataDirectory;
  DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;

  if (!rva)
    return ULONG_MAX;

  PIMAGE_RUNTIME_FUNCTION_ENTRY rtf = RVA2VA(PIMAGE_RUNTIME_FUNCTION_ENTRY, ntdll, rva);

  rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  PIMAGE_EXPORT_DIRECTORY exp = RVA2VA(PIMAGE_EXPORT_DIRECTORY, ntdll, rva);

  PDWORD adr = RVA2VA(PDWORD, ntdll, exp->AddressOfFunctions);
  PDWORD sym = RVA2VA(PDWORD, ntdll, exp->AddressOfNames);
  PWORD ord = RVA2VA(PWORD, ntdll, exp->AddressOfNameOrdinals);

  int ssn = 0;
  for (int i = 0; rtf[i].BeginAddress; i++) { // Search runtime function table.
    for (int j = 0; j < exp->NumberOfFunctions; j++) { // Search export address table.
      if (adr[ord[j]] == rtf[i].BeginAddress) { // begin address rva?
        PCHAR function_name = RVA2VA(PCHAR, ntdll, sym[j]);

        if (*(USHORT*) function_name == 0x775a) { // 'wZ'
          // duplicate string and replace wZ with tN
          CHAR buf[256] = {0};
          deinamig_strlcpy(buf, function_name, 256);
          buf[0] = 'N';
          buf[1] = 't';

          // our system call? if true, return ssn
          if (hash_function(buf, key) == function_hash)
            return ssn;

          ssn++;
        }
      }
    }
  }

  DPRINT("Syscall number not found.\n");
  return ULONG_MAX;
}

unsigned long deinamig_get_syscall_number_runtbl_by_name(void* ntdll,
                                                         const char* export_name) {

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) ntdll;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, ntdll, dos->e_lfanew);
  PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY) nt->OptionalHeader.DataDirectory;
  DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;

  if (!rva)
    return ULONG_MAX;

  PIMAGE_RUNTIME_FUNCTION_ENTRY rtf = RVA2VA(PIMAGE_RUNTIME_FUNCTION_ENTRY, ntdll, rva);

  rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  PIMAGE_EXPORT_DIRECTORY exp = RVA2VA(PIMAGE_EXPORT_DIRECTORY, ntdll, rva);

  PDWORD adr = RVA2VA(PDWORD, ntdll, exp->AddressOfFunctions);
  PDWORD sym = RVA2VA(PDWORD, ntdll, exp->AddressOfNames);
  PWORD ord = RVA2VA(PWORD, ntdll, exp->AddressOfNameOrdinals);

  int ssn = 0;
  for (int i = 0; rtf[i].BeginAddress; i++) { // Search runtime function table.
    for (int j = 0; j < exp->NumberOfFunctions; j++) { // Search export address table.
      if (adr[ord[j]] == rtf[i].BeginAddress) { // begin address rva?
        PCHAR function_name = RVA2VA(PCHAR, ntdll, sym[j]);

        if (*(USHORT*) function_name == 0x775a) { // 'wZ'
          // duplicate string and replace wZ with tN
          CHAR buf[256] = {0};
          deinamig_strlcpy(buf, function_name, 256);
          buf[0] = 'N';
          buf[1] = 't';

          // our system call? if true, return ssn
          if (deinamig_strcmp(export_name, buf) == 0)
            return ssn;

          ssn++;
        }
      }
    }
  }

  DPRINT("Syscall number not found.\n");
  return ULONG_MAX;
}

/*
 * https://www.mdsec.co.uk/2022/01/edr-parallel-asis-through-analysis/
 */

int deinamig_get_syscall_number_ldrp_thunk_signature(PDWORD syscall_NtOpenFile, PDWORD syscall_NtOpenSection, PDWORD syscall_NtCreateSection, PDWORD syscall_NtMapViewOfSection, PDWORD syscall_NtQueryAttributesFile) {
  char ntdll_dll[] = {'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l', '\0'};
  void* ntdll = deinamig_get_peb_ldr_library_entry_by_name(ntdll_dll);

  if (ntdll == NULL) {
    return FALSE;
  }

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) ntdll;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, ntdll, dos->e_lfanew);
  PIMAGE_SECTION_HEADER sh = IMAGE_FIRST_SECTION(nt);
  ULONG_PTR data_section_address = 0;
  DWORD data_section_size = 0;

  for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
    if (!strcmp((char*)sh[i].Name, ".data")) {
      data_section_address = (ULONG_PTR)ntdll + sh[i].VirtualAddress;
      data_section_size = sh[i].Misc.VirtualSize;
      break;
    }
  }

  if (!data_section_address || data_section_size < 16 * 5) {
    return FALSE;
  }

  BOOL got_them = FALSE;
  for (UINT uiOffset = 0; uiOffset < data_section_size - (16 * 5); uiOffset++) {
    if (*(DWORD*)(data_section_address + uiOffset) == 0xb8d18b4c &&
        *(DWORD*)(data_section_address + uiOffset + 16) == 0xb8d18b4c &&
        *(DWORD*)(data_section_address + uiOffset + 32) == 0xb8d18b4c &&
        *(DWORD*)(data_section_address + uiOffset + 48) == 0xb8d18b4c &&
        *(DWORD*)(data_section_address + uiOffset + 64) == 0xb8d18b4c)
    {
      *syscall_NtOpenFile = *(DWORD*)(data_section_address + uiOffset + 4);
      *syscall_NtCreateSection = *(DWORD*)(data_section_address + uiOffset + 16 + 4);
      *syscall_NtQueryAttributesFile = *(DWORD*)(data_section_address + uiOffset + 32 + 4);
      *syscall_NtOpenSection = *(DWORD*)(data_section_address + uiOffset + 48 + 4);
      *syscall_NtMapViewOfSection = *(DWORD*)(data_section_address + uiOffset + 64 + 4);
      got_them = TRUE;
      break;
    }
  }

  if (!got_them)
    return FALSE;

  return TRUE;
}

// https://github.com/vxunderground/VXUG-Papers/blob/main/Hells%20Gate/C%23%20Implementation/SharpHellsGate/HellsGate.cs#L62

unsigned char* deinamig_build_direct_syscall_stub(unsigned char* stub_region, unsigned long syscall_number, int* stub_size)
{
#ifdef _WIN64
  unsigned char syscall_stub[] = {
      0x4c, 0x8b, 0xd1,             // mov     r10,rcx
      0xb8, 0x00, 0x00, 0x00, 0x00, // mov     eax,xxx
      0x0f, 0x05,                   // syscall
      0xc3                          // ret
  };

  if (stub_region != NULL) {
    deinamig_memcpy((PBYTE) stub_region, syscall_stub, sizeof(syscall_stub));
    deinamig_memcpy((PBYTE) stub_region + 4, &syscall_number, sizeof(DWORD));
  }
#else
  // TODO test that this fully works
  unsigned char syscall_stub[] = {
	// assign syscall number for later use
	0xB8, 0x00, 0x00, 0x00, 0x00,			        // mov eax, syscall number

	// validate the architecture of the operating system
	0x64, 0x8B, 0x0D, 0xC0, 0x00, 0x00, 0x00, 	    // mov ecx, dword ptr fs:[C0]
	0x85, 0xC9,					                    // test ecx, ecx
	0x75, 0x0f,					                    // jne 18 <wow64>
	0xE8, 0x01, 0x00, 0x00, 0x00,			        // call 1
	0xC3,						                    // ret

	// x86 syscall for 32-bit OS (Windows 8+)
	0x89, 0xE2,					                    // mov edx, esp
	0x0F, 0x34,					                    // sysenter
	0xC3,						                    // ret

    // Perhaps use int 2Eh to make it compatible with older Windows versions?
	//0x89, 0xE2,                                     // mov edx, esp
    //0xcd, 0x2e,                                     // int 2Eh
	//0xC3,                                           // ret

    // x86 syscall for 32-bit OS (Windows 7)
    //0xBA, 0x00, 0x03, 0xFE, 0x7F,                   // mov edx, 7FFE0300h (0x7FFE0308 on Windows 10+?) // http://www.rohitab.com/discuss/topic/40881-a-quick-way-to-detect-64-bit-windows/
    //0xFF, 0x12,                                     // call dword ptr [edx]
	//0xC3	[??]					                  // ret ..argbytes..

	// x64 syscall for 64-bit OS					// wow64
	0x64, 0xFF, 0x15, 0xC0, 0x00, 0x00, 0x00,   	// call dword ptr fs:[C0] ; call KiFastSystemCall
	0xC3						                    // ret
  };

  if (stub_region != NULL) {
    deinamig_memcpy((PBYTE)stub_region, syscall_stub, sizeof(syscall_stub));
    deinamig_memcpy((PBYTE)stub_region + 1, &syscall_number, sizeof(DWORD));
    deinamig_memcpy((PBYTE)stub_region + 25, &syscall_address, sizeof(void*));
  }
#endif

  /*
  // TODO test and fix
  // This likely will only work on Windows 10+ as need to reference SharedUserData+0x300 on earlier versions of windows
  unsigned char syscall_stub[] = {
    0x4c, 0x8b, 0xd1,                                      // mov  r10, rcx
    0xb8, 0x00, 0x00, 0x00, 0x00,                          // mov  eax, <syscall>
    0xf6, 0x04, 0x25, 0x08, 0x03, 0xfe, 0x7f, 0x01,        // test byte ptr [SharedUserData+0x308],1
    0x75, 0x03,                                            // jne  ntdll!<function>+0x15
    0x0f, 0x05,                                            // syscall
    0xc3,                                                  // ret
    0xcd, 0x2e,                                            // int  2Eh
    0xc3                                                   // ret
  };

  if (stub_region != NULL) {
    deinamig_memcpy((PBYTE)stub_region, syscall_stub, sizeof(syscall_stub));
    deinamig_memcpy((PBYTE)stub_region + 4, &syscall_number, sizeof(DWORD));
  }
  */

  if (stub_size != NULL) {
    *stub_size = sizeof(syscall_stub) / sizeof(syscall_stub[0]);
  }
  return stub_region;
}

void* deinamig_get_syscall_address(void* function_address, int try_halo) {

  if (function_address == NULL) {
    return NULL;
  }

#ifndef _WIN64
  if (deinamig_is_wow32()) {
    // if we are a WoW64 process, just jump to WOW32Reserved
    return (PVOID)__readfsdword(0xc0);
  }
#endif

#ifdef _WIN64
  // All Windows 64 bit
  BYTE syscall_code[] = { 0x0f, 0x05, 0xc3 };
  ULONG distance_to_syscall = 0x12;
#else
  // Windows 8, 8.1, 10
  BYTE syscall_code[] = { 0x0f, 0x34, 0xc3 };
  ULONG distance_to_syscall = 0x0f;
#endif

  PVOID syscall_address = NULL;

  // Let's try to find a 'syscall' instruction near to function address
  for (WORD index = 0; index < 32; index++) {
    syscall_address = RVA2VA(PVOID, function_address, index);

    // check if ret or retn, in this case we have probably gone too far
    if (*((PBYTE) syscall_address) == 0xc3 || *((PBYTE) syscall_address) == 0xc2)
      break;

    if (!deinamig_memcmp((PVOID)syscall_code, syscall_address, sizeof(syscall_code))) {
      DPRINT("Found syscall instruction within function body at 0x%p.\n", syscall_address);
      return syscall_address;
    }
  }

  if (!try_halo) {
    DPRINT("Syscall instruction not found.\n");
    return NULL;
  }

  DPRINT("Syscall instruction not found, searching neighbouring functions.\n");

  // Instruction has not been found, let's try to use one in a neighbouring function
  for (ULONG32 num_jumps = 1; num_jumps < DEINAMIG_MAX_ENTRIES; num_jumps++) {
    // let's try with an API below our syscall
    syscall_address = RVA2VA(PVOID, function_address, distance_to_syscall + num_jumps * 0x20);
    if (!deinamig_memcmp((PVOID)syscall_code, syscall_address, sizeof(syscall_code))) {
      DPRINT("Found syscall instruction in neighbouring function body at 0x%p.\n", syscall_address);
      return syscall_address;
    }

    // let's try with an API above our syscall
    syscall_address = RVA2VA(PVOID, function_address, distance_to_syscall - num_jumps * 0x20);
    if (!deinamig_memcmp((PVOID)syscall_code, syscall_address, sizeof(syscall_code))) {
      DPRINT("Found syscall instruction in neighbouring function body at 0x%p.\n", syscall_address);
      return syscall_address;
    }
  }

  DPRINT("Syscall instruction not found.\n");
  return NULL;
}

unsigned char* deinamig_build_indirect_syscall_stub(unsigned char* stub_region, unsigned long syscall_number, void* syscall_address, int* stub_size)
{
#ifdef _WIN64
  unsigned char syscall_stub[] = {
    0x4C, 0x8B, 0xD1,                                           // mov r10, rcx
    0xB8, 0x00, 0x00, 0x00, 0x00,    	                        // mov eax, syscall number
    0x49, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs r11, syscall address
    0x41, 0xFF, 0xE3 				       	                    // jmp r11
  };

  if (stub_region != NULL) {
    deinamig_memcpy((PBYTE) stub_region, syscall_stub, sizeof(syscall_stub));
    deinamig_memcpy((PBYTE) stub_region + 4, &syscall_number, sizeof(DWORD));
    deinamig_memcpy((PBYTE) stub_region + 10, &syscall_address, sizeof(void *));
  }
#else
  unsigned char syscall_stub[] = {
	// assign syscall number for later use
	0xB8, 0x00, 0x00, 0x00, 0x00,			        // mov eax, syscall number

	// validate the architecture of the operating system
	0x64, 0x8B, 0x0D, 0xC0, 0x00, 0x00, 0x00, 	    // mov ecx, dword ptr fs:[C0]
	0x85, 0xC9,					                    // test ecx, ecx
	0x75, 0x0f,					                    // jne 18 <wow64>
	0xE8, 0x01, 0x00, 0x00, 0x00,			        // call 1
	0xC3,						                    // ret

	// x86 syscall for 32-bit OS
	0x89, 0xE2,					                    // mov edx, esp
	0xB9, 0x00, 0x00, 0x00, 0x00,			        // mov ecx, syscall address
	0xFF, 0xE1,					                    // jmp ecx

	// x64 syscall for 64-bit OS					// wow64
	0x64, 0xFF, 0x15, 0xC0, 0x00, 0x00, 0x00,   	// call dword ptr fs:[C0] ; call KiFastSystemCall
	0xC3						                    // ret
  };

  if (stub_region != NULL) {
    deinamig_memcpy((PBYTE)stub_region, syscall_stub, sizeof(syscall_stub));
    deinamig_memcpy((PBYTE)stub_region + 1, &syscall_number, sizeof(DWORD));
    deinamig_memcpy((PBYTE)stub_region + 25, &syscall_address, sizeof(void*));
  }
#endif

  if (stub_size != NULL) {
    *stub_size = sizeof(syscall_stub) / sizeof(syscall_stub[0]);
  }
  return stub_region;
}

/*
void* deinamig_get_syscall_stub_address(unsigned long function_hash,
                                        unsigned long key,
                                        deinamig_hash_function deinamig_hash_function) {

  char ntdll_dll[] = {'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l', '\0'};
  void* function_address = deinamig_get_library_address_by_hash(ntdll_dll,
                                                                function_hash,
                                                                key,
                                                                deinamig_hash_function,
                                                                FALSE,
                                                                FALSE);

  if (function_address == NULL) {
    DPRINT("Could not find function with hash %lu.\n", function_hash);
    return NULL;
  }

  // Quick and dirty fix in case the function has been hooked
  DWORD index = 0;
  while (TRUE) {
#ifdef _WIN64
    // check for syscall, in this case we are too far
    if (*((PBYTE) function_address + index) == 0x0f && *((PBYTE) function_address + index + 1) == 0x05)
      break;
#else
    // check for sysenter, in this case we are too far
    if (*((PBYTE) function_address + index) == 0x0f && *((PBYTE) function_address + index + 1) == 0x34)
        break;
#endif

    // check if ret or retn, in this case we are also probably too far
    if (*((PBYTE) function_address + index) == 0xc3 || *((PBYTE) function_address + index) == 0xc2)
      break;

    DWORD hell = extract_syscall_hellsgate(function_address + index);
    if (hell != ULONG_MAX) {
      DPRINT("Found syscall stub address as number %p.\n", function_address + index);
      return function_address + index;
    }

    index++;
  }

  return NULL;
}
*/

unsigned char* deinamig_carve_syscall_stub(void* start_address,
                                           int carve_amount,
                                           unsigned char* stub_region) {

  if (start_address == NULL) {
    return NULL;
  }

  if (stub_region != NULL) {
    deinamig_memcpy((PBYTE) stub_region, start_address, carve_amount);
  }

  return stub_region;
}

static DWORD rva_to_offset(PIMAGE_NT_HEADERS nt, DWORD rva)
{
  if(rva == 0) return 0;

  PIMAGE_SECTION_HEADER sh = IMAGE_FIRST_SECTION(nt);
  for(DWORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
    DWORD size = sh[i].Misc.VirtualSize ? sh[i].Misc.VirtualSize : sh[i].SizeOfRawData;
    if(sh[i].VirtualAddress <= rva && rva <= (DWORD)sh[i].VirtualAddress + sh[i].SizeOfRawData) {
      if(rva >= sh[i].VirtualAddress && rva <  sh[i].VirtualAddress + size) {
        return sh[i].PointerToRawData + (rva - sh[i].VirtualAddress);
      }
    }
  }
  return 0;
}

void *deinamig_get_export_address_from_raw_by_name(void *module_base,
                                                   const char *export_name) {
  DPRINT("Searching for function %s in library with base address of 0x%p.\n", export_name, module_base);

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);
  PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY) nt->OptionalHeader.DataDirectory;
  DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  DWORD offset = rva_to_offset(nt, rva);

  PIMAGE_EXPORT_DIRECTORY exp = (PIMAGE_EXPORT_DIRECTORY)((PBYTE) module_base + offset);
  DWORD cnt = exp->NumberOfNames;

  // if no api names, return NULL
  if (cnt == 0) return NULL;

  offset = rva_to_offset(nt, exp->AddressOfNames);
  PDWORD sym = (PDWORD)((PBYTE) module_base + offset);

  offset = rva_to_offset(nt, exp->AddressOfFunctions);
  PDWORD adr = (PDWORD)((PBYTE) module_base + offset);

  offset = rva_to_offset(nt, exp->AddressOfNameOrdinals);
  PWORD ord = (PWORD)((PBYTE) module_base + offset);

  LPVOID addr = NULL;

  do {
    // search function name in list of exported names
    PCHAR api = (PCHAR)(rva_to_offset(nt, sym[cnt - 1]) + (PBYTE) module_base);

    if (deinamig_strcmp(export_name, api) == 0) {
      addr = (PVOID)((PBYTE) module_base + rva_to_offset(nt, adr[ord[cnt - 1]]));
      break;
    }
  } while (--cnt);

  if (addr != NULL)
    DPRINT("Found function %s at 0x%p.\n", export_name, addr);
  else
    DPRINT("Could not find function %s.\n", export_name);

  return addr;
}

void *deinamig_get_export_address_from_raw_by_hash(void *module_base,
                                                   unsigned long function_hash,
                                                   unsigned long key,
                                                   deinamig_hash_function hash_function) {
  DPRINT("Searching for function with hash 0x%lu in library with base address of 0x%p.\n", function_hash, module_base);

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);
  PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY) nt->OptionalHeader.DataDirectory;
  DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  DWORD offset = rva_to_offset(nt, rva);

  PIMAGE_EXPORT_DIRECTORY exp = (PIMAGE_EXPORT_DIRECTORY)((PBYTE) module_base + offset);
  DWORD cnt = exp->NumberOfNames;

  // if no api names, return NULL
  if (cnt == 0) return NULL;

  offset = rva_to_offset(nt, exp->AddressOfNames);
  PDWORD sym = (PDWORD)((PBYTE) module_base + offset);

  offset = rva_to_offset(nt, exp->AddressOfFunctions);
  PDWORD adr = (PDWORD)((PBYTE) module_base + offset);

  offset = rva_to_offset(nt, exp->AddressOfNameOrdinals);
  PWORD ord = (PWORD)((PBYTE) module_base + offset);

  LPVOID addr = NULL;

  do {
    // search function name in list of exported names
    PCHAR api = (PCHAR)(rva_to_offset(nt, sym[cnt - 1]) + (PBYTE) module_base);

    if (hash_function(api, key) == function_hash) {
      addr = (PVOID)((PBYTE) module_base + rva_to_offset(nt, adr[ord[cnt - 1]]));
      break;
    }
  } while (--cnt);

  if (addr != NULL)
    DPRINT("Found function with hash 0x%lu at 0x%p.\n", function_hash, addr);
  else
    DPRINT("Could not find function with hash 0x%lu.\n", function_hash);

  return addr;
}