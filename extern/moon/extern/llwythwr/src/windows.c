#include "llwythwr/windows.h"

#include <windows.h>
#include <winternl.h>
#include <stddef.h>

#include "debug.h"
#include "llwythwr/windows.h"

/*
 * Typedefs
 */

typedef BOOL (WINAPI *Type_DllMain)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
typedef BOOLEAN (NTAPI *PLDR_INIT_ROUTINE)(PVOID DllHandle, ULONG Reason, PVOID Context);
typedef VOID (WINAPI *Type_Start)(PPEB);
//typedef HMODULE (WINAPI *Type_LoadLibraryA)(LPCSTR);              // LdrLoadDll
//typedef BOOL (WINAPI *Type_FreeLibrary)(HMODULE);                 // LdrUnloadDll
//typedef FARPROC (WINAPI *Type_GetProcAddress)(HMODULE, LPCSTR);   // LdrGetProcedureAddress

/*
 * Defines
 */

// Relative Virtual Address to Virtual Address
#define RVA2VA(type, base, rva) (type)((ULONG_PTR) base + rva)

// LDR_DATA_TABLE_ENTRY->Flags
#define LDRP_PACKAGED_BINARY 0x00000001
#define LDRP_MARKED_FOR_REMOVAL 0x00000002
#define LDRP_IMAGE_DLL 0x00000004
#define LDRP_LOAD_NOTIFICATIONS_SENT 0x00000008
#define LDRP_TELEMETRY_ENTRY_PROCESSED 0x00000010
#define LDRP_PROCESS_STATIC_IMPORT 0x00000020
#define LDRP_IN_LEGACY_LISTS 0x00000040
#define LDRP_IN_INDEXES 0x00000080
#define LDRP_SHIM_DLL 0x00000100
#define LDRP_IN_EXCEPTION_TABLE 0x00000200
#define LDRP_LOAD_IN_PROGRESS 0x00001000
#define LDRP_LOAD_CONFIG_PROCESSED 0x00002000
#define LDRP_ENTRY_PROCESSED 0x00004000
#define LDRP_PROTECT_DELAY_LOAD 0x00008000 //  LDRP_ENTRY_INSERTED
#define LDRP_DONT_CALL_FOR_THREADS 0x00040000
#define LDRP_PROCESS_ATTACH_CALLED 0x00080000
#define LDRP_PROCESS_ATTACH_FAILED 0x00100000
#define LDRP_COR_DEFERRED_VALIDATE 0x00200000
#define LDRP_COR_IMAGE 0x00400000
#define LDRP_DONT_RELOCATE 0x00800000
#define LDRP_COR_IL_ONLY 0x01000000
#define LDRP_CHPE_IMAGE 0x02000000
#define LDRP_CHPE_EMULATOR_IMAGE 0x04000000
#define LDRP_REDIRECTED 0x10000000
#define LDRP_COMPAT_DATABASE_PROCESSED 0x80000000

#define LDR_DATA_TABLE_ENTRY_SIZE_WINXP FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, DdagNode)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN7 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, BaseNameHashValue)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN8 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, ImplicitPathOptions)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN10 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, SigningLevel)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN11 sizeof(LDR_DATA_TABLE_ENTRY)

#define LDR_HASH_TABLE_ENTRIES 32

/*
 * Enums
 */

typedef enum LDR_DDAG_STATE
{
  LdrModulesMerged=-5,
  LdrModulesInitError=-4,
  LdrModulesSnapError=-3,
  LdrModulesUnloaded=-2,
  LdrModulesUnloading=-1,
  LdrModulesPlaceHolder=0,
  LdrModulesMapping=1,
  LdrModulesMapped=2,
  LdrModulesWaitingForDependencies=3,
  LdrModulesSnapping=4,
  LdrModulesSnapped=5,
  LdrModulesCondensed=6,
  LdrModulesReadyToInit=7,
  LdrModulesInitializing=8,
  LdrModulesReadyToRun=9
} LDR_DDAG_STATE;

typedef enum LDR_DLL_LOAD_REASON
{
  LoadReasonStaticDependency=0,
  LoadReasonStaticForwarderDependency=1,
  LoadReasonDynamicForwarderDependency=2,
  LoadReasonDelayloadDependency=3,
  LoadReasonDynamicLoad=4,
  LoadReasonAsImageLoad=5,
  LoadReasonAsDataLoad=6,
  LoadReasonUnknown=-1
} LDR_DLL_LOAD_REASON;

typedef enum _SECTION_INHERIT {
  ViewShare = 1,
  ViewUnmap = 2
} SECTION_INHERIT;

typedef enum LIBRARY_TYPE {
  Mapped = 1,
  Image = 2,
  Private = 3
} LIBRARY_TYPE;

/*
 * Structures
 */

typedef struct _IMAGE_RELOC {
  WORD offset: 12;
  WORD type: 4;
} IMAGE_RELOC, *PIMAGE_RELOC;

typedef struct RTL_BALANCED_NODE1
{
  union {
    struct RTL_BALANCED_NODE1 * Children[2];
    struct {
      struct RTL_BALANCED_NODE1 * Left;
      struct RTL_BALANCED_NODE1 * Right;
    };
  };
  union {
    union {
      struct {
        unsigned char Red: 1;
      };
      struct {
        unsigned char Balance: 2;
      };
    };

    size_t ParentValue;
  };
} RTL_BALANCED_NODE1;

typedef struct LDR_DDAG_NODE1
{
  LIST_ENTRY Modules;
  struct LDR_SERVICE_TAG_RECORD * ServiceTagList;
  unsigned long LoadCount;
  unsigned long ReferenceCount;
  unsigned long DependencyCount;
  SINGLE_LIST_ENTRY RemovalLink;
  void* IncomingDependencies;
  LDR_DDAG_STATE State;
  SINGLE_LIST_ENTRY CondenseLink;
  unsigned long PreorderNumber;
  unsigned long LowestLink;
} LDR_DDAG_NODE1;

typedef struct LDR_DATA_TABLE_ENTRY1
{
  LIST_ENTRY InLoadOrderLinks;
  LIST_ENTRY InMemoryOrderLinks;
  union
  {
    LIST_ENTRY InInitializationOrderLinks;
    LIST_ENTRY InProgressLinks;
  };
  PVOID DllBase;
  PLDR_INIT_ROUTINE EntryPoint;
  ULONG SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
  union
  {
    UCHAR FlagGroup[4];
    ULONG Flags;
    struct
    {
      ULONG PackagedBinary : 1;
      ULONG MarkedForRemoval : 1;
      ULONG ImageDll : 1;
      ULONG LoadNotificationsSent : 1;
      ULONG TelemetryEntryProcessed : 1;
      ULONG ProcessStaticImport : 1;
      ULONG InLegacyLists : 1;
      ULONG InIndexes : 1;
      ULONG ShimDll : 1;
      ULONG InExceptionTable : 1;
      ULONG ReservedFlags1 : 2;
      ULONG LoadInProgress : 1;
      ULONG LoadConfigProcessed : 1;
      ULONG EntryProcessed : 1;
      ULONG ProtectDelayLoad : 1;
      ULONG ReservedFlags3 : 2;
      ULONG DontCallForThreads : 1;
      ULONG ProcessAttachCalled : 1;
      ULONG ProcessAttachFailed : 1;
      ULONG CorDeferredValidate : 1;
      ULONG CorImage : 1;
      ULONG DontRelocate : 1;
      ULONG CorILOnly : 1;
      ULONG ChpeImage : 1;
      ULONG ReservedFlags5 : 2;
      ULONG Redirected : 1;
      ULONG ReservedFlags6 : 2;
      ULONG CompatDatabaseProcessed : 1;
    };
  };
  USHORT ObsoleteLoadCount;
  USHORT TlsIndex;
  LIST_ENTRY HashLinks;
  ULONG TimeDateStamp;
  struct _ACTIVATION_CONTEXT *EntryPointActivationContext;
  PVOID Lock; // RtlAcquireSRWLockExclusive
  LDR_DDAG_NODE1* DdagNode;
  LIST_ENTRY NodeModuleLink;
  struct _LDRP_LOAD_CONTEXT *LoadContext;
  PVOID ParentDllBase;
  PVOID SwitchBackContext;
  RTL_BALANCED_NODE1 BaseAddressIndexNode;
  RTL_BALANCED_NODE1 MappingInfoIndexNode;
  ULONG_PTR OriginalBase;
  LARGE_INTEGER LoadTime;
  ULONG BaseNameHashValue;
  LDR_DLL_LOAD_REASON LoadReason;
  ULONG ImplicitPathOptions;
  ULONG ReferenceCount;
  ULONG DependentLoadFlags;
  //UCHAR SigningLevel; // since REDSTONE2
  //ULONG CheckSum; // since 22H1
  //PVOID ActivePatchImageBase;
  //LDR_HOT_PATCH_STATE HotPatchState;
} LDR_DATA_TABLE_ENTRY1;

typedef struct PEB_LDR_DATA1
{
  unsigned long Length;
  unsigned char Initialized;
  void * SsHandle;
  LIST_ENTRY InLoadOrderModuleList;
  LIST_ENTRY InMemoryOrderModuleList;
  LIST_ENTRY InInitializationOrderModuleList;
  void * EntryInProgress;
  unsigned char ShutdownInProgress;
  void * ShutdownThreadId;
} PEB_LDR_DATA1;

#ifdef _WIN64
typedef struct POINTER_LIST {
  struct POINTER_LIST *next;
  void *address;
} POINTER_LIST;
#endif

typedef struct llwythwr_dependency_library
{
  char* name;
  void* handle;
} llwythwr_dependency_library;

struct llwythwr_windows_library
{
  void* base_address;
  HANDLE section;
  BOOL isDLL;
  LPVOID entry_point;
  BOOL entry_point_called;
  LPVOID* dependences;
  int number_of_dependences;
  unsigned long size_of_image;
  unsigned long size_of_headers;
  LIBRARY_TYPE type;
  void* userdata;
  llwythwr_free_library_function free_library_function;
  llwythwr_windows_api* api;
  wchar_t* base_name;
  wchar_t* full_path;
  LDR_DATA_TABLE_ENTRY1* ldr_data_table_entry;
  PIMAGE_EXPORT_DIRECTORY export_directory;
  size_t export_directory_size;
#ifdef _WIN64
  POINTER_LIST* blocked_memory;
#endif
};

/*
 * Forward declarations
 */

