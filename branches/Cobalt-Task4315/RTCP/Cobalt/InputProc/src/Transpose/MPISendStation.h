/* MPISendStation.h
 * Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#ifndef LOFAR_INPUT_PROC_MPI_SEND_STATIONS_H
#define LOFAR_INPUT_PROC_MPI_SEND_STATIONS_H

#include <mpi.h>

#include <Common/LofarTypes.h>
#include <CoInterface/RSPTimeStamp.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/SparseSet.h>

#include <InputProc/Buffer/Block.h>
#include <InputProc/Buffer/BufferSettings.h>
#include "MPIProtocol.h"

#include <map>
#include <set>
#include <vector>

namespace LOFAR
{
  namespace Cobalt
  {

    /*
     * Sends a Block of beamlets to all receiving MPI nodes.
     * Blocks are sent in a sequential fashion: a block must be received
     * completely before the next one is sent. Performance is barely affected,
     * because the output to all nodes has to go through a shared pipe (IB
     * port). We thus cannot really start sending to the first node earlier if
     * we're not yet done sending to other nodes.
     */
    class MPISendStation
    {
    public:
      // Create a sender of blocks over MPI.
      //
      // settings
      //   The station to send info from.
      //
      // stationIdx
      //   The station index within this observation.
      // beamletDistribution
      //   The distribution of beamlets:
      //     key   = receiver MPI rank
      //     value = beamlets to send in [0, ps.nrSubbands())
      MPISendStation( const struct BufferSettings &settings, size_t stationIdx, const std::map<int, std::vector<size_t> > &beamletDistribution );

      // Send one block. The caller is responsible for matching the number of
      // posted receiveBlocks.
      template<typename T>
      void sendBlock( const struct Block<T> &block, const std::vector<char> &metaDataBlob );

    private:
      const std::string logPrefix;
      const BufferSettings &settings;

      // Station number in observation [0..nrStations)
      const size_t stationIdx;

      // Which beamlets to send to which rank:
      //   beamletDistribution[rank] = beamlets
      const std::map<int, std::vector<size_t> > beamletDistribution;

      // Ranks to send data to
      const std::vector<int> targetRanks;

      // The rank to which to send each beamlet.
      const std::map<size_t, int> beamletTargets;

      // Cache for the headers to send
      std::map<int, MPIProtocol::Header> headers;

    public:
      // Construct and send a header to the given rank (async).
      template<typename T>
      MPI_Request sendHeader( int rank, MPIProtocol::Header &header, const struct Block<T> &block, const std::vector<char> &metaDataBlob );

      // Send beamlet data (in 1 or 2 transfers) to the given rank (async).
      // Returns the number of MPI_Requests made.
      template<typename T>
      unsigned sendData( int rank, unsigned beamlet, const struct Block<T>::Beamlet &ib, MPI_Request requests[2] );

      // Send flags data to the given rank (async).
      MPI_Request sendFlags( int rank, unsigned beamlet, const SparseSet<uint64> &flags );

      size_t flagsSize() const
      {
        return SparseSet<uint64>::marshallSize(this->settings.nrAvailableRanges);
      }
    };

  }
}

#endif

