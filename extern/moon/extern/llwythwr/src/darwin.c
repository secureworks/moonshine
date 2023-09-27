#include "llwythwr/darwin.h"

#include <stdio.h>
#include <dlfcn.h>
#include "debug.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

struct llwythwr_mac_library {
  NSModule mod;
  void* code_address;
  NSObjectFileImage file_image;
  size_t size;
  llwythwr_macos_api* api;
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

void* llwythwr_get_symbol_address_disk(void *handle, const char *name) {
  DPRINT("dlsym(%p, %s)\n", handle, name);
  return dlsym(handle, name);
}

llwythwr_mac_library* llwythwr_load_library_from_memory(llwythwr_macos_api *api, const char *name, const void *data, size_t size, int flags)
{
  llwythwr_mac_library* library = (llwythwr_mac_library*) api->malloc(sizeof(llwythwr_mac_library));
  library->api = api;
  library->size = size;
  library->code_address = api->mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  if (library->code_address == MAP_FAILED) {
    DPRINT("mmap(NUL, %zu, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0) failed for %s\n", size, name);
    api->free(library);
    return NULL;
  }

  api->_memcpy(library->code_address, data, size);
  if (((int *)library->code_address)[3] != 0x8) {
    // https://gist.github.com/johnkhbaek/d9f7c7db926b516f2a64daa12a26eb1e
    // 0x8 == MH_BUNDLE, 0x6 == MH_DYLIB, 0x2 == MH_EXECUTE
    DPRINT("Changing type from 0x%x\n", ((int *)library->code_address)[3]);
    // change type to mh_bundle so that NSCreateObjectFileImageFromMemory accepts whatever is provided
    ((int *)library->code_address)[3] = 0x8;
  }

  DPRINT("Type is 0x%x\n", ((int *)library->code_address)[3]);

  api->NSCreateObjectFileImageFromMemory(library->code_address, size, &library->file_image);
  if (library->file_image == NULL) {
    DPRINT("NSCreateObjectFileImageFromMemory(%p, %zu, NULL) failed for %s\n", library->code_address, size, name);
    api->munmap(library->code_address, size);
    api->free(library);
    return NULL;
  }
  library->mod = api->NSLinkModule(library->file_image, name, NSLINKMODULE_OPTION_NONE);
  /*
  if(monterey) {
      library->modiwl = ((uintptr_t)library->modiwl) >> 1;
  }
  */
  if (library->mod == NULL) {
    DPRINT("NSLinkModule(%p, %s) failed\n", library->file_image, name);
    api->NSDestroyObjectFileImage(library->file_image); // This will manages release mmap'd memory
    api->free(library);
    return NULL;
  }

  DPRINT("NSLinkModule(%p, %s, NSLINKMODULE_OPTION_NONE) -> %p\n", library->file_image, name, library->mod);
  return library;
}

void llwythwr_unload_library_from_memory(llwythwr_mac_library* library)
{
  library->api->NSUnLinkModule(library->mod, NSUNLINKMODULE_OPTION_NONE);
  DPRINT("NSUnLinkModule(%p, NSUNLINKMODULE_OPTION_NONE)\n", library->mod);
  library->api->NSDestroyObjectFileImage(library->file_image);
  library->api->free(library);
}

void* llwythwr_get_symbol_address_memory(llwythwr_mac_library* library, const char *name)
{
  void *addr = NULL;
  char* symname = NULL;
  int len = library->api->asprintf(&symname, "_%s", name);
  if (len != -1) {
    NSSymbol symbol = library->api->NSLookupSymbolInModule(library->mod, symname);
    DPRINT("NSLookupSymbolInModule(%p, %s)\n", library->mod, name);
    addr = library->api->NSAddressOfSymbol(symbol);
    if (addr != NULL)
      DPRINT("NSAddressOfSymbol(%p) -> %p\n", symbol, addr);
    else
      DPRINT("NSAddressOfSymbol(%p) failed\n", symbol);
  }
  library->api->free(symname);
  return (void*) addr;
}
