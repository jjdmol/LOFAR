//# MPIReceiver.cc
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
//# $Id$

#include <lofar_config.h>

#include "MPIReceiver.h"
#include <InputProc/Transpose/MPIProtocol.h>

namespace LOFAR
{
  namespace Cobalt
  {

    template<typename SampleT> void MPIRecvData::allocate(
        size_t nrStations, size_t nrBeamlets, size_t nrSamples)
    {
      data = (char*)mpiAllocator.allocate(
              nrStations * nrBeamlets * nrSamples * sizeof(SampleT));
      metaData = (char*)mpiAllocator.allocate(
            nrStations * nrBeamlets * sizeof(struct MPIProtocol::MetaData));
    }

    template void MPIRecvData::allocate< SampleType<i16complex> >(
        size_t nrStations, size_t nrBeamlets, size_t nrSamples);
    template void MPIRecvData::allocate< SampleType<i8complex> >(
        size_t nrStations, size_t nrBeamlets, size_t nrSamples);
    template void MPIRecvData::allocate< SampleType<i4complex> >(
      size_t nrStations, size_t nrBeamlets, size_t nrSamples);

    MPIReceiver::MPIReceiver(Pool<struct MPIRecvData> &pool,
      const std::vector<size_t> &subbandIndices,
      const bool processingSubband0,
      size_t i_nrSamplesPerSubband,
      size_t i_nrStations,
      size_t i_nrBitsPerSample)
      :
      mpiPool(pool),
      subbandIndices(subbandIndices),
      processingSubband0(processingSubband0),
      nrSamplesPerSubband(i_nrSamplesPerSubband),
      nrStations(i_nrStations),
      nrBitsPerSample(i_nrBitsPerSample)
    {}

    template<typename SampleT> void MPIReceiver::receiveInput()
    {
      NSTimer receiveTimer("MPI: Receive station data", true, true);
     
      // RECEIVE: Set up to receive our subbands as indicated by subbandIndices
      MPIReceiveStations receiver(nrStations, subbandIndices, 
                                  nrSamplesPerSubband);

      // Fill the pool with data items
      size_t N_PoolItems = 4;
      for (size_t i = 0; i < N_PoolItems; i++)
      {
        // Create a raw + meta datablock
        SmartPtr<struct MPIRecvData> mpiData = new MPIRecvData;

        // allocate the data (using mpi allocate)
        mpiData->allocate<SampleT>(nrStations, subbandIndices.size(),
         nrSamplesPerSubband);

        // add to the free pool
        mpiPool.free.append(mpiData, false);
      }

      bool allDone = false;

      // Receive input from StationInput::sendInputToPipeline.
      //
      // Start processing from block -1, and don't process anything if the
      // observation is empty.
      for (ssize_t block = -1; !allDone; block++) 
      {
        // Receive the samples from all subbands from the ant fields for this block.
        LOG_INFO_STR("[block " << block << "] Collecting input buffers");

        // Get a free data item
        SmartPtr<struct MPIRecvData> mpiData = mpiPool.free.remove();

        mpiData->block = block;

        // Lay a multidim array on the raw data
        MultiDimArray<SampleT, 3> data(
          boost::extents[nrStations][subbandIndices.size()][
              nrSamplesPerSubband],
          (SampleT*)mpiData->data.get(), false);

        // Idem for data: map multidim array
        MultiDimArray<struct MPIProtocol::MetaData, 2> metaData(
          boost::extents[nrStations][subbandIndices.size()],
          (struct MPIProtocol::MetaData*)mpiData->metaData.get(), false);

        LOG_INFO_STR("[block " << block << "] Receive input");

        if (block > 2) // allow warmup before starting the timers
          receiveTimer.start();
        allDone = receiver.receiveBlock<SampleT>(data, metaData);
        if (block > 2) 
          receiveTimer.stop();

        if (processingSubband0)
          LOG_INFO_STR("[block " << block << "] Input received");
        else
          LOG_INFO_STR("[block " << block << "] Input received");

        if (allDone)
          mpiPool.free.append(mpiData);
        else
          mpiPool.filled.append(mpiData);
      }

      // Signal end of input
      mpiPool.filled.append(NULL);
    }

    void MPIReceiver::receiveInput()
    {
      switch (nrBitsPerSample) {
      default:
      case 16:
        receiveInput< SampleType<i16complex> >();
        break;
      case 8:
        receiveInput< SampleType<i8complex> >();
        break;
      case 4:
        receiveInput< SampleType<i4complex> >();
        break;
      }
    }
  }
}
