//# StationInput.h: Routines to manage I/O from the stations.
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


#ifndef LOFAR_GPUPROC_STATIONINPUT_H
#define LOFAR_GPUPROC_STATIONINPUT_H

#include <map>
#include <vector>
#include <cstring>

#include <Stream/Stream.h>
#include <Common/Thread/Semaphore.h>
#include <MACIO/RTmetadata.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Pool.h>
#include <CoInterface/SubbandMetaData.h>
#include <CoInterface/Queue.h>
#include <CoInterface/BestEffortQueue.h>
#include <InputProc/Buffer/StationID.h>
#include <InputProc/Buffer/BoardMode.h>
#include <InputProc/RSPTimeStamp.h>
#include <InputProc/Station/RSP.h>

#include "StationTranspose.h"

namespace LOFAR {
  namespace Cobalt {
    /*
     * Generates MPIData<Sample> blocks, and computes its meta data (delays, etc).
     */
    template <typename SampleT>
    class StationMetaData {
    public:
      StationMetaData( const Parset &ps, size_t stationIdx, const SubbandDistribution &subbandDistribution );

      void computeMetaData();

    private:
      const Parset &ps;
      const size_t stationIdx;
      const struct StationID stationID;
      const std::string logPrefix;

      const TimeStamp startTime;
      const TimeStamp stopTime;

      const size_t nrSamples;
    public:
      const size_t nrBlocks;

      Pool< MPIData<SampleT> > metaDataPool;
    private:

      const std::vector<size_t> subbands;
    };


    class StationInput {
    public:
      StationInput( const Parset &ps, size_t stationIdx, 
      const SubbandDistribution &subbandDistribution );

      template <typename SampleT>
      void processInput( Queue< SmartPtr< MPIData<SampleT> > > &inputQueue, 
                         Queue< SmartPtr< MPIData<SampleT> > > &outputQueue,
                         MACIO::RTmetadata &mdLogger, const string &mdKeyPrefix );

    private:
      // Each packet is expected to have 16 samples per subband, i.e. ~80 us worth of data @ 200 MHz.
      // So 512 packets is ~40 ms of data.
      static const size_t RT_PACKET_BATCH_SIZE = 512;

      // Data received from an RSP board
      struct RSPData {
        std::vector<struct RSP> packets;
        size_t board; // annotation used in non-rt mode

        RSPData(size_t numPackets):
          packets(numPackets)
        {
        }
      };

      const Parset &ps;

      const size_t stationIdx;
      const struct StationID stationID;

      const std::string logPrefix;

      const BoardMode mode;
      const size_t nrBoards;
      std::vector< SmartPtr< Pool< RSPData > > > rspDataPool; // [nrboards]

      const std::vector<size_t> targetSubbands;

      // Mapping of
      // [board][slot] -> offset of this beamlet in the MPIData struct
      //                  or: -1 = discard this beamlet (not used in obs)
      const MultiDimArray<ssize_t, 2> beamletIndices;

      MultiDimArray<ssize_t, 2> generateBeamletIndices();

      SmartPtr<Stream> inputStream(size_t board) const;

      /*
       * Reads data from all the station input streams, and puts their packets in rspDataPool.
       *
       * Real-time mode:
       *   - Packets are collected in batches per board
       *   - Batches are interleaved as they arrive
       *
       * Non-real-time mode:
       *   - Packets are interleaved between the streams,
       *     staying as close to in-order as possible.
       *   - An "+inf" TimeStamp is added at the end to signal
       *     end-of-data
       *
       * Reads:  rspDataPool.free
       * Writes: rspDataPool.filled
       *
       * Read data from one board in real-time mode.
       */
      void readRSPRealTime( size_t board, MACIO::RTmetadata &mdLogger,
                            const std::string &mdKeyPrefix );

      /*
       * Read data from all boards in non-real-time mode.
       */
      void readRSPNonRealTime( MACIO::RTmetadata &mdLogger,
                               const std::string &mdKeyPrefix );

      /*
       * Fills 'current' with RSP data. Potentially spills into 'next'.
       */
      template <typename SampleT>
      void writeRSPRealTime( MPIData<SampleT> &current, MPIData<SampleT> *next );
      template <typename SampleT>
      void writeRSPNonRealTime( MPIData<SampleT> &current, MPIData<SampleT> *next );
    };


    void sendInputToPipeline(const Parset &ps, size_t stationIdx,
                             const SubbandDistribution &subbandDistribution,
                             MACIO::RTmetadata &mdLogger,
                             const std::string &mdKeyPrefix);
  }
}

#endif

