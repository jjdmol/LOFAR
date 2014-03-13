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
#include <InputProc/Transpose/MapUtil.h>
#include <InputProc/Transpose/MPIUtil2.h>
#endif

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <CoInterface/Parset.h>

#include <InputProc/SampleType.h>
#include <InputProc/Station/PacketReader.h>
#include <InputProc/Buffer/BoardMode.h>
#include <InputProc/Delays/Delays.h>
#include <InputProc/OMPThread.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

namespace LOFAR {
  namespace Cobalt {

template<typename SampleT>
bool MPIData<SampleT>::write(const struct RSP &packet, const ssize_t *beamletIndices) {
  /* An optimisation as we'll never encounter anything else */
  ASSERTSTR(packet.header.nrBlocks == 16, "Packet has " << (int)packet.header.nrBlocks << " samples/beamlet, expected 16.");
  const size_t nrSamples = 16;

  const uint64_t packetBegin = packet.timeStamp();
  const uint64_t packetEnd   = packetBegin + nrSamples;

  bool consider_next = false;

  const SampleT *srcPtr = reinterpret_cast<const SampleT*>(&packet.payload.data[0]);

  for (size_t b = 0; b < packet.header.nrBeamlets; ++b) {
    const ssize_t absBeamlet = beamletIndices[b];

    /* Discard beamlets that are not used in the observation */
    if (absBeamlet == -1)
      continue;

    /* Reading with offset X is the same as writing that data with offset -X */
    const ssize_t offset = read_offsets[absBeamlet];
    const uint64_t beamletBegin = packetBegin - offset;
    const uint64_t beamletEnd   = packetEnd   - offset;

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
    size_t nrSamplesToCopy  = nrSamples;

    // first sample to write
    size_t firstSample = 0;

    /* XX [XX....] -> cut off start */
    if (beamletBegin < from) {
      firstSample      = from - beamletBegin;
      nrSamplesToCopy -= firstSample;
    }

    /* [...XX] XX -> cut off end */
    if (beamletEnd > to) {
      nrSamplesToCopy -= beamletEnd - to;
      consider_next = true;
    }

    if (beamletEnd == to) {
      // The next packet has at most 1 sample overlap with this packet,
      // due to the speed of light and the earth's rotation speed.
      consider_next = true;
    }

    /* Absolute offset within our block */
    const size_t absSample = beamletBegin - from + firstSample;

    /* Write the remaining data */
    memcpy(&mpi_samples[absBeamlet][absSample],
	   srcPtr + b * nrSamples + firstSample,
	   nrSamplesToCopy * sizeof(SampleT));

    /* Update the flags */
    metaData[absBeamlet].flags.include(absSample, absSample + nrSamplesToCopy);
  }

  return consider_next;
}

template <typename SampleT>
StationMetaData<SampleT>::StationMetaData( const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution )
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
void StationMetaData<SampleT>::computeMetaData()
{
  /*
   * Allocate buffer elements.
   */

  // Each element represents 1 block of buffer.
  for (size_t i = 0; i < 5; ++i)
    metaDataPool.free.append(new MPIData<SampleT>(ps.nrSubbands(), nrSamples));

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
    LOG_DEBUG_STR(logPrefix << str(format("[block %d] Retrieving delays") % block));

    // Fetch end delays (start delays are set by the previous block, or
    // before the loop).
    delays.getNextDelays(*delaysAfterEnd);

    // INPUT
    SmartPtr< MPIData<SampleT> > mpiData = metaDataPool.free.remove();

    LOG_DEBUG_STR(logPrefix << str(format("[block %d] Applying delays") % block));

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


StationInput::StationInput( const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution )
:
  ps(ps),
  stationIdx(stationIdx),
  stationID(StationID::parseFullFieldName(ps.settings.stations.at(stationIdx).name)),
  allocation(stationID, ps),

  logPrefix(str(format("[station %s] ") % stationID.name())),

  mode(ps.settings.nrBitsPerSample, ps.settings.clockMHz),
  nrBoards(ps.settings.stations.at(stationIdx).inputStreams.size()),

  targetSubbands(values(subbandDistribution)),
  beamletIndices(generateBeamletIndices())
{
}


MultiDimArray<ssize_t, 2> StationInput::generateBeamletIndices()
{
  /*
   * We need to create a mapping from
   *
   * [board][slot]          = the dimensions of the RSP data
   *
   * to
   *
   * [subband]              = the dimensions of the data sent over MPI,
   *                          which is ordered by `targetSubbands'.
   */

  MultiDimArray<ssize_t, 2> result(boost::extents[nrBoards][mode.nrBeamletsPerBoard()]);

  // Any untouched [board][slot] means we'll discard that input
  for(size_t n = 0; n < result.num_elements(); ++n)
    result.origin()[n] = -1;

  for(size_t i = 0; i < ps.nrSubbands(); ++i) {
    // The subband stored at position i
    const size_t sb = targetSubbands.at(i);

    // The corresponding (board,slot) combination for that subband,
    // for this station.
    const size_t board = ps.settings.stations[stationIdx].rspBoardMap[sb];
    const size_t slot  = ps.settings.stations[stationIdx].rspSlotMap[sb];

    ASSERT(board < nrBoards);
    ASSERT(slot < mode.nrBeamletsPerBoard());

    // The specified (board,slot) is stored at position i
    ASSERTSTR(result[board][slot] == -1, "Station " << stationID.name() << ": board " << board << " slot " << slot << " is used multiple times!");
    result[board][slot] = i;
  }

  return result;
}


bool StationInput::receivedHere() const
{
  return allocation.receivedHere();
}


void StationInput::readRSPRealTime( size_t board, Stream &inputStream )
{
  /*
   * In real-time mode, we can't get ahead of the `current'
   * block of the reader due to clock synchronisation.
   */

  PacketReader reader(str(format("%s[board %s] ") % logPrefix % board), inputStream, mode);

  Queue< SmartPtr<RSPData> > &inputQueue = rspDataPool[board].free;
  Queue< SmartPtr<RSPData> > &outputQueue = rspDataPool[board].filled;

  try {
    for(size_t i = 0; true; i++) {
      // Fill rspDataPool elements with RSP packets
      SmartPtr<RSPData> rspData = inputQueue.remove();
     
      // Abort condition needed to avoid getting stuck in free.remove()
      if (!rspData)
        break;

      reader.readPackets(rspData->packets);

      // Periodically log progress
      if (i % 256 == 0) // Each block is ~40ms, so log every ~10s worth of data.
        reader.logStatistics();

      outputQueue.append(rspData);
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


template <typename SampleT>
void StationInput::writeRSPRealTime( MPIData<SampleT> &current, MPIData<SampleT> *next )
{
  /* real-time */

  /*
   * 1. We can receive data out-of-order.
   * 2. We can't receive data from beyond `next', because
   *    we sync on the wall clock.
   * 3. We can receive old data from before `current'.
   */

  const TimeStamp deadline = TimeStamp(current.to + mode.secondsToSamples(0.25), mode.clockHz());

  LOG_INFO_STR(logPrefix << "[block " << current.block << "] Waiting until " << deadline);

  const TimeStamp now = TimeStamp::now(mode.clockHz());

  if (deadline < now) {
    // We're too late! Don't process data, or we'll get even further behind!
    LOG_ERROR_STR(logPrefix << "[block " << current.block << "] Not running at real time! Deadline was " <<
      TimeStamp(now - deadline, deadline.getClock()).getSeconds() << " seconds ago");

  } else {
    // One core can't handle the load, so use multiple
#   pragma omp parallel for num_threads(nrBoards)
    for (size_t board = 0; board < nrBoards; board++) {
      NSTimer copyRSPTimer(str(format("%s [board %i] copy RSP -> block") % logPrefix % board), true, true);

      Queue< SmartPtr<RSPData> > &inputQueue = rspDataPool[board].filled;
      Queue< SmartPtr<RSPData> > &outputQueue = rspDataPool[board].free;

      const ssize_t *beamletIndices = &this->beamletIndices[board][0];

      SmartPtr<RSPData> rspData;

      while ((rspData = inputQueue.remove(deadline, NULL)) != NULL) {
        // Write valid packets to the current and/or next packet
        copyRSPTimer.start();

        for (size_t p = 0; p < RT_PACKET_BATCH_SIZE; ++p) {
          struct RSP &packet = rspData->packets[p];

          if (packet.payloadError())
            continue;

          if (current.write(packet, beamletIndices)
           && next) {
            // We have data (potentially) spilling into `next'.

            if (next->write(packet, beamletIndices)) {
              LOG_WARN_STR(logPrefix << "Received data for several blocks into the future -- discarding.");
            }
          }
        }
        copyRSPTimer.stop();

        outputQueue.append(rspData);
        ASSERT(!rspData);
      }
    }
  }
}


void StationInput::readRSPNonRealTime()
{
  vector< SmartPtr<Stream> > inputStreams(allocation.inputStreams());
  vector< SmartPtr<PacketReader> > readers(nrBoards);

  for (size_t i = 0; i < nrBoards; ++i)
    readers[i] = new PacketReader(logPrefix, *inputStreams[i]);

  /* Since the boards will be read at different speeds, we need to
   * manually keep them in sync. We read a packet from each board,
   * and let the board providing the youngest packet read again.
   *
   * We will multiplex all packets using rspDataPool[0].
   */

  // Cache for last packets read from each board
  vector<struct RSP> last_packets(nrBoards);

  // Whether we should refresh last_packets[board]
  vector<bool> read_next_packet(nrBoards, true);

  for(;;) {
    for(size_t board = 0; board < nrBoards; board++) {
      if (!read_next_packet[board] || !readers[board])
        continue;

      try {
        // Retry until we have a valid packet
        while (!readers[board]->readPacket(last_packets[board]))
          ;
      } catch (Stream::EndOfStreamException &ex) {
        // Ran out of data
        LOG_INFO_STR( logPrefix << "End of stream");

        readers[board] = NULL;
      }
    }

    // Determine which board provided the youngest packet
    int youngest = -1;

    for (size_t board = 0; board < nrBoards; board++) {
      if (!readers[board])
        continue;

      if (youngest == -1 || last_packets[youngest].timeStamp() > last_packets[board].timeStamp())
        youngest = board;
    }

    // Break if all streams turned out to be inactive
    if (youngest == -1)
      break;

    // Emit youngest packet
    SmartPtr<RSPData> data = rspDataPool[0].free.remove();

    // Abort of writer does not desire any more data
    if (!data)
      return;

    data->packets[0] = last_packets[youngest];
    data->board = youngest;
   
    rspDataPool[0].filled.append(data);

    // Next packet should only be read from the stream we
    // emitted from
    for(size_t board = 0; board < nrBoards; board++)
      read_next_packet[board] = (board == (size_t)youngest);
  }

  // Signal EOD by inserting a packet beyond obs end
  SmartPtr<RSPData> data = rspDataPool[0].free.remove();

  // Abort if writer does not desire any more data
  if (!data)
    return;

  data->packets[0].payloadError(false);
  data->packets[0].timeStamp(TimeStamp::universe_heat_death(mode.clockHz()));
  data->board = 0;

  rspDataPool[0].filled.append(data);
}


template <typename SampleT>
void StationInput::writeRSPNonRealTime( MPIData<SampleT> &current, MPIData<SampleT> *next )
{
  LOG_INFO_STR("[block " << current.block << "] Waiting for data");

  /*
   * 1. We'll receive one packet at a time, in-order.
   * 2. We might receive data beyond what we're processing now.
   *    In that case, we break and keep rspData around.
   */


  for(;;) {
    SmartPtr<RSPData> data = rspDataPool[0].filled.remove();
    const ssize_t *beamletIndices = &this->beamletIndices[data->board][0];

    // Only packet 0 is used in non-rt mode
    ASSERT(!data->packets[0].payloadError());

    if (current.write(data->packets[0], beamletIndices)) {
      // We have data (potentially) spilling into `next'.
      if (!next || next->write(data->packets[0], beamletIndices)) {
	// Data is even later than next? Put this data back for a future block.
        rspDataPool[0].filled.prepend(data);
	      break;
      }
    }

    rspDataPool[0].free.append(data);
    ASSERT(!data);
  }
}


template <typename SampleT>
void StationInput::processInput( Queue< SmartPtr< MPIData<SampleT> > > &inputQueue, Queue< SmartPtr< MPIData<SampleT> > > &outputQueue )
{
  vector<OMPThread> packetReaderThreads(nrBoards);

  if (ps.realTime()) {
    // Each board has its own pool to reduce lock contention
    for (size_t board = 0; board < nrBoards; ++board)
      for (size_t i = 0; i < 16; ++i)
        rspDataPool[board].free.append(new RSPData(RT_PACKET_BATCH_SIZE));
  } else {
    // We just process one packet at a time, merging all the streams into rspDataPool[0].
    for (size_t i = 0; i < 16; ++i)
      rspDataPool[0].free.append(new RSPData(1));
  }

  #pragma omp parallel sections num_threads(2)
  {
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
      LOG_INFO_STR(logPrefix << "Processing packets");

      if (ps.realTime()) {
        vector< SmartPtr<Stream> > inputStreams(allocation.inputStreams());

        #pragma omp parallel for num_threads(nrBoards)
        for(size_t board = 0; board < nrBoards; board++) {
          OMPThread::ScopedRun sr(packetReaderThreads[board]);

          Thread::ScopedPriority sp(SCHED_FIFO, 10);

          readRSPRealTime(board, *inputStreams[board]);
        }
      } else {
        readRSPNonRealTime();
      }
    }

    /*
     * inputQueue -> outputQueue
     */
    #pragma omp section
    {
      //Thread::ScopedPriority sp(SCHED_FIFO, 10);

      /*
       * We maintain a `current' and a `next' block,
       * because the offsets between subbands can require
       * us to write to two blocks in edge cases.
       *
       * Also, data can arrive slightly out-of-order.
       */

      SmartPtr< MPIData<SampleT> > current, next;

      while((current = next ? next : inputQueue.remove()) != NULL) {
        next = inputQueue.remove();

        if (ps.realTime()) {
          writeRSPRealTime<SampleT>(*current, next);
        } else {
          writeRSPNonRealTime<SampleT>(*current, next);
        }

        outputQueue.append(current);
        ASSERT(!current);

        if (!next) {
          // We pulled the NULL, so we're done
          break;
        }
      }

      // Signal EOD to output
      outputQueue.append(NULL);

      // Signal EOD to input
      for (size_t i = 0; i < nrBoards; ++i)
        rspDataPool[i].free.append(NULL);

      if (ps.realTime()) {
        // kill reader threads
        LOG_INFO_STR( logPrefix << "Stopping all boards" );
#       pragma omp parallel for num_threads(nrBoards)
        for (size_t i = 0; i < nrBoards; ++i)
          packetReaderThreads[i].kill();
      }
    }
  }
}

MPISender::MPISender( const std::string &logPrefix, size_t stationIdx, const SubbandDistribution &subbandDistribution )
:
  logPrefix(logPrefix),
  stationIdx(stationIdx),
  subbandDistribution(subbandDistribution),
  targetRanks(keys(subbandDistribution)),
  subbandOffsets(targetRanks.size(), 0),
  nrSubbands(values(subbandDistribution).size())
{
  // Determine the offset of the set of subbands for each rank within
  // the members in MPIData<SampleT>.
  for (size_t rank = 0; rank < targetRanks.size(); ++rank)
    for(size_t i = 0; i < rank; ++i)
      subbandOffsets[rank] += subbandDistribution.at(i).size();
}

template <typename SampleT>
void MPISender::sendBlocks( Queue< SmartPtr< MPIData<SampleT> > > &inputQueue, Queue< SmartPtr< MPIData<SampleT> > > &outputQueue )
{
  SmartPtr< MPIData<SampleT> > mpiData;

  NSTimer mpiSendTimer(str(format("%s MPI send data") % logPrefix), true, true);

  size_t nrProcessedSamples = 0;
  size_t nrFlaggedSamples = 0;

  while((mpiData = inputQueue.remove()) != NULL) {
    const ssize_t block = mpiData->block;
    const size_t  nrSamples = mpiData->nrSamples;

    nrProcessedSamples += nrSamples * nrSubbands;

    LOG_INFO_STR(logPrefix << str(format("[block %d] Finalising metaData") % block));

    // Convert the metaData -> mpi_metaData for transfer over MPI
    for(size_t sb = 0; sb < mpiData->metaData.size(); ++sb) {
      SubbandMetaData &md = mpiData->metaData[sb];

      // MPIData::write adds flags for what IS present, but the receiver
      // needs flags for what IS NOT present. So invert the flags here.
      mpiData->metaData[sb].flags = md.flags.invert(0, nrSamples);

      nrFlaggedSamples += md.flags.count();

      // Write the meta data into the fixed buffer.
      mpiData->mpi_metaData[sb] = md;
    }

    LOG_INFO_STR(logPrefix << str(format("[block %d] Sending data") % block));

    mpiSendTimer.start();

    std::vector<MPI_Request> requests;

    {
      ScopedLock sl(MPIMutex);

      for(size_t i = 0; i < targetRanks.size(); ++i) {
        const int rank = targetRanks.at(i);

        if (subbandDistribution.at(rank).empty())
          continue;

        MPISendStation sender(stationIdx, rank, subbandDistribution.at(rank), nrSamples);

        const size_t offset = subbandOffsets[rank];

        requests.push_back(sender.sendData<SampleT>(&mpiData->mpi_samples[offset][0]));
        requests.push_back(sender.sendMetaData(&mpiData->mpi_metaData[offset]));
      }
    }

    RequestSet rs(requests, true, str(format("station %d block %d") % stationIdx % block));
    rs.waitAll();

    mpiSendTimer.stop();

    LOG_INFO_STR(logPrefix << str(format("[block %d] Data sent") % block));

    outputQueue.append(mpiData);
    ASSERT(!mpiData);
  }

  // report average loss
  const double avgloss = nrProcessedSamples == 0 ? 0.0 : 100.0 * nrFlaggedSamples / nrProcessedSamples;

  LOG_INFO_STR(logPrefix << str(format("Average data loss/flagged: %.4f%%") % avgloss));
}

template<typename SampleT> void sendInputToPipeline(const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution)
{
  StationMetaData<SampleT> sm(ps, stationIdx, subbandDistribution);
  StationInput si(ps, stationIdx, subbandDistribution);

  if (sm.nrBlocks == 0 || !si.receivedHere()) {
    // Station is not sending from this node
    return;
  }

  const struct StationID stationID(StationID::parseFullFieldName(ps.settings.stations.at(stationIdx).name));
  const std::string logPrefix = str(format("[station %s] ") % stationID.name());

  LOG_INFO_STR(logPrefix << "Processing station data");

  Queue< SmartPtr< MPIData<SampleT> > > mpiQueue;

  MPISender sender(logPrefix, stationIdx, subbandDistribution);

  /*
   * Stream the data.
   */
  #pragma omp parallel sections num_threads(3)
  {
    /*
     * Generate blocks and add delays
     *
     * sm.metaDataPool.free -> sm.metaDataPool.filled
     */
    #pragma omp section
    {
      sm.computeMetaData();
      LOG_INFO_STR(logPrefix << "StationMetaData: done");
    }

    /*
     * Adds samples from the input streams to the blocks
     *
     * sm.metaDataPool.filled -> mpiQueue
     */
    #pragma omp section
    {
      si.processInput<SampleT>( sm.metaDataPool.filled, mpiQueue );
      LOG_INFO_STR(logPrefix << "StationInput: done");
    }

    /*
     * MPI POOL: Send block to receivers
     *
     * mpiQueue -> sm.metaDataPool.free
     */
    #pragma omp section
    {
      sender.sendBlocks<SampleT>( mpiQueue, sm.metaDataPool.free );
      LOG_INFO_STR(logPrefix << "MPISender: done");
    } 
  }

  LOG_INFO_STR(logPrefix << "Done processing station data");
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