static void llwythwr_init_unicode_string(PUNICODE_STRING target, PCWSTR source);
static void *llwythwr_memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size);
static void *llwythwr_memset(void *ptr, int value, size_t num);
static int llwythwr_strcmp(const char *s1, const char *s2);
static size_t llwythwr_strlen(const char *str);
static size_t llwythwr_wcslen(const wchar_t *s);
//static int llwythwr_wcscmp(const wchar_t *s1, const wchar_t *s2);
static char *llwythwr_strcat(char *s, char *append);
static int llwythwr_memcmp(const void *s1, const void *s2, size_t n);

static llwythwr_windows_library *llwythwr_wire_library(llwythwr_windows_library *library,
                                                       const void *module_base,
                                                       llwythwr_get_lib_address_function x_get_lib_address,
                                                       llwythwr_get_proc_address_function x_get_proc_address,
                                                       llwythwr_free_library_function x_free_library_function,
                                                       void *user_data);

static LDR_DATA_TABLE_ENTRY1* llwythwr_link_library_win8plus(llwythwr_windows_api *api,
                                                             const void* module_base,
                                                             const WCHAR* dll_name,
                                                             const WCHAR* dll_path);

static void llwythwr_unlink_library_win8plus(llwythwr_windows_api *api, LDR_DATA_TABLE_ENTRY1* entry);

static void llwythwr_ldr_data_table_entry_free(llwythwr_windows_api *api, LDR_DATA_TABLE_ENTRY1* entry);

static void llwythwr_windows_library_init(llwythwr_windows_library *library);

#ifdef _WIN64
static void llwythwr_pointer_list_free(llwythwr_windows_api *api, POINTER_LIST *head);
#endif

/*
 * Public functions
 */

void llwythwr_unload_library(llwythwr_windows_library *library) {
  if (library == NULL) {
    DPRINT("Received NULL value as parameter.\n");
    return;
  }

  if (library->isDLL && library->entry_point_called && library->entry_point != NULL) {
    DPRINT("Executing entrypoint of DLL at %p, notifying process detach.\n", library->entry_point);
    Type_DllMain DllMain = (Type_DllMain) library->entry_point;
    DllMain((HINSTANCE) library->base_address, DLL_PROCESS_DETACH, NULL);
  }

  if (library->dependences != NULL) {
    DPRINT("Releasing modules.\n");
    int i;
    for (i = 0; i < library->number_of_dependences; i++) {
      if (library->dependences[i] != NULL) {
        if (library->free_library_function != NULL) {
          llwythwr_dependency_library* dependency = (llwythwr_dependency_library*)library->dependences[i];
          library->free_library_function(dependency->name, dependency->handle, library->userdata);
        }
        library->api->LocalFree(library->dependences[i]);
      }
    }
    library->api->LocalFree(library->dependences);
  }

  llwythwr_unlink_library(library);

  DPRINT("Releasing memory.\n");
  if (library->type == Private) {
#ifdef _WIN64
    llwythwr_pointer_list_free(library->api, library->blocked_memory);
#endif
    library->api->VirtualFree(library->base_address, 0, MEM_RELEASE);
  }
  else {
    library->api->NtUnmapViewOfSection((HANDLE) -1, library->base_address);
    library->api->CloseHandle(library->section);
  }
  library->api->LocalFree(library);
}

void llwythwr_wipe_headers(llwythwr_windows_library *library) {
  if (library == NULL) {
    DPRINT("Received NULL value as parameter.\n");
    return;
  }

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) library->base_address;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, library->base_address, dos->e_lfanew);

  // change perms of headers to readwrite
  size_t size = nt->OptionalHeader.BaseOfCode;
  DWORD oldprot = 0;
  DWORD newprot = PAGE_READWRITE;
  if (library->type == Mapped)
    newprot = PAGE_WRITECOPY;
  if(!library->api->VirtualProtect(library->base_address, size, newprot, &oldprot)) {
    DPRINT("VirtualProtect failed: %lu\n", library->api->GetLastError());
    return;
  }

  // Perform wipe
  llwythwr_memset(library->base_address, 0, nt->OptionalHeader.SizeOfHeaders);

  // change perms of empty headers back to readonly
  if(!library->api->VirtualProtect(library->base_address, size, PAGE_READONLY, &oldprot))
    DPRINT("VirtualProtect failed: %lu\n", library->api->GetLastError());
}

int llwythwr_call_entry_point(llwythwr_windows_library *library) {
  if (library == NULL || library->entry_point == NULL || library->entry_point_called == TRUE) {
    DPRINT("Received NULL value as parameter.\n");
    return -1;
  }

  library->entry_point_called = TRUE;

  if (library->isDLL) {
    Type_DllMain DllMain = (Type_DllMain) library->entry_point;
    DPRINT("Executing entrypoint of DLL at %p, notifying process attach.\n", DllMain);
    DllMain((HINSTANCE) library->base_address, DLL_PROCESS_ATTACH, NULL);
  } else {
    // if ExitProces is called, this will terminate the host process.
    Type_Start Start = (Type_Start) library->entry_point;
    DPRINT("Executing entrypoint of DLL at %p\n", (PVOID) Start);
    //Start(NtCurrentTeb()->ProcessEnvironmentBlock);
    Start(NULL);
  }

  return 1;
}

llwythwr_windows_library *llwythwr_map_library_from_disk_to_section(llwythwr_windows_api *api, const char *filename) {
  if (api == NULL || filename == NULL) {
    if (api != NULL) api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("Received NULL value as parameter.\n");
    return FALSE;
  }

  HANDLE file_handle = NULL;
  HANDLE section_handle = NULL;
  PVOID view_base = NULL;

  /*
  file_handle = api->CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if (file_handle == NULL) {
    DPRINT("CreateFileA failed 0x%08lx\n", api->GetLastError());
    goto cleanup;
  }
  */

  CHAR path[256] = {0};
  path[0] = '\\';
  path[1] = '?';
  path[2] = '?';
  path[3] = '\\';
  if (llwythwr_strlen(filename) >= 255 - llwythwr_strlen(path)) {
    api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("filename too long, %zu >= %zu\n", llwythwr_strlen(path), 255 - llwythwr_strlen(path));
    goto cleanup;
  }
  llwythwr_strcat(path, (char *) filename);

  WCHAR wpath[256] = {0};
  api->MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, 256);

  UNICODE_STRING FileName = {0};
  llwythwr_init_unicode_string(&FileName, wpath);

  OBJECT_ATTRIBUTES object_attributes = {0};
  InitializeObjectAttributes(&object_attributes, &FileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

  DPRINT("Opening file from %S\n", wpath);

  IO_STATUS_BLOCK status_block = {0};
  NTSTATUS status = api->NtOpenFile(&file_handle,
                                    FILE_READ_DATA | FILE_EXECUTE | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                                    &object_attributes,
                                    &status_block,
                                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);

  if (!NT_SUCCESS(status)) {
    DPRINT("NtOpenFile failed 0x%08lx\n", status);
    api->SetLastError(status);
    goto cleanup;
  }

  DPRINT("File handle is %p\n", file_handle);

  DPRINT("Creating section to store PE.\n");
  status = api->NtCreateSection(&section_handle, SECTION_ALL_ACCESS, NULL, NULL, PAGE_READONLY, SEC_IMAGE, file_handle);

  if (!NT_SUCCESS(status)) {
    DPRINT("NtCreateSection failed 0x%08lx\n", status);
    api->SetLastError(status);
    goto cleanup;
  }

  DPRINT("Mapping local view of section to store PE.\n");
  size_t view_size = 0;
  status =
      api->NtMapViewOfSection(section_handle, (HANDLE) -1, &view_base, 0, 0, 0, &view_size, ViewUnmap, 0, PAGE_READWRITE);

  if (!NT_SUCCESS(status)) {
    DPRINT("NtMapViewOfSection failed 0x%08lx\n", status);
    api->SetLastError(status);
    goto cleanup;
  }

  DPRINT("View size: %lld\n", view_size);
  DPRINT("Mapped to address: %p\n", view_base);

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) view_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, view_base, dos->e_lfanew);

  llwythwr_windows_library *library = (llwythwr_windows_library *) malloc(sizeof(llwythwr_windows_library));
  if (library == NULL) {
    DPRINT("Could not allocate memory.\n");
    SetLastError(ERROR_OUTOFMEMORY);
    goto cleanup;
  }

  llwythwr_windows_library_init(library);

  library->base_address = view_base;
  library->size_of_image = nt->OptionalHeader.SizeOfImage;
  library->size_of_headers = nt->OptionalHeader.SizeOfHeaders;
  library->section = section_handle;
  library->isDLL = (nt->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
  library->entry_point = RVA2VA(LPVOID, view_base, nt->OptionalHeader.AddressOfEntryPoint);
  library->api = api;
  library->type = Image;

  PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY) nt->OptionalHeader.DataDirectory;
  DWORD rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  if (rva != 0) library->export_directory = RVA2VA(PIMAGE_EXPORT_DIRECTORY, view_base, rva);
  library->export_directory_size = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

  api->CloseHandle(file_handle);

  return library;

  cleanup:
  if (view_base != NULL) {
    api->NtUnmapViewOfSection((HANDLE) -1, view_base);
  }

  if (section_handle != NULL) {
    api->CloseHandle(section_handle);
  }

  if (file_handle != NULL) {
    api->CloseHandle(file_handle);
  }

  return NULL;
}

