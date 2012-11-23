//#  TBBControl.h: Controller for the TBBDriver
//#
//#  Copyright (C) 2007
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

#ifndef TBBCONTROL_H
#define TBBCONTROL_H

//# Common Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <ApplCommon/PosixTime.h>
#include <APL/APLCommon/AntennaField.h>

//# ACC Includes
#include <Common/ParameterSet.h>

//# GCF Includes
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_ITCPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <GCF/TM/GCF_Task.h>
#include <MACIO/GCF_Event.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/RTDB/GCF_RTDBPort.h>

#include <APL/CR_Protocol/CRstopRequest.h>
#include <APL/CR_Protocol/CRreadRequest.h>

//# local includes
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>
#include <VHECR/TBBTrigger.h>
#include <VHECR/TBBReadCmd.h>
#include <VHECR/VHECRTask.h>
#include "TBBObservation.h"

// forward declaration


namespace LOFAR {
    using namespace CR_Protocol;
    namespace StationCU {

using   MACIO::GCFEvent;
using   GCF::TM::GCFTimerPort;
using   GCF::TM::GCFITCPort;
using   GCF::TM::GCFPort;
using   GCF::TM::GCFPortInterface;
using   GCF::TM::GCFTask;
using   GCF::RTDB::RTDBPropertySet;
using   GCF::RTDB::GCFRTDBPort;
using   APLCommon::ParentControl;
using   VHECR::TBBReadCmd;
using   VHECR::TBBTrigger;
using   VHECR::VHECRTask;

typedef bitset<MAX_RCUS> RCUset_t;

// RCU information
class RcuInfo
{
public:
    RcuInfo();
    ~RcuInfo(){};
    int              rcuNr;
    int              boardNr;
    char             rcuState;    // state 'I', 'A', 'R', 'S' or 'E'
    uint16           triggerMode;
    uint16           operatingMode;
    double           bufferTime;  // recording time of buffer
private:
};

// Board information
class BoardInfo
{
public:
    BoardInfo();
    ~BoardInfo(){};
    int  boardNr;
    bool cepActive; // true if cep is sending data
    int  cepDelay;  // last set cep delay
private:
};

class StopRequest
{
public:
    StopRequest();
    ~StopRequest(){};
    RCUset_t rcuSet;
    RTC::NsTimestamp stopTime;
private:
};

class ReadRequest
{
public:
    ReadRequest();
    ~ReadRequest(){};
    int    rcuNr;
    RTC::NsTimestamp readTime;
    RTC::NsTimestamp timeBefore;
    RTC::NsTimestamp timeAfter;
    int    cepDelay;
    int    cepDatapaths;
    bool   readActive;
private:
};


class TBBControl : public GCFTask
{
public:
    explicit TBBControl(const string& cntlrName);
    ~TBBControl();

    // used by externel program
    void readTBBdata(vector<TBBReadCmd> readCmd);

    // Interrupthandler for switching to finishingstate when exiting the program.
    static void sigintHandler (int signum);
    void finish();

private:
    // During the initial state all connections with the other programs are made.
    GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
    // connected to PVSS, waiting for CLAIM event
    GCFEvent::TResult started_state (GCFEvent& e, GCFPortInterface& p);
    // connected to TBBDriver, waiting for PREPARE event
    GCFEvent::TResult claimed_state (GCFEvent& e, GCFPortInterface& p);
    // released state, unsubscribed and freed tbb memory 
    GCFEvent::TResult released_state(GCFEvent& event, GCFPortInterface& port); 
    // TBB boards are setup, waiting for RESUME event
    GCFEvent::TResult prepared_state  (GCFEvent& e, GCFPortInterface& p);
    // Normal control mode, handling of trigger messages is active
    GCFEvent::TResult active_state  (GCFEvent& e, GCFPortInterface& p);
    // Quiting, shutdown connections, send FINISH and quit 
    GCFEvent::TResult quiting_state (GCFEvent& e, GCFPortInterface& p);


