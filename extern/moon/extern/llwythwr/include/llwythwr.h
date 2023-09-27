/*******************************************************************************
 * \mainpage
 * Llwythwr is a library with the goal of providing functionality to load
 * dynamic libraries (.so / .dll / .dylib) from memory, avoiding disk access.
 *
 * Methods to achieve this goal are provided for all three major operating
 * systems (Linux, macOS, and Windows).
 *
 * Some methods also include options for further obfuscation and/or blending in
 * with native loaded libraries, masking their use.
 ******************************************************************************/

#ifndef LLWYTHWR_INCLUDE_LLWYTHWR_H_
#define LLWYTHWR_INCLUDE_LLWYTHWR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <llwythwr/export.h>
#if defined(__APPLE__)
#include "llwythwr/darwin.h"
#elif defined(_WIN32)
#include "llwythwr/windows.h"
#else
#include "llwythwr/linux.h"
#endif

#ifdef __cplusplus
}
#endif

#endif //LLWYTHWR_INCLUDE_LLWYTHWR_H_
