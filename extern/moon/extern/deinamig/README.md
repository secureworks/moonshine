
# Deinamig

Deinamig is a library with the goal of providing functionality to mask the use of certain functions within the Windows API.

A couple of different approaches are provided to accomplish this goal.

* Reimplementation of common Windows API functions such as LoadLibrary and GetModuleHandle, allowing module handles and addresses of exported functions to be retrieved covertly either by name or a hashing scheme.
* Execution of functional equivalent assembly stubs allowing the user to make system calls directly to the kernel. Both direct and indirect variations of this technique are provided. Functions are also provided to retrieve system call numbers and addresses, and to create suitable stubs.
* Functions to find and extract syscall numbers and/or stubs from unloaded modules which has been read from disk or provided by other means.

The intention is for the library to be either referenced directly within toolsets or for specific functionally to be easily extracted and embedded within toolsets.

References:

* [https://www.forrest-orr.net/post/malicious-memory-artifacts-part-i-dll-hollowing](https://www.forrest-orr.net/post/malicious-memory-artifacts-part-i-dll-hollowing)
* [https://github.com/forrest-orr/phantom-dll-hollower-poc](https://github.com/forrest-orr/phantom-dll-hollower-poc)
* [https://www.secforce.com/blog/dll-hollowing-a-deep-dive-into-a-stealthier-memory-allocation-variant/](https://www.secforce.com/blog/dll-hollowing-a-deep-dive-into-a-stealthier-memory-allocation-variant/)
* [https://github.com/cobbr/SharpSploit/blob/master/SharpSploit/Execution/ManualMap/Map.cs](https://github.com/cobbr/SharpSploit/blob/master/SharpSploit/Execution/ManualMap/Map.cs)
* [https://github.com/cobbr/SharpSploit/blob/master/SharpSploit/Execution/ManualMap/Overload.cs](https://github.com/cobbr/SharpSploit/blob/master/SharpSploit/Execution/ManualMap/Overload.cs)
* [https://github.com/TheWover/donut/blob/master/loader/inmem_pe.c](https://github.com/TheWover/donut/blob/master/loader/inmem_pe.c)
* [https://www.unknowncheats.me/forum/programming-for-beginners/349013-eject-mapped-dll.html#post2538435](https://www.unknowncheats.me/forum/programming-for-beginners/349013-eject-mapped-dll.html#post2538435)
* [http://phrack.org/issues/62/12.html](http://phrack.org/issues/62/12.html)
* [https://www.mdsec.co.uk/2021/06/bypassing-image-load-kernel-callbacks/](https://www.mdsec.co.uk/2021/06/bypassing-image-load-kernel-callbacks/)
* [https://github.com/DarthTon/Blackbone](https://github.com/DarthTon/Blackbone)

-------------------------------

# Library API
```
#include "deinamig/windows.h"
```

-------------------------------
## Types

|                | Name           |
| -------------- | -------------- |
| typedef unsigned long(*)(const char *string, unsigned long key) | **[deinamig_hash_function](Files/windows_8h.md#typedef-hash-function)** <br>Hash function typedef.  |

-------------------------------
## Functions

|                | Name           |
| -------------- | -------------- |
| DEINAMIG_EXPORT void * | **[deinamig_load_library_from_disk](Files/windows_8h.md#function-deinamig-load-library-from-disk)**(const char * filename)<br>Resolves LoadLibraryA and uses that function to load a DLL from disk.  |
| DEINAMIG_EXPORT void | **[deinamig_unload_library_from_disk](Files/windows_8h.md#function-deinamig-unload-library-from-disk)**(void * handle)<br>Resolves FreeLibrary and uses that function to free the loaded library.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_native_export_address](Files/windows_8h.md#function-deinamig-get-native-export-address)**(void * module_base, const char * export_name)<br>Given a library base address, resolve the address of a function by calling GetProcAddress.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_library_address_by_name](Files/windows_8h.md#function-deinamig-get-library-address-by-name)**(const char * filename, const char * function_name, int load_from_disk, int resolve_forwards)<br>Helper for getting the pointer to a function from a DLL loaded by the process.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_library_address_by_hash](Files/windows_8h.md#function-deinamig-get-library-address-by-hash)**(const char * filename, unsigned long function_hash, unsigned long key, [deinamig_hash_function](Files/windows_8h.md#typedef-hash-function) deinamig_hash_function, int load_from_disk, int resolve_forwards)<br>Helper for getting the pointer to a function from a DLL loaded by the process.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_peb_ldr_library_entry_by_name](Files/windows_8h.md#function-deinamig-get-peb-ldr-library-entry-by-name)**(const char * filename)<br>Helper for getting the base address of a library loaded by the current process.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_peb_ldr_library_entry_by_hash](Files/windows_8h.md#function-deinamig-get-peb-ldr-library-entry-by-hash)**(unsigned long library_hash, unsigned long key, [deinamig_hash_function](Files/windows_8h.md#typedef-hash-function) deinamig_hash_function)<br>Helper for getting the base address of a library loaded by the current process.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_export_address_by_name](Files/windows_8h.md#function-deinamig-get-export-address-by-name)**(void * module_base, const char * export_name, int resolve_forwards, int load_from_disk)<br>Given a library base address, resolve the address of a function by manually walking the library export table.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_export_address_by_hash](Files/windows_8h.md#function-deinamig-get-export-address-by-hash)**(void * module_base, unsigned long function_hash, unsigned long key, [deinamig_hash_function](Files/windows_8h.md#typedef-hash-function) deinamig_hash_function, int resolve_forwards, int load_from_disk)<br>Given a library base address, resolve the address of a function by manually walking the library export table.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_forward_address](Files/windows_8h.md#function-deinamig-get-forward-address)**(void * export_address, int load_from_disk)<br>Check if an address to an exported function should be resolved to a forward.  |
| DEINAMIG_EXPORT unsigned long | **[deinamig_get_syscall_number_syswhispers2_by_hash](Files/windows_8h.md#function-deinamig-get-syscall-number-syswhispers2-by-hash)**(void * ntdll_base_address, unsigned long function_hash, unsigned long key, [deinamig_hash_function](Files/windows_8h.md#typedef-hash-function) deinamig_hash_function)<br>Obtain the syscall number for an Nt* function by sorting the address of system calls in ascending order using NTDLL in memory.  |
| DEINAMIG_EXPORT unsigned long | **[deinamig_get_syscall_number_hellsgate](Files/windows_8h.md#function-deinamig-get-syscall-number-hellsgate)**(void * function_address, int try_halo)<br>Obtain the syscall number for an Nt* function by searching for unique instruction patterns related to syscall invocation.  |
| DEINAMIG_EXPORT unsigned long | **[deinamig_get_syscall_number_runtbl_by_hash](Files/windows_8h.md#function-deinamig-get-syscall-number-runtbl-by-hash)**(void * ntdll_base_address, unsigned long function_hash, unsigned long key, [deinamig_hash_function](Files/windows_8h.md#typedef-hash-function) deinamig_hash_function)<br>Obtain the syscall number for an Nt* function by cross-referencing entries in the runtime function table and export address table.  |
| DEINAMIG_EXPORT unsigned long | **[deinamig_get_syscall_number_runtbl_by_name](Files/windows_8h.md#function-deinamig-get-syscall-number-runtbl-by-name)**(void * ntdll_base_address, const char * export_name)<br>Obtain the syscall number for an Nt* function by cross-referencing entries in the runtime function table and export address table.  |
| DEINAMIG_EXPORT int | **[deinamig_get_syscall_number_ldrp_thunk_signature](Files/windows_8h.md#function-deinamig-get-syscall-number-ldrp-thunk-signature)**(PDWORD syscall_NtOpenFile, PDWORD syscall_NtOpenSection, PDWORD syscall_NtCreateSection, PDWORD syscall_NtMapViewOfSection, PDWORD syscall_NtQueryAttributesFile)<br>Obtain the syscall numbers for a five critical Nt* functions by searching syscall stubs created by Windows for detour detection during parallel DLL loading.  |
| DEINAMIG_EXPORT unsigned char * | **[deinamig_build_direct_syscall_stub](Files/windows_8h.md#function-deinamig-build-direct-syscall-stub)**(unsigned char * stub_region, unsigned long syscall_number, int * stub_size)<br>Generate a byte array containing instructions for invoking a syscall directly.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_syscall_address](Files/windows_8h.md#function-deinamig-get-syscall-address)**(void * function_address, int try_halo)<br>Obtain a pointer to a syscall instruction that could be used for indirect invocation of a syscall.  |
| DEINAMIG_EXPORT unsigned char * | **[deinamig_build_indirect_syscall_stub](Files/windows_8h.md#function-deinamig-build-indirect-syscall-stub)**(unsigned char * stub_region, unsigned long syscall_number, void * syscall_address, int * stub_size)<br>Generate a byte array containing instructions for invoking a syscall indirectly.  |
| DEINAMIG_EXPORT unsigned char * | **[deinamig_carve_syscall_stub](Files/windows_8h.md#function-deinamig-carve-syscall-stub)**(void * start_address, int carve_amount, unsigned char * stub_region)<br>Extract a series of instructions from memory and place in a byte array.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_export_address_from_raw_by_hash](Files/windows_8h.md#function-deinamig-get-export-address-from-raw-by-hash)**(void * module_base, unsigned long function_hash, unsigned long key, [deinamig_hash_function](Files/windows_8h.md#typedef-hash-function) deinamig_hash_function)<br>Given an unloaded library base address, resolve the address of a function by manually walking the library export table.  |
| DEINAMIG_EXPORT void * | **[deinamig_get_export_address_from_raw_by_name](Files/windows_8h.md#function-deinamig-get-export-address-from-raw-by-name)**(void * module_base, const char * export_name)<br>Given an unloaded library base address, resolve the address of a function by manually walking the library export table.  |

-------------------------------
## Types Documentation



### typedef deinamig_hash_function

```cpp
typedef unsigned long(* deinamig_hash_function) (const char *string, unsigned long key);
```

Hash function typedef. 

**Parameters**: 

  * **string** String to hash. 
  * **key** 64-bit integer to initialize the hash. 

**Return**: 64-bit integer representing the hash result. 

-------------------------------
## Functions Documentation



### function deinamig_load_library_from_disk

```cpp
DEINAMIG_EXPORT void * deinamig_load_library_from_disk(
    const char * filename
)
```

Resolves LoadLibraryA and uses that function to load a DLL from disk. 

**Parameters**: 

  * **filename** The path to the DLL on disk. Uses the LoadLibrary convention. 

**Return**: The base address of the loaded library or NULL if the library was not loaded successfully. 



### function deinamig_unload_library_from_disk

```cpp
DEINAMIG_EXPORT void deinamig_unload_library_from_disk(
    void * handle
)
```

Resolves FreeLibrary and uses that function to free the loaded library. 

**Parameters**: 

  * **handle** The base address of the loaded library library. 



### function deinamig_get_native_export_address

```cpp
DEINAMIG_EXPORT void * deinamig_get_native_export_address(
    void * module_base,
    const char * export_name
)
```

Given a library base address, resolve the address of a function by calling GetProcAddress. 

**Parameters**: 

  * **module_base** A pointer to the base address where the library is loaded in the current process. 
  * **export_name** The name or ordinal of the export to search for (e.g. "NtAlertResumeThread"). 

**Return**: Pointer to the desired function. 



### function deinamig_get_library_address_by_name

```cpp
DEINAMIG_EXPORT void * deinamig_get_library_address_by_name(
    const char * filename,
    const char * function_name,
    int load_from_disk,
    int resolve_forwards
)
```

Helper for getting the pointer to a function from a DLL loaded by the process. 

**Parameters**: 

  * **filename** The name of the DLL (e.g. "ntdll.dll" or "C:\Windows\System32\ntdll.dll"). 
  * **function_name** Name or ordinal of the exported procedure. 
  * **load_from_disk** Indicates if the function can try to load the DLL from disk if it is not found in the loaded module list. 
  * **resolve_forwards** Whether or not to resolve export forwards. 

**Return**: Pointer to the desired function, or NULL if unsuccessful. 



### function deinamig_get_library_address_by_hash

```cpp
DEINAMIG_EXPORT void * deinamig_get_library_address_by_hash(
    const char * filename,
    unsigned long function_hash,
    unsigned long key,
    deinamig_hash_function deinamig_hash_function,
    int load_from_disk,
    int resolve_forwards
)
```

Helper for getting the pointer to a function from a DLL loaded by the process. 

**Parameters**: 

  * **filename** The name of the DLL (e.g. "ntdll.dll" or "C:\Windows\System32\ntdll.dll"). 
  * **function_hash** Hash of the exported procedure. 
  * **key** 64-bit integer to initialize the keyed hash. 
  * **deinamig_hash_function** The hash function to use. 
  * **load_from_disk** Indicates if the function can try to load the DLL from disk if it is not found in the loaded module list. 
  * **resolve_forwards** Whether or not to resolve export forwards. 

**Return**: Pointer to the desired function, or NULL if unsuccessful. 



### function deinamig_get_peb_ldr_library_entry_by_name

```cpp
DEINAMIG_EXPORT void * deinamig_get_peb_ldr_library_entry_by_name(
    const char * filename
)
```

Helper for getting the base address of a library loaded by the current process. 

**Parameters**: 

  * **filename** The name of the DLL (e.g. "ntdll.dll"). 

**Return**: The base address of the loaded library or NULL if the module was not loaded successfully. 



### function deinamig_get_peb_ldr_library_entry_by_hash

```cpp
DEINAMIG_EXPORT void * deinamig_get_peb_ldr_library_entry_by_hash(
    unsigned long library_hash,
    unsigned long key,
    deinamig_hash_function deinamig_hash_function
)
```

Helper for getting the base address of a library loaded by the current process. 

**Parameters**: 

  * **library_hash** Hash of the library name. IMPORTANT: Library name must have been in lowercase and have .dll suffix. 
  * **key** 64-bit integer to initialize the keyed hash. 
  * **deinamig_hash_function** The hash function to use. 

**Return**: The base address of the loaded library or NULL if the module was not loaded successfully. 



### function deinamig_get_export_address_by_name

```cpp
DEINAMIG_EXPORT void * deinamig_get_export_address_by_name(
    void * module_base,
    const char * export_name,
    int resolve_forwards,
    int load_from_disk
)
```

Given a library base address, resolve the address of a function by manually walking the library export table. 

**Parameters**: 

  * **module_base** A pointer to the base address where the library is loaded in the current process. 
  * **export_name** The name or ordinal of the export to search for (e.g. "NtAlertResumeThread"). 
  * **resolve_forwards** Whether or not to resolve export forwards. 
  * **load_from_disk** Indicates if the function can try to load a DLL containing a forwarded export from disk if it is not found in the loaded module list. 

**Return**: Pointer to the desired function. 



### function deinamig_get_export_address_by_hash

```cpp
DEINAMIG_EXPORT void * deinamig_get_export_address_by_hash(
    void * module_base,
    unsigned long function_hash,
    unsigned long key,
    deinamig_hash_function deinamig_hash_function,
    int resolve_forwards,
    int load_from_disk
)
```

Given a library base address, resolve the address of a function by manually walking the library export table. 

**Parameters**: 

  * **module_base** A pointer to the base address where the module is loaded in the current process. 
  * **function_hash** Hash of the exported procedure. 
  * **key** 64-bit integer to initialize the keyed hash. 
  * **deinamig_hash_function** The hash function to use. 
  * **resolve_forwards** Whether or not to resolve export forwards. 
  * **load_from_disk** Indicates if the function can try to load a DLL containing a forwarded export from disk if it is not found in the loaded module list. 

**Return**: Pointer to the desired function. 



### function deinamig_get_forward_address

```cpp
DEINAMIG_EXPORT void * deinamig_get_forward_address(
    void * export_address,
    int load_from_disk
)
```

Check if an address to an exported function should be resolved to a forward. 

**Parameters**: 

  * **export_address** Function of an exported address, found by parsing a PE file's export table. 
  * **load_from_disk** Indicates if the function can try to load the DLL from disk if it is not found in the loaded library list. 

**Return**: Pointer to the forward. If the function is not forwarded, return the original pointer. 

If so, return the address of the forward. 



### function deinamig_get_syscall_number_syswhispers2_by_hash

```cpp
DEINAMIG_EXPORT unsigned long deinamig_get_syscall_number_syswhispers2_by_hash(
    void * ntdll_base_address,
    unsigned long function_hash,
    unsigned long key,
    deinamig_hash_function deinamig_hash_function
)
```

Obtain the syscall number for an Nt* function by sorting the address of system calls in ascending order using NTDLL in memory. 

**Parameters**: 

  * **ntdll_base_address** A pointer to the base address of ntdll.dll loaded in memory. 
  * **function_hash** Hash of the exported procedure. 
  * **key** 64-bit integer to initialize the keyed hash. 
  * **deinamig_hash_function** The hash function to use. 

**Return**: The syscall number. 

Reference: [https://github.com/jthuraisamy/SysWhispers2](https://github.com/jthuraisamy/SysWhispers2) Reference: [https://github.com/crummie5/FreshyCalls](https://github.com/crummie5/FreshyCalls)



### function deinamig_get_syscall_number_hellsgate

```cpp
DEINAMIG_EXPORT unsigned long deinamig_get_syscall_number_hellsgate(
    void * function_address,
    int try_halo
)
```

Obtain the syscall number for an Nt* function by searching for unique instruction patterns related to syscall invocation. 

**Parameters**: 

  * **function_address** A start address for the search, usually the targeted function address. 
  * **try_halo** If the instruction pattern could not be found, search neighbouring functions and deduce syscall number from distance to origin. 

**Return**: The syscall number. 

Reference: [https://github.com/vxunderground/VXUG-Papers/tree/main/Hells%20Gate](https://github.com/vxunderground/VXUG-Papers/tree/main/Hells%20Gate)



### function deinamig_get_syscall_number_runtbl_by_hash

```cpp
DEINAMIG_EXPORT unsigned long deinamig_get_syscall_number_runtbl_by_hash(
    void * ntdll_base_address,
    unsigned long function_hash,
    unsigned long key,
    deinamig_hash_function deinamig_hash_function
)
```

Obtain the syscall number for an Nt* function by cross-referencing entries in the runtime function table and export address table. 

**Parameters**: 

  * **ntdll_base_address** A pointer to the base address of ntdll.dll loaded in memory. 
  * **function_hash** Hash of the exported procedure. 
  * **key** 64-bit integer to initialize the keyed hash. 
  * **deinamig_hash_function** The hash function to use. 

**Return**: The syscall number. 

Reference: [https://www.mdsec.co.uk/2022/04/resolving-system-service-numbers-using-the-exception-directory/](https://www.mdsec.co.uk/2022/04/resolving-system-service-numbers-using-the-exception-directory/)



### function deinamig_get_syscall_number_runtbl_by_name

```cpp
DEINAMIG_EXPORT unsigned long deinamig_get_syscall_number_runtbl_by_name(
    void * ntdll_base_address,
    const char * export_name
)
```

Obtain the syscall number for an Nt* function by cross-referencing entries in the runtime function table and export address table. 

**Parameters**: 

  * **ntdll_base_address** A pointer to the base address of ntdll.dll loaded in memory. 
  * **export_name** The name or ordinal of the export to search for (e.g. "NtAlertResumeThread"). 

**Return**: The syscall number. 

Reference: [https://www.mdsec.co.uk/2022/04/resolving-system-service-numbers-using-the-exception-directory/](https://www.mdsec.co.uk/2022/04/resolving-system-service-numbers-using-the-exception-directory/)



### function deinamig_get_syscall_number_ldrp_thunk_signature

```cpp
DEINAMIG_EXPORT int deinamig_get_syscall_number_ldrp_thunk_signature(
    PDWORD syscall_NtOpenFile,
    PDWORD syscall_NtOpenSection,
    PDWORD syscall_NtCreateSection,
    PDWORD syscall_NtMapViewOfSection,
    PDWORD syscall_NtQueryAttributesFile
)
```

Obtain the syscall numbers for a five critical Nt* functions by searching syscall stubs created by Windows for detour detection during parallel DLL loading. 

**Parameters**: 

  * **syscall_NtOpenFile** Retrieved syscall value for the NtOpenFile function. 
  * **syscall_NtOpenSection** Retrieved syscall value for the NtOpenSection function. 
  * **syscall_NtCreateSection** Retrieved syscall value for the NtCreateSection function. 
  * **syscall_NtMapViewOfSection** Retrieved syscall value for the NtMapViewOfSection function. 
  * **syscall_NtQueryAttributesFile** Retrieved syscall value for the NtQueryAttributesFile function. 

**Return**: TRUE on success, FALSE on failure. 

Reference: [https://www.mdsec.co.uk/2022/01/edr-parallel-asis-through-analysis/](https://www.mdsec.co.uk/2022/01/edr-parallel-asis-through-analysis/) Reference: [https://github.com/hlldz/RefleXXion](https://github.com/hlldz/RefleXXion)



### function deinamig_build_direct_syscall_stub

```cpp
DEINAMIG_EXPORT unsigned char * deinamig_build_direct_syscall_stub(
    unsigned char * stub_region,
    unsigned long syscall_number,
    int * stub_size
)
```

Generate a byte array containing instructions for invoking a syscall directly. 

**Parameters**: 

  * **stub_region** A pre-allocated array to store the stub. Set to NULL and call to ascertain the stub size required. 
  * **syscall_number** A syscall number to embed within the stub. 
  * **stub_size** Returns the size of the stub, or pass NULL to ignore. 

**Return**: A pointer to the stub. 



### function deinamig_get_syscall_address

```cpp
DEINAMIG_EXPORT void * deinamig_get_syscall_address(
    void * function_address,
    int try_halo
)
```

Obtain a pointer to a syscall instruction that could be used for indirect invocation of a syscall. 

**Parameters**: 

  * **function_address** A start address for the search, usually the targeted function address. 
  * **try_halo** If the instruction could not be found, search neighbouring functions for a syscall instruction. 

**Return**: 



### function deinamig_build_indirect_syscall_stub

```cpp
DEINAMIG_EXPORT unsigned char * deinamig_build_indirect_syscall_stub(
    unsigned char * stub_region,
    unsigned long syscall_number,
    void * syscall_address,
    int * stub_size
)
```

Generate a byte array containing instructions for invoking a syscall indirectly. 

**Parameters**: 

  * **stub_region** A pre-allocated array to store the stub. Set to NULL and call to ascertain the stub size required. 
  * **syscall_number** A syscall number to embed within the stub. 
  * **syscall_address** An address of a syscall instruction to jmp to. 
  * **stub_size** Returns the size of the stub, or pass NULL to ignore. 

**Return**: A pointer to the stub. 



### function deinamig_carve_syscall_stub

```cpp
DEINAMIG_EXPORT unsigned char * deinamig_carve_syscall_stub(
    void * start_address,
    int carve_amount,
    unsigned char * stub_region
)
```

Extract a series of instructions from memory and place in a byte array. 

**Parameters**: 

  * **start_address** The start address. 
  * **carve_amount** The amount of bytes to extract from the start address. 
  * **stub_region** A pre-allocated array to hold the extracted instructions. 

**Return**: 



### function deinamig_get_export_address_from_raw_by_hash

```cpp
DEINAMIG_EXPORT void * deinamig_get_export_address_from_raw_by_hash(
    void * module_base,
    unsigned long function_hash,
    unsigned long key,
    deinamig_hash_function deinamig_hash_function
)
```

Given an unloaded library base address, resolve the address of a function by manually walking the library export table. 

**Parameters**: 

  * **module_base** A pointer to the start address of an unloaded library obtained by reading from disk or other means. 
  * **function_hash** Hash of the exported procedure. 
  * **key** 64-bit integer to initialize the keyed hash. 
  * **deinamig_hash_function** The hash function to use. 

**Return**: Pointer to the desired function. 



### function deinamig_get_export_address_from_raw_by_name

```cpp
DEINAMIG_EXPORT void * deinamig_get_export_address_from_raw_by_name(
    void * module_base,
    const char * export_name
)
```

Given an unloaded library base address, resolve the address of a function by manually walking the library export table. 

**Parameters**: 

  * **module_base** A pointer to the start address of an unloaded library obtained by reading from disk or other means. 
  * **export_name** The name or ordinal of the export to search for (e.g. "NtAlertResumeThread"). 

**Return**: Pointer to the desired function. 

-------------------------------
## Source code

```cpp
#ifndef LIBLOADER_SRC_LIBLOADER_WINDOWS_H_
#define LIBLOADER_SRC_LIBLOADER_WINDOWS_H_

#include <stdint.h>
#include <windows.h>
#include <winternl.h>
#include <deinamig/export.h>

typedef unsigned long (*deinamig_hash_function)(const char* string, unsigned long key);

DEINAMIG_EXPORT
void* deinamig_load_library_from_disk(const char* filename);

DEINAMIG_EXPORT
void deinamig_unload_library_from_disk(void* handle);

DEINAMIG_EXPORT
void* deinamig_get_native_export_address(void* module_base, const char* export_name);

DEINAMIG_EXPORT
void* deinamig_get_library_address_by_name(const char* filename, const char* function_name, int load_from_disk, int resolve_forwards);

DEINAMIG_EXPORT
void* deinamig_get_library_address_by_hash(const char* filename, unsigned long function_hash, unsigned long key, deinamig_hash_function deinamig_hash_function, int load_from_disk, int resolve_forwards);

DEINAMIG_EXPORT
void* deinamig_get_peb_ldr_library_entry_by_name(const char* filename);

DEINAMIG_EXPORT
void* deinamig_get_peb_ldr_library_entry_by_hash(unsigned long library_hash, unsigned long key, deinamig_hash_function deinamig_hash_function);

DEINAMIG_EXPORT
void* deinamig_get_export_address_by_name(void* module_base, const char* export_name, int resolve_forwards, int load_from_disk);

DEINAMIG_EXPORT
void* deinamig_get_export_address_by_hash(void* module_base, unsigned long function_hash, unsigned long key, deinamig_hash_function deinamig_hash_function, int resolve_forwards, int load_from_disk);

DEINAMIG_EXPORT
void* deinamig_get_forward_address(void* export_address, int load_from_disk);

DEINAMIG_EXPORT
unsigned long deinamig_get_syscall_number_syswhispers2_by_hash(void* ntdll_base_address, unsigned long function_hash, unsigned long key, deinamig_hash_function deinamig_hash_function);

DEINAMIG_EXPORT
unsigned long deinamig_get_syscall_number_hellsgate(void* function_address, int try_halo);

DEINAMIG_EXPORT
unsigned long deinamig_get_syscall_number_runtbl_by_hash(void* ntdll_base_address, unsigned long function_hash, unsigned long key, deinamig_hash_function deinamig_hash_function);

DEINAMIG_EXPORT
unsigned long deinamig_get_syscall_number_runtbl_by_name(void* ntdll_base_address, const char* export_name);

DEINAMIG_EXPORT
int deinamig_get_syscall_number_ldrp_thunk_signature(PDWORD syscall_NtOpenFile, PDWORD syscall_NtOpenSection, PDWORD syscall_NtCreateSection, PDWORD syscall_NtMapViewOfSection, PDWORD syscall_NtQueryAttributesFile);

DEINAMIG_EXPORT
unsigned char* deinamig_build_direct_syscall_stub(unsigned char* stub_region, unsigned long syscall_number, int* stub_size);

DEINAMIG_EXPORT
void* deinamig_get_syscall_address(void* function_address, int try_halo);

DEINAMIG_EXPORT
unsigned char* deinamig_build_indirect_syscall_stub(unsigned char* stub_region, unsigned long syscall_number, void* syscall_address, int* stub_size);

DEINAMIG_EXPORT
unsigned char* deinamig_carve_syscall_stub(void* start_address, int carve_amount, unsigned char* stub_region);

DEINAMIG_EXPORT
void *deinamig_get_export_address_from_raw_by_hash(void *module_base, unsigned long function_hash, unsigned long key, deinamig_hash_function deinamig_hash_function);

DEINAMIG_EXPORT
void *deinamig_get_export_address_from_raw_by_name(void *module_base, const char *export_name);

#endif //LIBLOADER_SRC_LIBLOADER_WINDOWS_H_
```

-------------------------------
