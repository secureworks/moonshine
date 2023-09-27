#ifndef DEINAMIG_INCLUDE_DEINAMIG_WINDOWS_H_
#define DEINAMIG_INCLUDE_DEINAMIG_WINDOWS_H_

#include <stdint.h>
#include <windows.h>
#include <winternl.h>
#include <deinamig/export.h>

/**
 * Hash function typedef
 * @param string    String to hash.
 * @param key       64-bit integer to initialize the hash.
 * @return  64-bit integer representing the hash result.
 */
typedef unsigned long (*deinamig_hash_function)(const char* string, unsigned long key);

/**
 * Resolves LoadLibraryA and uses that function to load a DLL from disk.
 * @param filename  The path to the DLL on disk. Uses the LoadLibrary convention.
 * @return  The base address of the loaded library or NULL if the library was not loaded successfully.
 */
DEINAMIG_EXPORT
void* deinamig_load_library_from_disk(const char* filename);

/**
 * Resolves FreeLibrary and uses that function to free the loaded library
 * @param handle    The base address of the loaded library library.
 */
DEINAMIG_EXPORT
void deinamig_unload_library_from_disk(void* handle);

/**
 * Given a library base address, resolve the address of a function by calling GetProcAddress.
 * @param module_base   A pointer to the base address where the library is loaded in the current process.
 * @param export_name   The name or ordinal of the export to search for (e.g. "NtAlertResumeThread").
 * @return  Pointer to the desired function.
 */
DEINAMIG_EXPORT
void* deinamig_get_native_export_address(void* module_base, const char* export_name);

/**
 * Helper for getting the pointer to a function from a DLL loaded by the process.
 * @param filename          The name of the DLL (e.g. "ntdll.dll" or "C:\Windows\System32\ntdll.dll").
 * @param function_name     Name or ordinal of the exported procedure.
 * @param load_from_disk    Indicates if the function can try to load the DLL from disk if it is not found in the loaded module list.
 * @param resolve_forwards  Whether or not to resolve export forwards.
 * @return  Pointer to the desired function, or NULL if unsuccessful.
 */
DEINAMIG_EXPORT
void* deinamig_get_library_address_by_name(const char* filename, const char* function_name, int load_from_disk, int resolve_forwards);

/**
 * Helper for getting the pointer to a function from a DLL loaded by the process.
 * @param filename          The name of the DLL (e.g. "ntdll.dll" or "C:\Windows\System32\ntdll.dll").
 * @param function_hash     Hash of the exported procedure.
 * @param key               64-bit integer to initialize the keyed hash.
 * @param hash_function     The hash function to use.
 * @param load_from_disk    Indicates if the function can try to load the DLL from disk if it is not found in the loaded module list.
 * @param resolve_forwards  Whether or not to resolve export forwards.
 * @return  Pointer to the desired function, or NULL if unsuccessful.
 */
DEINAMIG_EXPORT
void* deinamig_get_library_address_by_hash(const char* filename, unsigned long function_hash, unsigned long key, deinamig_hash_function hash_function, int load_from_disk, int resolve_forwards);

/**
 * Helper for getting the base address of a library loaded by the current process.
 * @param filename  The name of the DLL (e.g. "ntdll.dll").
 * @return  The base address of the loaded library or NULL if the module was not loaded successfully.
 */
DEINAMIG_EXPORT
void* deinamig_get_peb_ldr_library_entry_by_name(const char* filename);

/**
 * Helper for getting the base address of a library loaded by the current process.
 * @param library_hash      Hash of the library name. IMPORTANT: Library name must have been in lowercase and have .dll suffix.
 * @param key               64-bit integer to initialize the keyed hash.
 * @param hash_function     The hash function to use.
 * @return  The base address of the loaded library or NULL if the module was not loaded successfully.
 */
DEINAMIG_EXPORT
void* deinamig_get_peb_ldr_library_entry_by_hash(unsigned long library_hash, unsigned long key, deinamig_hash_function hash_function);

/**
 * Given a library base address, resolve the address of a function by manually walking the library export table.
 * @param module_base       A pointer to the base address where the library is loaded in the current process.
 * @param export_name       The name or ordinal of the export to search for (e.g. "NtAlertResumeThread").
 * @param resolve_forwards  Whether or not to resolve export forwards.
 * @param load_from_disk    Indicates if the function can try to load a DLL containing a forwarded export from disk if it is not found in the loaded module list.
 * @return  Pointer to the desired function.
 */
