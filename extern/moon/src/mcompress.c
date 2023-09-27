#include "mcompress.h"

#include <stdio.h>

#include "libapultra.h"

#include "lua.h"
#include "lauxlib.h"

#include "mutil.h"


int compress_memory(const void* in, size_t in_size, void** out, size_t* out_size) {
  *out_size = apultra_get_max_compressed_size(in_size);
  *out = (void*)malloc(sizeof(unsigned char*) * *out_size);
  *out_size = apultra_compress((const unsigned char*) in, (unsigned char*) *out, in_size, *out_size, 0, 0, 0, NULL, NULL);
  if (*out_size == -1) {
    free(*out);
    *out = NULL;
    return 0;
  }
  return 1;
}

int compress_file(const char* path, void** out, size_t* out_size) {
  unsigned char *buf;
  FILE *f_input;
  unsigned file_size;

  f_input = fopen(path, "rb");
  if (f_input == NULL) {
    fprintf(stderr, "Can't open %s for reading.\n", path);
    return 0;
  }
  // Get the file length
  fseek(f_input, 0, SEEK_END);
  file_size = ftell(f_input);
  fseek(f_input, 0, SEEK_SET);

  buf = (unsigned char *) malloc(file_size);
  fread(buf, file_size, 1, f_input);
  fclose(f_input);

  *out_size = apultra_get_max_compressed_size(file_size);
  *out = (void*)malloc(sizeof(unsigned char*) * *out_size);
  *out_size = apultra_compress(buf, (unsigned char*)*out, file_size, *out_size, 0, 0, 0, NULL, NULL);
  if (*out_size == -1) {
    free(*out);
    *out = NULL;
    return 0;
  }
  free(buf);
  return 1;
}

int decompress_memory(const void* in, size_t in_size, void** out, size_t* out_size) {
  *out_size = apultra_get_max_decompressed_size((const unsigned char*)in, in_size, 0);
  *out = (void*)malloc(sizeof(unsigned char*) * *out_size);
  *out_size = apultra_decompress((const unsigned char*)in, (unsigned char*)*out, in_size, *out_size, 0, 0);
  if (*out_size == -1) {
    free(*out);
    *out = NULL;
    return 0;
  }
  return 1;
}

int compress(lua_State *L) {
  size_t decompressed_size = 0;
  const void* decompressed = luaL_checklstring(L, 1, &decompressed_size);

  void* buf;
  size_t buf_size;
  if (compress_memory(decompressed, decompressed_size, (void **) &buf, &buf_size)) {
    lua_pushlstring(L, buf, buf_size);
    free(buf);
    return 1;
  }
  lua_pushnil(L);
  return 1;

}

int decompress(lua_State *L) {
  size_t compressed_size = 0;
  const void* compressed = luaL_checklstring(L, 1, &compressed_size);

  void *buf = NULL;
  size_t buf_size = 0;
  if (decompress_memory(compressed, compressed_size, &buf, &buf_size)) {
    lua_pushlstring(L, buf, buf_size);
    memzero(buf, buf_size);
    free(buf);
    return 1;
  }
  lua_pushnil(L);
  return 1;
}

int luaopen_mcompress(lua_State *L) {
  lua_newtable(L);
  lua_pushcfunction (L, compress);
  lua_setfield (L, -2, "compress");
  lua_pushcfunction (L, decompress);
  lua_setfield (L, -2, "decompress");
  return 1;
}
