#pragma once

#include <cstddef>  // for size_t

// to correspond to OpenCL
class Platform
{
public:
  Platform(unsigned flags = 0);
  // Return number of devices
  size_t size();
};
