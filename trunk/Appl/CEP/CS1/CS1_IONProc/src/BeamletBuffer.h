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

#ifndef LOFAR_CS1_INPUTSECTION_BEAMLETBUFFER_H
#define LOFAR_CS1_INPUTSECTION_BEAMLETBUFFER_H

// \file
// a cyclic buffer that holds the beamlets from the rspboards

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_vector.h>
#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <Transport/TransportHolder.h>
#include <CS1_IONProc/LockedRange.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_Interface/SparseSet.h>

#include <boost/multi_array.hpp>
#include <pthread.h>

namespace LOFAR {
namespace CS1 {

typedef INPUT_SAMPLE_TYPE SampleType;

class Beamlet {
  SampleType Xpol, Ypol;
};

// A BeamletBuffer can hold the beamlets coming from the rspboards
// It is implemented as a cyclic buffer (using the mapTime2Index method).
// Locking is done using a LockedRange
// This buffer also reshuffles the data. It comes in in packets of different subbands per timestep.
// The data leaves as a time series per subband.
class BeamletBuffer
{
  public:
    BeamletBuffer(int bufferSize, unsigned nSubbands, unsigned history, unsigned readWriteDelay);
    ~BeamletBuffer();

    void writeElements(Beamlet* data, TimeStamp begin, unsigned nElements);
    //void getElements(boost::multi_array_ref<SampleType, 3> &buffers, SparseSet &flags, TimeStamp begin, unsigned nElements);

    void startReadTransaction(TimeStamp begin, unsigned nElements);
    void sendSubband(TransportHolder *, unsigned subband) /*const*/;
    unsigned alignmentShift() const;
    void readFlags(SparseSet<unsigned> &flags);
    void stopReadTransaction();

    //TimeStamp startBufferRead();
    TimeStamp startBufferRead(TimeStamp);

    void setAllowOverwrite(bool o) {itsLockedRange.setOverwriting(o);};

    void clear() {itsLockedRange.clear();};

  private:
    // Copying is not allowed
    BeamletBuffer (const BeamletBuffer& that);
    BeamletBuffer& operator= (const BeamletBuffer& that);

    // Needed for mapping a timestamp to a place in the buffer
    unsigned mapTime2Index(TimeStamp time) const;

    // checked for skipped data and flag it in chunks
    void checkForSkippedData(TimeStamp writeBegin);

    //# Datamembers
    //vector<Beamlet *> itsSBBuffers;
    pthread_mutex_t itsFlagsMutex;
    SparseSet<unsigned> itsFlags;
    unsigned itsNSubbands;
    int itsSize;

    boost::multi_array_ref<SampleType, 3> itsSBBuffers;

    TimeStamp itsHighestWritten;
    
    LockedRange<TimeStamp, int> itsLockedRange;

    // read internals
    TimeStamp itsEnd;
    size_t    itsStartI, itsEndI;
    unsigned  itsNInvalid;

    // These are for statistics
    unsigned itsDroppedItems, itsDummyItems, itsSkippedItems;
    NSTimer  itsReadTimer, itsWriteTimer;
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
