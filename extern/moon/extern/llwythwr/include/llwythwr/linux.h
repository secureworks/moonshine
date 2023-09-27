#ifndef LLWYTHWR_INCLUDE_LLWYTHWR_LINUX_H_
#define LLWYTHWR_INCLUDE_LLWYTHWR_LINUX_H_

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


typedef struct llwythwr_linux_library llwythwr_linux_library, llwythwr_library;

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

#endif //LLWYTHWR_INCLUDE_LLWYTHWR_LINUX_H_
