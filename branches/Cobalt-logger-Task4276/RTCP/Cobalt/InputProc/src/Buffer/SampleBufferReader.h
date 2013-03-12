#ifndef __SAMPLEBUFFERREADER__
#define __SAMPLEBUFFERREADER__

#include <CoInterface/RSPTimeStamp.h>
#include "Buffer/BufferSettings.h"
#include "Buffer/SampleBuffer.h"

#include <vector>
#include <string>

namespace LOFAR
{
  namespace RTCP
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

      virtual ssize_t beamletOffset( unsigned beamlet, const TimeStamp &from, const TimeStamp &to )
      {
        (void)beamlet;
        (void)from;
        (void)to;
        return 0;
      }

      virtual void copyStart( const TimeStamp &from, const TimeStamp &to, const std::vector<size_t> &wrapOffsets )
      {
        (void)from;
        (void)to;
        (void)wrapOffsets;
      }
      virtual void copy( const struct CopyInstructions & )
      {
      }
      virtual void copyEnd( const TimeStamp &from, const TimeStamp &to )
      {
        (void)from;
        (void)to;
      }

      void copy( const TimeStamp &from, const TimeStamp &to );

    private:
      WallClockTime waiter;
    };

  }
}

#include "SampleBufferReader.tcc"

#endif
