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
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <Stream/FileStream.h>
#include <Stream/StreamFactory.h>
#include <CoInterface/Parset.h>
#include <CoInterface/OMPThread.h>
#include <CoInterface/TimeFuncs.h>
#include <CoInterface/Stream.h>
#include <CoInterface/PrintVector.h>

#include <InputProc/SampleType.h>
#include <InputProc/Station/PacketReader.h>
#include <InputProc/Station/PacketFactory.h>
#include <InputProc/Station/PacketStream.h>
#include <InputProc/Buffer/BoardMode.h>
#include <InputProc/Transpose/MapUtil.h>
#include <InputProc/Delays/Delays.h>
#include <InputProc/RSPTimeStamp.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;
using boost::format;

namespace LOFAR {
  namespace Cobalt {

    template <typename SampleT>
    StationMetaData<SampleT>::StationMetaData( const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution )
    :
      ps(ps),
      stationIdx(stationIdx),
      stationID(StationID::parseFullFieldName(ps.settings.antennaFields.at(stationIdx).name)),
      logPrefix(str(format("[station %s] ") % stationID.name())),

      startTime(ps.settings.startTime * ps.settings.subbandWidth(), ps.settings.clockHz()),
      stopTime(ps.settings.stopTime * ps.settings.subbandWidth(), ps.settings.clockHz()),

      nrSamples(ps.settings.blockSize),
      nrBlocks(ps.settings.nrBlocks()),

      metaDataPool(str(format("StationMetaData::metaDataPool [station %s]") % stationID.name()), false),

      subbands(values(subbandDistribution))
    {
    }

    /*
     * Initialises blocks from metaDataPool, and adds meta data.
     *
     * Input: metaDataPool.free
     * Output: metaDataPool.filled
     */
    template <typename SampleT>
    void StationMetaData<SampleT>::computeMetaData(Trigger *stopSwitch)
    {
      /*
       * Allocate buffer elements.
       */

      // Each element represents 1 block of buffer.
      for (size_t i = 0; i < 5; ++i)
        metaDataPool.free.append(new MPIData<SampleT>(startTime, ps.settings.subbands.size(), nrSamples), false);

      /*
       * Set up delay compensation.
       */

      Delays delays(ps, stationIdx, startTime, nrSamples);

      // We keep track of the delays at the beginning and end of each block.
      // After each block, we'll swap the afterEnd delays into atBegin.
      Delays::AllDelays delaySet1(ps), delaySet2(ps);
      Delays::AllDelays *delaysAtBegin  = &delaySet1;
      Delays::AllDelays *delaysAfterEnd = &delaySet2;

      // Get delays at begin of first block
      delays.getNextDelays(*delaysAtBegin);

      for (ssize_t block = -1; block < (ssize_t)nrBlocks; ++block) {
        if (stopSwitch && stopSwitch->test()) {
          LOG_WARN_STR(logPrefix << "Requested to stop");
          break;
        }

        LOG_DEBUG_STR(logPrefix << str(format("[block %d] Retrieving delays") % block));

        // Fetch end delays (start delays are set by the previous block, or
        // before the loop).
        delays.getNextDelays(*delaysAfterEnd);

        // INPUT
        SmartPtr< MPIData<SampleT> > mpiData = metaDataPool.free.remove();

        // Annotate
        mpiData->reset(block);

        LOG_DEBUG_STR(logPrefix << str(format("[block %d] Applying delays") % block));

        // Compute the next set of metaData and read_offsets from the new
        // delays pair.
        delays.generateMetaData(*delaysAtBegin, *delaysAfterEnd, subbands, mpiData->metaData, mpiData->read_offsets);

        // OUTPUT
        metaDataPool.filled.append(mpiData);
        ASSERT(!mpiData);

        // Swap delay sets to accomplish delaysAtBegin = delaysAfterEnd
        swap(delaysAtBegin, delaysAfterEnd);
      }

      // Signal EOD
      metaDataPool.filled.append(NULL);
    }


