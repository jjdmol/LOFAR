#ifndef COBALT_GPUPROC_TESTUTIL
#define COBALT_GPUPROC_TESTUTIL

#include <GPUProc/gpu_wrapper.h>

namespace LOFAR {
  namespace Cobalt {

    // Allocate and initialise host memory
    template<typename T> gpu::HostMemory getInitializedArray(gpu::Context &ctx, unsigned size, const T &defaultValue)
    {
      gpu::HostMemory memory(ctx, size);
      T* createdArray = memory.get<T>();

      for (unsigned idx = 0; idx < size / sizeof(T); ++idx)
        createdArray[idx] = defaultValue;

      return memory;
    }

  }
}

#endif
