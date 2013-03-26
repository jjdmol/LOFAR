/* BeamletBuffer.h: a cyclic buffer that holds the beamlets from the rspboards
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

#ifndef LOFAR_GPUPROC_BEAMLET_BUFFER_H
#define LOFAR_GPUPROC_BEAMLET_BUFFER_H

// \file
// a cyclic buffer that holds the beamlets from the rspboards

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <string>
#include <vector>

#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <Common/Thread/Mutex.h>
#include <Stream/Stream.h>

#include <CoInterface/Config.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Parset.h>
#include <CoInterface/RSPTimeStamp.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/SparseSet.h>

#include "LockedRanges.h"
#include "ReaderWriterSynchronization.h"

namespace LOFAR
{
  namespace Cobalt
  {

    // define a "simple" type of which the size equals the size of two samples
    // (X and Y polarizations)


    template<typename SAMPLE_TYPE>
    class BeamletBuffer
    {
    public:
      BeamletBuffer(const Parset &, const std::string &stationName, unsigned rspBoard);

      void     writePacketData(const SAMPLE_TYPE *data, const TimeStamp &begin);
      void     writeMultiplePackets(const void *rspData, const std::vector<TimeStamp> &);

      void     startReadTransaction(const std::vector<TimeStamp> &begin, unsigned nrElements);
      void     sendSubband(Stream *, unsigned subband, unsigned currentBeam) const;
      void     sendUnalignedSubband(Stream *, unsigned subband, unsigned currentBeam) const;
      unsigned alignmentShift(unsigned beam) const;
      SparseSet<unsigned> readFlags(unsigned beam);
      void     stopReadTransaction();

      void     noMoreReading();
      void     noMoreWriting();

      const static unsigned itsNrTimesPerPacket = 16;

    private:
      unsigned mapTime2Index(TimeStamp time) const;

      std::string itsLogPrefix;

      Mutex itsValidDataMutex;
      SparseSet<TimeStamp>                  itsValidData;
      unsigned itsRSPboard;
      unsigned itsNrSubbands;
      size_t itsPacketSize;
      unsigned itsSize, itsHistorySize;
      bool itsIsRealTime;
      SmartPtr<SynchronizedReaderAndWriter> itsSynchronizedReaderWriter;
      LockedRanges itsLockedRanges;
      Cube<SAMPLE_TYPE>                     itsSBBuffers;
      int itsOffset;
      const static unsigned itsAlignment = 32 / (NR_POLARIZATIONS * sizeof(SAMPLE_TYPE));

      // read internals
      std::vector<TimeStamp>                itsBegin, itsEnd;
      std::vector<size_t>                   itsStartI, itsEndI;
      size_t itsMinStartI, itsMaxEndI;
      TimeStamp itsMinEnd;
      Mutex itsReadMutex;

      // write internals
      void                                  writePacket(SAMPLE_TYPE *dst, const SAMPLE_TYPE *src);
      void                                  updateValidData(const TimeStamp &begin, const TimeStamp &end);
      void                                  writeConsecutivePackets(unsigned count);
      void                                  resetCurrentTimeStamp(const TimeStamp &);

      TimeStamp itsPreviousTimeStamp;
      unsigned itsPreviousI;
      TimeStamp itsCurrentTimeStamp;
      unsigned itsCurrentI;
      size_t itsStride;
      const char                            *itsCurrentPacketPtr;

      NSTimer itsReadTimer, itsWriteTimer;
    };


    template<typename SAMPLE_TYPE>
    inline unsigned BeamletBuffer<SAMPLE_TYPE>::alignmentShift(unsigned beam) const
    {
      return itsStartI[beam] % itsAlignment;
    }

    template<typename SAMPLE_TYPE>
    inline unsigned BeamletBuffer<SAMPLE_TYPE>::mapTime2Index(TimeStamp time) const
    {
      // TODO: this is very slow because of the %
      return (time + itsOffset) % itsSize;
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif

