
# LLwythwr

Llwythwr is a library with the goal of providing functionality to load dynamic libraries (.so / .dll / .dylib) from memory, avoiding disk access.

Methods to achieve this goal are provided for all three major operating systems (Linux, macOS, and Windows).

Some methods also include options for further obfuscation and/or blending in with native loaded libraries, masking their use. 

-------------------------------

# Library API
```
#include "llwythwr/windows.h"
```

-------------------------------
## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[llwythwr_windows_api](Classes/structllwythwr__windows__api.md)** <br>Struct containing function pointers to operating system API functions that are used by llwythwr.  |

-------------------------------
## Types

|                | Name           |
| -------------- | -------------- |
| typedef unsigned long(*)(const char *string, unsigned long key) | **[llwythwr_hash_function](Files/windows_8h.md#typedef-hash-function)** <br>Hash function typedef.  |
| typedef void *(*)(const char *name, void *user_data) | **[llwythwr_get_lib_address_function](Files/windows_8h.md#typedef-get-lib-address-function)** <br>Get pointer to library function typedef.  |
| typedef void *(*)(void *handle, const char *name, int ordinal, void *user_data) | **[llwythwr_get_proc_address_function](Files/windows_8h.md#typedef-get-proc-address-function)** <br>Get pointer to a function function typedef.  |
| typedef void(*)(void *handle, void *user_data) | **[llwythwr_free_library_function](Files/windows_8h.md#typedef-free-library-function)** <br>Free a module function typedef.  |
| typedef struct _PRTL_RB_TREE * | **[PRTL_RB_TREE](Files/windows_8h.md#typedef-prtl-rb-tree)**  |
| typedef struct _RTL_BALANCED_NODE * | **[PRTL_BALANCED_NODE](Files/windows_8h.md#typedef-prtl-balanced-node)**  |
| typedef PVOID(WINAPI *)(PVOID, SIZE_T, DWORD, DWORD) | **[Type_VirtualAlloc](Files/windows_8h.md#typedef-type-virtualalloc)**  |
| typedef BOOL(WINAPI *)(LPVOID, SIZE_T, DWORD) | **[Type_VirtualFree](Files/windows_8h.md#typedef-type-virtualfree)**  |
| typedef BOOL(WINAPI *)(LPVOID, SIZE_T, DWORD, PDWORD) | **[Type_VirtualProtect](Files/windows_8h.md#typedef-type-virtualprotect)**  |
| typedef BOOL(WINAPI *)(HANDLE) | **[Type_CloseHandle](Files/windows_8h.md#typedef-type-closehandle)**  |
| typedef DWORD(WINAPI *)(VOID) | **[Type_GetLastError](Files/windows_8h.md#typedef-type-getlasterror)**  |
| typedef VOID(WINAPI *)(DWORD) | **[Type_SetLastError](Files/windows_8h.md#typedef-type-setlasterror)**  |
| typedef HLOCAL(WINAPI *)(UINT, SIZE_T) | **[Type_LocalAlloc](Files/windows_8h.md#typedef-type-localalloc)**  |
| typedef HLOCAL(WINAPI *)(HLOCAL) | **[Type_LocalFree](Files/windows_8h.md#typedef-type-localfree)**  |
| typedef HLOCAL(WINAPI *)(HLOCAL, SIZE_T, UINT) | **[Type_LocalReAlloc](Files/windows_8h.md#typedef-type-localrealloc)**  |
| typedef int(WINAPI *)(UINT, DWORD, LPCCH, int, LPWSTR, int) | **[Type_MultiByteToWideChar](Files/windows_8h.md#typedef-type-multibytetowidechar)**  |
| typedef BOOL(NTAPI *)(PRUNTIME_FUNCTION, DWORD, DWORD64) | **[Type_RtlAddFunctionTable](Files/windows_8h.md#typedef-type-rtladdfunctiontable)**  |
| typedef VOID(NTAPI *)(PUNICODE_STRING, PCWSTR) | **[Type_RtlInitUnicodeString](Files/windows_8h.md#typedef-type-rtlinitunicodestring)**  |
| typedef NTSTATUS(NTAPI *)(PCUNICODE_STRING, BOOLEAN, ULONG, PULONG) | **[Type_RtlHashUnicodeString](Files/windows_8h.md#typedef-type-rtlhashunicodestring)**  |
| typedef VOID(NTAPI *)(PRTL_RB_TREE, PRTL_BALANCED_NODE, BOOLEAN, PRTL_BALANCED_NODE) | **[Type_RtlRbInsertNodeEx](Files/windows_8h.md#typedef-type-rtlrbinsertnodeex)**  |
| typedef int(NTAPI *)(PRTL_RB_TREE, PRTL_BALANCED_NODE) | **[Type_RtlRbRemoveNode](Files/windows_8h.md#typedef-type-rtlrbremovenode)**  |
| typedef NTSTATUS(NTAPI *)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG) | **[Type_NtOpenFile](Files/windows_8h.md#typedef-type-ntopenfile)**  |
| typedef NTSTATUS(NTAPI *)(PHANDLE, ULONG, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE) | **[Type_NtCreateSection](Files/windows_8h.md#typedef-type-ntcreatesection)**  |
| typedef NTSTATUS(NTAPI *)(HANDLE, HANDLE, PVOID *, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG) | **[Type_NtMapViewOfSection](Files/windows_8h.md#typedef-type-ntmapviewofsection)**  |
| typedef NTSTATUS(NTAPI *)(HANDLE, PVOID) | **[Type_NtUnmapViewOfSection](Files/windows_8h.md#typedef-type-ntunmapviewofsection)**  |
| typedef NTSTATUS(NTAPI *)(PLARGE_INTEGER) | **[Type_NtQuerySystemTime](Files/windows_8h.md#typedef-type-ntquerysystemtime)**  |
| typedef struct [llwythwr_windows_api](Classes/structllwythwr__windows__api.md) | **[llwythwr_windows_api](Files/windows_8h.md#typedef-llwythwr-windows-api)** <br>Struct containing function pointers to operating system API functions that are used by llwythwr.  |
| typedef struct [llwythwr_windows_api](Classes/structllwythwr__windows__api.md) | **[llwythwr_os_api](Files/windows_8h.md#typedef-llwythwr-os-api)**  |
| typedef struct [llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) | **[llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library)** <br>Struct representing a library created by llwythwr.  |

-------------------------------
## Functions

|                | Name           |
| -------------- | -------------- |
| LLWYTHWR_EXPORT [llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * | **[llwythwr_map_library](Files/windows_8h.md#function-llwythwr-map-library)**([llwythwr_windows_api](Classes/structllwythwr__windows__api.md) * api, void * module_base, [llwythwr_get_lib_address_function](Files/windows_8h.md#typedef-get-lib-address-function) x_get_lib_address, [llwythwr_get_proc_address_function](Files/windows_8h.md#typedef-get-proc-address-function) x_get_proc_address, [llwythwr_free_library_function](Files/windows_8h.md#typedef-free-library-function) x_free_library_function, void * user_data)<br>Manually map a PE into current process using NtCreateSection and NtMapViewOfSection, then process the PE tables accordingly.  |
| LLWYTHWR_EXPORT [llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * | **[llwythwr_alloc_library](Files/windows_8h.md#function-llwythwr-alloc-library)**([llwythwr_windows_api](Classes/structllwythwr__windows__api.md) * api, void * module_base, [llwythwr_get_lib_address_function](Files/windows_8h.md#typedef-get-lib-address-function) x_get_lib_address, [llwythwr_get_proc_address_function](Files/windows_8h.md#typedef-get-proc-address-function) x_get_proc_address, [llwythwr_free_library_function](Files/windows_8h.md#typedef-free-library-function) x_free_library_function, void * user_data)<br>Copy a PE into private memory allocated for the current process, then process the PE tables accordingly.  |
| LLWYTHWR_EXPORT [llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * | **[llwythwr_map_hollow_library](Files/windows_8h.md#function-llwythwr-map-hollow-library)**([llwythwr_windows_api](Classes/structllwythwr__windows__api.md) * api, void * module_base, const char * decoy_filepath, BOOL keep_decoy_headers, [llwythwr_get_lib_address_function](Files/windows_8h.md#typedef-get-lib-address-function) x_get_lib_address, [llwythwr_get_proc_address_function](Files/windows_8h.md#typedef-get-proc-address-function) x_get_proc_address, [llwythwr_free_library_function](Files/windows_8h.md#typedef-free-library-function) x_free_library_function, void * user_data)<br>Manually map an existing library into the current process using NtOpenFile, NtCreateSection and NtMapViewOfSection and overlay a PE on top.  |
| LLWYTHWR_EXPORT [llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * | **[llwythwr_load_hollow_library](Files/windows_8h.md#function-llwythwr-load-hollow-library)**([llwythwr_windows_api](Classes/structllwythwr__windows__api.md) * api, void * module_base, HMODULE decoy, BOOL keep_decoy_headers, [llwythwr_get_lib_address_function](Files/windows_8h.md#typedef-get-lib-address-function) x_get_lib_address, [llwythwr_get_proc_address_function](Files/windows_8h.md#typedef-get-proc-address-function) x_get_proc_address, [llwythwr_free_library_function](Files/windows_8h.md#typedef-free-library-function) x_free_library_function, void * user_data)<br>Overlay a PE on top of an existing library that has been loaded with LoadLibraryEx and the DONT_RESOLVE_DLL_REFERENCES flag, then process the PE tables accordingly.  |
| LLWYTHWR_EXPORT [llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * | **[llwythwr_map_library_from_disk_to_section](Files/windows_8h.md#function-llwythwr-map-library-from-disk-to-section)**([llwythwr_windows_api](Classes/structllwythwr__windows__api.md) * api, const char * filename)<br>Maps a PE from disk into a Section using NtOpenFile, NtCreateSection and NtMapViewOfSection.  |
| LLWYTHWR_EXPORT void | **[llwythwr_unload_library](Files/windows_8h.md#function-llwythwr-unload-library)**([llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * library)<br>Free a library that was mapped into the current process.  |
| LLWYTHWR_EXPORT void | **[llwythwr_wipe_headers](Files/windows_8h.md#function-llwythwr-wipe-headers)**([llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * library)<br>Wipe PE headers.  |
| LLWYTHWR_EXPORT int | **[llwythwr_call_entry_point](Files/windows_8h.md#function-llwythwr-call-entry-point)**([llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * library)<br>Execute entry point, be it Main (EXE) or DllMain (DLL).  |
| LLWYTHWR_EXPORT void * | **[llwythwr_get_export_address_by_name](Files/windows_8h.md#function-llwythwr-get-export-address-by-name)**([llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * library, const char * export_name)<br>Given a library, resolve the address of a function using the provided export name.  |
| LLWYTHWR_EXPORT void * | **[llwythwr_get_export_address_by_hash](Files/windows_8h.md#function-llwythwr-get-export-address-by-hash)**([llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * library, unsigned long function_hash, unsigned long key, [llwythwr_hash_function](Files/windows_8h.md#typedef-hash-function) llwythwr_hash_function)<br>Given a library, resolve the address of a function using the provided hash.  |
| LLWYTHWR_EXPORT int | **[llwythwr_link_library](Files/windows_8h.md#function-llwythwr-link-library)**([llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * library, const char * basename, const char * fullpath)<br>Given a library, link it within internal windows structures in an attempt to make it appear as a system loaded library.  |
| LLWYTHWR_EXPORT int | **[llwythwr_unlink_library](Files/windows_8h.md#function-llwythwr-unlink-library)**([llwythwr_windows_library](Files/windows_8h.md#typedef-llwythwr-windows-library) * library)<br>Given a library which has already been linked using "llwythwr_link_library", unlink it from internal windows structures.  |

-------------------------------
## Types Documentation



### typedef llwythwr_hash_function

```cpp
typedef unsigned long(* llwythwr_hash_function) (const char *string, unsigned long key);
```

Hash function typedef. 

**Parameters**: 

  * **string** String to hash. 
  * **key** 64-bit integer to initialize the hash. 

**Return**: 64-bit integer representing the hash result. 



### typedef llwythwr_get_lib_address_function

```cpp
typedef void *(* llwythwr_get_lib_address_function) (const char *name, void *user_data);
```

Get pointer to library function typedef. 

**Parameters**: 

  * **name** The name of the DLL (e.g. "ntdll.dll" or "C:\Windows\System32\ntdll.dll"). 
  * **user_data** User supplied data provided when creating the module. 

**Return**: The base address of the loaded module or NULL if the module was not loaded successfully. 



### typedef llwythwr_get_proc_address_function

```cpp
typedef void *(* llwythwr_get_proc_address_function) (void *handle, const char *name, int ordinal, void *user_data);
```

Get pointer to a function function typedef. 

**Parameters**: 

  * **handle** The base address of a loaded library module. 
  * **name** Name of the exported procedure. 
  * **ordinal** Ordinal of the exported procedure. 
  * **user_data** User supplied data provided when creating the module. 

**Return**: Pointer to the desired function, or NULL if unsuccessful. 



### typedef llwythwr_free_library_function

```cpp
typedef void(* llwythwr_free_library_function) (void *handle, void *user_data);
```

Free a module function typedef. 

**Parameters**: 

  * **handle** The base address of a loaded library module. 
  * **user_data** User supplied data provided when creating the module. 



### typedef PRTL_RB_TREE

```cpp
typedef struct _PRTL_RB_TREE* PRTL_RB_TREE;
```



### typedef PRTL_BALANCED_NODE

```cpp
typedef struct _RTL_BALANCED_NODE* PRTL_BALANCED_NODE;
```



### typedef Type_VirtualAlloc

```cpp
typedef PVOID(WINAPI * Type_VirtualAlloc) (PVOID, SIZE_T, DWORD, DWORD);
```



### typedef Type_VirtualFree

```cpp
typedef BOOL(WINAPI * Type_VirtualFree) (LPVOID, SIZE_T, DWORD);
```



### typedef Type_VirtualProtect

```cpp
typedef BOOL(WINAPI * Type_VirtualProtect) (LPVOID, SIZE_T, DWORD, PDWORD);
```



### typedef Type_CloseHandle

```cpp
typedef BOOL(WINAPI * Type_CloseHandle) (HANDLE);
```



### typedef Type_GetLastError

```cpp
typedef DWORD(WINAPI * Type_GetLastError) (VOID);
```



### typedef Type_SetLastError

```cpp
typedef VOID(WINAPI * Type_SetLastError) (DWORD);
```



### typedef Type_LocalAlloc

```cpp
typedef HLOCAL(WINAPI * Type_LocalAlloc) (UINT, SIZE_T);
```



### typedef Type_LocalFree

```cpp
typedef HLOCAL(WINAPI * Type_LocalFree) (HLOCAL);
```



### typedef Type_LocalReAlloc

```cpp
typedef HLOCAL(WINAPI * Type_LocalReAlloc) (HLOCAL, SIZE_T, UINT);
```



### typedef Type_MultiByteToWideChar

```cpp
typedef int(WINAPI * Type_MultiByteToWideChar) (UINT, DWORD, LPCCH, int, LPWSTR, int);
```



### typedef Type_RtlAddFunctionTable

```cpp
typedef BOOL(NTAPI * Type_RtlAddFunctionTable) (PRUNTIME_FUNCTION, DWORD, DWORD64);
```



### typedef Type_RtlInitUnicodeString

```cpp
typedef VOID(NTAPI * Type_RtlInitUnicodeString) (PUNICODE_STRING, PCWSTR);
```



### typedef Type_RtlHashUnicodeString

```cpp
typedef NTSTATUS(NTAPI * Type_RtlHashUnicodeString) (PCUNICODE_STRING, BOOLEAN, ULONG, PULONG);
```



### typedef Type_RtlRbInsertNodeEx

```cpp
typedef VOID(NTAPI * Type_RtlRbInsertNodeEx) (PRTL_RB_TREE, PRTL_BALANCED_NODE, BOOLEAN, PRTL_BALANCED_NODE);
```



### typedef Type_RtlRbRemoveNode

```cpp
typedef int(NTAPI * Type_RtlRbRemoveNode) (PRTL_RB_TREE, PRTL_BALANCED_NODE);
```



### typedef Type_NtOpenFile

```cpp
typedef NTSTATUS(NTAPI * Type_NtOpenFile) (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);
```



### typedef Type_NtCreateSection

```cpp
typedef NTSTATUS(NTAPI * Type_NtCreateSection) (PHANDLE, ULONG, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
```



### typedef Type_NtMapViewOfSection

```cpp
typedef NTSTATUS(NTAPI * Type_NtMapViewOfSection) (HANDLE, HANDLE, PVOID *, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG);
```



### typedef Type_NtUnmapViewOfSection

```cpp
typedef NTSTATUS(NTAPI * Type_NtUnmapViewOfSection) (HANDLE, PVOID);
```



### typedef Type_NtQuerySystemTime

```cpp
typedef NTSTATUS(NTAPI * Type_NtQuerySystemTime) (PLARGE_INTEGER);
```



### typedef llwythwr_windows_api

```cpp
typedef struct llwythwr_windows_api llwythwr_windows_api;
```

Struct containing function pointers to operating system API functions that are used by llwythwr. 

This will need to be populated manually with pointers to valid functions with equivalent functionality. An example initialisation routine can be found within the [init.h](Files/init_8h.md#file-init.h) header, although it is probably not a good idea to use it as-is if OPSEC is a concern! 



### typedef llwythwr_os_api

```cpp
typedef struct llwythwr_windows_api llwythwr_os_api;
```



### typedef llwythwr_windows_library

```cpp
typedef struct llwythwr_windows_library llwythwr_windows_library;
```

Struct representing a library created by llwythwr. 

-------------------------------
## Functions Documentation



### function llwythwr_map_library

```cpp
LLWYTHWR_EXPORT llwythwr_windows_library * llwythwr_map_library(
    llwythwr_windows_api * api,
    void * module_base,
    llwythwr_get_lib_address_function x_get_lib_address,
    llwythwr_get_proc_address_function x_get_proc_address,
    llwythwr_free_library_function x_free_library_function,
    void * user_data
)
```

Manually map a PE into current process using NtCreateSection and NtMapViewOfSection, then process the PE tables accordingly. 

**Parameters**: 

  * **api** Operating System API abstraction struct. 
  * **module_base** Pointer to the module base. 
  * **x_get_lib_address** Function to call to load a dependant module during IAT processing 
  * **x_get_proc_address** Function to call to resolve the address of a function during IAT processing 
  * **x_free_library_function** Function to call to unload a dependant module when unloading the module 
  * **user_data** User supplied data that is passed as a parameter on callbacks 

**Return**: Manually mapped library, or NULL if an error was encountered. 



### function llwythwr_alloc_library

```cpp
LLWYTHWR_EXPORT llwythwr_windows_library * llwythwr_alloc_library(
    llwythwr_windows_api * api,
    void * module_base,
    llwythwr_get_lib_address_function x_get_lib_address,
    llwythwr_get_proc_address_function x_get_proc_address,
    llwythwr_free_library_function x_free_library_function,
    void * user_data
)
```

Copy a PE into private memory allocated for the current process, then process the PE tables accordingly. 

**Parameters**: 

  * **api** Operating System API abstraction struct. 
  * **module_base** Pointer to the module base. 
  * **x_get_lib_address** Function to call to load a dependant module during IAT processing 
  * **x_get_proc_address** Function to call to resolve the address of a function during IAT processing 
  * **x_free_library_function** Function to call to unload a dependant module when unloading the module 
  * **user_data** User supplied data that is passed as a parameter on callbacks 

**Return**: Manually mapped library, or NULL if an error was encountered. 



### function llwythwr_map_hollow_library

```cpp
LLWYTHWR_EXPORT llwythwr_windows_library * llwythwr_map_hollow_library(
    llwythwr_windows_api * api,
    void * module_base,
    const char * decoy_filepath,
    BOOL keep_decoy_headers,
    llwythwr_get_lib_address_function x_get_lib_address,
    llwythwr_get_proc_address_function x_get_proc_address,
    llwythwr_free_library_function x_free_library_function,
    void * user_data
)
```

Manually map an existing library into the current process using NtOpenFile, NtCreateSection and NtMapViewOfSection and overlay a PE on top. 

**Parameters**: 

  * **api** Operating System API abstraction struct. 
  * **module_base** Pointer to the module base. 
  * **decoy_filepath** Path to decoy library 
  * **keep_decoy_headers** Keep decoy headers intact 
  * **x_get_lib_address** Function to call to load a dependant module during IAT processing 
  * **x_get_proc_address** Function to call to resolve the address of a function during IAT processing 
  * **x_free_library_function** Function to call to unload a dependant module when unloading the module 
  * **user_data** User supplied data that is passed as a parameter on callbacks 

**Return**: Manually mapped library, or NULL if an error was encountered. 

PE tables are also processed accordingly. 



### function llwythwr_load_hollow_library

```cpp
LLWYTHWR_EXPORT llwythwr_windows_library * llwythwr_load_hollow_library(
    llwythwr_windows_api * api,
    void * module_base,
    HMODULE decoy,
    BOOL keep_decoy_headers,
    llwythwr_get_lib_address_function x_get_lib_address,
    llwythwr_get_proc_address_function x_get_proc_address,
    llwythwr_free_library_function x_free_library_function,
    void * user_data
)
```

Overlay a PE on top of an existing library that has been loaded with LoadLibraryEx and the DONT_RESOLVE_DLL_REFERENCES flag, then process the PE tables accordingly. 

**Parameters**: 

  * **api** Operating System API abstraction struct. 
  * **module_base** Pointer to the module base. 
  * **decoy** Valid handle to the decoy library 
  * **keep_decoy_headers** Keep decoy headers intact 
  * **x_get_lib_address** Function to call to load a dependant module during IAT processing 
  * **x_get_proc_address** Function to call to resolve the address of a function during IAT processing 
  * **x_free_library_function** Function to call to unload a dependant module when unloading the module 
  * **user_data** User supplied data that is passed as a parameter on callbacks 

**Return**: Manually mapped library, or NULL if an error was encountered. 



### function llwythwr_map_library_from_disk_to_section

```cpp
LLWYTHWR_EXPORT llwythwr_windows_library * llwythwr_map_library_from_disk_to_section(
    llwythwr_windows_api * api,
    const char * filename
)
```

Maps a PE from disk into a Section using NtOpenFile, NtCreateSection and NtMapViewOfSection. 

**Parameters**: 

  * **api** Operating System API abstraction struct. 
  * **filename** Full path fo the DLL on disk. 

**Return**: Manually mapped library, or NULL if an error was encountered. 

Important: Processing of the various PE tables (import, delayed import, exception, tls etc) will not been performed! 



### function llwythwr_unload_library

```cpp
LLWYTHWR_EXPORT void llwythwr_unload_library(
    llwythwr_windows_library * library
)
```

Free a library that was mapped into the current process. 

**Parameters**: 

  * **library** The manually mapped library. 



### function llwythwr_wipe_headers

```cpp
LLWYTHWR_EXPORT void llwythwr_wipe_headers(
    llwythwr_windows_library * library
)
```

Wipe PE headers. 

**Parameters**: 

  * **library** The manually mapped library. 



### function llwythwr_call_entry_point

```cpp
LLWYTHWR_EXPORT int llwythwr_call_entry_point(
    llwythwr_windows_library * library
)
```

Execute entry point, be it Main (EXE) or DllMain (DLL). 

**Parameters**: 

  * **library** The manually mapped library. 

**Return**: A negative value if the entry point could not be executed. 

Important: calling this function on a EXE will not return, i.e. once the loaded EXE finished running, the process will terminate. 



### function llwythwr_get_export_address_by_name

```cpp
LLWYTHWR_EXPORT void * llwythwr_get_export_address_by_name(
    llwythwr_windows_library * library,
    const char * export_name
)
```

Given a library, resolve the address of a function using the provided export name. 

**Parameters**: 

  * **library** The manually mapped library. 
  * **export_name** The name or ordinal of the export to search for (e.g. "NtAlertResumeThread"). 

**Return**: Pointer to the desired function, or NULL if not found. 



### function llwythwr_get_export_address_by_hash

```cpp
LLWYTHWR_EXPORT void * llwythwr_get_export_address_by_hash(
    llwythwr_windows_library * library,
    unsigned long function_hash,
    unsigned long key,
    llwythwr_hash_function llwythwr_hash_function
)
```

Given a library, resolve the address of a function using the provided hash. 

**Parameters**: 

  * **library** The manually mapped library. 
  * **function_hash** Hash of the exported procedure. 
  * **key** 64-bit integer to initialize the keyed hash. 
  * **llwythwr_hash_function** The hash function to use. 

**Return**: Pointer to the desired function, or NULL if not found. 



### function llwythwr_link_library

```cpp
LLWYTHWR_EXPORT int llwythwr_link_library(
    llwythwr_windows_library * library,
    const char * basename,
    const char * fullpath
)
```

Given a library, link it within internal windows structures in an attempt to make it appear as a system loaded library. 

**Parameters**: 

  * **library** The manually mapped library. 
  * **basename** The name the library appears to have e.g. "Legit.dll" 
  * **fullpath** The full path on disk that the library appears to be loaded from e.g. "C:\Windows\System32\Legit.dll" 

**Return**: TRUE on success, FALSE on failure. 



### function llwythwr_unlink_library

```cpp
LLWYTHWR_EXPORT int llwythwr_unlink_library(
    llwythwr_windows_library * library
)
```

Given a library which has already been linked using "llwythwr_link_library", unlink it from internal windows structures. 

**Parameters**: 

  * **library** The manually mapped library. 

**Return**: TRUE on success, FALSE on failure. 

-------------------------------
## Source code

```cpp
#ifndef LIBLOADER_SRC_LIBLOADER_WINDOWS_H_
#define LIBLOADER_SRC_LIBLOADER_WINDOWS_H_

#include <windows.h>
#include <winternl.h>
#include <llwythwr/export.h>

/*
 * Typedef
 */

typedef unsigned long (*llwythwr_hash_function)(const char* string, unsigned long key);

typedef void* (*llwythwr_get_lib_address_function)(const char* name, void* user_data);

typedef void* (*llwythwr_get_proc_address_function)(void* handle, const char* name, int ordinal, void* user_data);

typedef void (*llwythwr_free_library_function)(void* handle, void* user_data);

typedef struct _PRTL_RB_TREE *PRTL_RB_TREE;
typedef struct _RTL_BALANCED_NODE *PRTL_BALANCED_NODE;

typedef PVOID (WINAPI *Type_VirtualAlloc)(PVOID, SIZE_T, DWORD, DWORD);     // NtAllocateVirtualMemory
typedef BOOL (WINAPI *Type_VirtualFree)(LPVOID, SIZE_T, DWORD);             // NtFreeVirtualMemory
typedef BOOL (WINAPI *Type_VirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD);  // NtProtectVirtualMemory
typedef BOOL (WINAPI *Type_CloseHandle)(HANDLE);                            // NtClose
typedef DWORD (WINAPI *Type_GetLastError)(VOID);
typedef VOID (WINAPI *Type_SetLastError)(DWORD);
typedef HLOCAL (WINAPI *Type_LocalAlloc)(UINT, SIZE_T);
typedef HLOCAL (WINAPI *Type_LocalFree)(HLOCAL);
typedef HLOCAL (WINAPI *Type_LocalReAlloc)(HLOCAL, SIZE_T, UINT);
typedef int (WINAPI *Type_MultiByteToWideChar)(UINT, DWORD, LPCCH, int, LPWSTR, int);
typedef BOOL (NTAPI* Type_RtlAddFunctionTable)(PRUNTIME_FUNCTION, DWORD, DWORD64);
typedef VOID (NTAPI *Type_RtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
typedef NTSTATUS (NTAPI *Type_RtlHashUnicodeString)(PCUNICODE_STRING, BOOLEAN, ULONG, PULONG);
typedef VOID (NTAPI *Type_RtlRbInsertNodeEx)(PRTL_RB_TREE, PRTL_BALANCED_NODE, BOOLEAN, PRTL_BALANCED_NODE);
typedef int (NTAPI* Type_RtlRbRemoveNode)(PRTL_RB_TREE, PRTL_BALANCED_NODE);
typedef NTSTATUS (NTAPI *Type_NtOpenFile)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);
typedef NTSTATUS (NTAPI *Type_NtCreateSection)(PHANDLE, ULONG, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
typedef NTSTATUS (NTAPI *Type_NtMapViewOfSection)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG);
typedef NTSTATUS (NTAPI *Type_NtUnmapViewOfSection)(HANDLE, PVOID);
typedef NTSTATUS (NTAPI *Type_NtQuerySystemTime)(PLARGE_INTEGER);

/*
 * Structs
 */

typedef struct llwythwr_windows_api
{
  Type_VirtualAlloc VirtualAlloc;
  Type_VirtualFree VirtualFree;
  Type_VirtualProtect VirtualProtect;
  Type_CloseHandle CloseHandle;
  Type_GetLastError GetLastError;
  Type_SetLastError SetLastError;
  Type_LocalAlloc LocalAlloc;
  Type_LocalFree LocalFree;
  Type_LocalReAlloc LocalReAlloc;
  Type_MultiByteToWideChar MultiByteToWideChar;
  Type_RtlAddFunctionTable RtlAddFunctionTable;
  Type_RtlInitUnicodeString RtlInitUnicodeString;
  Type_RtlHashUnicodeString RtlHashUnicodeString;
  Type_RtlRbInsertNodeEx RtlRbInsertNodeEx;
  Type_RtlRbRemoveNode RtlRbRemoveNode;
  Type_NtOpenFile NtOpenFile;
  Type_NtCreateSection NtCreateSection;
  Type_NtMapViewOfSection NtMapViewOfSection;
  Type_NtUnmapViewOfSection NtUnmapViewOfSection;
  Type_NtQuerySystemTime NtQuerySystemTime;
} llwythwr_windows_api, llwythwr_os_api;

typedef struct llwythwr_windows_library llwythwr_windows_library;

/*
 * Module Loading
 */

LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_map_library(llwythwr_windows_api *api, void* module_base, llwythwr_get_lib_address_function x_get_lib_address, llwythwr_get_proc_address_function x_get_proc_address, llwythwr_free_library_function x_free_library_function, void* user_data);

LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_alloc_library(llwythwr_windows_api *api, void* module_base, llwythwr_get_lib_address_function x_get_lib_address, llwythwr_get_proc_address_function x_get_proc_address, llwythwr_free_library_function x_free_library_function, void* user_data);

LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_map_hollow_library(llwythwr_windows_api *api, void* module_base, const char* decoy_filepath, BOOL keep_decoy_headers, llwythwr_get_lib_address_function x_get_lib_address, llwythwr_get_proc_address_function x_get_proc_address, llwythwr_free_library_function x_free_library_function, void* user_data);

LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_load_hollow_library(llwythwr_windows_api *api, void* module_base, HMODULE decoy, BOOL keep_decoy_headers, llwythwr_get_lib_address_function x_get_lib_address, llwythwr_get_proc_address_function x_get_proc_address, llwythwr_free_library_function x_free_library_function, void* user_data);

LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_map_library_from_disk_to_section(llwythwr_windows_api *api, const char* filename);

LLWYTHWR_EXPORT
void llwythwr_unload_library(llwythwr_windows_library *library);

LLWYTHWR_EXPORT
void llwythwr_wipe_headers(llwythwr_windows_library *library);

LLWYTHWR_EXPORT
int llwythwr_call_entry_point(llwythwr_windows_library *library);

LLWYTHWR_EXPORT
void* llwythwr_get_export_address_by_name(llwythwr_windows_library *library, const char* export_name);

LLWYTHWR_EXPORT
void* llwythwr_get_export_address_by_hash(llwythwr_windows_library *library, unsigned long function_hash, unsigned long key, llwythwr_hash_function llwythwr_hash_function);

LLWYTHWR_EXPORT
int llwythwr_link_library(llwythwr_windows_library* library, const char* basename, const char* fullpath);

LLWYTHWR_EXPORT
int llwythwr_unlink_library(llwythwr_windows_library* library);

#endif //LIBLOADER_SRC_LIBLOADER_WINDOWS_H_
```

-------------------------------

# Library API
```
#include "llwythwr/darwin.h"
```

-------------------------------
## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[llwythwr_macos_api](Classes/structllwythwr__macos__api.md)**  |

-------------------------------
## Types

|                | Name           |
| -------------- | -------------- |
| typedef NSObjectFileImageReturnCode(*)(const void *, size_t, NSObjectFileImage *) | **[func_NSCreateObjectFileImageFromMemory](Files/darwin_8h.md#typedef-func-nscreateobjectfileimagefrommemory)**  |
| typedef NSModule(*)(NSObjectFileImage, const char *, uint32_t) | **[func_NSLinkModule](Files/darwin_8h.md#typedef-func-nslinkmodule)**  |
| typedef bool(*)(NSObjectFileImage) | **[func_NSDestroyObjectFileImage](Files/darwin_8h.md#typedef-func-nsdestroyobjectfileimage)**  |
| typedef bool(*)(NSModule, unsigned int) | **[func_NSUnLinkModule](Files/darwin_8h.md#typedef-func-nsunlinkmodule)**  |
| typedef NSSymbol(*)(NSModule, const char *) | **[func_NSLookupSymbolInModule](Files/darwin_8h.md#typedef-func-nslookupsymbolinmodule)**  |
| typedef void *(*)(NSSymbol symbol) | **[func_NSAddressOfSymbol](Files/darwin_8h.md#typedef-func-nsaddressofsymbol)**  |
| typedef void *(*)(void *, size_t, int, int, int, off_t) | **[func_mmap](Files/darwin_8h.md#typedef-func-mmap)**  |
| typedef int(*)(void *, size_t) | **[func_munmap](Files/darwin_8h.md#typedef-func-munmap)**  |
| typedef void *(*)(size_t) | **[func_malloc](Files/darwin_8h.md#typedef-func-malloc)**  |
| typedef void(*)(void *) | **[func_free](Files/darwin_8h.md#typedef-func-free)**  |
| typedef void *(*)(void *, const void *, size_t) | **[func_memcpy](Files/darwin_8h.md#typedef-func-memcpy)**  |
| typedef int(*)(char **, const char *,...) | **[func_asprintf](Files/darwin_8h.md#typedef-func-asprintf)**  |
| typedef void *(*)(const char *, int) | **[func_dlopen](Files/darwin_8h.md#typedef-func-dlopen)**  |
| typedef int(*)(void *) | **[func_dlclose](Files/darwin_8h.md#typedef-func-dlclose)**  |
| typedef void *(*)(void *, const char *) | **[func_dlsym](Files/darwin_8h.md#typedef-func-dlsym)**  |
| typedef struct [llwythwr_macos_api](Classes/structllwythwr__macos__api.md) | **[llwythwr_macos_api](Files/darwin_8h.md#typedef-llwythwr-macos-api)**  |
| typedef struct [llwythwr_macos_api](Classes/structllwythwr__macos__api.md) | **[llwythwr_os_api](Files/darwin_8h.md#typedef-llwythwr-os-api)**  |
| typedef struct [llwythwr_mac_library](Files/darwin_8h.md#typedef-llwythwr-mac-library) | **[llwythwr_mac_library](Files/darwin_8h.md#typedef-llwythwr-mac-library)**  |

-------------------------------
## Functions

|                | Name           |
| -------------- | -------------- |
| LLWYTHWR_EXPORT void * | **[llwythwr_load_library_from_disk](Files/darwin_8h.md#function-llwythwr-load-library-from-disk)**(const char * name, int flags) |
| LLWYTHWR_EXPORT void | **[llwythwr_unload_library_from_disk](Files/darwin_8h.md#function-llwythwr-unload-library-from-disk)**(void * handle) |
| LLWYTHWR_EXPORT void * | **[llwythwr_get_symbol_address_disk](Files/darwin_8h.md#function-llwythwr-get-symbol-address-disk)**(void * handle, const char * name) |
| LLWYTHWR_EXPORT [llwythwr_mac_library](Files/darwin_8h.md#typedef-llwythwr-mac-library) * | **[llwythwr_load_library_from_memory](Files/darwin_8h.md#function-llwythwr-load-library-from-memory)**([llwythwr_macos_api](Classes/structllwythwr__macos__api.md) * os_api, const char * name, const void * data, size_t size, int flags) |
| LLWYTHWR_EXPORT void | **[llwythwr_unload_library_from_memory](Files/darwin_8h.md#function-llwythwr-unload-library-from-memory)**([llwythwr_mac_library](Files/darwin_8h.md#typedef-llwythwr-mac-library) * library) |
| LLWYTHWR_EXPORT void * | **[llwythwr_get_symbol_address_memory](Files/darwin_8h.md#function-llwythwr-get-symbol-address-memory)**([llwythwr_mac_library](Files/darwin_8h.md#typedef-llwythwr-mac-library) * library, const char * name) |

-------------------------------
## Types Documentation



### typedef func_NSCreateObjectFileImageFromMemory

```cpp
typedef NSObjectFileImageReturnCode(* func_NSCreateObjectFileImageFromMemory) (const void *, size_t, NSObjectFileImage *);
```



### typedef func_NSLinkModule

```cpp
typedef NSModule(* func_NSLinkModule) (NSObjectFileImage, const char *, uint32_t);
```



### typedef func_NSDestroyObjectFileImage

```cpp
typedef bool(* func_NSDestroyObjectFileImage) (NSObjectFileImage);
```



### typedef func_NSUnLinkModule

```cpp
typedef bool(* func_NSUnLinkModule) (NSModule, unsigned int);
```



### typedef func_NSLookupSymbolInModule

```cpp
typedef NSSymbol(* func_NSLookupSymbolInModule) (NSModule, const char *);
```



### typedef func_NSAddressOfSymbol

```cpp
typedef void *(* func_NSAddressOfSymbol) (NSSymbol symbol);
```



### typedef func_mmap

```cpp
typedef void *(* func_mmap) (void *, size_t, int, int, int, off_t);
```



### typedef func_munmap

```cpp
typedef int(* func_munmap) (void *, size_t);
```



### typedef func_malloc

```cpp
typedef void *(* func_malloc) (size_t);
```



### typedef func_free

```cpp
typedef void(* func_free) (void *);
```



### typedef func_memcpy

```cpp
typedef void *(* func_memcpy) (void *, const void *, size_t);
```



### typedef func_asprintf

```cpp
typedef int(* func_asprintf) (char **, const char *,...);
```



### typedef func_dlopen

```cpp
typedef void *(* func_dlopen) (const char *, int);
```



### typedef func_dlclose

```cpp
typedef int(* func_dlclose) (void *);
```



### typedef func_dlsym

```cpp
typedef void *(* func_dlsym) (void *, const char *);
```



### typedef llwythwr_macos_api

```cpp
typedef struct llwythwr_macos_api llwythwr_macos_api;
```



### typedef llwythwr_os_api

```cpp
typedef struct llwythwr_macos_api llwythwr_os_api;
```



### typedef llwythwr_mac_library

```cpp
typedef struct llwythwr_mac_library llwythwr_mac_library;
```

-------------------------------
## Functions Documentation



### function llwythwr_load_library_from_disk

```cpp
LLWYTHWR_EXPORT void * llwythwr_load_library_from_disk(
    const char * name,
    int flags
)
```



### function llwythwr_unload_library_from_disk

```cpp
LLWYTHWR_EXPORT void llwythwr_unload_library_from_disk(
    void * handle
)
```



### function llwythwr_get_symbol_address_disk

```cpp
LLWYTHWR_EXPORT void * llwythwr_get_symbol_address_disk(
    void * handle,
    const char * name
)
```



### function llwythwr_load_library_from_memory

```cpp
LLWYTHWR_EXPORT llwythwr_mac_library * llwythwr_load_library_from_memory(
    llwythwr_macos_api * os_api,
    const char * name,
    const void * data,
    size_t size,
    int flags
)
```



### function llwythwr_unload_library_from_memory

```cpp
LLWYTHWR_EXPORT void llwythwr_unload_library_from_memory(
    llwythwr_mac_library * library
)
```



### function llwythwr_get_symbol_address_memory

```cpp
LLWYTHWR_EXPORT void * llwythwr_get_symbol_address_memory(
    llwythwr_mac_library * library,
    const char * name
)
```

-------------------------------
## Source code

```cpp
#ifndef LIBLOADER_SRC_LIBLOADER_DARWIN_H_
#define LIBLOADER_SRC_LIBLOADER_DARWIN_H_

#include <stddef.h>
#include <llwythwr/export.h>
#include <mach-o/dyld.h>
#include <sys/mman.h>

typedef NSObjectFileImageReturnCode (*func_NSCreateObjectFileImageFromMemory)(const void*, size_t, NSObjectFileImage*);
typedef NSModule (*func_NSLinkModule)(NSObjectFileImage, const char*, uint32_t);
typedef bool (*func_NSDestroyObjectFileImage)(NSObjectFileImage);
typedef bool (*func_NSUnLinkModule)(NSModule, unsigned int);
typedef NSSymbol (*func_NSLookupSymbolInModule)(NSModule, const char*);
typedef void* (*func_NSAddressOfSymbol)(NSSymbol symbol);
typedef void* (*func_mmap)(void*, size_t, int, int, int, off_t);
typedef int (*func_munmap)(void*, size_t);
typedef void* (*func_malloc)(size_t);
typedef void (*func_free)(void*);
typedef void* (*func_memcpy)(void*, const void*, size_t);
typedef int (*func_asprintf)(char **, const char *, ...);
typedef void* (*func_dlopen)(const char*, int);
typedef int (*func_dlclose)(void*);
typedef void* (*func_dlsym)(void*, const char*);

typedef struct llwythwr_macos_api {
  func_NSCreateObjectFileImageFromMemory NSCreateObjectFileImageFromMemory;
  func_NSLinkModule NSLinkModule;
  func_NSDestroyObjectFileImage NSDestroyObjectFileImage;
  func_NSUnLinkModule NSUnLinkModule;
  func_NSLookupSymbolInModule NSLookupSymbolInModule;
  func_NSAddressOfSymbol NSAddressOfSymbol;
  func_mmap mmap;
  func_munmap munmap;
  func_malloc malloc;
  func_free free;
  func_memcpy _memcpy;
  func_asprintf asprintf;
  func_dlopen dlopen;
  func_dlclose dlclose;
  func_dlsym dlsym;
} llwythwr_macos_api, llwythwr_os_api;

typedef struct llwythwr_mac_library llwythwr_mac_library;

LLWYTHWR_EXPORT
void* llwythwr_load_library_from_disk(const char* name, int flags);

LLWYTHWR_EXPORT
void llwythwr_unload_library_from_disk(void* handle);

LLWYTHWR_EXPORT
void* llwythwr_get_symbol_address_disk(void *handle, const char *name);

LLWYTHWR_EXPORT
llwythwr_mac_library* llwythwr_load_library_from_memory(llwythwr_macos_api *os_api, const char *name, const void *data, size_t size, int flags);

LLWYTHWR_EXPORT
void llwythwr_unload_library_from_memory(llwythwr_mac_library* library);

LLWYTHWR_EXPORT
void* llwythwr_get_symbol_address_memory(llwythwr_mac_library* library, const char *name);

#endif //LIBLOADER_SRC_LIBLOADER_DARWIN_H_
```

-------------------------------

# Library API
```
#include "llwythwr/linux.h"
```

-------------------------------
## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[llwythwr_linux_api](Classes/structllwythwr__linux__api.md)**  |

-------------------------------
## Types

|                | Name           |
| -------------- | -------------- |
| typedef void *(*)(const char *, int) | **[func_dlopen](Files/linux_8h.md#typedef-func-dlopen)**  |
| typedef int(*)(void *) | **[func_dlclose](Files/linux_8h.md#typedef-func-dlclose)**  |
| typedef void *(*)(void *, const char *) | **[func_dlsym](Files/linux_8h.md#typedef-func-dlsym)**  |
| typedef char *(*)(void) | **[func_dlerror](Files/linux_8h.md#typedef-func-dlerror)**  |
| typedef pid_t(*)(void) | **[func_getpid](Files/linux_8h.md#typedef-func-getpid)**  |
| typedef int(*)(const char *, unsigned int) | **[func_memfd_create](Files/linux_8h.md#typedef-func-memfd-create)**  |
| typedef ssize_t(*)(int, const void *, size_t) | **[func_write](Files/linux_8h.md#typedef-func-write)**  |
| typedef int(*)(char *, size_t, const char *,...) | **[func_snprintf](Files/linux_8h.md#typedef-func-snprintf)**  |
| typedef void *(*)(size_t) | **[func_malloc](Files/linux_8h.md#typedef-func-malloc)**  |
| typedef void(*)(void *) | **[func_free](Files/linux_8h.md#typedef-func-free)**  |
| typedef struct [llwythwr_linux_api](Classes/structllwythwr__linux__api.md) | **[llwythwr_linux_api](Files/linux_8h.md#typedef-llwythwr-linux-api)**  |
| typedef struct [llwythwr_linux_api](Classes/structllwythwr__linux__api.md) | **[llwythwr_os_api](Files/linux_8h.md#typedef-llwythwr-os-api)**  |
| typedef struct [llwythwr_linux_library](Files/linux_8h.md#typedef-llwythwr-linux-library) | **[llwythwr_linux_library](Files/linux_8h.md#typedef-llwythwr-linux-library)**  |

-------------------------------
## Functions

|                | Name           |
| -------------- | -------------- |
| LLWYTHWR_EXPORT void * | **[llwythwr_load_library_from_disk](Files/linux_8h.md#function-llwythwr-load-library-from-disk)**(const char * name, int flags) |
| LLWYTHWR_EXPORT void | **[llwythwr_unload_library_from_disk](Files/linux_8h.md#function-llwythwr-unload-library-from-disk)**(void * handle) |
| LLWYTHWR_EXPORT void * | **[llwythwr_get_symbol_address_disk](Files/linux_8h.md#function-llwythwr-get-symbol-address-disk)**(void * handle, const char * name) |
| LLWYTHWR_EXPORT [llwythwr_linux_library](Files/linux_8h.md#typedef-llwythwr-linux-library) * | **[llwythwr_load_library_from_memory](Files/linux_8h.md#function-llwythwr-load-library-from-memory)**([llwythwr_linux_api](Classes/structllwythwr__linux__api.md) * os_api, const char * name, const void * data, size_t size, int flags) |
| LLWYTHWR_EXPORT void | **[llwythwr_unload_library_from_memory](Files/linux_8h.md#function-llwythwr-unload-library-from-memory)**([llwythwr_linux_library](Files/linux_8h.md#typedef-llwythwr-linux-library) * library) |
| LLWYTHWR_EXPORT void * | **[llwythwr_get_symbol_address_memory](Files/linux_8h.md#function-llwythwr-get-symbol-address-memory)**([llwythwr_linux_library](Files/linux_8h.md#typedef-llwythwr-linux-library) * library, const char * name) |

-------------------------------
## Types Documentation



### typedef func_dlopen

```cpp
typedef void *(* func_dlopen) (const char *, int);
```



### typedef func_dlclose

```cpp
typedef int(* func_dlclose) (void *);
```



### typedef func_dlsym

```cpp
typedef void *(* func_dlsym) (void *, const char *);
```



### typedef func_dlerror

```cpp
typedef char *(* func_dlerror) (void);
```



### typedef func_getpid

```cpp
typedef pid_t(* func_getpid) (void);
```



### typedef func_memfd_create

```cpp
typedef int(* func_memfd_create) (const char *, unsigned int);
```



### typedef func_write

```cpp
typedef ssize_t(* func_write) (int, const void *, size_t);
```



### typedef func_snprintf

```cpp
typedef int(* func_snprintf) (char *, size_t, const char *,...);
```



### typedef func_malloc

```cpp
typedef void *(* func_malloc) (size_t);
```



### typedef func_free

```cpp
typedef void(* func_free) (void *);
```



### typedef llwythwr_linux_api

```cpp
typedef struct llwythwr_linux_api llwythwr_linux_api;
```



### typedef llwythwr_os_api

```cpp
typedef struct llwythwr_linux_api llwythwr_os_api;
```



### typedef llwythwr_linux_library

```cpp
typedef struct llwythwr_linux_library llwythwr_linux_library;
```

-------------------------------
## Functions Documentation



### function llwythwr_load_library_from_disk

```cpp
LLWYTHWR_EXPORT void * llwythwr_load_library_from_disk(
    const char * name,
    int flags
)
```



### function llwythwr_unload_library_from_disk

```cpp
LLWYTHWR_EXPORT void llwythwr_unload_library_from_disk(
    void * handle
)
```



### function llwythwr_get_symbol_address_disk

```cpp
LLWYTHWR_EXPORT void * llwythwr_get_symbol_address_disk(
    void * handle,
    const char * name
)
```



### function llwythwr_load_library_from_memory

```cpp
LLWYTHWR_EXPORT llwythwr_linux_library * llwythwr_load_library_from_memory(
    llwythwr_linux_api * os_api,
    const char * name,
    const void * data,
    size_t size,
    int flags
)
```



### function llwythwr_unload_library_from_memory

```cpp
LLWYTHWR_EXPORT void llwythwr_unload_library_from_memory(
    llwythwr_linux_library * library
)
```



### function llwythwr_get_symbol_address_memory

```cpp
LLWYTHWR_EXPORT void * llwythwr_get_symbol_address_memory(
    llwythwr_linux_library * library,
    const char * name
)
```

-------------------------------
## Source code

```cpp
#ifndef LIBLOADER_SRC_LIBLOADER_LINUX_H_
#define LIBLOADER_SRC_LIBLOADER_LINUX_H_

#include <stddef.h>
#include <sys/types.h>
#include <llwythwr/export.h>

typedef void* (*func_dlopen)(const char*, int);
typedef int (*func_dlclose)(void*);
typedef void* (*func_dlsym)(void*, const char*);
typedef char* (*func_dlerror)(void);
typedef pid_t (*func_getpid)(void);
typedef int (*func_memfd_create)(const char*, unsigned int);
typedef ssize_t (*func_write)(int, const void *, size_t);
typedef int (*func_snprintf)(char *, size_t, const char *, ...);
typedef void* (*func_malloc)(size_t);
typedef void (*func_free)(void*);

typedef struct llwythwr_linux_api
{
  func_dlopen dlopen;
  func_dlclose dlclose;
  func_dlsym dlsym;
  func_dlerror dlerror;
  func_getpid getpid;
  func_memfd_create memfd_create;
  func_snprintf snprintf;
  func_write write;
  func_malloc malloc;
  func_free free;
} llwythwr_linux_api, llwythwr_os_api;

typedef struct llwythwr_linux_library llwythwr_linux_library;

LLWYTHWR_EXPORT
void* llwythwr_load_library_from_disk(const char* name, int flags);

LLWYTHWR_EXPORT
void llwythwr_unload_library_from_disk(void* handle);

LLWYTHWR_EXPORT
void* llwythwr_get_symbol_address_disk(void* handle, const char *name);

LLWYTHWR_EXPORT
llwythwr_linux_library* llwythwr_load_library_from_memory(llwythwr_linux_api* os_api, const char *name, const void *data, size_t size, int flags);

LLWYTHWR_EXPORT
void llwythwr_unload_library_from_memory(llwythwr_linux_library* library);

LLWYTHWR_EXPORT
void* llwythwr_get_symbol_address_memory(llwythwr_linux_library* library, const char *name);

#endif //LIBLOADER_SRC_LIBLOADER_LINUX_H_
```

-------------------------------

# Library API
```
#include "llwythwr/init.h"
```

-------------------------------
## Functions

|                | Name           |
| -------------- | -------------- |
| int | **[memfd_create_wrapper](Files/init_8h.md#function-memfd-create-wrapper)**(const char * name, unsigned int flags) |
| int | **[llwythwr_init](Files/init_8h.md#function-llwythwr-init)**([llwythwr_os_api](Files/darwin_8h.md#typedef-llwythwr-os-api) * os_api)<br>Auxiliary function to initialise an os_api struct for testing.  |

-------------------------------
## Functions Documentation



### function memfd_create_wrapper

```cpp
static int memfd_create_wrapper(
    const char * name,
    unsigned int flags
)
```



### function llwythwr_init

```cpp
int llwythwr_init(
    llwythwr_os_api * os_api
)
```

Auxiliary function to initialise an os_api struct for testing. 

**Parameters**: 

  * **os_api** The api struct to initialize. 

**Return**: Zero on success 

Very likely **NOT** what you want to use as OPSEC is poor!!! 

-------------------------------
## Source code

```cpp
#ifndef LIBLOADER_INCLUDE_LIBLOADER_DEFAULT_API_H_
#define LIBLOADER_INCLUDE_LIBLOADER_DEFAULT_API_H_

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
  char cRtlInitUnicodeString[] = {'R', 't', 'l', 'I', 'n', 'i', 't', 'U', 'n', 'i', 'c', 'o', 'd', 'e', 'S', 't', 'r', 'i', 'n', 'g', '\0'};
  os_api->RtlInitUnicodeString = (Type_RtlInitUnicodeString) GetProcAddress(ntdll, cRtlInitUnicodeString);
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

#endif //LIBLOADER_INCLUDE_LIBLOADER_DEFAULT_API_H_
```

-------------------------------
