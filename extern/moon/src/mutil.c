#include "mutil.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

void bin2c(const unsigned char* input, unsigned int size, const char* name, const char* prefix, const char* suffix)
{
  unsigned int i, need_comma;
  need_comma = 0;

  char nbuf[255] = "";
  if (name != NULL) snprintf(nbuf, sizeof(nbuf), "%s", name);
  char pbuf[255] = "";
  if (prefix != NULL) snprintf(pbuf, sizeof(pbuf), "%s", prefix);
  char sbuf[255] = "";
  if (suffix != NULL)  snprintf(sbuf, sizeof(sbuf), "%s", suffix);

  printf("#ifndef %s%s_%s_h_\n", pbuf, nbuf, sbuf);
  printf("#define %s%s_%s_h_\n\n", pbuf, nbuf, sbuf);
  printf("inline const unsigned char %s%s_%s_array[%i] = {", pbuf, nbuf, sbuf, size);
  for (i = 0; i < size; ++i) {
    if (need_comma) printf(", ");
    else need_comma = 1;
    if ((i % 11) == 0) printf("\n\t");
    // Mask the cast against 0xff to display the least significant byte
    printf("0x%.2x", 0xff & (unsigned int) input[i]);
  }
  printf("\n};\n");
  printf("inline const unsigned int %s%s_%s_size = %i;\n\n", pbuf, nbuf, sbuf, size);
  printf("#endif\n");
}

int read_file(const char* path, void** out, size_t* out_size) {
  FILE *f_input;

  f_input = fopen(path, "rb");
  if (f_input == NULL) return 0;

  fseek(f_input, 0, SEEK_END);
  *out_size = ftell(f_input);
  fseek(f_input, 0, SEEK_SET);

  *out = (unsigned char *) malloc(*out_size);
  fread(*out, *out_size, 1, f_input);
  fclose(f_input);
  return 1;
}


/*
 * Based on https://groups.google.com/g/gnu.gcc.help/c/0dKxhmV4voE/m/eKqm_ab-xcwJ
 * Split a path into its parts
 * Parameters:
 * path: Object to split
 * drive: always NULL on linux/unix
 * directory: Directory part of path
 * filename: File part of path
 * extension: Extension part of path (includes the leading point)
 * Returns: Directory, Filename, and Extension are changed
 */

void split_path(const char* path, char* drive, char* directory, char* filename, char* extension)
{
#ifdef _WIN32
  _splitpath( path, drive, directory, filename, extension);
#else
  char* copy_of_path = (char*) path;
  int counter = 0;
  int last = 0;
  int rest = 0;

  // no drives available in linux .
  // extensions are not common in linux
  // but considered anyway
  if (drive != NULL)
    drive = NULL;

  while(*copy_of_path != '\0')
  {
    // search for the last slash
    while(*copy_of_path != '/' && *copy_of_path != '\0')
    {
      copy_of_path++;
      counter++;
    }
    if(*copy_of_path == '/')
    {
      copy_of_path++;
      counter++;
      last = counter;
    }
    else
      rest = counter - last;
  }

  if (directory != NULL) {
    // directory is the first part of the path until the
    // last slash appears
    strncpy(directory, path, last);
    // strncpy doesnt add a '\0'
    directory[last] = '\0';
  }

  // Filename is the part behind the last slash
  if (filename != NULL) {
    strcpy(filename, copy_of_path -= rest);
    char *dot = strrchr(filename, '.');
    if(dot != NULL) {
      *dot = '\0';
    }
  }

  if (extension != NULL) {
    char fname[256];
    strncpy(fname, path + last, rest + 1);
    const char *dot = strrchr(fname, '.');
    if(!dot || dot == fname) {
      *extension = '\0';
      return;
    }
    strcpy(extension, dot++);
  }
#endif
}

// Abstract: Make a path out of its parts
// Parameters: Path: Object to be made
// Drive: Logical drive, only for compatibility, not considered
// Directory: Directory part of path
// Filename: File part of path
// Extension: Extension part of path (includes the leading point)
// Returns: Path is changed
// Comment: Note that the concept of an extension is not available in Linux, nevertheless it is considered

void make_path(char* path, const char* drive, const char* directory, const char* file, const char* extension)
{
  while(*drive != '\0' && drive != NULL) {
    *path = *drive;
    path++;
    drive++;
  }

  while(*directory != '\0' && directory != NULL) {
    *path = *directory;
    path ++;
    directory ++;
  }
  while(*file != '\0' && file != NULL) {
    *path = *file;
    path ++;
    file ++;
  }
  while(*extension != '\0' && extension != NULL) {
    *path = *extension;
    path ++;
    extension ++;
  }
  *path = '\0';
}

