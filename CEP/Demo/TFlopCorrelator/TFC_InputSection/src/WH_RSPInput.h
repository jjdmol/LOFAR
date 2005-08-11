//#  WH_RSPInput.h: Catch RSP ethernet frames and synchronize RSP inputs 
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

#ifndef TFLOPCORRELATOR_WH_RSPINPUT_H
#define TFLOPCORRELATOR_WH_RSPINPUT_H

#include <pthread.h>
#include <tinyCEP/TinyDataManager.h>
#include <tinyCEP/WorkHolder.h>
#include <APS/ParameterSet.h>
#include <Transport/TransportHolder.h>
#include <TFC_Interface/RSPTimeStamp.h>
#include <TFC_InputSection/BufferController.h>


namespace LOFAR
{

  using ACC::APS::ParameterSet;
  
  typedef struct 
  {
    char data[8]; // (2 * u16complex)
  } subbandType;

  typedef struct
  {
    //int stationid;
    int invalid;
    timestamp_t timestamp;
  } metadataType;

  typedef struct 
  {
    BufferController** BufControl;
    TransportHolder* Connection; 
    int FrameSize;
    int SubbandSize;
    int EPAHeaderSize;
    int EPAPacketSize;
    int nrPacketsInFrame;
    int nrSubbandsInPacket;
    int nrRSPoutputs;
    int* StationIDptr;
    bool Stopthread;
  }  thread_args;
  
  class WH_RSPInput: public WorkHolder
  {
    public:

      explicit WH_RSPInput(const string& name, 
                           ParameterSet& ps,
                           TransportHolder& th,
                           const bool isSyncMaster);
      virtual ~WH_RSPInput();
    
      static WorkHolder* construct(const string& name, 
                                   ParameterSet& ps,
                                   TransportHolder& th,
				   const bool isSyncMaster);
	
      virtual WH_RSPInput* make(const string& name);
     
      virtual void startThread();
    
      virtual void process();

      virtual void postprocess();
    
      // Show the work holder on stdout.
      virtual void dump() const;

    private:
    
      // forbid copy constructor
      WH_RSPInput (const WH_RSPInput&);
    
      // forbid assignment
      WH_RSPInput& operator= (const WH_RSPInput&);

      // writer thread
      pthread_t writerthread;
      thread_args writerinfo;

      // raw ethernet interface 
      TransportHolder& itsTH;
      
      // ACC parameters interface
      ParameterSet &itsPS;
      
      // Sync Master or slave
      bool itsSyncMaster;

      // detect First process loop
      bool itsFirstProcessLoop;

      // synced stamp
      timestamp_t itsSyncedStamp;
     
      int itsSzRSPframe;
      int itsNPackets;
      int itsNRSPOutputs;
      int itsNSubbands;
      int itsNSamplesToCopy;
      int itsNPolarisations;
      int itsCyclicBufferSize;
      int itsStationID;
      int itsEPAHeaderSize;
      int itsEPAPacketSize;
     
      BufferController** itsBufControl;
  };

} // namespace LOFAR

#endif
