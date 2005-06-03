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

#ifndef STATIONCORRELATOR_WH_RSPINPUT_H
#define STATIONCORRELATOR_WH_RSPINPUT_H


#include <Transport/TH_Ethernet.h>

#include <Common/KeyValueMap.h>
#include <Common/lofar_string.h>
#include <tinyCEP/WorkHolder.h>
#include <DH_RSPSync.h>
#include <BufferController.h>

namespace LOFAR
{
  
  struct BufferType1 {
    char element[9000];
  };

  struct BufferType2 {
    bool element;
  };


  class WH_RSPInput: public WorkHolder
  {
  public:

    explicit WH_RSPInput(const string& name, 
                         const KeyValueMap kvm,
                         const string device,
                         const string srcMAC,
                         const string destMAC);
    virtual ~WH_RSPInput();
    
    static WorkHolder* construct(const string& name, 
                                 const KeyValueMap kvm,
                                 const string device,
                                 const string srcMAC,
                                 const string destMAC);
	
    virtual WH_RSPInput* make(const string& name);
    
    virtual void preprocess();
    
    virtual void process();
    
    /// Show the work holder on stdout.
    virtual void dump();

  private:
    /// forbid copy constructor
    WH_RSPInput (const WH_RSPInput&);
    /// forbid assignment
    WH_RSPInput& operator= (const WH_RSPInput&);

    // read data from input connection
    void readInput();

    // raw ethernet interface 
    TH_Ethernet* itsInputConnection;
    string itsDevice;
    string itsSrcMAC;
    string itsDestMAC;
    char* itsRecvFrame;
    int itsSzRSPframe;
    int itsNpackets;
    
    
    // cyclic buffer for rsp-data
    BufferController<BufferType1> *itsDataBuffer;
    
    // cyclic buffer for invalid data flag
    BufferController<BufferType2> *itsFlagBuffer;
    
    // keyvalue map
    KeyValueMap itsKVM;

    // timestamps
    DH_RSPSync::syncStamp_t itsActualStamp, itsNextStamp;
    
    // Do we need to read at the beginning of the next process()?
    bool itsReadNext;

    static ProfilingState theirOldDataState;
    static ProfilingState theirMissingDataState;
  };

} // namespace LOFAR

#endif