    StationInput::StationInput( const Parset &ps, size_t stationIdx,
                                const SubbandDistribution &subbandDistribution )
    :
      ps(ps),
      stationIdx(stationIdx),
      stationID(StationID::parseFullFieldName(ps.settings.antennaFields.at(stationIdx).name)),

      logPrefix(str(format("[station %s] ") % stationID.name())),

      mode(ps.settings.nrBitsPerSample, ps.settings.clockMHz),
      nrBoards(ps.settings.antennaFields.at(stationIdx).inputStreams.size()),

      targetSubbands(values(subbandDistribution)),
      beamletIndices(generateBeamletIndices())
    {
      for (size_t i = 0; i < nrBoards; ++i) {
        rspDataPool.push_back(new Pool<RSPData>(str(format("StationInput::rspDataPool[%u] [station %s]") % i % stationID.name()), ps.settings.realTime));
      }

      // Log all input descriptions
      LOG_INFO_STR(logPrefix << "Input streams: " << ps.settings.antennaFields.at(stationIdx).inputStreams);

      ASSERTSTR(nrBoards > 0, logPrefix << "No input streams");
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

      for(size_t i = 0; i < targetSubbands.size(); ++i) {
        // The subband stored at position i
        const size_t sb = targetSubbands.at(i);

        // The corresponding (board,slot) combination for that subband,
        // for this station.
        const size_t board = ps.settings.antennaFields[stationIdx].rspBoardMap[sb];
        const size_t slot  = ps.settings.antennaFields[stationIdx].rspSlotMap[sb];

        ASSERT(board < nrBoards);
        ASSERT(slot < mode.nrBeamletsPerBoard());

        // The specified (board,slot) is stored at position i
        ASSERTSTR(result[board][slot] == -1, "Station " << stationID.name() << ": board " << board << " slot " << slot << " is used multiple times!");
        result[board][slot] = i;
      }

      return result;
    }


    SmartPtr<Stream> StationInput::inputStream(size_t board) const
    {
      SmartPtr<Stream> stream;

      // Connect to specified input stream
      const string &desc = ps.settings.antennaFields.at(stationIdx).inputStreams.at(board);

      LOG_DEBUG_STR(logPrefix << "Connecting input stream for board " << board << ": " << desc);

      // Sanity checks
      if (ps.settings.realTime) {
        ASSERTSTR(desc.find("udp:") == 0, logPrefix << "Real-time observations should read input from UDP, not " << desc);
      } else {
        ASSERTSTR(desc.find("udp:") != 0, logPrefix << "Non-real-time observations should NOT read input from UDP, got " << desc);
      }

      if (desc == "factory:") {
        const TimeStamp from(ps.settings.startTime * ps.settings.subbandWidth(), ps.settings.clockHz());
        const TimeStamp to(ps.settings.stopTime * ps.settings.subbandWidth(), ps.settings.clockHz());

        const struct BoardMode mode(ps.settings.nrBitsPerSample, ps.settings.clockMHz);
        PacketFactory factory(mode);

        stream = new PacketStream(factory, from, to, board);
      } else {
        if (ps.settings.realTime) {
          try {
            stream = createStream(desc, true);
          } catch (Exception &ex) {
            LOG_ERROR_STR(logPrefix << "Caught exception creating stream (continuing on /dev/null): " << ex.what());
            stream = new FileStream("/dev/null"); /* will read end-of-stream: avoid spamming illegal packets */
          }
        } else { // non real time: un-tried call, so no rethrow (changes exc backtrace)
          stream = createStream(desc, true);
        }
      }

      return stream;
    }


