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
#include <boost/format.hpp>

#ifdef HAVE_MPI
#include <mpi.h>
#include <InputProc/Transpose/MPISendStation.h>
#include <InputProc/Transpose/MPIProtocol.h>
#include <InputProc/Transpose/MapUtil.h>
#endif

#include <Common/LofarLogger.h>
#include <Common/Thread/Semaphore.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Pool.h>

#include <InputProc/SampleType.h>
#include <InputProc/Buffer/StationID.h>
#include <InputProc/Buffer/BufferSettings.h>
#include <InputProc/Buffer/BoardMode.h>
#include <InputProc/Buffer/BlockReader.h>
#include <InputProc/Buffer/SampleBuffer.h>
#include <InputProc/Station/PacketsToBuffer.h>
#include <InputProc/Delays/Delays.h>

#include "StationNodeAllocation.h"

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

namespace LOFAR {
  namespace Cobalt {

// Data received from an RSP board
struct RSPData {
  int board;

  std::vector<struct RSP> packets;
  std::vector<bool>       valid; 

  RSPData(size_t numPackets):
    packets(numPackets),
    valid(numPackets, false)
  {
  }
};

// Data meant to be sent over MPI to the receivers
template<typename SampleT>
struct MPIData {
  ssize_t block;
  TimeStamp from;
  TimeStamp to;
  size_t nrSamples;

  /*
   * The order of the subbands in the arrays below is
   * those of the subbands processed by all receiving
   * ranks concatenated (values(subbandDistribution).
   *
   * For example, with 2 ranks and 4 subbands, the
   * order will likely be:
   *
   *   0, 2, 1, 3
   *
   * because rank 0 will process [0, 2] and rank 1
   * will process [1, 3].
   */

  MultiDimArray<SampleT, 2> mpi_samples; // [subband][sample]
  MultiDimArray<MPIProtocol::MetaData, 1> mpi_metaData; // [subband]

  std::vector<struct SubbandMetaData> metaData; // [subband]
  std::vector<ssize_t> read_offsets; // [subband]

  MPIData(size_t nrSubbands, size_t nrSamples):
    mpi_samples(boost::extents[nrSubbands][nrSamples], 1, mpiAllocator),
    mpi_metaData(boost::extents[nrSubbands], 1, mpiAllocator),
    metaData(nrSubbands),
    read_offsets(nrSubbands)
  {
  }

  // Returns true if we're spilling into the next packet
  bool write(struct RSP &packet, const vector<ssize_t> &beamletIndices);
};

template<typename SampleT>
bool MPIData<SampleT>::write(struct RSP &packet, const vector<ssize_t> &beamletIndices) {
  const TimeStamp packetBegin = packet.timeStamp();
  const TimeStamp packetEnd   = packetBegin + packet.header.nrBlocks;

  bool consider_next = false;

  const SampleT *srcPtr = reinterpret_cast<const SampleT*>(&packet.payload.data[0]);

  for (size_t b = 0; b < packet.header.nrBeamlets; ++b) {
    const ssize_t absBeamlet = beamletIndices[b];

    /* Discard beamlets that are not used in the observation */
    if (absBeamlet == -1)
      continue;

    const ssize_t offset = read_offsets[absBeamlet];
    const TimeStamp beamletBegin = packetBegin + offset;
    const TimeStamp beamletEnd   = packetEnd   + offset;

    /* XXXX    = packet data
     * [.....] = this->data
     */

    /* XXXX [......] -> discard */
    if (beamletEnd < from)
      continue;

    /* [......] XXXX -> discard */
    if (beamletBegin >= to) {
      consider_next = true;
      continue;
    }

    // number of samples to transfer
    size_t nrSamplesToCopy   = packet.header.nrBlocks;

    // first sample to write
    size_t firstSample = 0;

    /* XX [XX....] -> cut off start */
    if (beamletBegin < from) {
      firstSample      = from - beamletBegin;
      nrSamplesToCopy -= firstSample;
    }

    /* [...XX] XX -> cut off end */
    if (beamletEnd >= to) {
      nrSamplesToCopy -= beamletEnd - to;
    }

    if (beamletEnd - 1 >= to) {
      // The next packet has at most 1 sample overlap with this packet,
      // due to the speed of light and the earth's rotation speed.
      consider_next = true;
    }

    /* Write the remaining data */
    memcpy(&mpi_samples[absBeamlet][beamletBegin - from + firstSample],
	   srcPtr + b * packet.header.nrBlocks + firstSample,
	   nrSamplesToCopy * sizeof(SampleT));

    /* Update the flags */
    metaData[absBeamlet].flags.include(firstSample, firstSample + nrSamplesToCopy);
  }

  return consider_next;
}


template <typename SampleT>
class StationInput {
public:
  StationInput( const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution );

