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
     * Sends a set of beamlets from the SHM buffer to one MPI node.
     */
    template<typename T>
    class MPISendStation : public SampleBufferReader<T>
    {
    public:
      MPISendStation( const struct BufferSettings &settings, const TimeStamp &from, const TimeStamp &to, size_t blockSize, size_t nrHistorySamples, const std::map<size_t, int> &beamletDistribution );

      struct Header {
        StationID station;

        int64 from, to;
        size_t wrapOffsets[1024];

        size_t nrBeamlets;
        size_t metaDataSize;
      };

      union tag_t {
        struct {
          unsigned type : 2;
          unsigned beamlet : 10;
          unsigned transfer : 1;
        } bits;

        int value;

        tag_t() : value(0) {
        }
      };

      enum tag_types { CONTROL = 0, BEAMLET = 1, FLAGS = 2 };

    protected:
      virtual void sendBlock( const struct SampleBufferReader<T>::CopyInstructions &info );

      size_t metaDataSize() const
      {
        return sizeof(uint32_t) + this->settings.nrFlagRanges * sizeof(int64) * 2;
      }

    private:
      const std::string logPrefix;
      const std::map<size_t, int> beamletDistribution;
      const std::set<int> targetRanks;
      const std::map<int, std::vector<size_t> > beamletsOfTarget;

      MPI_Request sendHeader( int rank, Header &header, const struct SampleBufferReader<T>::CopyInstructions &info );
      void sendData( int rank, unsigned beamlet, const struct SampleBufferReader<T>::CopyInstructions::Beamlet &ib );
      void sendFlags( int rank, unsigned beamlet, const SparseSet<int64> &flags );
    };


    /*
     * We receive all station data in one loop, because MPI wants to
     * have a single thread listening to all requests.
     *
     * This could be changed into one thread/station to overlap the data
     * transfers between different blocks from different stations. However,
     * such seems to require polling MPI_Testall loops like in MPISendStation.
     */
    template<typename T>
    class MPIReceiveStations
    {
    public:
      MPIReceiveStations( const std::vector<int> stationRanks, const std::vector<size_t> &beamlets, size_t blockSize );

      struct Block {
        MultiDimArray<T, 2> samples;            // [beamlet][sample]
        MultiDimArray<SparseSet<int64>, 1> flags; // [beamlet]
      };

      std::vector<struct Block> lastBlock; // [station]

      // Fill lastBlock with the next block
      void receiveBlock();

    private:
      const std::string logPrefix;
      const std::vector<int> stationRanks;

    public:
      const std::vector<size_t> beamlets;
      const size_t blockSize;

      MPI_Request receiveHeader( int rank, struct MPISendStation<T>::Header &header );
      MPI_Request receiveBeamlet( int rank, size_t beamlet, int transfer, T *from, size_t nrSamples );
      MPI_Request receiveFlags( int rank, size_t beamlet, std::vector<char> &buffer );

      int waitAny( std::vector<MPI_Request> &requests );
      void waitAll( std::vector<MPI_Request> &requests );
    };


  }
}

#include "MPITransferStations.tcc"

#endif

