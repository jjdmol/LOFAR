/* SampleBufferReader.h
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_INPUT_PROC_SAMPLE_BUFFER_READER_H
#define LOFAR_INPUT_PROC_SAMPLE_BUFFER_READER_H

#include <string>
#include <vector>

#include <CoInterface/RSPTimeStamp.h>

#include <WallClockTime.h>

#include "BufferSettings.h"
#include "SampleBuffer.h"


namespace LOFAR
{
  namespace Cobalt
  {

    /*
     * An abstract class for the implementation of a reader for SampleBuffers.
     */
    template<typename T>
    class SampleBufferReader
    {
    public:
      SampleBufferReader( const BufferSettings &settings, const std::vector<size_t> beamlets, const TimeStamp &from, const TimeStamp &to, size_t blockSize, size_t nrHistorySamples = 0);

      void process( double maxDelay );

    protected:
      const BufferSettings settings;
      SampleBuffer<T> buffer;

      const std::vector<size_t> beamlets;
      const TimeStamp from, to;
      const size_t blockSize;

      // Number of samples to include before `from', to initialise the FIR taps,
      // included in blockSize.
      const size_t nrHistorySamples;

      struct CopyInstructions {
        // Relevant time range
        TimeStamp from;
        TimeStamp to;

        struct Beamlet {
          // Copy as one or two ranges of [from, to).
          struct Range {
            const T* from;
            const T* to;
          } ranges[2];

          unsigned nrRanges;

          // The offset at which the data is accessed.
          ssize_t offset;

          SparseSet<int64> flagsAtBegin;
        };

        std::vector<struct Beamlet> beamlets; // [beamlet]
      };

      /*
       * Read the flags for a specific beamlet. Readers should read the flags
       * before and after reading the data. The valid data is then indicated by
       * the intersection of both sets (flagsBefore & flagsAfter).
      */
      SparseSet<int64> flags( const struct CopyInstructions &, unsigned beamlet );

      /*
       * Provide the offset in samples for a certain beamlet, based on the
       * geometric delays for the respective subband.
       */
      virtual ssize_t beamletOffset( unsigned beamlet, const TimeStamp &from, const TimeStamp &to )
      {
        (void)beamlet;
        (void)from;
        (void)to;
        return 0;
      }

      /*
       * Copy one block.
       */
      virtual void sendBlock( const struct CopyInstructions & )
      {
      }

    private:
      WallClockTime waiter;

      struct CopyInstructions getCopyInstructions( const TimeStamp &from, const TimeStamp &to );

      void sendBlock( const TimeStamp &from, const TimeStamp &to );
    };

  }
}

#include "SampleBufferReader.tcc"

#endif

