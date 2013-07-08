//# StationInput.cc: Routines to manage I/O from the stations.
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

#include <lofar_config.h>

#include "StationInput.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#ifdef HAVE_MPI
#include <mpi.h>
#include <InputProc/Transpose/MPISendStation.h>
#endif

#include <Common/LofarLogger.h>
#include <Common/Thread/Semaphore.h>
#include <CoInterface/Parset.h>

#include <InputProc/SampleType.h>
#include <InputProc/Buffer/StationID.h>
#include <InputProc/Buffer/BufferSettings.h>
#include <InputProc/Buffer/BlockReader.h>
#include <InputProc/Buffer/SampleBuffer.h>
#include <InputProc/Station/PacketsToBuffer.h>
#include <InputProc/Delays/Delays.h>

#include "StationNodeAllocation.h"

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

namespace LOFAR {
  namespace Cobalt {

#ifndef HAVE_MPI
    MultiDimArray< SmartPtr< BestEffortQueue< SmartPtr<struct InputBlock> > >, 2> stationDataQueues; // [stationIdx][globalSubbandIdx]
#endif

void receiveStation(const Parset &ps, const struct StationID &stationID, Semaphore &stopSignal)
{
  // settings for the circular buffer
  struct BufferSettings settings(stationID, false);
  settings.setBufferSize(2.0);

  // Remove lingering buffers
  removeSampleBuffers(settings);

  // fetch input streams
  StationNodeAllocation allocation(stationID, ps);
  vector< SmartPtr<Stream> > inputStreams(allocation.inputStreams());

  settings.nrBoards = inputStreams.size();

  // Force buffer reader/writer syncing if observation is non-real time
  SyncLock syncLock(settings);
  if (!ps.realTime()) {
    settings.sync = true;
    settings.syncLock = &syncLock;
  }

  // Set up the circular buffer
  MultiPacketsToBuffer station(settings, inputStreams);

  #pragma omp parallel sections
  {
    // Start a circular buffer
    #pragma omp section
    {
      LOG_INFO_STR("Starting circular buffer");
      station.process();
    }

    // Wait for parent to stop us (for cases that do not
    // trigger EOF in the inputStreams, such as reading from
    // UDP).
    #pragma omp section
    {
      stopSignal.down();
      station.stop();
    }
  }
}

template<typename SampleT> void sendInputToPipeline(const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution)
{
  /*
   * Construct our stationID.
   */

  // fetch station name (e.g. CS001HBA0)
  const string fullFieldName = ps.settings.stations[stationIdx].name;

  // split into station name and antenna field name
  const string stationName = fullFieldName.substr(0,5); // CS001
  const string fieldName   = fullFieldName.substr(5);   // HBA0

  const struct StationID stationID(stationName, fieldName);

  StationNodeAllocation allocation(stationID, ps);

  if (!allocation.receivedHere()) {
    // Station is not sending from this node
    return;
  }

  LOG_INFO_STR("Processing data from station " << stationID);

  Semaphore stopSignal;

  /*
   * Stream the data.
   */
  #pragma omp parallel sections
  {
    // Start a circular buffer
    #pragma omp section
    { 
      receiveStation(ps, stationID, stopSignal);
    }

    // Send data to receivers
    #pragma omp section
    {
      // Fetch buffer settings from SHM.
      const struct BufferSettings settings(stationID, true);
      const struct BoardMode mode(ps.settings.nrBitsPerSample, ps.settings.clockMHz);

      LOG_INFO_STR("Detected " << settings);

      /*
       * Set up circular buffer data reader.
       */
      vector<size_t> beamlets(ps.nrSubbands());
      for( size_t i = 0; i < beamlets.size(); ++i) {
        // Determine the beamlet number of subband i for THIS station
        unsigned board = ps.settings.stations[stationIdx].rspBoardMap[i];
        unsigned slot  = ps.settings.stations[stationIdx].rspSlotMap[i];

        unsigned beamlet = board * mode.nrBeamletsPerBoard() + slot;

        beamlets[i] = beamlet;
      }

      BlockReader<SampleT> reader(settings, mode, beamlets, ps.nrHistorySamples(), 0.25);

      const TimeStamp from(ps.startTime() * ps.subbandBandwidth(), ps.clockSpeed());
      const TimeStamp to(ps.stopTime() * ps.subbandBandwidth(), ps.clockSpeed());

      LOG_INFO_STR("Connecting to receivers to send " << from << " to " << to);


#ifdef HAVE_MPI
      /*
       * Set up the MPI send engine.
       */
      MPISendStation sender(settings, stationIdx, subbandDistribution);
#endif

      /*
       * Set up delay compensation.
       */
      Delays delays(ps, stationIdx, from, ps.nrSamplesPerSubband());
      delays.start();

      // We keep track of the delays at the beginning and end of each block.
      // After each block, we'll swap the afterEnd delays into atBegin.
      Delays::AllDelays delaySet1(ps), delaySet2(ps);
      Delays::AllDelays *delaysAtBegin  = &delaySet1;
      Delays::AllDelays *delaysAfterEnd = &delaySet2;

      // Get delays at begin of first block
      delays.getNextDelays(*delaysAtBegin);

      /*
       * Transfer all blocks.
       */
      LOG_INFO_STR("Sending to receivers");

      vector<SubbandMetaData> metaDatas(ps.nrSubbands());
      vector<ssize_t> read_offsets(ps.nrSubbands());

      for (TimeStamp current = from; current + ps.nrSamplesPerSubband() < to; current += ps.nrSamplesPerSubband()) {
        // Fetch end delays (start delays are set by the previous block, or
        // before the loop).
        delays.getNextDelays(*delaysAfterEnd);

        // Compute the next set of metaData and read_offsets from the new
        // delays pair.
        delays.generateMetaData(*delaysAtBegin, *delaysAfterEnd, metaDatas, read_offsets);

        //LOG_DEBUG_STR("Delays obtained");

        // Read the next block from the circular buffer.
        SmartPtr<struct BlockReader<SampleT>::LockedBlock> block(reader.block(current, current + ps.nrSamplesPerSubband(), read_offsets));

        //LOG_INFO_STR("Block read");

#ifdef HAVE_MPI
        // Send the block to the receivers
        sender.sendBlock<SampleT>(*block, metaDatas);
#else
        // Send the block to the stationDataQueues global object
        for (size_t subband = 0; subband < block->beamlets.size(); ++subband) {
          const struct Block<SampleT>::Beamlet &beamlet = block->beamlets[subband];

          /* create new block */
          SmartPtr<struct InputBlock> pblock = new InputBlock;

          pblock->samples.resize((ps.nrHistorySamples() + ps.nrSamplesPerSubband()) * sizeof(SampleT));

          /* copy metadata */
          pblock->metaData = metaDatas[subband];

          /* copy data */
          beamlet.copy(reinterpret_cast<SampleT*>(&pblock->samples[0]));

          if (subband == 0) {
            LOG_DEBUG_STR("Flags at begin: " << beamlet.flagsAtBegin);
          }

          /* obtain flags (after reading the data!) */
          pblock->metaData.flags = beamlet.flagsAtBegin | block->flags(subband);

          /* send to pipeline */
          stationDataQueues[stationIdx][subband]->append(pblock);
        }
#endif

        //LOG_INFO_STR("Block sent");

        // Swap delay sets to accomplish delaysAtBegin = delaysAfterEnd
        swap(delaysAtBegin, delaysAfterEnd);
      }

      /*
       * The end.
       */
      stopSignal.up();
    }
  }
}

void sendInputToPipeline(const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution)
{
  switch (ps.nrBitsPerSample()) {
    default:
    case 16: 
      sendInputToPipeline< SampleType<i16complex> >(ps, stationIdx, subbandDistribution);
      break;

    case 8: 
      sendInputToPipeline< SampleType<i8complex> >(ps, stationIdx, subbandDistribution);
      break;

    case 4: 
      sendInputToPipeline< SampleType<i4complex> >(ps, stationIdx, subbandDistribution);
      break;
  }
}

void sendInputInit(const Parset &ps) {
#ifdef HAVE_MPI
  (void)ps;
#else
  // Create queues to forward station data
  stationDataQueues.resize(boost::extents[ps.nrStations()][ps.nrSubbands()]);

  for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
    for (size_t sb = 0; sb < ps.nrSubbands(); ++sb) {
      stationDataQueues[stat][sb] = new BestEffortQueue< SmartPtr<struct InputBlock> >(1, ps.realTime());
    }
  }
#endif
}

  }
}