    void StationInput::readRSPRealTime( size_t board, MACIO::RTmetadata &mdLogger,
                                        const string &mdKeyPrefix )
    {
      /*
       * In real-time mode, we can't get ahead of the `current'
       * block of the reader due to clock synchronisation.
       */

      try {
        SmartPtr<Stream> stream = inputStream(board);
        PacketReader reader(str(format("%s[board %s] ") % logPrefix % board), *stream, mode);

        Queue< SmartPtr<RSPData> > &inputQueue = rspDataPool[board]->free;
        Queue< SmartPtr<RSPData> > &outputQueue = rspDataPool[board]->filled;

        for(size_t i = 1 /* avoid printing statistics immediately */; true; i++) {
          // Fill rspDataPool elements with RSP packets
          SmartPtr<RSPData> rspData = inputQueue.remove();
         
          // Abort condition needed to avoid getting stuck in free.remove()
          if (!rspData)
            break;

          reader.readPackets(rspData->packets);

          // Periodically LOG() and log() (for monitoring (PVSS)) progress
          if (i % 256 == 0) // Each block is ~40ms, so log every ~10s worth of data.
            reader.logStatistics(board, mdLogger, mdKeyPrefix);

          outputQueue.append(rspData);
        }
      } catch (EndOfStreamException &ex) {
        // Ran out of data
        LOG_INFO_STR( logPrefix << "End of stream");

      } catch (SystemCallException &ex) {
        if (ex.error == EINTR || ex.error == 512 /* ERESTARTSYS, should not be propagated to user space. */)
          LOG_INFO_STR( logPrefix << "Stopped");
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
          //NSTimer copyRSPTimer(str(format("%s [board %i] copy RSP -> block") % logPrefix % board), true, true);
          OMPThread::ScopedName sn(str(format("%s wr %u") % ps.settings.antennaFields.at(stationIdx).name % board));

          Queue< SmartPtr<RSPData> > &inputQueue = rspDataPool[board]->filled;
          Queue< SmartPtr<RSPData> > &outputQueue = rspDataPool[board]->free;

          const ssize_t *beamletIndices = &this->beamletIndices[board][0];
          const size_t nrBeamletIndices = mode.nrBeamletsPerBoard();

          SmartPtr<RSPData> rspData;

          while ((rspData = inputQueue.remove(deadline, NULL)) != NULL) {
            // Write valid packets to the current and/or next packet
            //copyRSPTimer.start();

            for (size_t p = 0; p < RT_PACKET_BATCH_SIZE; ++p) {
              struct RSP &packet = rspData->packets[p];

              if (packet.payloadError())
                continue;

              if (current.write(packet, beamletIndices, nrBeamletIndices)
               && next) {
                // We have data (potentially) spilling into `next'.

                if (next->write(packet, beamletIndices, nrBeamletIndices)) {
                  LOG_WARN_STR(logPrefix << "Received data for several blocks into the future -- discarding.");
                }
              }
            }
            //copyRSPTimer.stop();

            outputQueue.append(rspData);
            ASSERT(!rspData);
          }
        }
      }
    }


    void StationInput::readRSPNonRealTime( MACIO::RTmetadata &mdLogger,
                                           const string &mdKeyPrefix )
    {
      vector< SmartPtr<Stream> > streams(nrBoards);
      vector< SmartPtr<PacketReader> > readers(nrBoards);

      for (size_t i = 0; i < nrBoards; ++i) {
        streams[i] = inputStream(i);
        readers[i] = new PacketReader(logPrefix, *streams[i], mode);
      }

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
          } catch (EndOfStreamException &ex) {
            // Ran out of data
            LOG_INFO_STR( logPrefix << "End of stream");

            readers[board]->logStatistics(board, mdLogger, mdKeyPrefix);
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
        SmartPtr<RSPData> data = rspDataPool[0]->free.remove();

        // Abort of writer does not desire any more data
        if (!data) {
          LOG_INFO_STR(logPrefix << "readRSPNonRealTime: received EOS");
          return;
        }

        data->packets[0] = last_packets[youngest];
        data->board = youngest;
       
        rspDataPool[0]->filled.append(data);

        // Next packet should only be read from the stream we
        // emitted from
        for(size_t board = 0; board < nrBoards; board++)
          read_next_packet[board] = (board == (size_t)youngest);
      }

