//#  BeamletBuffer.h: a cyclic buffer that holds the beamlets from the rspboards
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_IONPROC_BEAMLET_BUFFER_H
#define LOFAR_IONPROC_BEAMLET_BUFFER_H

// \file
// a cyclic buffer that holds the beamlets from the rspboards

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_vector.h>
#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/SparseSet.h>
#include <LockedRanges.h>
#include <ReaderWriterSynchronization.h>
#include <Stream/Stream.h>

#include <pthread.h>


namespace LOFAR {
namespace RTCP {

// define a "simple" type of which the size equals the size of two samples
// (X and Y polarizations)


template<typename SAMPLE_TYPE> class BeamletBuffer
{
  public:
	     BeamletBuffer(unsigned bufferSize, unsigned nrTimesPerPacket, unsigned nrSubbands, unsigned nrBeams, unsigned history, bool isSynchronous, unsigned maxNetworkDelay);
	     ~BeamletBuffer();

    void     writePacketData(const SAMPLE_TYPE *data, const TimeStamp &begin);
    void     writeMultiplePackets(const void *rspData, const std::vector<TimeStamp> &);

    void     startReadTransaction(const std::vector<TimeStamp> &begin, unsigned nrElements);
    void     sendSubband(Stream *, unsigned subband, unsigned currentBeam) const;
    void     sendUnalignedSubband(Stream *, unsigned subband, unsigned currentBeam) const;
    unsigned alignmentShift(unsigned beam) const;
    SparseSet<unsigned> readFlags(unsigned beam);
    void     stopReadTransaction();
    
    const static unsigned		  itsNrTimesPerPacket = 16;

  private:
    unsigned mapTime2Index(TimeStamp time) const;

    pthread_mutex_t			  itsValidDataMutex;
    SparseSet<TimeStamp>		  itsValidData;
    unsigned				  itsNSubbands;
    size_t				  itsPacketSize;
    unsigned				  itsSize, itsHistorySize;
    ReaderAndWriterSynchronization	  *itsSynchronizedReaderWriter;
    LockedRanges			  itsLockedRanges;
    Cube<SAMPLE_TYPE>			  itsSBBuffers;
    int					  itsOffset;
    const static unsigned		  itsAlignment = 32 / (NR_POLARIZATIONS * sizeof(SAMPLE_TYPE));

    // read internals
    std::vector<TimeStamp>		  itsBegin, itsEnd;
    std::vector<size_t>			  itsStartI, itsEndI;
    size_t                                itsMinStartI, itsMaxEndI;
    TimeStamp                             itsMinEnd;

    // write internals
    void				  writePacket(SAMPLE_TYPE *dst, const SAMPLE_TYPE *src);
    void				  updateValidData(const TimeStamp &begin, const TimeStamp &end);
    void				  writeConsecutivePackets(unsigned count);
    void				  resetCurrentTimeStamp(const TimeStamp &);

    TimeStamp				  itsPreviousTimeStamp;
    unsigned				  itsPreviousI;
    TimeStamp				  itsCurrentTimeStamp;
    unsigned				  itsCurrentI;
    size_t				  itsStride;
    const char				  *itsCurrentPacketPtr;

    NSTimer				  itsReadTimer, itsWriteTimer;
};


template<typename SAMPLE_TYPE> inline unsigned BeamletBuffer<SAMPLE_TYPE>::alignmentShift(unsigned beam) const
{
  return itsStartI[beam] % itsAlignment;
}

template<typename SAMPLE_TYPE> inline unsigned BeamletBuffer<SAMPLE_TYPE>::mapTime2Index(TimeStamp time) const
{ 
  // TODO: this is very slow because of the %
  return (time + itsOffset) % itsSize;
}

} // namespace RTCP
} // namespace LOFAR

#endif