DEINAMIG_EXPORT
void* deinamig_get_export_address_by_name(void* module_base, const char* export_name, int resolve_forwards, int load_from_disk);


/**
 * Given a library base address, resolve the address of a function by manually walking the library export table.
 * @param module_base       A pointer to the base address where the module is loaded in the current process.
 * @param function_hash     Hash of the exported procedure.
 * @param key               64-bit integer to initialize the keyed hash.
 * @param hash_function     The hash function to use.
 * @param resolve_forwards  Whether or not to resolve export forwards.
 * @param load_from_disk    Indicates if the function can try to load a DLL containing a forwarded export from disk if it is not found in the loaded module list.
 * @return  Pointer to the desired function.
 */
DEINAMIG_EXPORT
void* deinamig_get_export_address_by_hash(void* module_base, unsigned long function_hash, unsigned long key, deinamig_hash_function hash_function, int resolve_forwards, int load_from_disk);

/**
 * Check if an address to an exported function should be resolved to a forward. If so, return the address of the forward.
 * @param export_address    Function of an exported address, found by parsing a PE file's export table.
 * @param load_from_disk    Indicates if the function can try to load the DLL from disk if it is not found in the loaded library list.
 * @return  Pointer to the forward. If the function is not forwarded, return the original pointer.
 */
DEINAMIG_EXPORT
void* deinamig_get_forward_address(void* export_address, int load_from_disk);

/**
 * Obtain the syscall number for an Nt* function by sorting the address of system calls in ascending order using NTDLL in memory.
 * Reference: https://github.com/jthuraisamy/SysWhispers2
 * Reference: https://github.com/crummie5/FreshyCalls
 * @param ntdll_base_address    A pointer to the base address of ntdll.dll loaded in memory.
 * @param function_hash         Hash of the exported procedure.
 * @param key                   64-bit integer to initialize the keyed hash.
 * @param hash_function         The hash function to use.
 * @return  The syscall number.
 */
DEINAMIG_EXPORT
unsigned long deinamig_get_syscall_number_syswhispers2_by_hash(void* ntdll_base_address, unsigned long function_hash, unsigned long key, deinamig_hash_function hash_function);

/**
 * Obtain the syscall number for an Nt* function by searching for unique instruction patterns related to syscall invocation.
 * Reference: https://github.com/vxunderground/VXUG-Papers/tree/main/Hells%20Gate
 * @param function_address      A start address for the search, usually the targeted function address.
 * @param try_halo              If the instruction pattern could not be found, search neighbouring functions and deduce syscall number from distance to origin.
 * @return  The syscall number.
 */
DEINAMIG_EXPORT
unsigned long deinamig_get_syscall_number_hellsgate(void* function_address, int try_halo);

/**
 * Obtain the syscall number for an Nt* function by cross-referencing entries in the runtime function table and export address table.
 * Reference: https://www.mdsec.co.uk/2022/04/resolving-system-service-numbers-using-the-exception-directory/
 * @param ntdll_base_address    A pointer to the base address of ntdll.dll loaded in memory.
 * @param function_hash         Hash of the exported procedure.
 * @param key                   64-bit integer to initialize the keyed hash.
 * @param hash_function         The hash function to use.
 * @return  The syscall number.
 */
DEINAMIG_EXPORT
unsigned long deinamig_get_syscall_number_runtbl_by_hash(void* ntdll_base_address, unsigned long function_hash, unsigned long key, deinamig_hash_function hash_function);

/**
 * Obtain the syscall number for an Nt* function by cross-referencing entries in the runtime function table and export address table.
 * Reference: https://www.mdsec.co.uk/2022/04/resolving-system-service-numbers-using-the-exception-directory/
 * @param ntdll_base_address    A pointer to the base address of ntdll.dll loaded in memory.
 * @param export_name       The name or ordinal of the export to search for (e.g. "NtAlertResumeThread").
 * @return  The syscall number.
 */
DEINAMIG_EXPORT
unsigned long deinamig_get_syscall_number_runtbl_by_name(void* ntdll_base_address, const char* export_name);

