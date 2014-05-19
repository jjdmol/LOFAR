//# MPI_utils.h: Helper functions for cpu specific functionality
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

// \file
// Include for processor optimalizetion functionality

#ifndef LOFAR_GPUPROC_MPI_UTILS_H
#define LOFAR_GPUPROC_MPI_UTILS_H

#include <InputProc/Transpose/MPIUtil.h>
#include <InputProc/SampleType.h>
#include <CoInterface/SmartPtr.h>
#include <Common/ComplexStdInt.h>

// From here MPIInput

#include <CoInterface/Parset.h>
#include <vector>
#include <CoInterface/Pool.h>
#include <Common/Timer.h>
#include <InputProc/Transpose/MPIReceiveStations.h>
namespace LOFAR
{
  namespace Cobalt
  {
    // Move to separate class
    struct MPIRecvData
    {
      size_t block;

      SmartPtr<char, SmartPtrMPI<char> > data;
      SmartPtr<char, SmartPtrMPI<char> > metaData;

      template<typename SampleT>
      void allocate(size_t nrStations, size_t nrBeamlets, size_t nrSamples);
    };

    struct MPIInput
    {
      const Parset &ps;
      Pool<struct MPIRecvData> &mpiPool;
      const std::vector<size_t> subbandIndices;
      const bool processingSubband0;

      // For each block, read all data and put it (untransposed) in the mpiPool
      MPIInput(const Parset &ps,
        Pool<struct MPIRecvData> &pool,
        const std::vector<size_t> &subbandIndices,
        const bool processingSubband0);

      void receiveInput(size_t nrBlocks);
      template<typename SampleT> void receiveInput(size_t nrBlocks);
    };


  }
}

#endif