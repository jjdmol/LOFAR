#ifndef FCNP_CN_H
#define FCNP_CN_H

#include <cstddef>
#include <vector>


namespace FCNP_CN
{
  void init();

  // ptr and size must be a multiple of 16!
  void CNtoION_ZeroCopy(const void *ptr, size_t size);
  void IONtoCN_ZeroCopy(void *ptr, size_t size);
}

#endif