      // Signal EOD by inserting a packet beyond obs end
      SmartPtr<RSPData> data = rspDataPool[0]->free.remove();

      // Abort if writer does not desire any more data
      if (!data) {
        LOG_INFO_STR(logPrefix << "readRSPNonRealTime: received EOS");
        return;
      }

      LOG_INFO_STR(logPrefix << "readRSPNonRealTime: sending EOS");
      rspDataPool[0]->filled.append(NULL);
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

      const size_t nrBeamletIndices = mode.nrBeamletsPerBoard();

      for(;;) {
        SmartPtr<RSPData> data = rspDataPool[0]->filled.remove();

        if (!data) {
          LOG_DEBUG_STR(logPrefix << "writeRSPNonRealTime: received EOS");

          // reinsert EOS for next call to writeRSPNonRealTime
          rspDataPool[0]->filled.prepend(NULL);
          return;
        }

        const ssize_t *beamletIndices = &this->beamletIndices[data->board][0];

        // Only packet 0 is used in non-rt mode

        if (current.write(data->packets[0], beamletIndices, nrBeamletIndices)) {
          // We have data (potentially) spilling into `next'.
          if (!next || next->write(data->packets[0], beamletIndices, nrBeamletIndices)) {
            // Data is even later than next? Put this data back for a future block.
            rspDataPool[0]->filled.prepend(data);
            ASSERT(!data);
            return;
          }
        }

        rspDataPool[0]->free.append(data);
        ASSERT(!data);
      }
    }


    template <typename SampleT>
    void StationInput::processInput( Queue< SmartPtr< MPIData<SampleT> > > &inputQueue,
                                     Queue< SmartPtr< MPIData<SampleT> > > &outputQueue,
                                     MACIO::RTmetadata &mdLogger, const string &mdKeyPrefix )
    {
      OMPThreadSet packetReaderThreads;

      if (ps.settings.realTime) {
        // Each board has its own pool to reduce lock contention
        for (size_t board = 0; board < nrBoards; ++board)
          for (size_t i = 0; i < 16; ++i)
            rspDataPool[board]->free.append(new RSPData(RT_PACKET_BATCH_SIZE), false);
      } else {
        // We just process one packet at a time, merging all the streams into rspDataPool[0].
        for (size_t i = 0; i < 16; ++i)
          rspDataPool[0]->free.append(new RSPData(1), false);
      }

      // Make sure we only read RSP packets when we're ready to actually process them. Otherwise,
      // the rspDataPool[*]->free queues will starve, causing both WARNings and blocking the
      // reading of the RSP packets we do need.
      Trigger startSwitch;

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
          OMPThread::ScopedName sn(str(format("%s rd") % ps.settings.antennaFields.at(stationIdx).name));

          LOG_INFO_STR(logPrefix << "Processing packets");

          // Wait until RSP packets can actually be processed
          startSwitch.wait();

          if (ps.settings.realTime) {
            #pragma omp parallel for num_threads(nrBoards)
            for(size_t board = 0; board < nrBoards; board++) {
              OMPThreadSet::ScopedRun sr(packetReaderThreads);
              OMPThread::ScopedName sn(str(format("%s rd %u") % ps.settings.antennaFields.at(stationIdx).name % board));

              Thread::ScopedPriority sp(SCHED_FIFO, 10);

              readRSPRealTime(board, mdLogger, mdKeyPrefix);
            }
          } else {
            readRSPNonRealTime(mdLogger, mdKeyPrefix);
          }
        }

        /*
         * inputQueue -> outputQueue
         */
        #pragma omp section
        {
          OMPThread::ScopedName sn(str(format("%s wr") % ps.settings.antennaFields.at(stationIdx).name));
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

            // We can now process RSP packets
            startSwitch.trigger();

            if (ps.settings.realTime) {
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

          // Signal EOD to input. It's a free queue, so prepend to avoid
          // having the reader flush the whole queue first.
          for (size_t i = 0; i < nrBoards; ++i)
            rspDataPool[i]->free.prepend(NULL);

          // Make sure we don't get stuck in startup
          startSwitch.trigger();

          if (ps.settings.realTime) {
            // kill reader threads
            LOG_INFO_STR( logPrefix << "Stopping all boards" );
            packetReaderThreads.killAll();
          }
        }
      }
    }


    template<typename SampleT> void sendInputToPipeline(const Parset &ps, 
            size_t stationIdx, const SubbandDistribution &subbandDistribution,
            MACIO::RTmetadata &mdLogger, const string &mdKeyPrefix, Trigger *stopSwitch)
    {
      // sanity check: Find out if we should actual start working here.
      StationMetaData<SampleT> sm(ps, stationIdx, subbandDistribution);

      StationInput si(ps, stationIdx, subbandDistribution);

      const struct StationID stationID(StationID::parseFullFieldName(
        ps.settings.antennaFields.at(stationIdx).name));
      
      const std::string logPrefix = str(format("[station %s] ") % stationID.name());

      LOG_INFO_STR(logPrefix << "Processing station data");

      Queue< SmartPtr< MPIData<SampleT> > > mpiQueue(str(format(
            "sendInputToPipeline::mpiQueue [station %s]") % stationID.name()));

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
          OMPThread::ScopedName sn(str(format("%s meta") % ps.settings.antennaFields.at(stationIdx).name));

          sm.computeMetaData(stopSwitch);
          LOG_INFO_STR(logPrefix << "StationMetaData: done");
        }

        /*
         * Adds samples from the input streams to the blocks
         *
         * sm.metaDataPool.filled -> mpiQueue
         */
        #pragma omp section
        {
          OMPThread::ScopedName sn(str(format("%s proc") % ps.settings.antennaFields.at(stationIdx).name));

          si.processInput<SampleT>( sm.metaDataPool.filled, mpiQueue,
                                    mdLogger, mdKeyPrefix );
          LOG_INFO_STR(logPrefix << "StationInput: done");
        }

        /*
         * MPI POOL: Send block to receivers
         *
         * mpiQueue -> sm.metaDataPool.free
         */
        #pragma omp section
        {
          OMPThread::ScopedName sn(str(format("%s send") % ps.settings.antennaFields.at(stationIdx).name));

          sender.sendBlocks<SampleT>( mpiQueue, sm.metaDataPool.free );
          LOG_INFO_STR(logPrefix << "MPISender: done");
        } 
      }

      LOG_INFO_STR(logPrefix << "Done processing station data");
    }

    void sendInputToPipeline(const Parset &ps, size_t stationIdx, 
                             const SubbandDistribution &subbandDistribution,
                             MACIO::RTmetadata &mdLogger, const string &mdKeyPrefix, Trigger *stopSwitch)
    {
      switch (ps.nrBitsPerSample()) {
        default:
        case 16: 
          sendInputToPipeline< SampleType<i16complex> >(ps, stationIdx,
                                                        subbandDistribution,
                                                        mdLogger, mdKeyPrefix, stopSwitch);
          break;

        case 8: 
          sendInputToPipeline< SampleType< i8complex> >(ps, stationIdx,
                                                        subbandDistribution,
                                                        mdLogger, mdKeyPrefix, stopSwitch);
          break;

        case 4: 
          sendInputToPipeline< SampleType< i4complex> >(ps, stationIdx,
                                                        subbandDistribution,
                                                        mdLogger, mdKeyPrefix, stopSwitch);
          break;
      }
    }
  }
}