llwythwr_windows_library* llwythwr_alloc_library(llwythwr_windows_api *api,
                                                 const void* module_base,
                                                 llwythwr_get_lib_address_function x_get_lib_address,
                                                 llwythwr_get_proc_address_function x_get_proc_address,
                                                 llwythwr_free_library_function x_free_library_function,
                                                 void* user_data) {
  if (api == NULL || module_base == NULL) {
    if (api != NULL) api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("Received NULL value as parameter.\n");
    return FALSE;
  }

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);

  // before doing anything. check compatibility between exe/dll and host process.
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LDR_DATA_TABLE_ENTRY1 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY1, InInitializationOrderLinks);
  PIMAGE_DOS_HEADER doshost = (PIMAGE_DOS_HEADER) ntdll->DllBase;
  PIMAGE_NT_HEADERS nthost = RVA2VA(PIMAGE_NT_HEADERS, ntdll->DllBase, doshost->e_lfanew);

  if (nt->FileHeader.Machine != nthost->FileHeader.Machine) {
    DPRINT("Host process %08hx and file %08hx are not compatible...cannot load.\n",
           nthost->FileHeader.Machine, nt->FileHeader.Machine);
    return NULL;
  }

  BOOL isDLL = (nt->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
  if (isDLL)
    DPRINT("PE is a DLL.\n");
  else
    DPRINT("PE is an EXE.\n");

    // check if the binary has relocation information
  DWORD size = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
  BOOL has_reloc = size == 0 ? FALSE : TRUE;
  PVOID image_base = NULL;
  if (!has_reloc) {
    DPRINT("No relocation information present, setting the base to: 0x%p\n", (PVOID) nt->OptionalHeader.ImageBase);
    image_base = (PVOID) nt->OptionalHeader.ImageBase;
  }

  DPRINT("Allocating private memory of size %lu bytes.\n", nt->OptionalHeader.SizeOfImage);

  image_base = (unsigned char *) api->VirtualAlloc(image_base,
                                                   nt->OptionalHeader.SizeOfImage,
                                                   MEM_RESERVE | MEM_COMMIT,
                                                   PAGE_READWRITE);

  if (image_base == NULL) {
    image_base = (unsigned char *) api->VirtualAlloc(NULL,
                                                     nt->OptionalHeader.SizeOfImage,
                                                     MEM_RESERVE | MEM_COMMIT,
                                                     PAGE_READWRITE);
    if (image_base == NULL) {
      api->SetLastError(ERROR_OUTOFMEMORY);
      DPRINT("Could not allocate private memory for library.\n");
      return NULL;
    }
  }

#ifdef _WIN64
  POINTER_LIST* blocked_memory = NULL;
  // Memory block may not span 4 GB boundaries.
  while ((((uintptr_t) image_base) >> 32) < (((uintptr_t) (image_base + nt->OptionalHeader.SizeOfImage)) >> 32)) {
    POINTER_LIST* node = (POINTER_LIST*) api->LocalAlloc(LPTR, sizeof(POINTER_LIST));
    if (!node) {
      api->VirtualFree(image_base, 0, MEM_RELEASE);
      llwythwr_pointer_list_free(api, blocked_memory);
      api->SetLastError(ERROR_OUTOFMEMORY);
      DPRINT("Could not allocate private memory for library.\n");
      return NULL;
    }

    node->next = blocked_memory;
    node->address = image_base;
    blocked_memory = node;

    image_base = (unsigned char *) api->VirtualAlloc(NULL,
                                                     nt->OptionalHeader.SizeOfImage,
                                                     MEM_RESERVE | MEM_COMMIT,
                                                     PAGE_READWRITE);
    if (image_base == NULL) {
      llwythwr_pointer_list_free(api, blocked_memory);
      api->SetLastError(ERROR_OUTOFMEMORY);
      DPRINT("Could not allocate private memory for library.\n");
      return NULL;
    }
  }
#endif

  DPRINT("Mapped to address: %p\n", image_base);

  llwythwr_windows_library *library = (llwythwr_windows_library *) malloc(sizeof(llwythwr_windows_library));
  if (library == NULL) {
    DPRINT("Could not allocate memory.\n");
    SetLastError(ERROR_OUTOFMEMORY);
    goto cleanup;
  }

  llwythwr_windows_library_init(library);

  library->base_address = image_base;
  library->size_of_image = nt->OptionalHeader.SizeOfImage;
  library->size_of_headers = nt->OptionalHeader.SizeOfHeaders;
  library->isDLL = (nt->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
  library->entry_point = RVA2VA(LPVOID, image_base, nt->OptionalHeader.AddressOfEntryPoint);
  library->api = api;
  library->type = Private;
#ifdef _WIN64
  library->blocked_memory = blocked_memory;
#endif

  llwythwr_windows_library *module_wired = llwythwr_wire_library(library,
                                                                 module_base,
                                                                 x_get_lib_address,
                                                                 x_get_proc_address,
                                                                 x_free_library_function,
                                                                 user_data);

  if (module_wired != NULL)
    return module_wired;

  llwythwr_unload_library(library);
  return NULL;

cleanup:
  if (image_base != NULL) {
    api->VirtualFree(image_base, 0, MEM_RELEASE);
  }

  return NULL;
}

llwythwr_windows_library* llwythwr_map_library(llwythwr_windows_api *api,
                                               const void *module_base,
                                               llwythwr_get_lib_address_function x_get_lib_address,
                                               llwythwr_get_proc_address_function x_get_proc_address,
                                               llwythwr_free_library_function x_free_library_function,
                                               void *user_data) {
  if (api == NULL || module_base == NULL) {
    if (api != NULL) api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("Received NULL value as parameter.\n");
    return FALSE;
  }

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);

  // before doing anything. check compatibility between exe/dll and host process.
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LDR_DATA_TABLE_ENTRY1 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY1, InInitializationOrderLinks);
  PIMAGE_DOS_HEADER doshost = (PIMAGE_DOS_HEADER) ntdll->DllBase;
  PIMAGE_NT_HEADERS nthost = RVA2VA(PIMAGE_NT_HEADERS, ntdll->DllBase, doshost->e_lfanew);

  if (nt->FileHeader.Machine != nthost->FileHeader.Machine) {
    DPRINT("Host process %08hx and file %08hx are not compatible...cannot load.\n",
           nthost->FileHeader.Machine, nt->FileHeader.Machine);
    return NULL;
  }

  BOOL isDLL = (nt->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
  if (isDLL)
    DPRINT("PE is a DLL.\n");
  else
    DPRINT("PE is an EXE.\n");

  DPRINT("Creating section of size %lu bytes.\n", nt->OptionalHeader.SizeOfImage);

  LARGE_INTEGER liSectionSize;
  liSectionSize.QuadPart = nt->OptionalHeader.SizeOfImage;

  // check if the binary has relocation information
  DWORD size = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
  BOOL has_reloc = size == 0 ? FALSE : TRUE;
  PVOID image_base = NULL;
  if (!has_reloc) {
    DPRINT("No relocation information present, setting the base to: 0x%p\n", (PVOID) nt->OptionalHeader.ImageBase);
    image_base = (PVOID) nt->OptionalHeader.ImageBase;
  }

  DPRINT("Creating section to store PE.\n");
  DPRINT("Requesting section size: %lu\n", nt->OptionalHeader.SizeOfImage);
  HANDLE section_handle = NULL;
  NTSTATUS status = api->NtCreateSection(&section_handle,
                                     SECTION_ALL_ACCESS,
                                     0,
                                     &liSectionSize,
                                     PAGE_EXECUTE_READWRITE,
                                     SEC_COMMIT,
                                     NULL);
  if (!NT_SUCCESS(status)) {
    DPRINT("NtCreateSection failed 0x%08lx\n", status);
    api->SetLastError(status);
    return NULL;
  }

  DPRINT("Mapping local view of section to store PE.\n");
  SIZE_T view_size = 0;
  status =
      api->NtMapViewOfSection(section_handle, (HANDLE) -1, &image_base, 0, 0, 0, &view_size, ViewUnmap, 0, PAGE_READWRITE);
  if (status != 0 && status != 0x40000003) {
    DPRINT("NtMapViewOfSection failed 0x%08lx\n", status);
    api->SetLastError(status);
    goto cleanup;
  }

  DPRINT("View size: %lld\n", view_size);
  DPRINT("Mapped to address: %p\n", image_base);

  llwythwr_windows_library *library = (llwythwr_windows_library *) malloc(sizeof(llwythwr_windows_library));
  if (library == NULL) {
    DPRINT("Could not allocate memory.\n");
    SetLastError(ERROR_OUTOFMEMORY);
    goto cleanup;
  }

  llwythwr_windows_library_init(library);

  library->base_address = image_base;
  library->size_of_image = nt->OptionalHeader.SizeOfImage;
  library->size_of_headers = nt->OptionalHeader.SizeOfHeaders;
  library->section = section_handle;
  library->isDLL = (nt->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
  library->entry_point = RVA2VA(LPVOID, image_base, nt->OptionalHeader.AddressOfEntryPoint);
  library->api = api;
  library->type = Mapped;

  llwythwr_windows_library *module_wired = llwythwr_wire_library(library,
                                                                 module_base,
                                                                 x_get_lib_address,
                                                                 x_get_proc_address,
                                                                 x_free_library_function,
                                                                 user_data);

  if (module_wired != NULL)
    return module_wired;

  llwythwr_unload_library(library);
  return NULL;

cleanup:
  if (image_base != NULL) {
    api->NtUnmapViewOfSection((HANDLE) -1, image_base);
  }

  if (section_handle != NULL) {
    api->CloseHandle(section_handle);
  }

  return NULL;
}

llwythwr_windows_library* llwythwr_load_hollow_library(llwythwr_windows_api *api,
                                                       const void *module_base,
                                                       HMODULE decoy_module,
                                                       BOOL keep_decoy_headers,
                                                       llwythwr_get_lib_address_function x_get_lib_address,
                                                       llwythwr_get_proc_address_function x_get_proc_address,
                                                       llwythwr_free_library_function x_free_library_function,
                                                       void *user_data) {
  if (api == NULL || module_base == NULL || decoy_module == NULL) {
    if (api != NULL) api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("Received NULL value as parameter.\n");
    return FALSE;
  }

  llwythwr_windows_library* decoy = (llwythwr_windows_library *) malloc(sizeof(llwythwr_windows_library));
  if (decoy == NULL) {
    DPRINT("Could not allocate memory.\n");
    SetLastError(ERROR_OUTOFMEMORY);
    goto cleanup;
  }

  llwythwr_windows_library_init(decoy);

  /*
#define LDR_IS_DATAFILE(handle)      (((ULONG_PTR)(handle)) &  (ULONG_PTR)1)
#define LDR_IS_IMAGEMAPPING(handle)  (((ULONG_PTR)(handle)) & (ULONG_PTR)2)
#define LDR_IS_RESOURCE(handle)      (LDR_IS_IMAGEMAPPING(handle) || LDR_IS_DATAFILE(handle))

  void* decoy_module = decoy_handle;
  if (LDR_IS_RESOURCE(decoy_handle))
    decoy_module = (PVOID)((DWORD_PTR)decoy_handle & ~0xFFFF);
*/
  PIMAGE_DOS_HEADER dos_decoy = (PIMAGE_DOS_HEADER) decoy_module;
  PIMAGE_NT_HEADERS nt_decoy = RVA2VA(PIMAGE_NT_HEADERS, decoy_module, dos_decoy->e_lfanew);

  decoy->base_address = decoy_module;
  decoy->size_of_image = nt_decoy->OptionalHeader.SizeOfImage;
  decoy->size_of_headers = nt_decoy->OptionalHeader.SizeOfHeaders;
  decoy->isDLL = (nt_decoy->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
  decoy->entry_point = RVA2VA(LPVOID, decoy_module, nt_decoy->OptionalHeader.AddressOfEntryPoint);
  decoy->api = api;
  decoy->type = Image;

  PIMAGE_NT_HEADERS decoy_headers = NULL;
  unsigned long decoy_size_of_headers = decoy->size_of_headers;
  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);

  if (nt->OptionalHeader.SizeOfImage > decoy->size_of_image) {
    DPRINT("Decoy file too small, it cannot be used. %lu > %lu\n",
           nt->OptionalHeader.SizeOfImage,
           decoy->size_of_image);
    api->SetLastError(ERROR_FILE_TOO_LARGE);
    goto cleanup;
  }

  // before doing anything. check compatibility between pe files
  if (nt->FileHeader.Machine != nt_decoy->FileHeader.Machine) {
    DPRINT("Decoy process %08hx and file %08hx are not compatible...cannot load.\n",
           nt_decoy->FileHeader.Machine, nt->FileHeader.Machine);
    return NULL;
  }

  BOOL isDLL = (nt->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
  if (isDLL)
    DPRINT("PE is a DLL.\n");
  else
    DPRINT("PE is an EXE.\n");

  decoy->isDLL = isDLL;

  DWORD old_protect;
  if(!api->VirtualProtect(decoy->base_address, decoy->size_of_image, PAGE_READWRITE, &old_protect)) {
    DPRINT("VirtualProtect failed: %lu\n", api->GetLastError());
    goto cleanup;
  }

  if (keep_decoy_headers) {
    DPRINT("Making copy of decoy module's headers for later use.\n");
    decoy_headers = api->LocalAlloc(LPTR, decoy->size_of_headers);
    llwythwr_memcpy(decoy_headers, decoy->base_address, decoy->size_of_headers);
  }

  DPRINT("Wiping section view before mapping to it.\n");
  llwythwr_memset(decoy->base_address, 0, decoy->size_of_image);

  llwythwr_windows_library *decoy_wired = llwythwr_wire_library(decoy,
                                                                module_base,
                                                                x_get_lib_address,
                                                                x_get_proc_address,
                                                                x_free_library_function,
                                                                user_data);

  if (decoy_wired == NULL)
    goto cleanup;

  decoy = decoy_wired;

  if (keep_decoy_headers) {
    DPRINT("Overwriting PE headers with the ones from the decoy module.\n");

    // change perms of headers to readwrite
    size_t size = nt->OptionalHeader.SizeOfHeaders;
    DWORD oldprot = 0;
    DWORD newprot = PAGE_READWRITE;
    if (decoy->type == Mapped)
      newprot = PAGE_WRITECOPY;
    if(!api->VirtualProtect(decoy->base_address, size, newprot, &oldprot)) {
      DPRINT("VirtualProtect failed: %lu\n", api->GetLastError());
      goto cleanup;
    }

    // Write headers
    if (decoy_size_of_headers > nt->OptionalHeader.SizeOfHeaders) {
      DPRINT("Decoy PE headers are bigger! Only coping %lu out of %lu bytes.\n", nt->OptionalHeader.SizeOfHeaders, decoy_size_of_headers);
      llwythwr_memcpy(decoy->base_address, decoy_headers, nt->OptionalHeader.SizeOfHeaders);
    }
    else {
      llwythwr_memset(decoy->base_address, 0, nt->OptionalHeader.SizeOfHeaders);
      llwythwr_memcpy(decoy->base_address, decoy_headers, decoy_size_of_headers);
    }

    // change perms of empty headers back to readonly
    if(!api->VirtualProtect(decoy->base_address, size, PAGE_READONLY, &oldprot)) {
      DPRINT("VirtualProtect failed: %lu\n", api->GetLastError());
      goto cleanup;
    }

    api->LocalFree(decoy_headers);
  }

  return decoy;

  cleanup:
  if (decoy_headers != NULL)
    api->LocalFree(decoy_headers);

  llwythwr_unload_library(decoy);

  return NULL;
}

llwythwr_windows_library* llwythwr_map_hollow_library(llwythwr_windows_api *api,
                                                      const void *module_base,
                                                      const char *decoy_filepath,
                                                      BOOL keep_decoy_headers,
                                                      llwythwr_get_lib_address_function x_get_lib_address,
                                                      llwythwr_get_proc_address_function x_get_proc_address,
                                                      llwythwr_free_library_function x_free_library_function,
                                                      void *user_data) {
  if (api == NULL || module_base == NULL || decoy_filepath == NULL) {
    if (api != NULL) api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("Received NULL value as parameter.\n");
    return FALSE;
  }

  llwythwr_windows_library *decoy = llwythwr_map_library_from_disk_to_section(api, decoy_filepath);
  if (decoy == NULL)
    return NULL;

  PIMAGE_NT_HEADERS decoy_headers = NULL;
  unsigned long decoy_size_of_headers = decoy->size_of_headers;
  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);

  if (nt->OptionalHeader.SizeOfImage > decoy->size_of_image) {
    DPRINT("Decoy file too small, it cannot be used. %lu > %lu\n",
           nt->OptionalHeader.SizeOfImage,
           decoy->size_of_image);
    api->SetLastError(ERROR_FILE_TOO_LARGE);
    goto cleanup;
  }

  // before doing anything. check compatibility between exe/dll and host process.
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LDR_DATA_TABLE_ENTRY1 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY1, InInitializationOrderLinks);
  PIMAGE_DOS_HEADER doshost = (PIMAGE_DOS_HEADER) ntdll->DllBase;
  PIMAGE_NT_HEADERS nthost = RVA2VA(PIMAGE_NT_HEADERS, ntdll->DllBase, doshost->e_lfanew);

  if (nt->FileHeader.Machine != nthost->FileHeader.Machine) {
    DPRINT("Host process %08hx and file %08hx are not compatible...cannot load.\n",
           nthost->FileHeader.Machine, nt->FileHeader.Machine);
    return NULL;
  }

  BOOL isDLL = (nt->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
  if (isDLL)
    DPRINT("PE is a DLL.\n");
  else
    DPRINT("PE is an EXE.\n");

  decoy->isDLL = isDLL;

  DWORD old_protect;
  if(!api->VirtualProtect(decoy->base_address, decoy->size_of_image, PAGE_READWRITE, &old_protect)) {
    DPRINT("VirtualProtect failed: %lu\n", api->GetLastError());
    goto cleanup;
  }

  if (keep_decoy_headers) {
    DPRINT("Making copy of decoy module's headers for later use.\n");
    decoy_headers = api->LocalAlloc(LPTR, decoy->size_of_headers);
    llwythwr_memcpy(decoy_headers, decoy->base_address, decoy->size_of_headers);
  }

  DPRINT("Wiping section view before mapping to it.\n");
  llwythwr_memset(decoy->base_address, 0, decoy->size_of_image);

  llwythwr_windows_library *decoy_wired = llwythwr_wire_library(decoy,
                                                                module_base,
                                                                x_get_lib_address,
                                                                x_get_proc_address,
                                                                x_free_library_function,
                                                                user_data);

  if (decoy_wired == NULL)
    goto cleanup;

  decoy = decoy_wired;

  if (keep_decoy_headers) {
    DPRINT("Overwriting PE headers with the ones from the decoy module.\n");

    // change perms of headers to readwrite
    size_t size = nt->OptionalHeader.SizeOfHeaders;
    DWORD oldprot = 0;
    DWORD newprot = PAGE_READWRITE;
    if (decoy->type == Mapped)
      newprot = PAGE_WRITECOPY;
    if(!api->VirtualProtect(decoy->base_address, size, newprot, &oldprot)) {
      DPRINT("VirtualProtect failed: %lu\n", api->GetLastError());
      goto cleanup;
    }

    // Write headers
    if (decoy_size_of_headers > nt->OptionalHeader.SizeOfHeaders) {
      DPRINT("Decoy PE headers are bigger! Only coping %lu out of %lu bytes.\n", nt->OptionalHeader.SizeOfHeaders, decoy_size_of_headers);
      llwythwr_memcpy(decoy->base_address, decoy_headers, nt->OptionalHeader.SizeOfHeaders);
    }
    else {
      llwythwr_memset(decoy->base_address, 0, nt->OptionalHeader.SizeOfHeaders);
      llwythwr_memcpy(decoy->base_address, decoy_headers, decoy_size_of_headers);
    }

    // change perms of empty headers back to readonly
    if(!api->VirtualProtect(decoy->base_address, size, PAGE_READONLY, &oldprot)) {
      DPRINT("VirtualProtect failed: %lu\n", api->GetLastError());
      goto cleanup;
    }

    api->LocalFree(decoy_headers);
  }

  return decoy;

cleanup:
  if (decoy_headers != NULL)
    api->LocalFree(decoy_headers);

  llwythwr_unload_library(decoy);

  return NULL;
}

int llwythwr_link_library(llwythwr_windows_library* library, const char* basename, const char* fullpath) {
  if (library == NULL || basename == NULL || fullpath == NULL) {
    if (library != NULL) library->api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("Received NULL value as parameter.\n");
    return FALSE;
  }

  if (library->base_name != NULL || library->full_path != NULL || library->ldr_data_table_entry != NULL) {
    library->api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("Library already linked.\n");
    return FALSE;
  }

  size_t size = llwythwr_strlen(basename);
  WCHAR* wbase_name = (WCHAR*) library->api->LocalAlloc(LPTR, (size + 1) * sizeof(WCHAR));
  library->api->MultiByteToWideChar(CP_ACP, 0, basename, -1, wbase_name, 510);

  size = llwythwr_strlen(fullpath);
  WCHAR* wfull_path = (WCHAR*) library->api->LocalAlloc(LPTR, (size + 1) * sizeof(WCHAR));
  library->api->MultiByteToWideChar(CP_ACP, 0, fullpath, -1, wfull_path, 510);

  LDR_DATA_TABLE_ENTRY1* entry = llwythwr_link_library_win8plus(library->api, library->base_address, wbase_name, wfull_path);

  if (entry == NULL) {
    DPRINT("Could not link library with name %S.\n", library->base_name);
    library->api->LocalFree(wbase_name);
    library->api->LocalFree(wfull_path);
    return FALSE;
  }

  library->ldr_data_table_entry = entry;
  library->base_name = wbase_name;
  library->full_path = wfull_path;

  DPRINT("Linked library with name %S.\n", library->base_name);
  return TRUE;
}

int llwythwr_unlink_library(llwythwr_windows_library* library) {
  if (library == NULL) {
    DPRINT("Received NULL value as parameter.\n");
    return FALSE;
  }

  if (library->base_name != NULL) {
    library->api->LocalFree(library->base_name);
    library->base_name = NULL;
  }

  if (library->full_path != NULL) {
    library->api->LocalFree(library->full_path);
    library->full_path = NULL;
  }

  if (library->ldr_data_table_entry != NULL) {
    DPRINT("Unlinking library.\n");
    llwythwr_unlink_library_win8plus(library->api, library->ldr_data_table_entry);
    llwythwr_ldr_data_table_entry_free(library->api, library->ldr_data_table_entry);
    library->ldr_data_table_entry = NULL;
  }
  else
    DPRINT("Library does not appear to be linked.\n");

  return TRUE;
}

void* llwythwr_get_export_address_by_name(llwythwr_windows_library *library, const char* export_name) {
  if (library == NULL || export_name == NULL) {
    if (library != NULL) library->api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("Received NULL value as parameter.\n");
    return FALSE;
  }

  PIMAGE_EXPORT_DIRECTORY exp = library->export_directory;
  DWORD cnt = exp->NumberOfNames;

  // if no api names, return NULL
  if (cnt == 0) return NULL;

  PDWORD adr = RVA2VA(PDWORD, library->base_address, exp->AddressOfFunctions);
  PDWORD sym = RVA2VA(PDWORD, library->base_address, exp->AddressOfNames);
  PWORD ord = RVA2VA(PWORD, library->base_address, exp->AddressOfNameOrdinals);
  PCHAR dll = RVA2VA(PCHAR, library->base_address, exp->Name);

  LPVOID addr = NULL;

  if (HIWORD(export_name) == 0) {
    // load function by ordinal value
    if (LOWORD(export_name) >= exp->Base) {
      DWORD idx = LOWORD(export_name) - exp->Base;
      //addr = (FARPROC)(LPVOID)(handle + (*(DWORD *) (handle + exp->AddressOfFunctions + (idx * 4))));
      //addr = RVA2VA(LPVOID, handle, (PDWORD) adr + (idx * 4));
      addr = RVA2VA(PVOID, library->base_address, adr[idx]);
    }
  }
  else {
    do {
      // search function name in list of exported names
      PCHAR api = RVA2VA(PCHAR, library->base_address, sym[cnt - 1]);

      if (llwythwr_strcmp(export_name, api) == 0) {
        addr = RVA2VA(LPVOID, library->base_address, adr[ord[cnt - 1]]);
      }
    } while (--cnt && addr == NULL);
  }

  // is this a forward reference?
  if (addr != NULL && ((PBYTE) addr >= (PBYTE) exp && (PBYTE) addr < (PBYTE) exp + library->export_directory_size)) {
    DPRINT("Found a forward export....FIXME!");
  }

  return addr;
}

void* llwythwr_get_export_address_by_hash(llwythwr_windows_library *library, unsigned long function_hash, unsigned long key, llwythwr_hash_function hash_function) {
  if (library == NULL || hash_function == NULL) {
    if (library != NULL) library->api->SetLastError(ERROR_INVALID_PARAMETER);
    DPRINT("Received NULL value as parameter.\n");
    return FALSE;
  }

  PIMAGE_EXPORT_DIRECTORY exp = library->export_directory;
  DWORD cnt = exp->NumberOfNames;

  // if no api names, return NULL
  if (cnt == 0) return NULL;

  PDWORD adr = RVA2VA(PDWORD, library->base_address, exp->AddressOfFunctions);
  PDWORD sym = RVA2VA(PDWORD, library->base_address, exp->AddressOfNames);
  PWORD ord = RVA2VA(PWORD, library->base_address, exp->AddressOfNameOrdinals);
  PCHAR dll = RVA2VA(PCHAR, library->base_address, exp->Name);

  LPVOID addr = NULL;

  do {
    PCHAR api = RVA2VA(PCHAR, library->base_address, sym[cnt - 1]);
    if (hash_function(api, key, library->userdata) == function_hash) {
      addr = RVA2VA(LPVOID, library->base_address, adr[ord[cnt - 1]]);
    }
  } while (--cnt && addr == NULL);

  // is this a forward reference?
  if (addr != NULL && ((PBYTE) addr >= (PBYTE) exp && (PBYTE) addr < (PBYTE) exp + library->export_directory_size)) {
    DPRINT("Found a forward export....FIXME!");
  }

  return addr;
}


/*
 * Utility functions
 */

static void *llwythwr_memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size) {
  unsigned char *dst = (unsigned char *) dstptr;
  const unsigned char *src = (const unsigned char *) srcptr;
  for (size_t i = 0; i < size; i++)
    dst[i] = src[i];
  return dstptr;
}

