//#  WH_RSPInput.cc: Catch RSP ethernet frames and synchronize RSP inputs
//#
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

#include <lofar_config.h>

// General includes
#include <Common/LofarLogger.h>

#include <CEPFrame/DataManager.h>
#include <tinyCEP/Profiler.h>

// Application specific includes
#include <WH_RSPInput.h>
#include <DH_RSP.h>
#include <DH_RSPSync.h>
#include <BufferController.h>

using namespace LOFAR;

ProfilingState WH_RSPInput::theirOldDataState;
ProfilingState WH_RSPInput::theirMissingDataState;

WH_RSPInput::WH_RSPInput(const string& name, 
                         const KeyValueMap kvm,
                         const string device,
                         const string srcMAC,
                         const string destMAC)
  : WorkHolder (1, 1, name, "WH_RSPInput"),
    itsKVM (kvm),
    itsDevice(device),
    itsSrcMAC(srcMAC),
    itsDestMAC(destMAC),
    itsActualStamp(0,0),
    itsNextStamp(-1,0),
    itsReadNext(true)
{
  // number of EPA packets per RSP frame
  int itsNpackets = kvm.getInt("NoPacketsInFrame", 8);
  
  // size of an EPA packet in bytes 
  int sizeofpacket   = ( kvm.getInt("polarisations",2) * 
                          sizeof(complex<int16>) * 
                          kvm.getInt("NoRSPBeamlets", 92)
                       ) + 
                       kvm.getInt("SzEPAheader", 14); 
 
  // size of a RSP frame in bytes
  itsSzRSPframe = itsNpackets * sizeofpacket;

  // create raw ethernet interface to catch incoming RSP data
  itsInputConnection = new TH_Ethernet(itsDevice, itsSrcMAC, itsDestMAC, 0x000 );
  
  // create a buffer to hold imcoming RSP frame  
   itsRecvFrame = new char[itsSzRSPframe]; 
 
  // create incoming dataholder holding the delay information 
  //getDataManager().addInDataHolder(0, new DH_Delay("DH_delay"));

  // create outgoing dataholder holding the delay controlled RSP data
  getDataManager().addOutDataHolder(0, new DH_RSP("DH_RSP_out", itsKVM));

  // use a cyclic buffer to hold the rsp data
  itsDataBuffer = new BufferController< BufferType1 >(100); // 100 elements

  // use a cyclic buffer to hold the valid/invalid flag
  itsFlagBuffer = new BufferController< BufferType2 >(100); // 100 elements
 
  // init profiling states
  theirOldDataState.init ("WH_RSPInput old packets", "yellow");
  theirMissingDataState.init ("WH_RSPInput missing packets", "orange");  

}

WH_RSPInput::~WH_RSPInput() 
{
  delete itsDataBuffer;
  delete itsFlagBuffer;
  delete itsInputConnection;

  delete [] itsRecvFrame;
}

WorkHolder* WH_RSPInput::construct(const string& name,
                                   const KeyValueMap kvm,
                                   const string device,
                                   const string srcMAC,
                                   const string destMAC)
{
  return new WH_RSPInput(name, kvm, device, srcMAC, destMAC);
}

WH_RSPInput* WH_RSPInput::make(const string& name)
{
  return new WH_RSPInput(name, itsKVM, itsDevice, itsSrcMAC, itsDestMAC);
}

void WH_RSPInput::preprocess()
{
  cout << "WH_RSPInput::preprocess() begin" << endl;
  //itsDataBuffer->preprocess();
  //itsFlagBuffer->preprocess();
  cout << "WH_RSPInput::preprocess() end" << endl;
}

void WH_RSPInput::process() 
{ 
  if (itsReadNext) {
    // read a new RSP packet from input
    readInput();  
  }
    
  if (itsNextStamp.getSeqId() == -1) {
    // first loop so set itsNextStamp equal to thisStamp
    itsNextStamp.setStamp(itsActualStamp.getSeqId(), itsActualStamp.getBlockId());
  }
  
  BufferType1* edata;
  BufferType2* eflag;
  DH_RSP* outDHp;
  bool newStamp = false;

  while (!newStamp) {
      
    if (itsActualStamp < itsNextStamp) { 
      // this packets time stamp is too old
      // packet could be delayed in ethernet connection link
      //cout << "old packet" << endl;
  
      // to do: store this packet in right place of output buffer to 
      // overwrite already made dummy-data for this timestamp
      
      // set profiling state
      theirMissingDataState.leave();
      theirOldDataState.enter();

      LOG_TRACE_COND_STR("Old packet received");
      
      // stay in the while loop and trigger a new read
      readInput();
      newStamp = false;
    } 
    else if (itsNextStamp + (itsNpackets - 1) < itsActualStamp) {
      // we missed a packet 
      //cout << "missed a packet" << endl;
      // set profiling state
      theirOldDataState.leave();
      theirMissingDataState.enter();
 
      // copy dummy data for missing timestamp in OutDataholder 
      edata = (BufferType1*)itsDataBuffer->getWritePtr();
      eflag = (BufferType2*)itsFlagBuffer->getWritePtr();
      //to do: set the missing stamp
      eflag->element = true;
     
      itsDataBuffer->readyWriting();
      itsFlagBuffer->readyWriting();
      
      // step out of the while loop and do not trigger a new read 
      // so this packet will be read again next loop
      newStamp = true;
      itsReadNext = false;
    }
    else { 
      // excpected packet received
 
      // reset profiling states
      theirMissingDataState.leave();
      theirOldDataState.leave();

      // copy contents of InDataHolder to OutDataholder
      edata = (BufferType1*)itsDataBuffer->getWritePtr();
      eflag = (BufferType2*)itsFlagBuffer->getWritePtr();
      memcpy(edata->element, itsRecvFrame, itsSzRSPframe);
      eflag->element = false;      
     
      itsDataBuffer->readyWriting();
      itsFlagBuffer->readyWriting();
      
      // step out of the while loop and trigger a new read
      newStamp = true;
      itsReadNext = true;
    }
  }
  // increment itsNextStamp
  itsNextStamp += itsNpackets;
}

void WH_RSPInput::readInput()
{    
  // catch a new ethernet frame 
  itsInputConnection->recvBlocking( (void*)itsRecvFrame, itsSzRSPframe, 0);
    
  // get the timestamp
  int seqid   = ((int*)&itsRecvFrame[6])[0];
  int blockid = ((int*)&itsRecvFrame[10])[0];
  itsActualStamp.setStamp(seqid ,blockid);
}

void WH_RSPInput::dump() 
{
}
