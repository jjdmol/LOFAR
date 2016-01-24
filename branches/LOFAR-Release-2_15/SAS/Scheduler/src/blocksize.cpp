#include "blocksize.h"

#include <cmath>

const double BlockSize::MAXBLOCKSIZE = 1.3;

const double BlockSize::MINBLOCKSIZE = 0.6;

#ifdef _WIN32
  double round(double value)
  {
      return floor(value + 0.5);
  }

#endif