static void *llwythwr_memset(void *ptr, int value, size_t num) {
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

static int llwythwr_strcmp(const char *s1, const char *s2) {
  while (*s1 == *s2++) {
    if (*s1++ == '\0') return (0);
  }
  return (*(const unsigned char *) s1 - *(const unsigned char *) (s2 - 1));
}

static size_t llwythwr_strlen(const char *str) {
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

static size_t llwythwr_wcslen(const wchar_t *s) {
  const wchar_t *p;
  p = s;
  while (*p)
    p++;
  return p - s;
}

/*
static int llwythwr_wcscmp(const wchar_t *s1, const wchar_t *s2) {
  while (*s1 == *s2++)
    if (*s1++ == '\0')
      return (0);
  // XXX assumes wchar_t = int
  return (*(const unsigned int *)s1 - *(const unsigned int *)--s2);
}
*/

static char *llwythwr_strcat(char *s, char *append) {
  char *save = s;
  for (; *s; ++s);
  while ((*s++ = *append++) != 0);
  return (save);
}

static int llwythwr_memcmp(const void *s1, const void *s2, size_t n) {
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
static int llwythwr_strcasecmp(const char *s1, const char *s2) {
  const u_char *us1 = (const u_char *) s1, *us2 = (const u_char *) s2;
  while (llwythwr_tolower(*us1) == llwythwr_tolower(*us2)) {
    if (*us1++ == '\0') return (0);
    us2++;
  }
  return (llwythwr_tolower(*us1) - llwythwr_tolower(*us2));
}
*/

/*
 * Internal functions
 */

static llwythwr_windows_library *llwythwr_wire_library(llwythwr_windows_library *mapped_module,
                                                       const void *module_base,
                                                       llwythwr_get_lib_address_function x_get_lib_address,
                                                       llwythwr_get_proc_address_function x_get_proc_address,
                                                       llwythwr_free_library_function x_free_library_function,
                                                       void *user_data) {

  llwythwr_windows_api* api = mapped_module->api;

  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);

  DPRINT("Binary size: %lu\n", nt->OptionalHeader.SizeOfImage);

  // check if the binary has relocation information
  DWORD size = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
  BOOL has_reloc = size == 0 ? FALSE : TRUE;
  DPRINT("Binary has relocation information: %d\n", has_reloc);

  PVOID cs = mapped_module->base_address;

  PIMAGE_NT_HEADERS ntnew = RVA2VA(PIMAGE_NT_HEADERS, cs, dos->e_lfanew);

  DPRINT("Mapped to address: %p\n", cs);

  if (cs == NULL) return NULL;

  DPRINT("Copying Headers\n");
  DPRINT("nt->FileHeader.SizeOfOptionalHeader: %d\n", nt->FileHeader.SizeOfOptionalHeader);
  DPRINT("nt->OptionalHeader.SizeOfHeaders: %lu\n", nt->OptionalHeader.SizeOfHeaders);

  DPRINT("Copying first section\n");
  DPRINT("Copying %lu bytes\n", nt->OptionalHeader.SizeOfHeaders);
  llwythwr_memcpy(cs, module_base, nt->OptionalHeader.SizeOfHeaders);

  DPRINT("DOS Signature (Magic): %08hx, %p\n", ((PIMAGE_DOS_HEADER) cs)->e_magic, &(((PIMAGE_DOS_HEADER) cs)->e_magic));
  DPRINT("NT Signature: %lx, %p\n", ntnew->Signature, &(ntnew->Signature));

  DPRINT("Updating ImageBase to final base address\n");
  ntnew->OptionalHeader.ImageBase = (ULONGLONG) cs;
  DPRINT("Updated ImageBase is %p\n", (PVOID) ntnew->OptionalHeader.ImageBase);

  DPRINT("Copying each section to memory: %p\n", cs);
  PIMAGE_SECTION_HEADER sh = IMAGE_FIRST_SECTION(ntnew);

  DWORD i;
  for (i = 0; i < ntnew->FileHeader.NumberOfSections; i++) {
    PBYTE dest = (PBYTE) cs + sh[i].VirtualAddress;
    PBYTE source = (PBYTE) module_base + sh[i].PointerToRawData;

    if (sh[i].SizeOfRawData == 0)
      DPRINT("Section is empty of data, but may contain uninitialized data.\n");

    // Copy the section data
    llwythwr_memcpy(dest, source, sh[i].SizeOfRawData);

    // Update the actual address of the section
    sh[i].Misc.PhysicalAddress = (DWORD) *dest;

    DPRINT("Copied section name: %s\n", sh[i].Name);
    DPRINT("Copied section source offset: 0x%lX\n", sh[i].VirtualAddress);
    DPRINT("Copied section dest offset: 0x%lX\n", sh[i].PointerToRawData);
    DPRINT("Copied section absolute address: 0x%lX\n", sh[i].Misc.PhysicalAddress);
    //DPRINT("Copied section size: 0x%lX\n", sh[i].SizeOfRawData);
    DPRINT("Copied section size: %lu\n", sh[i].SizeOfRawData);
  }

  DPRINT("Sections copied.\n");

  PBYTE ofs = (PBYTE) cs - nt->OptionalHeader.ImageBase;
  DPRINT("Image Relocation Offset: 0x%p\n", ofs);

  DWORD rva;
  PIMAGE_BASE_RELOCATION ibr;
  PIMAGE_RELOC list;
  if (has_reloc && ofs != 0) {
    DPRINT("Applying Relocations\n");

    rva = ntnew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
    ibr = RVA2VA(PIMAGE_BASE_RELOCATION, cs, rva);

    while ((PBYTE) ibr < ((PBYTE) cs + rva + size) && ibr->SizeOfBlock != 0) {
      list = (PIMAGE_RELOC) (ibr + 1);

      while ((PBYTE) list != (PBYTE) ibr + ibr->SizeOfBlock) {
        // check that the RVA is within the boundaries of the PE
        if (ibr->VirtualAddress + list->offset < ntnew->OptionalHeader.SizeOfImage) {
          PULONG_PTR address = (PULONG_PTR) ((PBYTE) cs + ibr->VirtualAddress + list->offset);
          if (list->type == IMAGE_REL_BASED_DIR64) {
            *address += (ULONG_PTR) ofs;
          } else if (list->type == IMAGE_REL_BASED_HIGHLOW) {
            *address += (DWORD) (ULONG_PTR) ofs;
          } else if (list->type == IMAGE_REL_BASED_HIGH) {
            *address += HIWORD(ofs);
          } else if (list->type == IMAGE_REL_BASED_LOW) {
            *address += LOWORD(ofs);
          } else if (list->type != IMAGE_REL_BASED_ABSOLUTE) {
            DPRINT("ERROR: Unrecognized Relocation type %08hx.\n", list->type);
            api->SetLastError(ERROR_INVALID_DATA);
            return NULL;
          }
        }
        list++;
      }
      ibr = (PIMAGE_BASE_RELOCATION) list;
    }
  }

  rva = ntnew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

  LPVOID *dependences = NULL;
  int number_of_dependences = 0;
  if (rva != 0) {
    DPRINT("Processing the Import Table\n");

    PIMAGE_IMPORT_DESCRIPTOR imp = RVA2VA(PIMAGE_IMPORT_DESCRIPTOR, cs, rva);

    // For each DLL
    for (; imp->Name != 0; imp++) {
      PCHAR name = RVA2VA(PCHAR, cs, imp->Name);

      LPVOID dll = x_get_lib_address(name, user_data);

      if (dll == NULL) {
        DPRINT("Dependent module %s could not be found\n", name);
        api->SetLastError(ERROR_MOD_NOT_FOUND);
        return NULL;
      }

      DPRINT("Loaded dependent module %s\n", name);

      if (dependences == NULL)
        dependences = (LPVOID *) api->LocalAlloc(LPTR, sizeof(LPVOID));
      else
        dependences =
            (LPVOID *) api->LocalReAlloc(dependences, (number_of_dependences + 1) * sizeof(LPVOID), LMEM_MOVEABLE | LMEM_ZEROINIT);

      if (dependences == NULL) {
        DPRINT("Could not allocate memory for modules list\n");
        api->SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
      }

      llwythwr_dependency_library* dependency = (llwythwr_dependency_library*) api->LocalAlloc(LPTR, sizeof(llwythwr_dependency_library));
      dependency->name = name;
      dependency->handle = dll;
      dependences[number_of_dependences++] = dependency;

      // Resolve the API for this library
      PIMAGE_THUNK_DATA oft = RVA2VA(PIMAGE_THUNK_DATA, cs, imp->OriginalFirstThunk);
      PIMAGE_THUNK_DATA ft = RVA2VA(PIMAGE_THUNK_DATA, cs, imp->FirstThunk);

      // For each API
      for (;; oft++, ft++) {
        // No API left?
        if (oft->u1.AddressOfData == 0) break;

        ULONG_PTR func_ref;
        if (IMAGE_SNAP_BY_ORDINAL(oft->u1.Ordinal)) {
          // Resolve by ordinal
          func_ref = (ULONG_PTR) x_get_proc_address(dependency->name, dependency->handle, NULL, oft->u1.Ordinal, user_data);
        } else {
          // Resolve by name
          PIMAGE_IMPORT_BY_NAME ibn = RVA2VA(PIMAGE_IMPORT_BY_NAME, cs, oft->u1.AddressOfData);
          func_ref = (ULONG_PTR) x_get_proc_address(dependency->name, dependency->handle, ibn->Name, 0, user_data);
        }

        if (!func_ref) {
          DPRINT("Function could not be found in module %s\n", name);
          api->SetLastError(ERROR_PROC_NOT_FOUND);
          break;
        }

        ft->u1.Function = func_ref;
      }
    }
  }

  rva = ntnew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress;

  if (rva != 0) {
    DPRINT("Processing Delayed Import Table");

    PIMAGE_DELAYLOAD_DESCRIPTOR del = RVA2VA(PIMAGE_DELAYLOAD_DESCRIPTOR, cs, rva);

    // For each DLL
    for (; del->DllNameRVA != 0; del++) {
      PCHAR name = RVA2VA(PCHAR, cs, del->DllNameRVA);

      LPVOID dll = x_get_lib_address(name, user_data);

      //if(dll == NULL) continue;
      if (dll == NULL) {
        DPRINT("Dependent module %s could not be loaded\n", name);
        api->SetLastError(ERROR_MOD_NOT_FOUND);
        return NULL;
      }

      DPRINT("Loaded dependent module %s\n", name);

      if (dependences == NULL)
        dependences = (LPVOID *) api->LocalAlloc(LPTR, sizeof(LPVOID));
      else
        dependences =
            (LPVOID *) api->LocalReAlloc(dependences, (number_of_dependences + 1) * sizeof(LPVOID), LMEM_MOVEABLE | LMEM_ZEROINIT);

      if (dependences == NULL) {
        DPRINT("Could not allocate memory for modules list\n");
        api->SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
      }

      llwythwr_dependency_library* dependency = (llwythwr_dependency_library*) api->LocalAlloc(LPTR, sizeof(llwythwr_dependency_library));
      dependency->name = name;
      dependency->handle = dll;
      dependences[number_of_dependences++] = dependency;

      // Resolve the API for this library
      PIMAGE_THUNK_DATA oft = RVA2VA(PIMAGE_THUNK_DATA, cs, del->ImportNameTableRVA);
      PIMAGE_THUNK_DATA ft = RVA2VA(PIMAGE_THUNK_DATA, cs, del->ImportAddressTableRVA);

      // For each API
      for (;; oft++, ft++) {
        // No API left?
        if (oft->u1.AddressOfData == 0) break;

        ULONG_PTR func_ref;
        if (IMAGE_SNAP_BY_ORDINAL(oft->u1.Ordinal)) {
          // Resolve by ordinal
          func_ref = (ULONG_PTR) x_get_proc_address(dependency->name, dependency->handle, NULL, oft->u1.Ordinal, user_data);
        }
        else {
          // Resolve by name
          PIMAGE_IMPORT_BY_NAME ibn = RVA2VA(PIMAGE_IMPORT_BY_NAME, cs, oft->u1.AddressOfData);
          func_ref = (ULONG_PTR) x_get_proc_address(dependency->name, dependency->handle, ibn->Name, 0, user_data);
        }

        if (!func_ref) {
          DPRINT("Function could not be found in module %s\n", name);
          api->SetLastError(ERROR_PROC_NOT_FOUND);
          break;
        }

        ft->u1.Function = func_ref;
      }
    }
  }

  DWORD size_of_img = ntnew->OptionalHeader.SizeOfImage;
  LPVOID entry_point = NULL;
  if (ntnew->OptionalHeader.AddressOfEntryPoint != 0)
    entry_point = RVA2VA(LPVOID, cs, ntnew->OptionalHeader.AddressOfEntryPoint);

  if (mapped_module->type == Mapped) {
    DPRINT("Unmapping temporary local view of section to persist changes.\n");
    NTSTATUS status = api->NtUnmapViewOfSection((HANDLE) -1, cs);
    DPRINT("NTSTATUS: 0x%08lx\n", status);
    if (status != 0) return NULL;

    // if no reloc information is present, make sure we use the preferred address
    if (!has_reloc) {
      DPRINT("No relocation information present, so using preferred address...\n");
      cs = NULL;
    }

    SIZE_T viewSize = 0;
    DPRINT("Mapping writecopy local view of section to execute PE.\n");
    status = api->NtMapViewOfSection(mapped_module->section,
                                     (HANDLE) -1,
                                     &cs,
                                     0,
                                     0,
                                     0,
                                     &viewSize,
                                     ViewUnmap,
                                     0,
                                     PAGE_EXECUTE_WRITECOPY);
    DPRINT("View size: %lld\n", viewSize);
    DPRINT("NTSTATUS: 0x%08lx\n", status);
    if (status != 0)
      return NULL;

    DPRINT("Mapped to address: %p\n", cs);
  }

  DWORD oldprot;
  if (mapped_module->type != Private) {
    // start everything out as WC as some sections are padded and we'll end up
    // with extra RWX memory if you don't pre-mark the padding as WC
    DPRINT("Pre-marking module as WC to avoid padding between PE sections staying RWX.\n");
    if (!api->VirtualProtect(cs, mapped_module->size_of_image, PAGE_WRITECOPY, &oldprot)) {
      DPRINT("VirtualProtect failed: %lu\n", api->GetLastError());
      return NULL;
    }
  }

  DPRINT("Setting permissions for each PE section\n");
  // done with binary manipulation, mark section permissions appropriately
  for (i = 0; i < nt->FileHeader.NumberOfSections; i++) {
    DWORD newprot = 0;
    PVOID baseAddress;
    SIZE_T numBytes;

    BOOL isRead = (sh[i].Characteristics & IMAGE_SCN_MEM_READ) ? TRUE : FALSE;
    BOOL isWrite = (sh[i].Characteristics & IMAGE_SCN_MEM_WRITE) ? TRUE : FALSE;
    BOOL isExecute = (sh[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) ? TRUE : FALSE;

    if (isWrite & isExecute) {
      if (mapped_module->type == Private)
        newprot = PAGE_EXECUTE_READWRITE;
      else
        continue; // do nothing, already WCX
    }
    else if (isRead & isExecute)
      newprot = PAGE_EXECUTE_READ;
    else if (isRead & isWrite & !isExecute) {
      if (mapped_module->type == Mapped)
        newprot = PAGE_WRITECOPY; // must use WC because RW is incompatible with permissions of initial view (WCX)
      else
        newprot = PAGE_READWRITE;
    }
    else if (!isRead & !isWrite & isExecute)
      newprot = PAGE_EXECUTE;
    else if (isRead & !isWrite & !isExecute)
      newprot = PAGE_READONLY;

    baseAddress = (PBYTE) cs + sh[i].VirtualAddress;

    if (i < (nt->FileHeader.NumberOfSections - 1))
      numBytes = ((PBYTE) cs + sh[i + 1].VirtualAddress) - ((PBYTE) cs + sh[i].VirtualAddress);
    else
      numBytes = sh[i].SizeOfRawData;

    oldprot = 0;

    DPRINT("Section name: %s\n", sh[i].Name);
    DPRINT("Section offset: 0x%lX\n", sh[i].VirtualAddress);
    DPRINT("Section absolute address: 0x%p\n", baseAddress);
    DPRINT("Section size: 0x%llX\n", numBytes);
    DPRINT("Section protections: 0x%lX\n", newprot);

    if (!(api->VirtualProtect(baseAddress, numBytes, newprot, &oldprot))) {
      DPRINT("VirtualProtect failed: %lu\n", api->GetLastError());
      return NULL;
    }
  }

  // declare variables and set permissions of module header
  DPRINT("Setting permissions of module headers to READONLY (%lu bytes)\n", nt->OptionalHeader.BaseOfCode);
  oldprot = 0;

  if(!api->VirtualProtect(cs, nt->OptionalHeader.BaseOfCode, PAGE_READONLY, &oldprot)) {
    DPRINT("VirtualProtect failed: %lu\n", api->GetLastError());
    return NULL;
  }

#ifdef _WIN64
  PIMAGE_OPTIONAL_HEADER pOptHeader = (PIMAGE_OPTIONAL_HEADER) &nt->OptionalHeader;
  if (pOptHeader->Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC) {
    rva = ntnew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;

    if (rva != 0) {
      DPRINT("Registering Exception Handling\n");
      PRUNTIME_FUNCTION pFunctionTable = RVA2VA(PRUNTIME_FUNCTION, cs, rva);
      DWORD sizeFunctionTable = (pOptHeader->DataDirectory[3].Size / (DWORD) sizeof(RUNTIME_FUNCTION));
      if (!api->RtlAddFunctionTable(pFunctionTable, sizeFunctionTable, (DWORD64) cs)) {
        DPRINT("RtlAddFunctionTable failed\n");
      }
    }
  }
#else
  // TODO, see https://github.com/strivexjun/MemoryModulePP/blob/master/MemoryModulePP.c#L607
#endif

  /**
    Execute TLS callbacks. These are only called when the process starts, not when a thread begins, ends
    or when the process ends. TLS is not fully supported.
  */
  rva = ntnew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
  if (rva != 0) {
    DPRINT("Processing TLS directory\n");

    PIMAGE_TLS_DIRECTORY tls = RVA2VA(PIMAGE_TLS_DIRECTORY, cs, rva);

    // address of callbacks is absolute. requires relocation information
    PIMAGE_TLS_CALLBACK *callbacks = (PIMAGE_TLS_CALLBACK *) tls->AddressOfCallBacks;
    DPRINT("AddressOfCallBacks: %p\n", callbacks);

    // DebugBreak();

    if (callbacks) {
      while (*callbacks != NULL) {
        // call function
        DPRINT("Calling %p\n", *callbacks);
        (*callbacks)((LPVOID) cs, DLL_PROCESS_ATTACH, NULL);
        callbacks++;
      }
    }
  }

  mapped_module->base_address = cs;
  mapped_module->isDLL = (nt->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
  mapped_module->entry_point = entry_point;
  mapped_module->entry_point_called = FALSE;
  mapped_module->dependences = dependences;
  mapped_module->number_of_dependences = number_of_dependences;
  mapped_module->size_of_image = nt->OptionalHeader.SizeOfImage;
  mapped_module->size_of_image = nt->OptionalHeader.SizeOfHeaders;
  mapped_module->free_library_function = x_free_library_function;
  mapped_module->userdata = user_data;

  PIMAGE_DATA_DIRECTORY dir = (PIMAGE_DATA_DIRECTORY) ntnew->OptionalHeader.DataDirectory;
  rva = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  if (rva != 0) {
    mapped_module->export_directory = RVA2VA(PIMAGE_EXPORT_DIRECTORY, cs, rva);
    mapped_module->export_directory_size = dir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
  }
  else {
    mapped_module->export_directory = NULL;
    mapped_module->export_directory_size = 0;
  }

  return mapped_module;
}

static void llwythwr_windows_library_init(llwythwr_windows_library *library) {
  library->base_address = NULL;
  library->section = NULL;
  library->isDLL = TRUE;
  library->entry_point = NULL;
  library->entry_point_called = FALSE;
  library->dependences = NULL;
  library->number_of_dependences = 0;
  library->size_of_image = 0;
  library->size_of_headers = 0;
  library->type = Mapped;
  library->userdata = NULL;
  library->free_library_function = NULL;
  library->api = NULL;
  library->base_name = NULL;
  library->full_path = NULL;
  library->ldr_data_table_entry = NULL;
  library->export_directory = NULL;
  library->export_directory_size = 0;
#ifdef _WIN64
  library->blocked_memory = NULL;
#endif
}

static LDR_DATA_TABLE_ENTRY1* llwythwr_create_ldr_entry(llwythwr_windows_api *api, const void* module_base, const wchar_t* dll_name, const wchar_t* dll_path) {
  PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER) module_base;
  PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, module_base, dos->e_lfanew);

  UNICODE_STRING unicode_name = {0};
  llwythwr_init_unicode_string(&unicode_name, dll_name);

  UNICODE_STRING unicode_path = {0};
  llwythwr_init_unicode_string(&unicode_path, dll_path);

  LDR_DATA_TABLE_ENTRY1* entry = (LDR_DATA_TABLE_ENTRY1*) api->LocalAlloc(LPTR, sizeof(LDR_DATA_TABLE_ENTRY1));

  if (entry == NULL) {
    DPRINT("Could not allocate memory for entry.\n");
    api->SetLastError(ERROR_OUTOFMEMORY);
    return NULL;
  }

  entry->DdagNode = (LDR_DDAG_NODE1*) api->LocalAlloc(LPTR, sizeof(LDR_DDAG_NODE1));

  if (entry->DdagNode == NULL) {
    DPRINT("Could not allocate memory for entry->DdagNode.\n");
    api->SetLastError(ERROR_OUTOFMEMORY);
    api->LocalFree(entry);
    return NULL;
  }

  entry->ReferenceCount        = 1;
  entry->LoadReason            = LoadReasonDynamicLoad;
  entry->OriginalBase          = nt->OptionalHeader.ImageBase;
  entry->EntryPoint            = RVA2VA(LPVOID, module_base, nt->OptionalHeader.AddressOfEntryPoint);
  entry->DllBase               = (PVOID) module_base;
  entry->SizeOfImage           = nt->OptionalHeader.SizeOfImage;
  entry->TimeDateStamp         = nt->FileHeader.TimeDateStamp;
  entry->BaseDllName           = unicode_name;
  entry->FullDllName           = unicode_path;
  entry->ObsoleteLoadCount     = 1;
  entry->Flags                 = LDRP_IMAGE_DLL | LDRP_PROTECT_DELAY_LOAD | LDRP_ENTRY_PROCESSED | LDRP_PROCESS_ATTACH_CALLED | LDRP_LOAD_NOTIFICATIONS_SENT | LDRP_IN_LEGACY_LISTS | LDRP_IN_INDEXES | LDRP_PROCESS_STATIC_IMPORT;

  api->NtQuerySystemTime(&entry->LoadTime);

  ULONG hash;
  api->RtlHashUnicodeString(&entry->BaseDllName, TRUE, 0, &hash);
  entry->BaseNameHashValue = hash;

  entry->NodeModuleLink.Flink    = &entry->DdagNode->Modules;
  entry->NodeModuleLink.Blink    = &entry->DdagNode->Modules;
  entry->DdagNode->Modules.Flink = &entry->NodeModuleLink;
  entry->DdagNode->Modules.Blink = &entry->NodeModuleLink;
  entry->DdagNode->State         = LdrModulesReadyToRun;
  entry->DdagNode->LoadCount     = 1;

  return entry;
}

static PVOID llwythwr_find_ldrp_hash_table(llwythwr_windows_api *api)
{
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LDR_DATA_TABLE_ENTRY1 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY1, InInitializationOrderLinks);

  ULONG ntdl_hash_index;
  api->RtlHashUnicodeString(&ntdll->BaseDllName, TRUE, 0, &ntdl_hash_index);
  ntdl_hash_index &= 0x1F;

  ULONG_PTR NtdllBase = (ULONG_PTR) ntdll->DllBase;
  ULONG_PTR NtdllEndAddress = NtdllBase + ntdll->SizeOfImage - 1;

  // scan hash list to the head (head is located within ntdll)
  BOOL bHeadFound = 0;
  PLIST_ENTRY pNtdllHashHead = NULL;

  for (PLIST_ENTRY e = (PLIST_ENTRY) ntdll->HashLinks.Flink; e != (PLIST_ENTRY) &ntdll->HashLinks; e = e->Flink) {
    if ((ULONG_PTR) e >= NtdllBase && (ULONG_PTR)e < NtdllEndAddress) {
      bHeadFound = 1;
      pNtdllHashHead = e;
      break;
    }
  }

  if (bHeadFound)
    return pNtdllHashHead - ntdl_hash_index;

  return NULL;
}

static void llwythwr_insert_tail_list(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
  PLIST_ENTRY PrevEntry = ListHead->Blink;

  Entry->Flink = ListHead;
  Entry->Blink = PrevEntry;

  PrevEntry->Flink = Entry;
  ListHead->Blink  = Entry;
}

static void llwythwr_unlink_list_entry(PLIST_ENTRY Entry)
{
  Entry->Blink->Flink = Entry->Flink;
  Entry->Flink->Blink = Entry->Blink;
}

static int llwythwr_insert_hash_node(llwythwr_windows_api *api, PLIST_ENTRY pNodeLink, ULONG hash)
{
  if (pNodeLink == NULL)
    return FALSE;

  PVOID LdrpHashTable = llwythwr_find_ldrp_hash_table(api);
  if (LdrpHashTable == NULL) {
    DPRINT("Could not find LdrpHashTable\n");
    return FALSE;
  }
  // LrpHashTable record
  PLIST_ENTRY pHashList = (PLIST_ENTRY) (LdrpHashTable + sizeof(LIST_ENTRY) * (hash & 0x1F));
  llwythwr_insert_tail_list(pHashList, pNodeLink);

  return TRUE;
}

static void* llwythwr_find_ldrp_module_index_base()
{
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LDR_DATA_TABLE_ENTRY1 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY1, InInitializationOrderLinks);

  RTL_BALANCED_NODE1* pNode = &ntdll->BaseAddressIndexNode;

  // Get root node
  RTL_BALANCED_NODE1* lastNode = 0;
  for(; pNode->ParentValue; ) {
    // Ignore last few bits
    lastNode = (RTL_BALANCED_NODE1*) (pNode->ParentValue & (size_t) -8);
    pNode = lastNode;
  }

  if (!pNode->Red) {
    PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, ntdll->DllBase, ((PIMAGE_DOS_HEADER)ntdll->DllBase)->e_lfanew);
    PIMAGE_SECTION_HEADER sh = IMAGE_FIRST_SECTION(nt);

    PVOID begin = 0;
    DWORD length = 0;
    for (INT i = 0; i < nt->FileHeader.NumberOfSections; i++) {
      char dot_data[] = {'.','d','a','t','a','\0'};
      if (llwythwr_strcmp(dot_data, (LPCSTR) sh->Name) == 0) {
        begin = ntdll->DllBase + sh->VirtualAddress;
        length = sh->Misc.VirtualSize;
        break;
      }
      ++sh;
    }

    for (DWORD i = 0; i < length - sizeof(SIZE_T); begin++, i++) {
      if (llwythwr_memcmp((PVOID)begin, (PVOID)&pNode, sizeof(SIZE_T)) == 0) {
        return begin;
      }
    }
  }

  return NULL;
}

