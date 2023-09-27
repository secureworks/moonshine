#ifndef MOON_COMPRESS_H_
#define MOON_COMPRESS_H_

#include <stddef.h>

#include "lua.h"

int compress_memory(const void* in, size_t in_size, void** out, size_t* out_size);
int compress_file(const char* path, void** out, size_t* out_size);
int decompress_memory(const void* in, size_t in_size, void** out, size_t* out_size);

#define LUA_MCOMPRESSLIBNAME	"mcompress"
int luaopen_mcompress(lua_State *L);

#endif //MOON_COMPRESS_H_
