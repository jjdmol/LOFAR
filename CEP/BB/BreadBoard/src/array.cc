#include "array.h"

void arr::set(int i, float f[])
{
      a = new float[i];
      for (int j=0; j < i; ++j)
      {
            a[j] = f[j];// potential overflow err here
      }
}
