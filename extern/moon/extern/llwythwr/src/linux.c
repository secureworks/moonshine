#include "llwythwr/linux.h"

#include <stdio.h>
//#include <unistd.h>
#include <dlfcn.h>
#include "debug.h"

struct llwythwr_linux_library {
  void* handle;
  llwythwr_linux_api* api;
};

void* llwythwr_load_library_from_disk(const char* name, int flags) {
  void *handle = dlopen(name, flags);
  if (handle == NULL)
    DPRINT("dlopen(%s, %d) failed, %s\n", name, flags, dlerror());
  else
    DPRINT("dlopen(%s) -> %p\n", name, handle);
  return handle;
}

void llwythwr_unload_library_from_disk(void* handle) {
  DPRINT("dlclose(%p)\n", handle);
  dlclose(handle);
}

void* llwythwr_get_symbol_address_disk(void *handle, const char *name)
{
  void* addr = dlsym(handle, name);
  if (addr != NULL)
    DPRINT("dlsym(%p, %s) -> %p\n", handle, name, addr);
  else
    DPRINT("dlsym(%p, %s) failed, %s\n", handle, name, dlerror());
  return addr;
}

llwythwr_linux_library* llwythwr_load_library_from_memory(llwythwr_os_api* api, const char *name, const void *data, size_t size, int flags)
{
  int shm_fd = api->memfd_create(name, 1);
  if (shm_fd) {
    api->write(shm_fd, data, size);
    char path[4096];
    api->snprintf(path, 1024, "/proc/%d/fd/%d", api->getpid(), shm_fd);
    if (flags == 0)
      flags = RTLD_NOW | RTLD_GLOBAL;
    void *handle = api->dlopen(path, flags);
    if (handle == NULL) {
      DPRINT("dlopen(%s, %d) failed, %s\n", path, flags, api->dlerror());
      return NULL;
    }
    else
      DPRINT("dlopen(%s) -> %p\n", name, handle);

    llwythwr_linux_library* library = (llwythwr_linux_library*) api->malloc(sizeof(llwythwr_linux_library));
    library->handle = handle;
    library->api = api;
    return library;
  }
  DPRINT("syscall(SYS_memfd_create, %s, 1) failed\n", name);
  return NULL;
}

void llwythwr_unload_library_from_memory(llwythwr_linux_library* library)
{
  DPRINT("dlclose(%p)\n", library->handle);
  library->api->dlclose(library->handle);
  library->api->free(library);
}

void* llwythwr_get_symbol_address_memory(llwythwr_linux_library* library, const char *name)
{
  void* addr = library->api->dlsym(library->handle, name);
  if (addr != NULL)
    DPRINT("dlsym(%p, %s) -> %p\n", library->handle, name, addr);
  else
    DPRINT("dlsym(%p, %s) failed, %s\n", library->handle, name, library->api->dlerror());
  return addr;
}
