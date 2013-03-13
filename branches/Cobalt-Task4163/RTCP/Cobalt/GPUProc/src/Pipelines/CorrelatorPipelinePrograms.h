#ifndef GPUPROC_CORRELATORPIPELINEPROGRAMS_H
#define GPUPROC_CORRELATORPIPELINEPROGRAMS_H
#include "CL/cl.hpp"

#include "OpenCL_Support.h"
#include "global_defines.h"

namespace LOFAR
{
  namespace Cobalt
  {
    struct CorrelatorPipelinePrograms
    {
      cl::Program firFilterProgram;
      cl::Program delayAndBandPassProgram;
      cl::Program correlatorProgram;
    };
  }
}
#endif
