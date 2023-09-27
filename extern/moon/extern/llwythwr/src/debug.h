#ifndef LIBLOADER_SRC_DEBUG_H_
#define LIBLOADER_SRC_DEBUG_H_

#include <stdio.h>
#include <inttypes.h>

#ifdef DEBUG_BUILD
#define DPRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define DPRINT(fmt, args...) /* Don't do anything in release builds */
#endif

#endif //LIBLOADER_SRC_DEBUG_H_
