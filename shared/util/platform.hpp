#ifndef LUA_APPS_OS_H_
#define LUA_APPS_OS_H_

#if defined(_WIN32)
#define OS_WINDOWS
#elif defined(__linux__) || defined(__APPLE__) || defined(__CYGWIN__) || (defined(__sun__) && defined(__svr4__))
#define OS_POSIX
#else
#define OS_OTHER
#endif

#if defined(__linux__)
#define OS_LINUX
#endif

#if defined(__APPLE__)
#define OS_MACOS
#endif

#endif //LUA_APPS_OS_H_
