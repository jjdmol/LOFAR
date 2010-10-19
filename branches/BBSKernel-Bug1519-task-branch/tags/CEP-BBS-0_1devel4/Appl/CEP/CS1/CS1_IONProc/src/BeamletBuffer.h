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

#ifndef LOFAR_CS1_ION_PROC_BEAMLET_BUFFER_H
#define LOFAR_CS1_ION_PROC_BEAMLET_BUFFER_H

// \file
// a cyclic buffer that holds the beamlets from the rspboards

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_vector.h>
#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_Interface/SparseSet.h>
#include <LockedRanges.h>
#include <ReaderWriterSynchronization.h>
#include <Transport/TransportHolder.h>

#include <boost/multi_array.hpp>
#include <pthread.h>


namespace LOFAR {
namespace CS1 {

typedef INPUT_SAMPLE_TYPE SampleType;

struct Beamlet {
  SampleType Xpol, Ypol;
};

class BeamletBuffer
{
  public:
	     BeamletBuffer(unsigned bufferSize, unsigned nrSubbands, unsigned history, bool isSynchronous, unsigned maxNetworkDelay);
	     ~BeamletBuffer();

    void     writeElements(Beamlet *data, const TimeStamp &begin, unsigned nrElements);

    void     startReadTransaction(const TimeStamp &begin, unsigned nrElements);
    void     sendSubband(TransportHolder *, unsigned subband) /*const*/;
    unsigned alignmentShift() const;
    void     readFlags(SparseSet<unsigned> &flags);
    void     stopReadTransaction();

  private:
    unsigned mapTime2Index(TimeStamp time) const;

    pthread_mutex_t			  itsValidDataMutex;
    SparseSet<TimeStamp>		  itsValidData;
    unsigned				  itsNSubbands;
    unsigned				  itsSize, itsHistorySize;
    ReaderAndWriterSynchronization	  *itsSynchronizedReaderWriter;
    LockedRanges			  itsLockedRanges;
    boost::multi_array_ref<SampleType, 3> itsSBBuffers;

    // read internals
    TimeStamp				  itsBegin, itsEnd;
    size_t				  itsStartI, itsEndI;

    NSTimer				  itsReadTimer, itsWriteTimer;
};


inline unsigned BeamletBuffer::alignmentShift() const
{
  return itsStartI % (32 / sizeof(Beamlet));
}

inline unsigned BeamletBuffer::mapTime2Index(TimeStamp time) const
{ 
  // TODO: this is very slow because of the %
  return time % itsSize;
}

} // namespace CS1
} // namespace LOFAR

#endif
