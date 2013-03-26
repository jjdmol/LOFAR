/* MPITransferStations.h
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

#ifndef LOFAR_INPUT_PROC_MPI_TRANSFER_STATIONS_H
#define LOFAR_INPUT_PROC_MPI_TRANSFER_STATIONS_H

#include <vector>
#include <mpi.h>

#include <Common/LofarTypes.h>
#include <CoInterface/RSPTimeStamp.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/SparseSet.h>

#include <Buffer/SampleBufferReader.h>
#include <Buffer/BufferSettings.h>

#include <map>
#include <set>
#include <vector>

namespace LOFAR
{
  namespace Cobalt
  {

    /*
     * Sends a set of beamlets from the SHM buffer to all receiving MPI nodes.
     * Blocks are sent in a sequential fashion: a block must be received
     * completely before the next one is sent. Performance is barely affected,
     * because the output to all nodes has to go through a shared pipe (IB
     * port). We thus cannot really start sending to the first node earlier if
     * we're not yet done sending to other nodes.
     */
    class MPISendStation
    {
    public:
      MPISendStation( const struct BufferSettings &settings, size_t stationIdx, const std::map<size_t, int> &beamletDistribution );

      template<typename T>
      void sendBlock( const struct SampleBufferReader<T>::Block &block );

      // Header which prefixes each block. Contains identification information
      // for verification purposes, as well as the sizes of the data that
      // follow.
      struct Header {
        // Originating station
        StationID station;

        // Block will span [from,to)
        int64 from, to;

        // At which offset the data will be wrapped. If:
        //
        //   =0: the data will be sent in 1 transfer:
        //          1. a block of `to - from' samples
        //   >0: the data will be sent in 2 transfers:
        //          1. a block of `wrapOffsets[x]' samples
        //          2. a block of `(to - from) - wrapOffsets[x]' samples
        size_t wrapOffsets[1024]; // [beamlet]

        // Number of beamlets that will be sent
        size_t nrBeamlets;

        // Size of the marshalled flags
        size_t metaDataSize;
      };

      union tag_t {
        struct {
          unsigned type     :  2;
          unsigned station  :  8;
          unsigned beamlet  : 10;
          unsigned transfer :  1;
        } bits;

        int value;

        tag_t() : value(0) {
        }
      };

      enum tag_types { CONTROL = 0, BEAMLET = 1, FLAGS = 2 };

    protected:
      size_t metaDataSize() const
      {
        return SparseSet<int64>::marshallSize(this->settings.nrAvailableRanges);
      }

    private:
      const std::string logPrefix;
      const BufferSettings &settings;

      // Station number in observation [0..nrStations)
      const size_t stationIdx;

      // To which rank to send each beamlet:
      //   beamletDistribution[beamlet] = rank
      const std::map<size_t, int> beamletDistribution;

      // The ranks to which to send beamlets.
      const std::set<int> targetRanks;

      // The beamlets to send to each rank:
      //   beamletsOfTarget[rank] = [beamlet, beamlet, ...]
      const std::map<int, std::vector<size_t> > beamletsOfTarget;

      // Construct and send a header to the given rank (async).
      template<typename T>
      MPI_Request sendHeader( int rank, Header &header, const struct SampleBufferReader<T>::Block &block );

      // Send beamlet data (in 1 or 2 transfers) to the given rank (async).
      // Returns the number of MPI_Requests made.
      template<typename T>
      unsigned sendData( int rank, unsigned beamlet, const struct SampleBufferReader<T>::Block::Beamlet &ib, MPI_Request requests[2] );

      // Send flags data to the given rank (async).
      MPI_Request sendFlags( int rank, unsigned beamlet, const SparseSet<int64> &flags );
    };


    /*
     * We receive all station data in one loop, because MPI wants to
     * have a single thread listening to all requests.
     *
     * This could be changed into one thread/station to overlap the data
     * transfers between different blocks from different stations. However,
     * such seems to require polling MPI_Testall loops like in MPISendStation.
     */
    class MPIReceiveStations
    {
    public:
      MPIReceiveStations( const std::vector<int> stationRanks, const std::vector<size_t> &beamlets, size_t blockSize );

      template<typename T>
      struct Beamlet {
        std::vector<T>   samples;
        SparseSet<int64> flags;
      };

      template<typename T>
      void receiveBlock( MultiDimArray< struct Beamlet<T>, 2 > &block ); // block[station][beamlet]

    private:
      const std::string logPrefix;
      const std::vector<int> stationRanks;

    public:
      const std::vector<size_t> beamlets;
      const size_t blockSize;

      // Receive a header (async) from the given rank.
      MPI_Request receiveHeader( size_t station, struct MPISendStation::Header &header );

      // Receive beamlet data (async) from the given rank.
      template<typename T>
      MPI_Request receiveBeamlet( size_t station, size_t beamlet, int transfer, T *from, size_t nrSamples );

      // Receive marshalled flags (async) from the given rank.
      MPI_Request receiveFlags( size_t station, size_t beamlet, std::vector<char> &buffer );
    };


  }
}

#include "MPITransferStations.tcc"

#endif

