//# SubbandProc.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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
//# $Id$

#ifndef LOFAR_GPUPROC_CUDA_SUBBAND_PROC_H
#define LOFAR_GPUPROC_CUDA_SUBBAND_PROC_H

#include <string>
#include <map>
#include <complex>
#include <memory>

#include <boost/shared_ptr.hpp>
#include <Common/LofarLogger.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Pool.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Config.h>
#include <CoInterface/SubbandMetaData.h>
#include <GPUProc/PerformanceCounter.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>

#include "SubbandProcInputData.h"
#include "SubbandProcOutputData.h"
#include "CorrelatorStep.h"
#include "BeamFormerPreprocessingStep.h"
#include "BeamFormerCoherentStep.h"
#include "BeamFormerIncoherentStep.h"

// \file
// TODO: Update documentation

namespace LOFAR
{
  namespace Cobalt
  {
    //# Forward declarations
    struct KernelFactories;

    /*
     * The SubbandProc does the following transformation:
     *   SubbandProcInputData -> SubbandProcOutputData
     *
     * The SubbandProcInputData represents one block of one subband
     * of input data, and the SubbandProcOutputData (for example) the complex
     * visibilities of such a block.
     *
     * For both input and output, a fixed set of objects is created,
     * tied to the GPU specific for the SubbandProc, for increased
     * performance. The objects are recycled by using Pool objects.
     *
     * The data flows as follows:
     *
     *   // Fetch the next input object to fill
     *   SmartPtr<SubbandProcInputData> input = queue.inputPool.free.remove();
     *
     *   // Provide input
     *   receiveInput(input);
     *
     *   // Annotate input
     *   input->blockID.block = block;
     *   input->blockID.globalSubbandIdx = subband;
     *   input->blockID.localSubbandIdx  = subbandIdx;
     *
     *   // Fetch the next output object to fill
     *   SmartPtr<SubbandProcOutputData> output = queue.outputPool.free.remove();
     *
     *   // Process block
     *   queue.processSubband(input, output);
     *
     *   // Give back input and output objects to queue
     *   queue.inputPool.free.append(input);
     *   queue.outputPool.free.append(output);
     *
     *   The queue.inputPool.filled and queue.outputPool.filled can be used to
     *   temporarily store filled input and output objects. Such is needed to
     *   obtain parallellism (i.e. read/process/write in separate threads).
     */
    class SubbandProc {
    public:
      SubbandProc(const Parset &ps, gpu::Context &context,
                  KernelFactories &factories,
                  size_t nrSubbandsPerSubbandProc = 1);

      class Flagger
      {
      public:
        // 1.1 Convert the flags per station to channel flags, change time scale
        // if nchannel > 1
        static void convertFlagsToChannelFlags(Parset const &ps,
          MultiDimArray<SparseSet<unsigned>, 1> const &inputFlags,
          MultiDimArray<SparseSet<unsigned>, 2> &flagsPerChannel);
      };

      // A pool of input data, to allow items to be filled and
      // computed on in parallel.
      Pool<SubbandProcInputData> inputPool;

      // A pool of input data, that has been pre processed.
      Pool<SubbandProcInputData> processPool;

      // A pool of output data, to allow items to be filled
      // and written in parallel.
      Pool<SubbandProcOutputData> outputPool;

      // Correlate the data found in the input data buffer
      void processSubband(SubbandProcInputData &input, SubbandProcOutputData &output);

      // Do post processing on the CPU.
      void postprocessSubband(SubbandProcOutputData &output);

    protected:
      const Parset &ps;
      const size_t nrSubbandsPerSubbandProc;

      gpu::Stream queue;

      // The previously processed SAP/block, or -1 if nothing has been
      // processed yet. Used in order to determine if new delays have
      // to be uploaded.
      ssize_t prevBlock;
      signed int prevSAP;

      // @{
      // Device memory buffers. These buffers are used interleaved. For details,
      // please refer to the document bf-pipeline.txt in the directory
      // GPUProc/doc.
      boost::shared_ptr<gpu::DeviceMemory> devA;
      boost::shared_ptr<gpu::DeviceMemory> devB;
      // @}

      PerformanceCounter inputCounter;

      std::auto_ptr<CorrelatorStep> correlatorStep;
      std::auto_ptr<BeamFormerPreprocessingStep> preprocessingStep;
      std::auto_ptr<BeamFormerCoherentStep> coherentStep;
      std::auto_ptr<BeamFormerIncoherentStep> incoherentStep;

      // Returns the number of output elements to create to get a smooth
      // running pipeline.
      size_t nrOutputElements() const;
    };
  }
}

#endif

