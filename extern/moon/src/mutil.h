#ifndef MOON_UTIL_H_
#define MOON_UTIL_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#define LIB_SUFFIX ".dll"
#define PATH_SEPARATOR "\\"
#else
#define LIB_SUFFIX ".so"
#define PATH_SEPARATOR "/"
#define _MAX_DRIVE 3
#define _MAX_PATH 260
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 256
#endif

void bin2c(const unsigned char* input, unsigned int size, const char* name, const char* prefix, const char* suffix);
int read_file(const char* path, void** out, size_t* out_size);
void split_path(const char* path, char* drive, char* directory, char* filename, char* extension);
void make_path(char* path, const char* drive, const char* directory, const char* file, const char* extension);
int str_ends_with(const char *s, const char *suff);
char *concat_string(char *s1, const char *s2);
#ifdef _WIN32
wchar_t* string_to_wide_string(const char* str, int nbytes);
char* wide_string_to_string(const wchar_t* wstr, int nchars);
wchar_t* get_file_name_from_path(wchar_t* path);
wchar_t* remove_file_extension_wchar(wchar_t* FullFileName, wchar_t* OutputBuffer, unsigned long OutputBufferSize);
char* remove_file_extension_char(char* FullFileName);
#endif
void memzero(void *const pnt, const size_t len);

#endif //MOON_UTIL_H_
