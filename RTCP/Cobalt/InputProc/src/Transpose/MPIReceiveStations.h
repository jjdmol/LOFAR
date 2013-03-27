/* MPIReceiveStations.h
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

#ifndef LOFAR_INPUT_PROC_MPI_RECEIVE_STATIONS_H
#define LOFAR_INPUT_PROC_MPI_RECEIVE_STATIONS_H

#include <mpi.h>

#include <Common/LofarTypes.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/SparseSet.h>

#include "MPIProtocol.h"

#include <vector>

namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * A Receiver class for data sent by MPISendStation. This class receives
     * blocks of beamlets from all specified stations.
     */
    class MPIReceiveStations
    {
    public:
      // Set up a receiver for the given stations and beamlets, receiving
      // blocks of the given size.
      //
      // stationRanks:
      //   The rank of each station. MPI Nodes that host multiple stations
      //   occur multiple times in the list, making len(stationRanks) ==
      //   nrStations.
      //
      // beamlets:
      //   The list of beamlets to receive out of [0, nrBeamlets) w.r.t. the
      //   observation.
      //
      // blockSize:
      //   The number of samples in each block. Includes nrHistorySamples.
      MPIReceiveStations( const std::vector<int> &stationRanks, const std::vector<size_t> &beamlets, size_t blockSize );

      template<typename T>
      struct Beamlet {
        std::vector<T>    samples;
        SparseSet<int64>  flags;
      };

      template<typename T>
      struct Block {
        std::vector< struct Beamlet<T> > beamlets; // [beamlet]
        std::vector<char> metaDataBlob;
      };

      // Receive the next block. The `block' parameter is a structure allocated
      // by the caller, and needs to have dimensions
      // [stationRanks.size()][beamlets.size()]. Each sample block needs to be
      // blockSize in length.
      //
      // It is the callers responsibility to call receiveBlock exactly as often
      // as sendBlock is called by the stations.
      template<typename T>
      void receiveBlock( std::vector< struct Block<T> > &blocks );

    private:
      const std::string logPrefix;
      const std::vector<int> stationRanks;

    public:
      const std::vector<size_t> beamlets;
      const size_t blockSize;

      // Receive a header (async) from the given rank.
      MPI_Request receiveHeader( size_t station, struct MPIProtocol::Header &header );

      // Receive beamlet data (async) from the given rank.
      template<typename T>
      MPI_Request receiveData( size_t station, size_t beamlet, int transfer, T *from, size_t nrSamples );

      // Receive marshalled flags and metadata (async) from the given rank.
      MPI_Request receiveFlags( size_t station, size_t beamlet, std::vector<char> &buffer );
    };

  }
}

#endif

