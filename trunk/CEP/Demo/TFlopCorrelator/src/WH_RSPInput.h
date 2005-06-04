//#  WH_RSPInput.h: Catch RSP ethernet frames and synchronize RSP inputs 
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//#  $Id$
//#
/////////////////////////////////////////////////////////////////

#ifndef TFLOPCORRELATOR_WH_RSPINPUT_H
#define TFLOPCORRELATOR_WH_RSPINPUT_H

#include <Transport/TH_Ethernet.h>

#include <Common/KeyValueMap.h>
#include <Common/lofar_string.h>
#include <tinyCEP/WorkHolder.h>
//#include <DH_RSPSync.h>
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
                         const string device,
                         const string srcMAC,
                         const string destMAC);
    virtual ~WH_RSPInput();
    
    static WorkHolder* construct(const string& name, 
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
    BufferController<BufferType1*> *itsDataBuffer;
    
    // cyclic buffer for invalid data flag
    BufferController<BufferType2*> *itsFlagBuffer;
    
    // timestamps
    DH_RSPSync::syncStamp_t itsActualStamp, itsNextStamp;
    
    // Do we need to read at the beginning of the next process()?
    bool itsReadNext;

    static ProfilingState theirOldDataState;
    static ProfilingState theirMissingDataState;
  };

} // namespace LOFAR

#endif
