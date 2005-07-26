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

#include <APS/ParameterSet.h>
#include <Common/lofar_string.h>
#include <tinyCEP/TinyDataManager.h>
#include <tinyCEP/WorkHolder.h>
#include <Transport/TH_Ethernet.h>

#include <pthread.h>

#include <TFC_Interface/RSPTimeStamp.h>
#include <BufferController.h>


namespace LOFAR
{
  
  typedef struct 
  {
    char data[8]; // (2 * u16complex)
  } subbandType;

  typedef struct
  {
    int stationid;
    int invalid;
    timestamp_t timestamp;
  } metadataType;

  typedef struct 
  {
    BufferController<subbandType>** SubbandBuffer;
    BufferController<metadataType>** MetadataBuffer;
    TH_Ethernet* Connection; 
    int FrameSize;
    int SubbandSize;
    int nrPacketsInFrame;
    int nrSubbandsInPacket;
    int nrRSPoutputs;
    TinyDataManager* Datamanager;
    bool Syncmaster;
    bool Stopthread;
  }  thread_args;

  
  class WH_RSPInput: public WorkHolder
  {
    public:

      explicit WH_RSPInput(const string& name, 
                           const ACC::APS::ParameterSet pset,
                           const string device,
                           const string srcMAC,
                           const string destMAC,
                           const bool isSyncMaster);
      virtual ~WH_RSPInput();
    
      static WorkHolder* construct(const string& name, 
                                   const ACC::APS::ParameterSet pset,
                                   const string device,
                                   const string srcMAC,
                                   const string destMAC,
				   const bool isSyncMaster);
	
      virtual WH_RSPInput* make(const string& name);
     
      virtual void preprocess();
    
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
      TH_Ethernet* itsInputConnection;
      string itsDevice;
      string itsSrcMAC;
      string itsDestMAC;
      
      ACC::APS::ParameterSet itsPset;
      
      bool itsSyncMaster;
      int itsSzRSPframe;
      int itsNpackets;
      int itsNRSPOutputs;
      int itsNSubbands;
      int itsNSamplesToCopy;
      int itsNPolarisations;
      int itsCyclicBufferSize;
     
      BufferController<subbandType> **itsSubbandBuffer;
      BufferController<metadataType> **itsMetadataBuffer;
  };

} // namespace LOFAR

#endif
