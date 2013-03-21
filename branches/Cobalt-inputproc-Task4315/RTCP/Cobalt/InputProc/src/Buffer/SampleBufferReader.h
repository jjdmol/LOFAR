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
        // Beamlet index
        unsigned beamlet;

        // Relevant time range
        TimeStamp from;
        TimeStamp to;

        // Copy as one or two ranges of [from, to).
        struct Range {
          const T* from;
          const T* to;
        } ranges[2];

        unsigned nrRanges;

        // The flags for this range
        SparseSet<int64> flags;
      };

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
       * Setup the copying of one block.
       */
      virtual void copyStart( const TimeStamp &from, const TimeStamp &to, const std::vector<size_t> &wrapOffsets )
      {
        (void)from;
        (void)to;
        (void)wrapOffsets;
      }

      /*
       * Copy one block.
       */
      virtual void copy( const struct CopyInstructions & )
      {
      }

      /*
       * Tear down the copying of one block.
       */
      virtual void copyEnd( const TimeStamp &from, const TimeStamp &to )
      {
        (void)from;
        (void)to;
      }

    private:
      WallClockTime waiter;

      void copy( const TimeStamp &from, const TimeStamp &to );
    };

  }
}

#include "SampleBufferReader.tcc"

#endif

