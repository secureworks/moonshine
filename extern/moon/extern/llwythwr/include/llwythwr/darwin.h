#ifndef LLWYTHWR_INCLUDE_LLWYTHWR_DARWIN_H_
#define LLWYTHWR_INCLUDE_LLWYTHWR_DARWIN_H_

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


typedef struct llwythwr_mac_library llwythwr_mac_library, llwythwr_library;

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

#endif //LLWYTHWR_INCLUDE_LLWYTHWR_DARWIN_H_
