//# SubbandProcOutputData.h
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

#ifndef LOFAR_GPUPROC_CUDA_SUBBAND_PROC_OUTPUT_DATA_H
#define LOFAR_GPUPROC_CUDA_SUBBAND_PROC_OUTPUT_DATA_H

#include <CoInterface/BlockID.h>
#include <CoInterface/Parset.h>
#include <CoInterface/CorrelatedData.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>

// \file
// TODO: Update documentation

namespace LOFAR
{
  namespace Cobalt
  {
    // Our output data type
    class SubbandProcOutputData
    {
    public:
      struct BlockID blockID;

      MultiDimArrayHostBuffer<float, 4> coherentData;
      MultiDimArrayHostBuffer<float, 4> incoherentData;

      struct CorrelatedData:
        public MultiDimArrayHostBuffer<fcomplex,4>,
        public LOFAR::Cobalt::CorrelatedData
      {
        CorrelatedData(unsigned nrStations, 
                       unsigned nrChannels,
                       unsigned maxNrValidSamples,
                       gpu::Context &context);
      };

      CorrelatedData correlatedData;
      bool emit_correlatedData;

      SubbandProcOutputData(const Parset &ps, gpu::Context &context);
    };
  }
}

#endif

