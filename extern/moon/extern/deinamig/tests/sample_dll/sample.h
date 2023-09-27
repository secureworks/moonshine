#ifndef MALDL_TESTS_SAMPLE_DLL_SAMPLE_H_
#define MALDL_TESTS_SAMPLE_DLL_SAMPLE_H_

extern "C" {

#if defined(SAMPLE_BUILD)
#  if defined(_WIN32)
#    define SAMPLE_API __declspec(dllexport)
#  elif defined(__ELF__)
#    define SAMPLE_API __attribute__ ((visibility ("default")))
#  else
#    define SAMPLE_API
#  endif
#else
#  if defined(_WIN32)
#    define SAMPLE_API __declspec(dllimport)
#  else
#    define SAMPLE_API
#  endif
#endif

SAMPLE_API int addNumbers(int a, int b);

}

#endif //MALDL_TESTS_SAMPLE_DLL_SAMPLE_H_
