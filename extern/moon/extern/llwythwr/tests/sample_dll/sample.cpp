#define SAMPLE_BUILD
#include "sample.h"

extern "C" {

SAMPLE_API int addNumbers(int a, int b)
{
  return a + b;
}

}