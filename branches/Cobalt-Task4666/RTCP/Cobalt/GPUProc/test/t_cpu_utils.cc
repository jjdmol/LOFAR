//# tDelayAndBandPassKernel.cc: test Kernels/DelayAndBandPassKernel class
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: tDelayAndBandPassKernel.cc 25199 2013-06-05 23:46:56Z amesfoort $

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <CoInterface/Parset.h>
#include <GPUProc/cpu_utils.h>

#include <mpi.h>
#include <iostream>
#include <sched.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main(int argc, char **argv)
{
  INIT_LOGGER("t_cpu_utils.cc");

  Parset ps("t_cpu_utils.in_parset");
  
    // Initialise and query MPI
  int provided_mpi_thread_support;
  if (MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_mpi_thread_support) != MPI_SUCCESS) {
    cerr << "MPI_Init_thread failed" << endl;
    exit(1);
  }
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  //exercise the set processorAffinity functionality
  setProcessorAffinity(ps, rank);
  unsigned numCPU = sysconf( _SC_NPROCESSORS_ONLN );

  // Get the cpu of the current thread
  cpu_set_t mask;  
  bool sched_return = sched_getaffinity(0, sizeof(cpu_set_t), &mask);

  if(rank == 0) // the parset sets this rank to cpu 0
  {  
    // Test if it is set correctly!
    for (unsigned idx_cpu =0; idx_cpu < numCPU; idx_cpu += 2)
    {
      if (1 != CPU_ISSET(idx_cpu, &mask))
      {
        LOG_FATAL_STR("Found a cpu that is NOT set while is should be set!");
        exit(1);
      }
      if (0 != CPU_ISSET(idx_cpu + 1, &mask))
      {
        LOG_FATAL_STR("Found a cpu that is set while is should be NOT set!");
        exit(1);
      }
    }
  }
  else
  { // cpu is set to 1 for this rank
    for (unsigned idx_cpu =0; idx_cpu < numCPU; idx_cpu += 2)
    {
      if (0 != CPU_ISSET(idx_cpu, &mask))
      {
        LOG_FATAL_STR("Found a cpu that is NOT set while is should be set!");
        exit(1);
      }
      if (1 != CPU_ISSET(idx_cpu + 1, &mask))
      {
        LOG_FATAL_STR("Found a cpu that is set while is should be NOT set!");
        exit(1);
      }
    }
  }
  //size_t nSampledData = factory.bufferSize(IntToFloatKernel::INPUT_DATA) / sizeof(char);
  //size_t sizeSampledData = nSampledData * sizeof(char);

  //// Create some initialized host data
  //gpu::HostMemory sampledData(ctx, sizeSampledData);
  //char *samples = sampledData.get<char>();
  //for (unsigned idx =0; idx < nSampledData; ++idx)
  //  samples[idx] = -128;  // set all to -128
  //gpu::DeviceMemory devSampledData(ctx, factory.bufferSize(IntToFloatKernel::INPUT_DATA));
  //stream.writeBuffer(devSampledData, sampledData, true);
  //
  //// Device mem for output
  //gpu::DeviceMemory devConvertedData(ctx, factory.bufferSize(IntToFloatKernel::OUTPUT_DATA));
  //gpu::HostMemory convertedData(ctx,  factory.bufferSize(IntToFloatKernel::OUTPUT_DATA));
  ////stream.writeBuffer(devConvertedData, sampledData, true);

  //IntToFloatKernel::Buffers buffers(devSampledData, devConvertedData);
  //auto_ptr<IntToFloatKernel> kernel(factory.create(ctx, buffers));

  //kernel->enqueue(stream);
  //stream.synchronize();
  //stream.readBuffer(convertedData, devConvertedData, true);
  //stream.synchronize();
  //float *samplesFloat = convertedData.get<float>();
  //
  //// Validate the output:
  //// The inputs were all -128 with bits per sample 8. 
  //// Therefore they should all be converted to -127 (but scaled to 16 bit amplitute values).
  //for (size_t idx =0; idx < nSampledData; ++idx)
  //  if(samplesFloat[idx] != -127 * 256)
  //  {
  //      cerr << "Found an uncorrect sample in the output array at idx: " << idx << endl
  //           << "Value found: " << samplesFloat[idx] << endl
  //           << "Test failed "  << endl;
  //      return -1;
  //  }
      
  return 0;
}

