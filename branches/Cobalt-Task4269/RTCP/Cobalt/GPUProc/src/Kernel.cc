#include "lofar_config.h"
#include "Kernel.h"

#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "PerformanceCounter.h"

namespace LOFAR
{
  namespace RTCP
  {
    Kernel::Kernel(const Parset &ps, cl::Program &program, const char *name)
      :
      cl::Kernel(program, name),
      ps(ps)
    {
    }

    void Kernel::enqueue(cl::CommandQueue &queue, PerformanceCounter &counter)
    {
      // AMD complains if we submit 0-sized work
      for (unsigned dim = 0; dim < globalWorkSize.dimensions(); dim++)
        if (globalWorkSize[dim] == 0)
          return;

      queue.enqueueNDRangeKernel(*this, cl::NullRange, globalWorkSize, localWorkSize, 0, &event);
      counter.doOperation(event, nrOperations, nrBytesRead, nrBytesWritten);
    }
  }
}
