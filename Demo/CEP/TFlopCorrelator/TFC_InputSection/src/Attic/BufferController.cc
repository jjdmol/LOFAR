//#  BufferController.cc: Control cyclic buffers for subbands and metadata
//#
//#  Copyright (C) 2002-2005
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


#include <math.h>
#include <stdlib.h>
#include <TFC_InputSection/BufferController.h>
#include <Common/hexdump.h>

namespace LOFAR {

BufferController::BufferController()
{
  // get cyclicbuffersize
  itsCyclicBufferSize = 1000;
  
  // create cyclic buffer
  itsMetadataBuf = new CyclicBuffer<MetadataType>(itsCyclicBufferSize);  
 
  // create subband data structure
  itsSubbandData = new SubbandType[itsCyclicBufferSize];
}

BufferController::BufferController(ParameterSet &ps)
  : itsPS (ps)
{
  // get cyclicbuffersize
  itsCyclicBufferSize = ps.getInt32("Input.CyclicBufferSize");
  
  // create cyclic buffer
  itsMetadataBuf = new CyclicBuffer<MetadataType>(itsCyclicBufferSize);
  
  // create subband data structure
  itsSubbandData = new SubbandType[itsCyclicBufferSize];
  
}

BufferController::~BufferController()
{
  delete itsSubbandData;
  delete itsMetadataBuf;
}
 
timestamp_t BufferController::getFirstStamp()
{
  MetadataType* mt;
  timestamp_t stamp;
  int mid;
  
  // get element
  mt = itsMetadataBuf->getFirstReadPtr(mid);
  stamp = mt->timestamp;
  
  // unlock element
  itsMetadataBuf->ReadUnlockElements(mid, 1);

  return stamp;
}

bool BufferController::getElements(void* buf, int& invalidcount, timestamp_t startstamp, int nelements)
{
  MetadataType* mt;
  int mid, startid;

  // get timestamp of first element
  mt = itsMetadataBuf->getFirstReadPtr(mid);
  
  // calculate offset
  int offset = startstamp - mt->timestamp;

  // unlock element
  itsMetadataBuf->ReadUnlockElement(mid); 
  
  DBGASSERTSTR(std::abs(offset) <= MAX_OFFSET , 
               "BufferController: timestamp offset invalid");

  // determine start position in buffer
  itsMetadataBuf->setOffset(offset, startid);
  
  // get metadata of requested block
  invalidcount = 0;
  for (int i=0; i<nelements; i++) {
    mt = itsMetadataBuf->getAutoReadPtr(mid);
    if (mt->invalid != 0) {
      invalidcount++;
    }
  }
 
  // get subbanddata of requested block
  if (startid + nelements  > itsCyclicBufferSize) {
    // do copy in 2 blocks because end of subband data array will be crossed 
    int n1 = itsCyclicBufferSize - startid;
    memcpy(buf, &itsSubbandData[startid], n1*sizeof(SubbandType));

    int n2 = nelements - n1;
    (char*)buf += n1*sizeof(SubbandType);
    memcpy(buf, &itsSubbandData[0], n2*sizeof(SubbandType));
  }
  else {
    // copy can be executed in one block
    memcpy(buf, &itsSubbandData[startid], nelements*sizeof(SubbandType));
  }

  //reading is done so unlock the elements in the cyclic buffer
  itsMetadataBuf->ReadUnlockElements(startid, nelements);
  
  return true; 
}

bool BufferController::writeElements(void* buf, timestamp_t rspstamp, int nelements, int invalid)
{
  MetadataType* mt;
  int mid, startid;

  // write the metadata for this block
  for (int i=0; i<nelements; i++) {
    mt = itsMetadataBuf->getAutoWritePtr(mid);
    mt->invalid = invalid;
    mt->timestamp = rspstamp;
    // hold startid
    if (i==0) {
      startid = mid;
    }
    // increment rspstamp 
    rspstamp++;
  }
  
  // write the subbanddata
  if (startid + nelements  > itsCyclicBufferSize) {
    // do copy in 2 blocks because end of subband data array will be crossed 
    int n1 = itsCyclicBufferSize - startid;
    memcpy(&itsSubbandData[startid], buf, n1*sizeof(SubbandType));

    int n2 = nelements - n1;
    (char*)buf += n1*sizeof(SubbandType);
    memcpy(&itsSubbandData[0], buf, n2*sizeof(SubbandType));
  }
  else {
    // copy can be executed in one block
    memcpy(&itsSubbandData[startid], buf, nelements*sizeof(SubbandType));
  }
 
  // writing is done so unlock the elements in the cyclic buffer
  itsMetadataBuf->WriteUnlockElements(startid, nelements);

  return true;
}

bool BufferController::rewriteElements(void* buf, timestamp_t startstamp, int nelements)
{
  MetadataType* mt;
  int mid, firstid, startid, offset;

  // get timestamp of first element in cyclic buffer to find offset
  mt = itsMetadataBuf->getFirstReadPtr(firstid);
  offset = startstamp - mt->timestamp;
  itsMetadataBuf->ReadUnlockElement(firstid);

  
  // determine position in cyclic buffer where to start writing
  if (offset < 0 || offset >= itsCyclicBufferSize) {
    return false;
  } 
  startid = firstid + offset;
  if (startid >= itsCyclicBufferSize) {
    startid -= itsCyclicBufferSize;
  }

  // rewrite the metadata for this block
  mid = startid;
  for (int i=0; i<nelements; i++) {
    mt = itsMetadataBuf->getManualWritePtr(mid);
    mt->invalid = 0;
    mid++;
    if (mid >= itsCyclicBufferSize) {
      mid = 0;
    }
  }
  
  // rewrite the subbanddata for this block
  if (startid + nelements  > itsCyclicBufferSize) {
    // do copy in 2 blocks because end of subband data array will be crossed 
    int n1 = itsCyclicBufferSize - startid;
    memcpy(&itsSubbandData[startid], buf, n1*sizeof(SubbandType));

    int n2 = nelements - n1;
    (char*)buf += n1*sizeof(SubbandType);
    memcpy(&itsSubbandData[0], buf, n2*sizeof(SubbandType));
  }
  else {
    // copy can be executed in one block
    memcpy(&itsSubbandData[startid], buf, nelements*sizeof(SubbandType));
  }
  
  // writing is done so unlock the elements in the cyclic buffer
  itsMetadataBuf->WriteUnlockElements(startid, nelements);


  return true;
}

bool BufferController::overwritingAllowed(bool allowed)
{
  itsMetadataBuf->setOverwritingAllowed(allowed);
}




}
