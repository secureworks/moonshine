#ifndef MOONSHINE_SHARED_UTIL_DEBUG_H_
#define MOONSHINE_SHARED_UTIL_DEBUG_H_

#include <string>

#ifdef NDEBUG
#define DEBUG_PRINT(fmt, ...) ((void)0)
#define DEBUG_PUTS(msg) ((void)0)
#define ERROR_PRINT(fmt, ...) ((void)0)
#define ERROR_PUTS(msg) ((void)0)
#else
#define DEBUG_PRINT(fmt, ...) do { \
  util::debug::print(stdout, "DEBUG: %s(%d): " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
} while (false)

#define DEBUG_PUTS(msg) DEBUG_PRINT("%s", msg)

#define ERROR_PRINT(fmt, ...) do { \
  util::debug::print(stderr, "ERROR: %s(%d): " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
} while (false)

#define ERROR_PUTS(msg) ERROR_PRINT("%s", msg)
#endif

namespace util {

namespace debug {
/*
 * https://docs.microsoft.com/en-us/archive/msdn-magazine/2015/march/windows-with-c-using-printf-with-modern-c
*/

template<typename T>
T argument(T value) noexcept {
  return value;
}

template<typename T>
T const *argument(std::basic_string<T> const &value) noexcept {
return value.c_str();
}

template<typename ... Args>
inline void print(FILE *stream, char const *const format, Args const &... args) noexcept {
  fprintf(stream, format, argument(args) ...);
}

template<typename ... Args>
inline void print(char const *const format, Args const &... args) noexcept {
  print(stdout, format, args...);
}

inline void print(FILE *stream, char const *const value) noexcept {
  print(stream, "%s", value);
}

inline void print(char const *const value) noexcept {
  print(stdout, "%s", value);
}

inline void print(FILE *stream, wchar_t const *const value) noexcept {
  print(stream, "%ls", value);
}

inline void print(wchar_t const *const value) noexcept {
  print(stdout, "%ls", value);
}

template<typename T>
void print(FILE *stream, std::basic_string<T> const &value) noexcept {
  print(stream, value.c_str());
}

template<typename T>
void print(std::basic_string<T> const &value) noexcept {
print(stdout, value.c_str());
}

#if 0 //defined(_WIN32)
//  Originated from: https://sourceforge.net/projects/diagnostic/
//
//  Similar to windows API function, captures N frames of current call stack.
//  Unlike windows API function, works with managed and native functions.
//
//  [in] frames to skip, 0 - capture everything.
//  [in] frames to capture.
// [out] filled callstack with total size nFrames - FramesToSkip
inline int capture_stack_backtrace(int FramesToSkip, int nFrames, PVOID* BackTrace)
{
#ifdef _WIN64
  CONTEXT ContextRecord;
  RtlCaptureContext(&ContextRecord);

  UINT iFrame;
  for (iFrame = 0; iFrame < (UINT)nFrames; iFrame++)
  {
    DWORD64 ImageBase;
    PRUNTIME_FUNCTION pFunctionEntry = RtlLookupFunctionEntry(ContextRecord.Rip, &ImageBase, NULL);

    if (pFunctionEntry == NULL)
    {
      if (iFrame != -1)
        iFrame--;           // Eat last as it's not valid.
      break;
    }

    PVOID HandlerData;
    DWORD64 EstablisherFrame;
    RtlVirtualUnwind(0 /*UNW_FLAG_NHANDLER*/,
                     ImageBase,
                     ContextRecord.Rip,
                     pFunctionEntry,
                     &ContextRecord,
                     &HandlerData,
                     &EstablisherFrame,
                     NULL);

    if(FramesToSkip > (int)iFrame)
      continue;

    BackTrace[iFrame - FramesToSkip] = (PVOID)ContextRecord.Rip;
  }
#else
  //
  //  This approach was taken from StackInfoManager.cpp / FillStackInfo
  //  http://www.codeproject.com/Articles/11221/Easy-Detection-of-Memory-Leaks
  //  - slightly simplified the function itself.
  //
  int regEBP;
  __asm mov regEBP, ebp;

  long *pFrame = (long*)regEBP;               // pointer to current function frame
  void* pNextInstruction;
  int iFrame = 0;

  //
  // Using __try/_catch is faster than using ReadProcessMemory or VirtualProtect.
  // We return whatever frames we have collected so far after exception was encountered.
  //
  __try {
      for (; iFrame < nFrames; iFrame++)
      {
          pNextInstruction = (void*)(*(pFrame + 1));

          if (!pNextInstruction)     // Last frame
              break;

          if (FramesToSkip > iFrame)
              continue;

          BackTrace[iFrame - FramesToSkip] = pNextInstruction;
          pFrame = (long*)(*pFrame);
      }
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
  }

#endif //_WIN64
  iFrame -= FramesToSkip;
  if(iFrame < 0)
    iFrame = 0;

  return iFrame;
} //capture_stack_backtrace
#endif

} // debug

} // util


#endif //MOONSHINE_SHARED_UTIL_DEBUG_H_
