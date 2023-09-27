#ifndef LLWYTHWR_INCLUDE_LLWYTHWR_WINDOWS_H_
#define LLWYTHWR_INCLUDE_LLWYTHWR_WINDOWS_H_

#include <windows.h>
#include <winternl.h>
#include <llwythwr/export.h>

/*
 * Typedef
 */

/**
 * Hash function typedef
 * @param string        String to hash.
 * @param key           64-bit integer to initialize the hash.
 * @param user_data     User supplied data provided when creating the module.
 * @return  64-bit integer representing the hash result.
 */
typedef unsigned long (*llwythwr_hash_function)(const char* string, unsigned long key, void* user_data);

/**
 * Get pointer to library function typedef
 * @param name          The name of the DLL (e.g. "ntdll.dll" or "C:\Windows\System32\ntdll.dll").
 * @param user_data     User supplied data provided when creating the module.
 * @return  The base address of the loaded module or NULL if the module was not loaded successfully.
 */
typedef void* (*llwythwr_get_lib_address_function)(const char* name, void* user_data);

/**
 * Get pointer to a function function typedef
 * @param lib           The name of the loaded library module (e.g. "ntdll.dll" or "C:\Windows\System32\ntdll.dll").
 * @param handle        The base address of a loaded library module.
 * @param func          Name of the exported procedure.
 * @param ordinal       Ordinal of the exported procedure.
 * @param user_data     User supplied data provided when creating the module.
 * @return  Pointer to the desired function, or NULL if unsuccessful.
 */
typedef void* (*llwythwr_get_proc_address_function)(const char* lib, void* handle, const char* func, int ordinal, void* user_data);

/**
 * Free a module function typedef
 * @param name          The name of the loaded library module (e.g. "ntdll.dll" or "C:\Windows\System32\ntdll.dll").
 * @param handle        The base address of a loaded library module.
 * @param user_data     User supplied data provided when creating the module.
 */
typedef void (*llwythwr_free_library_function)(const char* name, void* handle, void* user_data);


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

/**
 * Struct containing function pointers to operating system API functions that are used by llwythwr.
 * This will need to be populated manually with pointers to valid functions with equivalent functionality.
 * An example initialisation routine can be found within the init.h header, although it is probably not a good idea to use it as-is if OPSEC is a concern!
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
  Type_RtlHashUnicodeString RtlHashUnicodeString;
  Type_RtlRbInsertNodeEx RtlRbInsertNodeEx;
  Type_RtlRbRemoveNode RtlRbRemoveNode;
  Type_NtOpenFile NtOpenFile;
  Type_NtCreateSection NtCreateSection;
  Type_NtMapViewOfSection NtMapViewOfSection;
  Type_NtUnmapViewOfSection NtUnmapViewOfSection;
  Type_NtQuerySystemTime NtQuerySystemTime;
} llwythwr_windows_api, llwythwr_os_api;

/**
 * Struct representing a library created by llwythwr.
 */
typedef struct llwythwr_windows_library llwythwr_windows_library, llwythwr_library;

/*
 * Module Loading
 */

/**
 * Manually map a PE into current process using NtCreateSection and NtMapViewOfSection, then process the PE tables accordingly.
 * @param api                       Operating System API abstraction struct.
 * @param module_base               Pointer to the module base.
 * @param x_get_lib_address         Function to call to load a dependant module during IAT processing
 * @param x_get_proc_address        Function to call to resolve the address of a function during IAT processing
 * @param x_free_library_function   Function to call to unload a dependant module when unloading the module
 * @param user_data                 User supplied data that is passed as a parameter on callbacks
 * @return  Manually mapped library, or NULL if an error was encountered.
 */
LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_map_library(llwythwr_windows_api *api, const void* module_base, llwythwr_get_lib_address_function x_get_lib_address, llwythwr_get_proc_address_function x_get_proc_address, llwythwr_free_library_function x_free_library_function, void* user_data);

/**
 * Copy a PE into private memory allocated for the current process, then process the PE tables accordingly.
 * @param api                       Operating System API abstraction struct.
 * @param module_base               Pointer to the module base.
 * @param x_get_lib_address         Function to call to load a dependant module during IAT processing
 * @param x_get_proc_address        Function to call to resolve the address of a function during IAT processing
 * @param x_free_library_function   Function to call to unload a dependant module when unloading the module
 * @param user_data                 User supplied data that is passed as a parameter on callbacks
 * @return  Manually mapped library, or NULL if an error was encountered.
 */
LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_alloc_library(llwythwr_windows_api *api, const void* module_base, llwythwr_get_lib_address_function x_get_lib_address, llwythwr_get_proc_address_function x_get_proc_address, llwythwr_free_library_function x_free_library_function, void* user_data);

