/* MPIProtocol.h
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

#ifndef LOFAR_INPUT_PROC_MPI_PROTOCOL_H
#define LOFAR_INPUT_PROC_MPI_PROTOCOL_H

#include <Common/LofarTypes.h>
#include <Buffer/BufferSettings.h>

namespace LOFAR
{
  namespace Cobalt
  {
    namespace MPIProtocol {

      // Header which prefixes each block. Contains identification information
      // for verification purposes, as well as the sizes of the data that
      // follow.
      struct Header {
        // Originating station
        StationID station;

        // Block will span [from,to)
        int64 from, to;

        // Block is prefixed by past samples (before `from')
        size_t nrHistorySamples;

        // Number of beamlets that will be sent
        size_t nrBeamlets;

        // The set of beamlets that will be sent
        unsigned beamlets[1024]; // [beamlet]

        // At which offset the data will be wrapped. If:
        //
        //   =0: the data will be sent in 1 transfer:
        //          1. a block of `to - from' samples
        //   >0: the data will be sent in 2 transfers:
        //          1. a block of `wrapOffsets[x]' samples
        //          2. a block of `(to - from) - wrapOffsets[x]' samples
        size_t wrapOffsets[1024]; // [beamlet]

        // Size of the marshalled flags
        size_t flagsSize;

        // The metaData blob
        size_t metaDataBlobSize;
        char metaDataBlob[4096];

        std::vector<char> getMetaDataBlob() const {
          ASSERT( metaDataBlobSize <= sizeof metaDataBlob );

          return std::vector<char>(&metaDataBlob[0], &metaDataBlob[metaDataBlobSize]);
        }
      };

      enum tag_types { CONTROL = 0, BEAMLET = 1, FLAGS = 2 };

      // MPI tag which identifies each block.
      union tag_t {
        struct {
          // One of tag_types
          unsigned type     :  2;

          // Station index
          unsigned station  :  8;

          // Beamlet index
          unsigned beamlet  : 10;

          // Transfer number for data
          unsigned transfer :  1;
        } bits;

        int value;

        tag_t() : value(0) {
        }
      };

    }
  }
}

#endif

