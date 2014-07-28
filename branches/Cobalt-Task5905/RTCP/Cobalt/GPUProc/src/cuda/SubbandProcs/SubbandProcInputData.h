//# SubbandProcInputData.h
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

#ifndef LOFAR_GPUPROC_CUDA_SUBBAND_PROC_INPUT_DATA_H
#define LOFAR_GPUPROC_CUDA_SUBBAND_PROC_INPUT_DATA_H

#include <vector>

#include <CoInterface/BlockID.h>
#include <CoInterface/Parset.h>
#include <CoInterface/SubbandMetaData.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/gpu_wrapper.h>

// \file
// TODO: Update documentation

namespace LOFAR
{
  namespace Cobalt
  {
    //   Collect all inputData for the correlatorSubbandProc item:
    //    \arg inputsamples
    //    \arg delays
    //    \arg phaseOffSets
    //    \arg flags
    // It also contains a read function parsing all this data from an input stream.   
    class SubbandProcInputData
    {
    public:
      // Which block this InputData represents
      struct BlockID blockID;

      // Delays are computed and applied in double precision,
      // otherwise the to be computed phase shifts become too inprecise.

      //!< Whole sample delays at the start of the workitem      
      MultiDimArrayHostBuffer<double, 3> delaysAtBegin;

      //!< Whole sample delays at the end of the workitem      
      MultiDimArrayHostBuffer<double, 3> delaysAfterEnd;

      //!< Remainder of delays
      MultiDimArrayHostBuffer<double, 2> phase0s;

      //!< Delays for TABs (aka pencil beams) after station beam correction
      MultiDimArrayHostBuffer<double, 3> tabDelays;

      // inputdata with flagged data set to zero
      MultiDimArrayHostBuffer<char, 4> inputSamples;

      // The input flags
      MultiDimArray<SparseSet<unsigned>, 1> inputFlags;

      // CPU-side holder for the Meta Data
      std::vector<SubbandMetaData> metaData; // [station]

      // Create the inputData object we need shared host/device memory on the
      // supplied devicequeue
      SubbandProcInputData(size_t n_beams, size_t n_stations, 
                           size_t n_polarizations, size_t n_coherent_tabs, 
                           size_t n_samples, size_t bytes_per_complex_sample,
                           gpu::Context &context,
                           unsigned int hostBufferFlags = 0);

      // Short-hand constructor pulling all relevant values from a Parset
      SubbandProcInputData(const Parset &ps,
                           gpu::Context &context,
                           unsigned int hostBufferFlags = 0);

      // process the given meta data 
      void applyMetaData(const Parset &ps, unsigned station,
                         unsigned SAP, const SubbandMetaData &metaData);

      // set all flagged inputSamples to zero.
      void flagInputSamples(unsigned station, const SubbandMetaData& metaData);
    };
  }
}

#endif