    // set boards in right  observation mode (transient or subbands
    void sendRspModeCmd();
    int handleRspModeAck(GCFEvent& event);
    
    // set storage node for boards
    void sendStorageCmd();
    int handleStorageAck(GCFEvent& event);
    
    // allocate memory for selected rcus
    void sendAllocCmd();
    int handleAllocAck(GCFEvent& event);
    
    // allocate memory for selected rcus
    void sendRcuInfoCmd();
    int handleRcuInfoAck(GCFEvent& event);
    
    // setup trigger system for selected rcus
    void sendTrigSetupCmd();
    int handleTrigSetupAck(GCFEvent& event);
    
    // setup filter coeffcients for selected rcus
    void sendTrigCoefCmd();
    int handleTrigCoefAck(GCFEvent& event);
    
    // start recording on selected rcus
    void sendRecordCmd(RCUset_t RCUset);
    int handleRecordAck(GCFEvent& event);
    
    // stop recording on selected rcus
    void sendStopCmd();
    int handleStopAck(GCFEvent& event);
    
    // get Cep status
    void sendCepStatusCmd();
    int handleCepStatusAck(GCFEvent& event);
    
    // send delay setting for CEP port
    void sendCepDelayCmd();
    int handleCepDelayAck(GCFEvent& event);
    
    // stop sending on CEP port
    void sendStopCepCmd();
    int handleStopCepAck(GCFEvent& event);
    
    // subscribe on tbb messages
    void sendSubscribeCmd();
    int handleSubscribeAck(GCFEvent& event);
    
    // release trigger system for selected rcus
    void sendReleaseCmd(RCUset_t RCUset);
    int handleReleaseAck(GCFEvent& event);
    
    // send data to CEP for selected rcu
    void sendReadCmd(ReadRequest read);
    int handleReadAck(GCFEvent& event);
    
    // unsubscribe tbb mesages
    void sendUnsubscribeCmd();
    int handleUnsubscribeAck(GCFEvent& event);
    
    // free memory for selected rcus
    void sendFreeCmd();
    int handleFreeAck(GCFEvent& event);

   void readCmdToRequests(vector<TBBReadCmd> read);

    // avoid defaultconstruction and copying
    TBBControl();
    TBBControl(const TBBControl&);
    TBBControl& operator=(const TBBControl&);

    void    setState    (CTState::CTstateNr     newState);
    
    RCUset_t strToBitset(string str);
    
    GCFEvent::TResult   handleTriggerEvent(GCFEvent& event);
    //GCFEvent::TResult   triggerReleaseAckEventHandler(GCFEvent& event);
    GCFEvent::TResult   _defaultEventHandler(GCFEvent& event, GCFPortInterface& port);

    bool isBoardUsed(int board);
    
    ParameterSet*       itsParameterSet;
    RTDBPropertySet*    itsPropertySet;
    bool                itsPropertySetInitialized;
    int                 itsNrTBBs;
    int                 itsNrRCUs;
    int                 itsMaxCepDatapaths;
    int                 itsActiveCepDatapaths;
    int                 itsCepDelay;  // 5 uSec/cnt
    bool                itsAutoRecord;
   
    // pointer to parent control task
    ParentControl*      itsParentControl;
    GCFITCPort*         itsParentPort;

    GCFTimerPort*       itsTimerPort;
    GCFTimerPort*       itsVHECRtimer;
    GCFTimerPort*       itsStopTimer;
    GCFTimerPort*       itsCepTimer;
    GCFRTDBPort*        itsTriggerPort;
    GCFTCPPort*         itsTBBDriver;
    GCFTCPPort*         itsRSPDriver;

    CTState::CTstateNr  itsState;
    VHECRTask*          itsVHECRTask;
    // ParameterSet variables
    string              itsTreePrefix;
    TBBObservation*     itsObs;
    vector<RcuInfo>     itsRcuInfo;
    vector<BoardInfo>   itsBoardInfo;
    vector<StopRequest> itsStopRequests;
    vector<ReadRequest> itsReadRequests;
};

    };//StationCU
};//LOFAR
#endif