  void computeMetaData( Pool< MPIData<SampleT> > &metaDataPool );

private:
  const Parset &ps;
  const size_t stationIdx;
  const struct StationID stationID;
  const std::string logPrefix;

  const TimeStamp startTime;
  const TimeStamp stopTime;

  const size_t nrSamples;
  const size_t nrBlocks;

  const SubbandDistribution subbandDistribution;
  const std::vector<size_t> targetSubbands;
};

template <typename SampleT>
StationInput<SampleT>::StationInput( const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution )
:
  ps(ps),
  stationIdx(stationIdx),
  stationID(StationID::parseFullFieldName(ps.settings.stations.at(stationIdx).name)),
  logPrefix(str(format("[station %s] ") % stationID.name())),

  startTime(ps.startTime() * ps.subbandBandwidth(), ps.clockSpeed()),
  stopTime(ps.stopTime() * ps.subbandBandwidth(), ps.clockSpeed()),

  nrSamples(ps.nrSamplesPerSubband()),
  nrBlocks((stopTime - startTime) / nrSamples),

  subbandDistribution(subbandDistribution),
  targetSubbands(values(subbandDistribution))
{
}

/*
 * Initialises blocks from metaDataPool, and adds meta data.
 *
 * Input: metaDataPool.free
 * Output: metaDataPool.filled
 */
template <typename SampleT>
void StationInput<SampleT>::computeMetaData( Pool< MPIData<SampleT> > &metaDataPool )
{
  /*
   * Set up delay compensation.
   */

  Delays delays(ps, stationIdx, startTime, nrSamples);
  delays.start();

  // We keep track of the delays at the beginning and end of each block.
  // After each block, we'll swap the afterEnd delays into atBegin.
  Delays::AllDelays delaySet1(ps), delaySet2(ps);
  Delays::AllDelays *delaysAtBegin  = &delaySet1;
  Delays::AllDelays *delaysAfterEnd = &delaySet2;

  // Get delays at begin of first block
  delays.getNextDelays(*delaysAtBegin);

  for (ssize_t block = -1; block < (ssize_t)nrBlocks; ++block) {
    LOG_INFO_STR(logPrefix << str(format("[block %d] Retrieving delays") % block));

    // Fetch end delays (start delays are set by the previous block, or
    // before the loop).
    delays.getNextDelays(*delaysAfterEnd);

    // INPUT
    SmartPtr< MPIData<SampleT> > mpiData = metaDataPool.free.remove();

    LOG_INFO_STR(logPrefix << str(format("[block %d] Applying delays") % block));

    // Compute the next set of metaData and read_offsets from the new
    // delays pair.
    delays.generateMetaData(*delaysAtBegin, *delaysAfterEnd, targetSubbands, mpiData->metaData, mpiData->read_offsets);

    // Annotate
    mpiData->block = block;
    mpiData->nrSamples = nrSamples;
    mpiData->from  =
      mpiData->block == -1 ? startTime - nrSamples
                           : startTime + mpiData->block * nrSamples;
    mpiData->to    = mpiData->from + nrSamples;

    // Clear flags
    for (size_t sb = 0; sb < ps.nrSubbands(); ++sb) {
      mpiData->metaData[sb].flags.reset();
    }

    // OUTPUT
    metaDataPool.filled.append(mpiData);
    ASSERT(!mpiData);

    // Swap delay sets to accomplish delaysAtBegin = delaysAfterEnd
    swap(delaysAtBegin, delaysAfterEnd);
  }

  // Signal EOD
  metaDataPool.filled.append(NULL);
}

template<typename SampleT> void sendInputToPipeline(const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution)
{
  StationInput<SampleT> si(ps, stationIdx, subbandDistribution);

  const struct StationID stationID(StationID::parseFullFieldName(ps.settings.stations.at(stationIdx).name));
  const std::string logPrefix = str(format("[station %s] ") % stationID.name());

  StationNodeAllocation allocation(stationID, ps);

  if (!allocation.receivedHere()) {
    // Station is not sending from this node
    return;
  }

  LOG_INFO_STR(logPrefix << "Processing station data");

  const TimeStamp startTime(ps.startTime() * ps.subbandBandwidth(), ps.clockSpeed());
  const TimeStamp stopTime(ps.stopTime() * ps.subbandBandwidth(), ps.clockSpeed());

  const ssize_t nrBlocks = (stopTime - startTime) / ps.nrSamplesPerSubband();

  // Don't run if there is no data to process
  if (nrBlocks == 0)
    return;

  Pool< MPIData<SampleT> > metaDataPool;
  Pool< MPIData<SampleT> > dataPool;

  Pool< RSPData > rspDataPool;

  for (size_t i = 0; i < 3; ++i)
    metaDataPool.free.append(new MPIData<SampleT>(ps.nrSubbands(), ps.nrSamplesPerSubband()));

  for (size_t i = 0; i < 1024; ++i)
    rspDataPool.free.append(new RSPData(256));

  const BoardMode mode(ps.settings.nrBitsPerSample, ps.settings.clockMHz);

  // Order the subbands by receiving rank
  const vector<size_t> targetSubbands = values(subbandDistribution);

  /*
   * Stream the data.
   */
  #pragma omp parallel sections num_threads(3)
  {
    /*
     * METADATA POOL: Adds delays to blocks
     */
    #pragma omp section
    {
      si.computeMetaData(metaDataPool);
    }

    /*
     * PACKET READERS: Read input data from station
     *
     *
     * Reads:  rspDataPool.free
     * Writes: rspDataPool.filled
     *
     * Packets written will have roughly equal time stamps (that is,
     * they won't differ by a block), or contain some lingering data
     * from before. The latter happens if packet loss occurs and no
     * full set of packets could be read.
     */
    #pragma omp section
    {
      // fetch input streams
      vector< SmartPtr<Stream> > rspStreams(allocation.inputStreams());
      vector< SmartPtr<PacketReader> > readers;

      for(size_t i = 0; i < rspStreams.size(); ++i)
        readers.push_back(new PacketReader(logPrefix, *rspStreams[i]));

      LOG_INFO_STR(logPrefix << "Processing packets");

      if (ps.realTime()) {
        /*
         * In real-time mode, we can't get ahead of the `current'
         * block of the reader due to clock synchronisation.
         */

        const time_t LOG_INTERVAL = 10;

        #pragma omp parallel for num_threads(rspStreams.size())
        for(size_t board = 0; board < rspStreams.size(); board++) {
          time_t lastlog_timestamp = 0;

          PacketReader &reader = *readers[board];

          try {
            for(;;) {
              // Fill rspDataPool elements with RSP packets
              SmartPtr<RSPData> data = rspDataPool.free.remove();

              reader.readPackets(data->packets, data->valid);
              data->board = board;

              rspDataPool.filled.append(data);

              // Periodically log progress
              if (data->valid[0] && data->packets[0].header.timestamp > lastlog_timestamp + LOG_INTERVAL) {
                lastlog_timestamp = data->packets[0].header.timestamp;

                reader.logStatistics();
              }
            }
          } catch (Stream::EndOfStreamException &ex) {
            // Ran out of data
            LOG_INFO_STR( logPrefix << "End of stream");

          } catch (SystemCallException &ex) {
            if (ex.error == EINTR || ex.error == 512 /* ERESTARTSYS, should not be propagated to user space?! */)
              LOG_INFO_STR( logPrefix << "Aborted: " << ex.what());
            else
              LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
          } catch (Exception &ex) {
            LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
          }
        }
      } else {
        /* non-rt */

        /* Since the boards will be read at different speeds, we need to
         * manually keep them in sync. We read a packet from each board,
         * and let the board providing the youngest packet read again.
         */

        // Cache for last packets read from each board
        vector<struct RSP> last_packets(rspStreams.size());

        // Whether we should refresh last_packets[board]
        vector<bool> read_next_packet(rspStreams.size(), true);

        // Whether the stream for each board is still available
        vector<bool> stream_active(rspStreams.size(), true);

        for(;;) {
          for(size_t board = 0; board < rspStreams.size(); board++) {
            if (!read_next_packet[board] || !stream_active[board])
              continue;

            PacketReader &reader = *readers[board];

            try {
              // Retry until we have a valid packet
              while (!reader.readPacket(last_packets[board]))
                ;
            } catch (Stream::EndOfStreamException &ex) {
              // Ran out of data
              LOG_INFO_STR( logPrefix << "End of stream");

              stream_active[board] = false;
            }
          }

          // Determine youngest packet
          int youngest = -1;

          for (size_t board = 0; board < rspStreams.size(); board++) {
            if (!stream_active[board])
              continue;

            if (youngest == -1 || last_packets[youngest].timeStamp() > last_packets[board].timeStamp())
              youngest = board;
          }

          // Break if all streams turned out to be inactive
          if (youngest == -1)
            break;

          // Emit youngest packet
          SmartPtr<RSPData> data = rspDataPool.free.remove();

          for (size_t i = 0; i < data->valid.size(); ++i)
            data->valid[i] = false;

          data->valid[0]   = true;
          data->packets[0] = last_packets[youngest];
          data->board      = youngest;
         
          rspDataPool.filled.append(data);

          // Next packet should only be read from the stream we
          // emitted from
          for(size_t board = 0; board < rspStreams.size(); board++)
            read_next_packet[board] = (board == (size_t)youngest);
        }
      }

      // Signal EOD by inserting a packet beyond obs end
      SmartPtr<RSPData> data = rspDataPool.free.remove();
      for (size_t i = 0; i < data->valid.size(); ++i)
        data->valid[i] = false;

      data->valid[0]   = true;
      data->packets[0].timeStamp(TimeStamp::universe_heat_death(mode.clockHz()));
      data->board      = 0;
 
      rspDataPool.filled.append(data);
    }

    /*
     * DATA POOL: Adds samples to block
     */
    #pragma omp section
    {
      Thread::ScopedPriority sp(SCHED_FIFO, 10);

      const TimeStamp maxDelay(mode.secondsToSamples(0.5), mode.clockHz());

      // Determine the beamlet -> targetSubband mapping for THIS station,
      // allowing us to translate the beamlets from an RSP packet to
      // offsets into mpiData->data and mpiData->metaData directly.
      vector< vector<ssize_t> > beamletIndices(
        4, // support 4 RSP boards
	vector<ssize_t>(mode.nrBeamletsPerBoard(), -1) // -1 = discard this beamlet
	);

      for(size_t i = 0; i < ps.nrSubbands(); ++i) {
        // The subband stored at position i
        const size_t sb = targetSubbands.at(i);

	// The corresponding (board,slot) combination for that subband
        const size_t board = ps.settings.stations[stationIdx].rspBoardMap[sb];
        const size_t slot  = ps.settings.stations[stationIdx].rspSlotMap[sb];

        // The specified (board,slot) is stored at position i
	ASSERT(beamletIndices.at(board).at(slot) == -1);
	beamletIndices.at(board).at(slot) = i;
      }

      /*
       * We maintain a `current' and a `next' block,
       * because the offsets between subbands can require
       * us to write to two blocks alternately.
       *
       * Also, data can arrive slightly out-of-order.
       */

      SmartPtr< MPIData<SampleT> > current, next;

      SmartPtr<RSPData> rspData;

      while((current = metaDataPool.filled.remove()) != NULL) {
        const ssize_t block = current->block;

        next = metaDataPool.filled.remove();

        /*
         * Wait until we've filled up the current block.
         * Due to out-of-order arrival, and because we give data
         * ample time to arrive, we will occasionally
         * spill into the next block.
         */

        if (ps.realTime()) {
          /* real-time */

          /*
           * 1. We can receive data out-of-order.
           * 2. We can't receive data from beyond `next', because
           *    we sync on the wall clock.
           * 3. We can receive old data from before `current'.
           */

          const TimeStamp deadline = current->to + maxDelay;

          LOG_INFO_STR("[block " << block << "] Waiting until " << deadline << " for " << current->from << " to " << current->to);

          const TimeStamp now = TimeStamp::now(deadline.getClock());

          if (deadline < now) {
            // We're too late! Don't process data, or we'll get even further behind!
            LOG_ERROR_STR("[block " << block << "] Not running at real time! Deadline was " <<
              TimeStamp(now - deadline, deadline.getClock()).getSeconds() << " seconds ago");
          } else {
            // Whether we have read data beyond `next'. This can happen in non-rt mode after a period of packet loss.
            // In that case, we retain rspData to be written into future blocks.

            while((rspData = rspDataPool.filled.remove(deadline, NULL)) != NULL) {
              // Write valid packets to the current and/or next packet
              for (size_t p = 0; p < rspData->valid.size(); ++p) {
                if (!rspData->valid[p])
                  continue;

	        if (current->write(rspData->packets[p], beamletIndices.at(rspData->board))
                 && next) {
                  // We have data (potentially) spilling into `next'.

                  next->write(rspData->packets[p], beamletIndices.at(rspData->board));
                }
              }
            }

	    rspDataPool.free.append(rspData);
	    ASSERT(!rspData);
          }
        } else {
          /* non-rt mode */

          LOG_INFO_STR("[block " << block << "] Waiting for data");

          /*
           * 1. We'll receive one packet at a time, in-order.
           * 2. We might receive data beyond what we're processing now.
           *    In that case, we break and keep rspData around.
           */

          for(;;) {
            if (!rspData)
              rspData = rspDataPool.filled.remove();

            // Only packet 0 is used in non-rt mode
            ASSERT(rspData->valid[0]);

            if (current->write(rspData->packets[0], beamletIndices.at(rspData->board))) {
              // We have data (potentially) spilling into `next'.
              if (next && next->write(rspData->packets[0], beamletIndices.at(rspData->board))) {
                // Even younger data than next? Retain this data for a future block.
                break;
              }
            }

	    rspDataPool.free.append(rspData);
	    ASSERT(!rspData);
          }
        }

        dataPool.filled.append(current);
        ASSERT(!current);

        current = next;
      }

      dataPool.filled.append(NULL);
    }

    /*
     * MPI POOL: Send block to receivers
     */
    #pragma omp section
    {
      // All receiver ranks -- we have a dedicated thread for each one
      const vector<int> targetRanks(keys(subbandDistribution));

      // Determine the offset of the set of subbands for each rank within
      // the members in MPIData<SampleT>.
      vector<size_t> subbandsOffset(targetRanks.size());

      for (size_t rank = 0, current_offset = 0; rank < targetRanks.size(); ++rank) {
        subbandsOffset[rank] = current_offset;
        current_offset      += subbandDistribution.at(rank).size();
      }

      SmartPtr< MPIData<SampleT> > mpiData;

      // Convert the metaData -> mpi_metaData for transfer over MPI
      for(size_t sb = 0; sb < ps.nrSubbands(); ++sb) {
        mpiData->mpi_metaData[sb] = mpiData->metaData[sb];
      }

      while((mpiData = dataPool.filled.remove()) != NULL) {
        const ssize_t block = mpiData->block;

        // Send to all receivers in PARALLEL for higher performance
#       pragma omp parallel for num_threads(targetRanks.size())
        for(size_t i = 0; i < targetRanks.size(); ++i) {
          const int rank = targetRanks.at(i);

          if (subbandDistribution.at(rank).empty())
            continue;

          LOG_INFO_STR(logPrefix << str(format("[block %d -> rank %i] Sending data") % block % rank));

          MPISendStation sender(stationIdx, rank, subbandDistribution.at(rank), ps.nrSamplesPerSubband());

          const size_t offset = subbandsOffset[rank];

          sender.sendBlock<SampleT>(&mpiData->mpi_samples[offset][0], &mpiData->mpi_metaData[offset]);

          LOG_DEBUG_STR(logPrefix << str(format("[block %d -> rank %i] Data sent") % block % rank));
        }

        metaDataPool.free.append(mpiData);
        ASSERT(!mpiData);
      }
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

#ifndef HAVE_MPI
DirectInput &DirectInput::instance(const Parset *ps) {
  static DirectInput *theInstance = NULL;

  if (!theInstance) {
    ASSERT(ps != NULL);
    theInstance = new DirectInput(*ps);
  }

  return *theInstance;
}

DirectInput::DirectInput(const Parset &ps)
:
  ps(ps),
  stationDataQueues(boost::extents[ps.nrStations()][ps.nrSubbands()])
{
  // Create queues to forward station data

  for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
    for (size_t sb = 0; sb < ps.nrSubbands(); ++sb) {
      stationDataQueues[stat][sb] = new BestEffortQueue< SmartPtr<struct InputBlock> >(1, ps.realTime());
    }
  }
}

template<typename T>
void DirectInput::sendBlock(unsigned stationIdx, const struct BlockReader<T>::LockedBlock &block, const vector<SubbandMetaData> &metaDatas)
{
  // Send the block to the stationDataQueues global object
  for (size_t subband = 0; subband < block.beamlets.size(); ++subband) {
    const struct Cobalt::Block<T>::Beamlet &beamlet = block.beamlets[subband];

    /* create new block */
    SmartPtr<struct InputBlock> pblock = new InputBlock;

    pblock->samples.resize(ps.nrSamplesPerSubband() * sizeof(T));

    /* copy metadata */
    pblock->metaData = metaDatas[subband];

    /* copy data */
    beamlet.copy(reinterpret_cast<T*>(&pblock->samples[0]));

    if (subband == 0) {
      LOG_DEBUG_STR("Flags at begin: " << beamlet.flagsAtBegin);
    }

    /* obtain flags (after reading the data!) */
    pblock->metaData.flags = beamlet.flagsAtBegin | block.flags(subband);

    /* send to pipeline */
    stationDataQueues[stationIdx][subband]->append(pblock);
  }
}

// Instantiate the required templates
template void DirectInput::sendBlock< SampleType<i16complex> >(unsigned stationIdx, const struct BlockReader< SampleType<i16complex> >::LockedBlock &block, const vector<SubbandMetaData> &metaDatas);
template void DirectInput::sendBlock< SampleType<i8complex> >(unsigned stationIdx, const struct BlockReader< SampleType<i8complex> >::LockedBlock &block, const vector<SubbandMetaData> &metaDatas);
template void DirectInput::sendBlock< SampleType<i4complex> >(unsigned stationIdx, const struct BlockReader< SampleType<i4complex> >::LockedBlock &block, const vector<SubbandMetaData> &metaDatas);

template<typename T>
void DirectInput::receiveBlock(std::vector<struct ReceiveStations::Block<T> > &blocks)
{
  for (size_t subbandIdx = 0; subbandIdx < ps.nrSubbands(); ++subbandIdx) {
    for (size_t stationIdx = 0; stationIdx < ps.nrStations(); ++stationIdx) {
      // Read all data directly
      SmartPtr<struct DirectInput::InputBlock> pblock = stationDataQueues[stationIdx][subbandIdx]->remove();

      // Copy data
      memcpy(blocks[stationIdx].beamlets[subbandIdx].samples, &pblock->samples[0], pblock->samples.size() * sizeof(pblock->samples[0]));

      // Copy meta data
      blocks[stationIdx].beamlets[subbandIdx].metaData = pblock->metaData;
    }
  }
}

// Instantiate the required templates
template void DirectInput::receiveBlock< SampleType<i16complex> >(std::vector<struct ReceiveStations::Block< SampleType<i16complex> > > &blocks);
template void DirectInput::receiveBlock< SampleType<i8complex> >(std::vector<struct ReceiveStations::Block< SampleType<i8complex> > > &blocks);
template void DirectInput::receiveBlock< SampleType<i4complex> >(std::vector<struct ReceiveStations::Block< SampleType<i4complex> > > &blocks);
#endif

  }
}