int str_ends_with(const char *s, const char *suff) {
  size_t slen = strlen(s);
  size_t sufflen = strlen(suff);

  return slen >= sufflen && !memcmp(s + slen - sufflen, suff, sufflen);
}


char * concat_string(char *s1, const char *s2)
{
  const size_t a = strlen(s1);
  const size_t b = strlen(s2);
  const size_t size_ab = a + b + 1;

  s1 = (char*) realloc(s1, size_ab);

  memcpy(s1 + a, s2, b + 1);

  return s1;
}

#ifdef _WIN32

wchar_t* string_to_wide_string(const char* str, int nbytes)
{
  int nchars = 0;
  if ( ( nchars = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS,
                                       str, nbytes,
                                       NULL, 0 ) ) == 0 ) {
    return NULL;
  }

  wchar_t* wstr = NULL;
  if ( !( wstr = malloc( ( ( size_t )nchars + 1 ) * sizeof( wchar_t ) ) ) ) {
    return NULL;
  }

  wstr[ nchars ] = L'\0';
  if ( MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS,
                            str, nbytes,
                            wstr, ( size_t )nchars ) == 0 ) {
    free( wstr );
    return NULL;
  }
  return wstr;
}

char* wide_string_to_string(const wchar_t* wstr, int nchars)
{
  int nbytes = 0;
  if ( ( nbytes = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS,
                                       wstr, nchars,
                                       NULL, 0,
                                       NULL, NULL ) ) == 0 ) {
    return NULL;
  }

  char* str = NULL;
  if ( !( str = malloc( ( size_t )nbytes + 1 ) ) ) {
    return NULL;
  }

  str[ nbytes ] = '\0';
  if ( WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS,
                            wstr, nchars,
                            str, nbytes,
                            NULL, NULL ) == 0 ) {
    free( str );
    return NULL;
  }
  return str;
}

wchar_t* get_file_name_from_path(wchar_t* path)
{
  wchar_t* LastSlash = NULL;
  for (DWORD i = 0; path[i] != L'\0'; i++)
  {
    if ( path[i] == '\\' )
      LastSlash = &path[i + 1];
  }
  return LastSlash;
}

wchar_t* remove_file_extension_wchar(wchar_t* FullFileName, wchar_t* OutputBuffer, unsigned long OutputBufferSize)
{
  wchar_t* LastDot = NULL;
  for (DWORD i = 0; FullFileName[i] != L'\0'; i++)
    if ( FullFileName[i] == '.' )
      LastDot = &FullFileName[i];

  for (DWORD j = 0; j < OutputBufferSize; j++)
  {
    OutputBuffer[j] = FullFileName[j];
    if ( &FullFileName[j] == LastDot )
    {
      OutputBuffer[j] = L'\0';
      break;
    }
  }
  OutputBuffer[OutputBufferSize-1] = L'\0';
  return OutputBuffer;
}

char* remove_file_extension_char(char* myStr) {
  char *retStr;
  char *lastExt;
  if (myStr == NULL) return NULL;
  if ((retStr = malloc (strlen (myStr) + 1)) == NULL) return NULL;
  strcpy (retStr, myStr);
  lastExt = strrchr (retStr, '.');
  if (lastExt != NULL)
    *lastExt = '\0';
  return retStr;
}

#endif


// C11's bounds-checking interface.
#if defined(__STDC_LIB_EXT1__)
#define HAVE_MEMSET_S 1
#endif
// GNU C Library version 2.25 or later.
#if defined(__GLIBC__) && \
    (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 25))
#define HAVE_EXPLICIT_BZERO 1
#endif
// FreeBSD version 11.0 or later.
#if defined(__FreeBSD__) && __FreeBSD_version >= 1100037
#define HAVE_EXPLICIT_BZERO 1
#endif
// OpenBSD version 5.5 or later.
#if defined(__OpenBSD__) && OpenBSD >= 201405
#define HAVE_EXPLICIT_BZERO 1
#endif
// NetBSD version 7.2 or later.
#if defined(__NetBSD__) && __NetBSD_Version__ >= 702000000
#define HAVE_EXPLICIT_MEMSET 1
#endif

void memzero(void *const pnt, const size_t len)
{
#ifdef _WIN32
  SecureZeroMemory(pnt, len);
#elif defined(HAVE_MEMSET_S)
  memset_s(pnt, (rsize_t) len, 0, (rsize_t) len);
#elif defined(HAVE_EXPLICIT_BZERO)
  explicit_bzero(pnt, len);
#else
  volatile unsigned char *volatile pnt_ = (volatile unsigned char *volatile) pnt;
  size_t i = (size_t) 0U;

  while (i < len) {
    pnt_[i++] = 0U;
  }
#endif
}