static void* llwythwr_find_ldrp_mapping_info_base()
{
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  LDR_DATA_TABLE_ENTRY1 *ntdll = CONTAINING_RECORD(ldr->InInitializationOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY1, InInitializationOrderLinks);

  RTL_BALANCED_NODE1* pNode = &ntdll->MappingInfoIndexNode;

  // Get root node
  RTL_BALANCED_NODE1* lastNode = 0;
  for(; pNode->ParentValue; ) {
    // Ignore last few bits
    lastNode = (RTL_BALANCED_NODE1*) (pNode->ParentValue & (size_t) -8);
    pNode = lastNode;
  }

  if (!pNode->Red) {
    PIMAGE_NT_HEADERS nt = RVA2VA(PIMAGE_NT_HEADERS, ntdll->DllBase, ((PIMAGE_DOS_HEADER)ntdll->DllBase)->e_lfanew);
    PIMAGE_SECTION_HEADER sh = IMAGE_FIRST_SECTION(nt);

    PVOID begin = 0;
    DWORD length = 0;
    for (INT i = 0; i < nt->FileHeader.NumberOfSections; i++) {
      char dot_data[] = {'.','d','a','t','a','\0'};
      if (llwythwr_strcmp(dot_data, (LPCSTR) sh->Name) == 0) {
        begin = ntdll->DllBase + sh->VirtualAddress;
        length = sh->Misc.VirtualSize;
        break;
      }
      ++sh;
    }

    for (DWORD i = 0; i < length - sizeof(SIZE_T); begin++, i++) {
      if (llwythwr_memcmp((PVOID)begin, (PVOID)&pNode, sizeof(SIZE_T)) == 0) {
        return begin;
      }
    }
  }

  return NULL;
}

