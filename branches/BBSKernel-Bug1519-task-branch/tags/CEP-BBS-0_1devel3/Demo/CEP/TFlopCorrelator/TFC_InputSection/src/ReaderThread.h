//#  ReaderThread.h: The thread that read from a TH and places data into the buffer of the input section
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

#ifndef TFLOPCORRELATOR_READERTHREAD_H
#define TFLOPCORRELATOR_READERTHREAD_H

#include <pthread.h>
#include <Common/Timer.h>
#include <Transport/TransportHolder.h>
#include <TFC_Interface/RSPTimeStamp.h>
#include <TFC_InputSection/BeamletBuffer.h>


namespace LOFAR
{
  typedef struct 
  {
    BeamletBuffer* BBuffer;
    TransportHolder* Connection; 
    int PayloadSize;
    int SubbandSize;
    int EPAHeaderSize;
    int EPAPacketSize;
    int IPHeaderSize;
    int nrPacketsInFrame;
    int nrSubbandsInPacket;
    int nrRSPoutputs;
    int* StationIDptr;
    bool Stopthread;
    bool IsMaster;
  }  ThreadArgs;
  
  typedef struct {
    timestamp_t receivedStamp;
    timestamp_t expectedStamp;
  } PacketStats;

  void printTimers(vector<NSTimer*>& timers);
  void* ReaderThread(void* arguments);

} // namespace LOFAR

#endif
