//# MPI_utils.cc
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

#include "MPI_utils.h"
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

    MPIInput::MPIInput(
      Pool<struct MPIRecvData> &pool,
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

    template<typename SampleT> void MPIInput::receiveInput(size_t nrBlocks)
    {
      // Need SubbandProcs to send work to
      //ASSERT(workQueues.size() > 0);

      NSTimer receiveTimer("MPI: Receive station data", true, true);

      // The length of a block in samples
      size_t blockSize = nrSamplesPerSubband;

      // RECEIVE: Set up to receive our subbands as indicated by subbandIndices
#ifdef HAVE_MPI
      MPIReceiveStations receiver(nrStations, subbandIndices, blockSize);

      for (size_t i = 0; i < 4; i++) {
        SmartPtr<struct MPIRecvData> mpiData = new MPIRecvData;

        mpiData->allocate<SampleT>(nrStations, subbandIndices.size(), blockSize);

        mpiPool.free.append(mpiData, false);
      }

#else
      DirectInput &receiver = DirectInput::instance();
#endif

      // Receive input from StationInput::sendInputToPipeline.
      //
      // Start processing from block -1, and don't process anything if the
      // observation is empty.
      for (ssize_t block = -1; nrBlocks > 0 && block < ssize_t(nrBlocks); block++) {
        // Receive the samples from all subbands from the ant fields for this block.
        LOG_INFO_STR("[block " << block << "] Collecting input buffers");

        SmartPtr<struct MPIRecvData> mpiData = mpiPool.free.remove();

        mpiData->block = block;

        MultiDimArray<SampleT, 3> data(
          boost::extents[nrStations][subbandIndices.size()][
              nrSamplesPerSubband],
          (SampleT*)mpiData->data.get(), false);

        MultiDimArray<struct MPIProtocol::MetaData, 2> metaData(
          boost::extents[nrStations][subbandIndices.size()],
          (struct MPIProtocol::MetaData*)mpiData->metaData.get(), false);

        // Receive all subbands from all antenna fields
        LOG_INFO_STR("[block " << block << "] Receive input");

        if (block > 2) receiveTimer.start();
        receiver.receiveBlock<SampleT>(data, metaData);
        if (block > 2) receiveTimer.stop();

        if (processingSubband0)
          LOG_INFO_STR("[block " << block << "] Input received");
        else
          LOG_INFO_STR("[block " << block << "] Input received");

        mpiPool.filled.append(mpiData);
      }

      // Signal end of input
      mpiPool.filled.append(NULL);
    }

    template void MPIInput::receiveInput< SampleType<i16complex> >(size_t nrBlocks);
    template void MPIInput::receiveInput< SampleType<i8complex> >(size_t nrBlocks);
    template void MPIInput::receiveInput< SampleType<i4complex> >(size_t nrBlocks);

    void MPIInput::receiveInput(size_t nrBlocks)
    {
      switch (nrBitsPerSample) {
      default:
      case 16:
        receiveInput< SampleType<i16complex> >(nrBlocks);
        break;
      case 8:
        receiveInput< SampleType<i8complex> >(nrBlocks);
        break;
      case 4:
        receiveInput< SampleType<i4complex> >(nrBlocks);
        break;
      }
    }

    

  }
}