static int llwythwr_insert_into_ldrp_module_index(llwythwr_windows_api *api, LDR_DATA_TABLE_ENTRY1* entry, const void* module_base)
{
  PVOID LdrpModuleIndexBase = llwythwr_find_ldrp_module_index_base();
  if (LdrpModuleIndexBase == NULL) {
    DPRINT("Could not find LdrpModuleIndexBase\n");
    return FALSE;
  }

  LDR_DATA_TABLE_ENTRY1 *pLdrNode = CONTAINING_RECORD(LdrpModuleIndexBase, LDR_DATA_TABLE_ENTRY1, BaseAddressIndexNode);
  LDR_DATA_TABLE_ENTRY1 LdrNode = *pLdrNode;

  BOOL right = FALSE;
  while (TRUE)
  {
    if (module_base < LdrNode.DllBase) {
      if (LdrNode.BaseAddressIndexNode.Left) {
        pLdrNode = CONTAINING_RECORD( LdrNode.BaseAddressIndexNode.Left, LDR_DATA_TABLE_ENTRY1, BaseAddressIndexNode );
        LdrNode = *pLdrNode;
      }
      else
        break;
    }
    else if (module_base  > LdrNode.DllBase) {
      if (LdrNode.BaseAddressIndexNode.Right) {
        pLdrNode = CONTAINING_RECORD( LdrNode.BaseAddressIndexNode.Right, LDR_DATA_TABLE_ENTRY1, BaseAddressIndexNode );
        LdrNode = *pLdrNode;
      }
      else {
        right = TRUE;
        break;
      }
    }
    else { // Already in tree (increase ref counter)
      // pLdrNode->DdagNode->ReferenceCount++;
      return TRUE;
    }
  }

  api->RtlRbInsertNodeEx((PRTL_RB_TREE) LdrpModuleIndexBase,
                         (PRTL_BALANCED_NODE) &pLdrNode->BaseAddressIndexNode, right,
                         (PRTL_BALANCED_NODE) &entry->BaseAddressIndexNode);

  return TRUE;
}

