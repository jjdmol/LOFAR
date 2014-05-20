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
    
    // Data contained for raw data plus metadata received over the MPI
    // interface between StationInput and the MPIReceiver
    // These blocks are typically stored in a pool to be filled be the receiver
    // and 'emptied' by transpose input. Which converts the data to a
    // valid multidim array
    struct MPIRecvData
    {
      // Index of the block current stored in this object
      size_t block;

      SmartPtr<char, SmartPtrMPI<char> > data;
      SmartPtr<char, SmartPtrMPI<char> > metaData;

      template<typename SampleT>
      void allocate(size_t nrStations, size_t nrBeamlets, size_t nrSamples);
    };

    // MPIReceiver receives MPI input from the StationInput 
    // receiveInput() creates a set number of MPIRecvData containers
    // which are filled raw data and inserted into the pool
    // The pool is shared with a pipeline which will empty the containers
    class MPIReceiver
    {
    public:
      // Simple constructer only responsible for settings data members
      MPIReceiver(
        Pool<struct MPIRecvData> &pool,
        const std::vector<size_t> &subbandIndices,
        const bool processingSubband0,
        size_t nrSamplesPerSubband,
        size_t nrStations,
        size_t nrBitsPerSample);


      // Creates a set number of poolitems
      // Reads nrBlocks from the mpi input filling the pool with
      // raw MPIRecvData     
      // This function is typically started in a seperate thread
      // Internally the type of samples depends on nrBitsPerSample
      void receiveInput(size_t nrBlocks);
      
    private:
      // The templeted receive function.
      template<typename SampleT> void receiveInput(size_t nrBlocks);

      Pool<struct MPIRecvData> &mpiPool;
      const std::vector<size_t> subbandIndices;
      const bool processingSubband0;

      size_t nrSamplesPerSubband;
      size_t nrStations;
      size_t nrBitsPerSample;
    };
  }
}

#endif