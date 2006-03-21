//#  BeamletBuffer.h: one line description
//#
//#  Copyright (C) 2005
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

#ifndef LOFAR_TFC_INPUTSECTION_BEAMLETBUFFER_H
#define LOFAR_TFC_INPUTSECTION_BEAMLETBUFFER_H

// \file
// one line description.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_complex.h>
#include <Common/lofar_vector.h>
#include <Common/Timer.h>
#include <TFC_Interface/RSPTimeStamp.h>
#include <TFC_InputSection/LockedRange.h>

namespace LOFAR 
{

    // \addtogroup TFC_InputSection
    // @{

    //# Forward Declarations
    //class forward;
    typedef struct 
    {
      i16complex Xpol;
      i16complex Ypol;
    } SubbandType;

    // Description of class.
    class BeamletBuffer
    {
    public:
      BeamletBuffer(int bufferSize, int nSubbands, int history, int readWriteDelay);
      ~BeamletBuffer();

      // write elements in the buffer, return value is number of succesfully written elements
      int writeElements(SubbandType* data, TimeStamp begin, int nElements, int stride);
      // get elements out of the buffer, return value is number of valid elements
      int getElements(vector<SubbandType*> buffers, int& invalidCount, TimeStamp begin, int nElements);

      TimeStamp startBufferRead();
      TimeStamp startBufferRead(TimeStamp);

      void setAllowOverwrite(bool o) {itsLockedRange.setOverwriting(o);};

      void clear() {itsLockedRange.clear();};

    private:
      // Copying is not allowed
      BeamletBuffer (const BeamletBuffer& that);
      BeamletBuffer& operator= (const BeamletBuffer& that);

      int mapTime2Index(TimeStamp time) { return ((((long long)time.getSeqId()) * ((long long)time.getMaxBlockId())) + time.getBlockId()) % itsSize; };

      vector<SubbandType *> itsSBBuffers;
      bool* itsInvalidFlags;
      int itsNSubbands;
      int itsSize;
      
      LockedRange<TimeStamp, int> itsLockedRange;

      int itsDroppedItems;
      int itsDummyItems;

      NSTimer itsWriteTimer;
      NSTimer itsReadTimer;
    };

    // @}

} // namespace LOFAR

#endif