static int llwythwr_insert_into_ldrp_mapping_info_index(llwythwr_windows_api *api, LDR_DATA_TABLE_ENTRY1* entry, const void* module_base)
{
  PVOID LdrpMappingInfoBase = llwythwr_find_ldrp_mapping_info_base();
  if (LdrpMappingInfoBase == NULL) {
    DPRINT("Could not find LdrpMappingInfoBase\n");
    return FALSE;
  }

  LDR_DATA_TABLE_ENTRY1 *pLdrNode = CONTAINING_RECORD(LdrpMappingInfoBase, LDR_DATA_TABLE_ENTRY1, MappingInfoIndexNode);
  LDR_DATA_TABLE_ENTRY1 LdrNode = *pLdrNode;

  BOOL right = FALSE;
  while (TRUE)
  {
    if (module_base < LdrNode.DllBase) {
      if (LdrNode.MappingInfoIndexNode.Left) {
        pLdrNode = CONTAINING_RECORD( LdrNode.MappingInfoIndexNode.Left, LDR_DATA_TABLE_ENTRY1, MappingInfoIndexNode );
        LdrNode = *pLdrNode;
      }
      else
        break;
    }
    else if (module_base  > LdrNode.DllBase) {
      if (LdrNode.MappingInfoIndexNode.Right) {
        pLdrNode = CONTAINING_RECORD( LdrNode.MappingInfoIndexNode.Right, LDR_DATA_TABLE_ENTRY1, MappingInfoIndexNode );
        LdrNode = *pLdrNode;
      }
      else {
        right = TRUE;
        break;
      }
    }
    else { // Already in tree (increase ref counter)
      // pLdrNode->DdagNode->ReferenceCount++;
      return TRUE;
    }
  }

  api->RtlRbInsertNodeEx((PRTL_RB_TREE) LdrpMappingInfoBase,
                         (PRTL_BALANCED_NODE) &pLdrNode->MappingInfoIndexNode, right,
                         (PRTL_BALANCED_NODE) &entry->MappingInfoIndexNode);

  return TRUE;
}