/**
 * Manually map an existing library into the current process using NtOpenFile, NtCreateSection and NtMapViewOfSection and overlay a PE on top. PE tables are also processed accordingly.
 * @param api                       Operating System API abstraction struct.
 * @param module_base               Pointer to the module base.
 * @param decoy_filepath            Path to decoy library
 * @param keep_decoy_headers        Keep decoy headers intact
 * @param x_get_lib_address         Function to call to load a dependant module during IAT processing
 * @param x_get_proc_address        Function to call to resolve the address of a function during IAT processing
 * @param x_free_library_function   Function to call to unload a dependant module when unloading the module
 * @param user_data                 User supplied data that is passed as a parameter on callbacks
 * @return  Manually mapped library, or NULL if an error was encountered.
 */
LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_map_hollow_library(llwythwr_windows_api *api, const void* module_base, const char* decoy_filepath, BOOL keep_decoy_headers, llwythwr_get_lib_address_function x_get_lib_address, llwythwr_get_proc_address_function x_get_proc_address, llwythwr_free_library_function x_free_library_function, void* user_data);

/**
 * Overlay a PE on top of an existing library that has been loaded with LoadLibraryEx and the DONT_RESOLVE_DLL_REFERENCES flag, then process the PE tables accordingly.
 * @param api                       Operating System API abstraction struct.
 * @param module_base               Pointer to the module base.
 * @param decoy                     Valid handle to the decoy library
 * @param keep_decoy_headers        Keep decoy headers intact
 * @param x_get_lib_address         Function to call to load a dependant module during IAT processing
 * @param x_get_proc_address        Function to call to resolve the address of a function during IAT processing
 * @param x_free_library_function   Function to call to unload a dependant module when unloading the module
 * @param user_data                 User supplied data that is passed as a parameter on callbacks
 * @return  Manually mapped library, or NULL if an error was encountered.
 */
LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_load_hollow_library(llwythwr_windows_api *api, const void* module_base, HMODULE decoy, BOOL keep_decoy_headers, llwythwr_get_lib_address_function x_get_lib_address, llwythwr_get_proc_address_function x_get_proc_address, llwythwr_free_library_function x_free_library_function, void* user_data);


/**
 * Maps a PE from disk into a Section using NtOpenFile, NtCreateSection and NtMapViewOfSection.
 * Important: Processing of the various PE tables (import, delayed import, exception, tls etc) will not been performed!
 * @param api       Operating System API abstraction struct.
 * @param filename  Full path fo the DLL on disk.
 * @return  Manually mapped library, or NULL if an error was encountered.
 */
LLWYTHWR_EXPORT
llwythwr_windows_library* llwythwr_map_library_from_disk_to_section(llwythwr_windows_api *api, const char* filename);

/**
 * Free a library that was mapped into the current process.
 * @param library The manually mapped library.
 */
LLWYTHWR_EXPORT
void llwythwr_unload_library(llwythwr_windows_library *library);

/**
 * Wipe PE headers.
 * @param library The manually mapped library.
 */
LLWYTHWR_EXPORT
void llwythwr_wipe_headers(llwythwr_windows_library *library);

/**
 * Execute entry point, be it Main (EXE) or DllMain (DLL).
 * Important: calling this function on a EXE will not return, i.e. once the
 * loaded EXE finished running, the process will terminate.
 * @param library  The manually mapped library.
 * @return  A negative value if the entry point could not be executed.
 */
LLWYTHWR_EXPORT
int llwythwr_call_entry_point(llwythwr_windows_library *library);

/**
 * Given a library, resolve the address of a function using the provided export name.
 * @param library       The manually mapped library.
 * @param export_name   The name or ordinal of the export to search for (e.g. "NtAlertResumeThread").
 * @return  Pointer to the desired function, or NULL if not found.
 */
LLWYTHWR_EXPORT
void* llwythwr_get_export_address_by_name(llwythwr_windows_library *library, const char* export_name);

/**
 * Given a library, resolve the address of a function using the provided hash
 * @param library           The manually mapped library.
 * @param function_hash     Hash of the exported procedure.
 * @param key               64-bit integer to initialize the keyed hash.
 * @param hash_function     The hash function to use.
 * @return  Pointer to the desired function, or NULL if not found.
 */
LLWYTHWR_EXPORT
void* llwythwr_get_export_address_by_hash(llwythwr_windows_library *library, unsigned long function_hash, unsigned long key, llwythwr_hash_function hash_function);

/**
 * Given a library, link it within internal windows structures in an attempt to make it appear as a system loaded library.
 * @param library           The manually mapped library.
 * @param basename          The name the library appears to have e.g. "Legit.dll"
 * @param fullpath          The full path on disk that the library appears to be loaded from e.g. "C:\Windows\System32\Legit.dll"
 * @return  TRUE on success, FALSE on failure.
 */
LLWYTHWR_EXPORT
int llwythwr_link_library(llwythwr_windows_library* library, const char* basename, const char* fullpath);

/**
 * Given a library which has already been linked using "llwythwr_link_library", unlink it from internal windows structures.
 * @param library           The manually mapped library.
 * @return  TRUE on success, FALSE on failure.
 */
LLWYTHWR_EXPORT
int llwythwr_unlink_library(llwythwr_windows_library* library);

#endif //LLWYTHWR_INCLUDE_LLWYTHWR_WINDOWS_H_
