//#  BufferController.h: template class to control a cyclic buffer.
//#
//#  Copyright (C) 2000, 2001
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


#ifndef TFLOPCORRELATOR_BUFFERCONTROLLER_H
#define TFLOPCORRELATOR_BUFFERCONTROLLER_H

#include <TFC_InputSection/CyclicBuffer.h>
#include <TFC_Interface/RSPTimeStamp.h>
#include <APS/ParameterSet.h>

/* Main purpose of the BufferController class is to control the cyclic buffers of
   a subband */

namespace LOFAR
{


using ACC::APS::ParameterSet;

#define MAX_OFFSET 5

typedef struct 
{
  u16complex Xpol;
  u16complex Ypol;
} SubbandType;

typedef struct
{
  int invalid;
  timestamp_t timestamp;
} MetadataType;


class BufferController
{
 public:
 
   BufferController();
   BufferController(ParameterSet &ps);
   ~BufferController();

   timestamp_t getFirstStamp();
   bool getElements(void* buf, int& invalidcount, timestamp_t startstamp, int nelements);
   bool writeElements(void* buf, timestamp_t rspstamp, int nelements, int invalid);
   bool rewriteElements(void* buf, timestamp_t startstamp, int nelements);
   
   // disable overwriting and remember the newest element
   // return the newest element (to be used on the master)
   timestamp_t startBufferRead();
   // disable overwrite at the given stamp (to be used on the client)
   void startBufferRead(timestamp_t stamp);
   
  private:
   
   // Cyclic buffer
   CyclicBuffer<MetadataType> *itsMetadataBuf;

   SubbandType* itsSubbandData;  

   // ACC parameters interface
   ParameterSet itsPS;
    
    int itsCyclicBufferSize;
};



}
#endif