/**
 * Obtain the syscall numbers for a five critical Nt* functions by searching syscall stubs created by Windows for detour detection during parallel DLL loading.
 * Reference: https://www.mdsec.co.uk/2022/01/edr-parallel-asis-through-analysis/
 * Reference: https://github.com/hlldz/RefleXXion
 * @param syscall_NtOpenFile                    Retrieved syscall value for the NtOpenFile function.
 * @param syscall_NtOpenSection                 Retrieved syscall value for the NtOpenSection function.
 * @param syscall_NtCreateSection               Retrieved syscall value for the NtCreateSection function.
 * @param syscall_NtMapViewOfSection            Retrieved syscall value for the NtMapViewOfSection function.
 * @param syscall_NtQueryAttributesFile         Retrieved syscall value for the NtQueryAttributesFile function.
 * @return  TRUE on success, FALSE on failure.
 */
DEINAMIG_EXPORT
int deinamig_get_syscall_number_ldrp_thunk_signature(PDWORD syscall_NtOpenFile, PDWORD syscall_NtOpenSection, PDWORD syscall_NtCreateSection, PDWORD syscall_NtMapViewOfSection, PDWORD syscall_NtQueryAttributesFile);

/**
 * Generate a byte array containing instructions for invoking a syscall directly.
 * @param stub_region       A pre-allocated array to store the stub. Set to NULL and call to ascertain the stub size required.
 * @param syscall_number    A syscall number to embed within the stub.
 * @param stub_size         Returns the size of the stub, or pass NULL to ignore.
 * @return  A pointer to the stub.
 */
DEINAMIG_EXPORT
unsigned char* deinamig_build_direct_syscall_stub(unsigned char* stub_region, unsigned long syscall_number, int* stub_size);

/**
 * Obtain a pointer to a syscall instruction that could be used for indirect invocation of a syscall.
 * @param function_address  A start address for the search, usually the targeted function address.
 * @param try_halo          If the instruction could not be found, search neighbouring functions for a syscall instruction.
 * @return
 */
DEINAMIG_EXPORT
void* deinamig_get_syscall_address(void* function_address, int try_halo);

/**
 * Generate a byte array containing instructions for invoking a syscall indirectly.
 * @param stub_region       A pre-allocated array to store the stub. Set to NULL and call to ascertain the stub size required.
 * @param syscall_number    A syscall number to embed within the stub.
 * @param syscall_address   An address of a syscall instruction to jmp to.
 * @param stub_size         Returns the size of the stub, or pass NULL to ignore.
 * @return  A pointer to the stub.
 */
DEINAMIG_EXPORT
unsigned char* deinamig_build_indirect_syscall_stub(unsigned char* stub_region, unsigned long syscall_number, void* syscall_address, int* stub_size);

/**
 * Extract a series of instructions from memory and place in a byte array.
 * @param start_address     The start address.
 * @param carve_amount      The amount of bytes to extract from the start address.
 * @param stub_region       A pre-allocated array to hold the extracted instructions.
 * @return
 */
DEINAMIG_EXPORT
unsigned char* deinamig_carve_syscall_stub(void* start_address, int carve_amount, unsigned char* stub_region);

/**
 * Given an unloaded library base address, resolve the address of a function by manually walking the library export table.
 * @param module_base       A pointer to the start address of an unloaded library obtained by reading from disk or other means.
 * @param function_hash     Hash of the exported procedure.
 * @param key               64-bit integer to initialize the keyed hash.
 * @param hash_function     The hash function to use.
 * @return  Pointer to the desired function.
 */
DEINAMIG_EXPORT
void *deinamig_get_export_address_from_raw_by_hash(void *module_base, unsigned long function_hash, unsigned long key, deinamig_hash_function hash_function);

/**
 * Given an unloaded library base address, resolve the address of a function by manually walking the library export table.
 * @param module_base       A pointer to the start address of an unloaded library obtained by reading from disk or other means.
 * @param export_name       The name or ordinal of the export to search for (e.g. "NtAlertResumeThread").
 * @return  Pointer to the desired function.
 */
DEINAMIG_EXPORT
void *deinamig_get_export_address_from_raw_by_name(void *module_base, const char *export_name);

#endif //DEINAMIG_INCLUDE_DEINAMIG_WINDOWS_H_