static LDR_DATA_TABLE_ENTRY1* llwythwr_link_library_win8plus(llwythwr_windows_api *api, const void* module_base, const wchar_t* dll_name, const wchar_t* dll_path) {
  LDR_DATA_TABLE_ENTRY1* entry = llwythwr_create_ldr_entry(api, module_base, dll_name, dll_path);
  if (entry == NULL) {
    DPRINT("Could not create ldr data table entry for %S.\n", dll_name);
    return NULL;
  }
  DPRINT("LDR_DATA_TABLE_ENTRY created for %S with hash %lu.\n", dll_name, entry->BaseNameHashValue);

  if (!llwythwr_insert_into_ldrp_mapping_info_index(api, entry, module_base)) {
    DPRINT("Could not add entry for %S in LdrpMappingInfoIndex.\n", dll_name);
    llwythwr_ldr_data_table_entry_free(api, entry);
    return NULL;
  }
  DPRINT("Entry added to LdrpMappingInfoIndex for %S.\n", dll_name);

  if (!llwythwr_insert_into_ldrp_module_index(api, entry, module_base)) {
    DPRINT("Could not add entry for %S in LdrpModuleIndex.\n", dll_name);
    // TODO remove entry from LdrpModuleBaseAddressIndex!
    llwythwr_ldr_data_table_entry_free(api, entry);
    return NULL;
  }
  DPRINT("Entry added to LdrpModuleIndex for %S.\n", dll_name);

  if (!llwythwr_insert_hash_node(api, &entry->HashLinks, entry->BaseNameHashValue)) {
    DPRINT("Could not add entry for %S to LdrpHashTable.\n", dll_name);
    // TODO remove entry from LdrpModuleIndex!
    // TODO remove entry from LdrpModuleBaseAddressIndex!
    api->LocalFree(entry->DdagNode);
    api->LocalFree(entry);
    return NULL;
  }
  DPRINT("Entry added to LdrpHashTable for %S.\n", dll_name);

  // insert into other lists
  PEB_LDR_DATA1 *ldr = (PEB_LDR_DATA1*) NtCurrentTeb()->ProcessEnvironmentBlock->Ldr;
  llwythwr_insert_tail_list(&ldr->InLoadOrderModuleList, &entry->InLoadOrderLinks);
  DPRINT("Entry added to InLoadOrderModuleList for %S.\n", dll_name);
  llwythwr_insert_tail_list(&ldr->InMemoryOrderModuleList, &entry->InMemoryOrderLinks);
  DPRINT("Entry added to InMemoryOrderModuleList for %S.\n", dll_name);
  llwythwr_insert_tail_list(&ldr->InInitializationOrderModuleList, &entry->InInitializationOrderLinks);
  DPRINT("Entry added to InInitializationOrderModuleList for %S.\n", dll_name);

  return entry;
}

static void llwythwr_unlink_library_win8plus(llwythwr_windows_api *api, LDR_DATA_TABLE_ENTRY1* entry) {
  DPRINT("Unlinking entry from InLoadOrderModuleList.\n");
  llwythwr_unlink_list_entry(&entry->InLoadOrderLinks);
  DPRINT("Unlinking entry from InMemoryOrderLinks.\n");
  llwythwr_unlink_list_entry(&entry->InMemoryOrderLinks);
  DPRINT("Unlinking entry from InInitializationOrderLinks.\n");
  llwythwr_unlink_list_entry(&entry->InInitializationOrderLinks);
  DPRINT("Unlinking entry from HashLinks.\n");
  llwythwr_unlink_list_entry(&entry->HashLinks);

  DPRINT("Removing entry from LdrpMappingInfoIndex.\n");
  PVOID LdrpMappingInfoBase = llwythwr_find_ldrp_mapping_info_base();
  api->RtlRbRemoveNode(LdrpMappingInfoBase, (PRTL_BALANCED_NODE) &entry->MappingInfoIndexNode);

  DPRINT("Removing entry from LdrpModuleIndex.\n");
  PVOID LdrpModuleIndexBase = llwythwr_find_ldrp_module_index_base();
  api->RtlRbRemoveNode(LdrpModuleIndexBase, (PRTL_BALANCED_NODE) &entry->BaseAddressIndexNode);

}

static void llwythwr_ldr_data_table_entry_free(llwythwr_windows_api *api, LDR_DATA_TABLE_ENTRY1* entry) {
  api->LocalFree(entry->DdagNode);
  api->LocalFree(entry);
}

#ifdef _WIN64
static void llwythwr_pointer_list_free(llwythwr_windows_api *api, POINTER_LIST *head)
{
  POINTER_LIST *node = head;
  while (node) {
    POINTER_LIST *next;
    api->VirtualFree(node->address, 0, MEM_RELEASE);
    next = node->next;
    api->LocalFree(node);
    node = next;
  }
}
#endif

static void llwythwr_init_unicode_string(PUNICODE_STRING target, PCWSTR source)
{
  if( (target->Buffer = (PWSTR)source) )
  {
    unsigned int length = llwythwr_wcslen( source ) * sizeof( WCHAR );
    if( length > 0xfffc )
      length = 0xfffc;

    target->Length = length;
    target->MaximumLength = target->Length + sizeof( WCHAR );
  }
  else target->Length = target->MaximumLength = 0;
}
