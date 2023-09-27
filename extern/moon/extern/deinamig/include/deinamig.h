/*******************************************************************************
 * \mainpage
 * Deinamig is a library with the goal of providing functionality to mask the
 * use of certain functions within the  Windows API.
 *
 * A couple of different approaches are provided to accomplish this goal.
 *
 * - Reimplementation of common Windows API functions such as LoadLibrary and
 * GetModuleHandle, allowing module handles and addresses of exported
 * functions to be retrieved covertly either by name or a hashing scheme.
 * - Execution of functional equivalent assembly stubs allowing the user to make
 * system calls directly to the kernel. Both direct and indirect variations of
 * this technique are provided. Functions are also provided to retrieve system
 * call numbers and addresses, and to create suitable stubs.
 * - Functions to find and extract syscall numbers and/or stubs from unloaded
 * modules which has been read from disk or provided by other means.
 *
 * The intention is for the library to be either referenced directly within
 * toolsets or for specific functionally to be easily extracted and embedded
 * within toolsets.
 *
 * References:
 * - https://www.forrest-orr.net/post/malicious-memory-artifacts-part-i-dll-hollowing
 * - https://github.com/forrest-orr/phantom-dll-hollower-poc
 * - https://www.secforce.com/blog/dll-hollowing-a-deep-dive-into-a-stealthier-memory-allocation-variant/
 * - https://github.com/cobbr/SharpSploit/blob/master/SharpSploit/Execution/ManualMap/Map.cs
 * - https://github.com/cobbr/SharpSploit/blob/master/SharpSploit/Execution/ManualMap/Overload.cs
 * - https://github.com/TheWover/donut/blob/master/loader/inmem_pe.c
 * - https://www.unknowncheats.me/forum/programming-for-beginners/349013-eject-mapped-dll.html#post2538435
 * - http://phrack.org/issues/62/12.html
 * - https://www.mdsec.co.uk/2021/06/bypassing-image-load-kernel-callbacks/
 * - https://github.com/DarthTon/Blackbone
 ******************************************************************************/

#ifndef DEINAMIG_INCLUDE_DEINAMIG_H_
#define DEINAMIG_INCLUDE_DEINAMIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <deinamig/export.h>

#if defined(__APPLE__)
#include "deinamig/darwin.h"
#elif defined(_WIN32)
#include "deinamig/windows.h"
#else
#include "deinamig/linux.h"
#endif

#ifdef __cplusplus
}
#endif

#endif //DEINAMIG_INCLUDE_DEINAMIG_H_
