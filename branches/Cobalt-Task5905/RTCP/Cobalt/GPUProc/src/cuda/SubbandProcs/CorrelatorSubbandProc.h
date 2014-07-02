//# CorrelatorSubbandProc.h
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

#ifndef LOFAR_GPUPROC_CUDA_CORRELATOR_SUBBAND_PROC_H
#define LOFAR_GPUPROC_CUDA_CORRELATOR_SUBBAND_PROC_H

// @file
#include <cmath>
#include <complex>
#include <utility>
#include <memory>

#include <boost/shared_ptr.hpp>

#include <Stream/Stream.h>
#include <CoInterface/Parset.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/SparseSet.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/PerformanceCounter.h>

#include "CorrelatorStep.h"
#include "SubbandProc.h"

namespace LOFAR
{
  namespace Cobalt
  {

    // A CorrelatedData object tied to a HostBuffer. Represents an output
    // data item that can be efficiently filled from the GPU.
    class CorrelatedDataHostBuffer: public MultiDimArrayHostBuffer<fcomplex, 4>,
                                    public CorrelatedData,
                                    public SubbandProcOutputData
    {
    public:
      CorrelatedDataHostBuffer(unsigned nrStations, 
                               unsigned nrChannels,
                               unsigned maxNrValidSamples,
                               gpu::Context &context);
      // Reset the MultiDimArrayHostBuffer and the CorrelatedData
      void reset();
    };

    class CorrelatorSubbandProc : public SubbandProc
    {
    public:
      CorrelatorSubbandProc(const Parset &parset, 
                            gpu::Context &context,
                            CorrelatorStep::Factories &factories,
                            size_t nrSubbandsPerSubbandProc = 1);

      // Correlate the data found in the input data buffer
      virtual void processSubband(SubbandProcInputData &input,
                                  SubbandProcOutputData &output);

      // Do post processing on the CPU
      virtual bool postprocessSubband(SubbandProcOutputData &output);

    private:
      PerformanceCounter inputCounter;

      // The previously processed SAP/block, or -1 if nothing has been
      // processed yet. Used in order to determine if new delays have
      // to be uploaded.
      ssize_t prevBlock;
      signed int prevSAP;

      boost::shared_ptr<gpu::DeviceMemory> devA;      
      boost::shared_ptr<gpu::DeviceMemory> devB;

      SmartPtr<CorrelatorStep> correlatorStep;
    };

  }
}

#endif

