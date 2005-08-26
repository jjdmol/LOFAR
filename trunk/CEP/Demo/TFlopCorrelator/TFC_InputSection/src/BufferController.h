//#  BufferController.h: class to control a cyclic buffer containing RSP data
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

#include <TFC_Interface/RSPTimeStamp.h>
#include <APS/ParameterSet.h>


namespace LOFAR
{


using ACC::APS::ParameterSet;

#define MAX_OFFSET  5
#define MIN_COUNT  10 
#define MAX_COUNT  (itsBufferSize-10)
    
typedef struct 
{
  i16complex Xpol;
  i16complex Ypol;
} SubbandType;

typedef struct
{
  int invalid;
  timestamp_t timestamp;
} MetadataType;

class BufferIndex 
{
 public:

  BufferIndex(const int maxidx = 0);
  
  void operator+= (int increment);
  void operator++ (int); 
  int operator+ (int increment);
  void operator-= (int decrement);
  void operator-- (int); 
  int operator- (BufferIndex& other);
  bool operator== (BufferIndex& other);
  
  int getIndex();

 private:
  int itsIndex;
  int itsMaxIndex;
  void checkIndex();
};

class BufferController
{
 public:
 
   BufferController(int buffersize, int nsubbands);
   ~BufferController();
   void getElements(vector<SubbandType*> buf, int& invalidcount, timestamp_t startstamp, int nelements);
   void writeElements(SubbandType* buf, timestamp_t rspstamp);
   void writeDummy(SubbandType* dum, timestamp_t startstamp, int nelements);
   bool rewriteElements(SubbandType* buf, timestamp_t startstamp);
   
   // disable overwriting and remember the newest element
   // return the newest element (to be used on the master)
   timestamp_t startBufferRead();
   // disable overwrite at the given stamp (to be used on the client)
   void startBufferRead(timestamp_t stamp);
   
  private:

   // the buffers
   vector<SubbandType*> itsSubbandBuffer;
   MetadataType* itsMetadataBuffer;

   // index pointers
   BufferIndex itsHead;
   BufferIndex itsTail;
   BufferIndex itsOldHead;
   BufferIndex itsOldTail;

   int itsBufferSize;
   int itsNSubbands; 

   // permission to overwrite previous written elements
   bool itsOverwritingAllowed;
   
   pthread_mutex_t buffer_mutex;    // lock/unlock shared data
   pthread_cond_t  data_available;  // 'buffer not empty' trigger
   pthread_cond_t  space_available; // 'buffer not full' trigger

   int getCount();
   int getWritePtr();
   int getReadPtr();
   int setReadOffset(int offset);
   int setRewriteOffset(int offset);   
   void releaseWriteBlock();
   void releaseReadBlock();
   void releaseRewriteBlock();
   timestamp_t getOldestStamp();
   timestamp_t getNewestStamp();

};



}
#endif

