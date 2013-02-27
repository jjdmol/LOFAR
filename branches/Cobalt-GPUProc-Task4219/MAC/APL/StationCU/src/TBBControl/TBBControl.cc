//#  TBBControl.cc: Implementation of the MAC Scheduler task
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



#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/LofarDirs.h>
#include <ApplCommon/StationInfo.h>
#include <ApplCommon/StationConfig.h>

#include <GCF/PVSS/GCF_PVTypes.h>
//#include <GCF/Utils.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/CR_Protocol/CR_Protocol.ph>
#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <signal.h>
#include <time.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/RTCCommon/NsTimestamp.h>
#include <StationCU/Package__Version.h>

//# local includes
#include <VHECR/TBBTrigger.h>
#include <VHECR/TBBReadCmd.h>
#include <VHECR/VHECRTask.h>
#include "TBBControl.h"
#include "PVSSDatapointDefs.h"
#include "TBBObservation.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace std;

namespace LOFAR {
    using namespace GCF::TM;
    using namespace GCF::PVSS;
    using namespace GCF::RTDB;
    using namespace VHECR;
    using namespace APLCommon;
    using namespace RTC;
    namespace StationCU {

#define SUCCESS 1
#define FAILURE 0

#define VHECR_INTERVAL 0.1

//
// Boardinfo
//
BoardInfo::BoardInfo():
    boardNr(0),
    cepActive(false),
    cepDelay(0)
{}

//
// RCUinfo
//
RcuInfo::RcuInfo():
    rcuNr(0),
    boardNr(0),
    rcuState('I'),
    triggerMode(0),
    operatingMode(0),
    bufferTime(0.0)
{}

//
// StopRequest
//
StopRequest::StopRequest():
    rcuSet(0),
    stopTime(0.0)
{}

//
// ReadRequest
//
ReadRequest::ReadRequest():
    rcuNr(0),
    readTime(0.0),
    timeBefore(0.0),
    timeAfter(0.0),
    cepDelay(0),
    readActive(false)
{}


// static pointer to this object for signal handler
static TBBControl*  thisTBBControl = 0;

//
// TBBControl()
//
TBBControl::TBBControl(const string&    cntlrName) :
    GCFTask             ((State)&TBBControl::initial_state,cntlrName),
    itsPropertySet      (0),
    itsPropertySetInitialized (false),
    itsParentControl    (0),
    itsParentPort       (0),
    itsTimerPort        (0),
    itsVHECRtimer       (0),
    itsStopTimer        (0),
    itsCepTimer         (0),
    itsTriggerPort      (0), 
    itsTBBDriver        (0),
    itsState            (CTState::NOSTATE)
{
    StationConfig* itsStationConfig;
    itsStationConfig = globalStationConfig();
    itsNrTBBs = itsStationConfig->nrTBBs;
    itsNrRCUs = itsNrTBBs * 16;
    itsMaxCepDatapaths = 1;
    itsActiveCepDatapaths = 0;
    itsCepDelay = 0;
    itsAutoRecord = true;
    //itsBoardCepActive.resize(itsNrTBBs, false);
    
    RcuInfo rcuinfo;
    for (int i = 0; i < itsNrRCUs; i++) {
        rcuinfo.rcuNr = i;
        rcuinfo.boardNr = i/16;
        rcuinfo.rcuState = 'I';
        rcuinfo.triggerMode = 0;
        rcuinfo.operatingMode = TBB_MODE_TRANSIENT;
        rcuinfo.bufferTime = 0.0;
        itsRcuInfo.push_back(rcuinfo);
    }
    
    BoardInfo boardinfo;
    for (int i = 0; i < itsNrTBBs; i++) {
        boardinfo.boardNr = i;
        boardinfo.cepActive = false;
        boardinfo.cepDelay = 0;
        itsBoardInfo.push_back(boardinfo);
    }
    
    LOG_TRACE_OBJ_STR (cntlrName << " construction");
    LOG_INFO(Version::getInfo<StationCUVersion>("TBBControl"));

    // First readin our observation related config file.
    LOFAR::ConfigLocator cl;
    LOG_DEBUG_STR("Reading parset file:" << cl.locate(cntlrName));
    itsParameterSet = new ParameterSet(cl.locate(cntlrName));
    itsObs = new TBBObservation(itsParameterSet);   // does all nasty conversions

    //LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
    //itsParameterSet = new ParameterSet(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);
    //itsObs = new TBBObservation(itsParameterSet); // does all nasty conversions
    //globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);
    //itsObs = new TBBObservation(globalParameterSet());    // does all nasty conversions

    LOG_DEBUG_STR(*itsObs);
    // Readin some parameters from the ParameterSet.
    itsTreePrefix = itsParameterSet->getString("prefix");
    //itsInstanceNr = globalParameterSet()->getUint32("_instanceNr");

    // attach to parent control task
    itsParentControl = ParentControl::instance();
    //itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport",
    //                                GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
    //ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
    //itsParentPort->open();      // will result in F_CONNECTED

    // need port for timers.
    itsTimerPort = new GCFTimerPort(*this, "TimerPort");

    // timer port for calling VHECRTask every 100mS
    itsVHECRtimer = new GCFTimerPort(*this, "VHECRtimer");
    
    // stop timer too stop recording on a specific time
    itsStopTimer = new GCFTimerPort(*this, "StopTimer");
    
    // Cep timer too check CEP status if read is active
    itsCepTimer = new GCFTimerPort(*this, "CepTimer");
    
    // prepare RTDB port for receiving External trigger messages.
    itsTriggerPort = new GCFRTDBPort (*this,
                                    "RTDB_TriggerPort",
                                    PSN_CR_TRIGGERPORT);
    
    // prepare TCP port to TBBDriver.
    itsTBBDriver = new GCFTCPPort (*this,
                                    MAC_SVCMASK_TBBDRIVER,
                                    GCFPortInterface::SAP,
                                    TBB_PROTOCOL);
    ASSERTSTR(itsTBBDriver, "Cannot allocate TCPport to TBBDriver");

    // prepare TCP port to RSPDriver.
    itsRSPDriver = new GCFTCPPort (*this,
                                    MAC_SVCMASK_RSPDRIVER,
                                    GCFPortInterface::SAP,
                                    RSP_PROTOCOL);
    ASSERTSTR(itsRSPDriver, "Cannot allocate TCPport to RSPDriver");



    // for debugging purposes
    registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
    registerProtocol (DP_PROTOCOL, DP_PROTOCOL_STRINGS);
    registerProtocol (CR_PROTOCOL, CR_PROTOCOL_STRINGS);
    registerProtocol (TBB_PROTOCOL, TBB_PROTOCOL_STRINGS);
    registerProtocol (RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);

    setState(CTState::CREATED);

    itsStopRequests.clear();
    itsReadRequests.clear();
    
    itsVHECRTask = new VHECRTask(cntlrName);
    ASSERTSTR(itsVHECRTask, "Could not create the VHECR task");
    //itsVHECRTask->setSaveTask(this);
}


//
// ~TBBControl()
//
TBBControl::~TBBControl()
{
    delete itsTimerPort;
    delete itsVHECRtimer;
    delete itsStopTimer;
    delete itsCepTimer;
    delete itsTriggerPort;
    delete itsTBBDriver;
    delete itsRSPDriver;
    delete itsVHECRTask;
    
    LOG_TRACE_OBJ_STR (getName() << " destruction");
}



/*
o- USED STATES -----------------------------------------------------------------o
|                                                                               |
| initial_state                                                                 |
| started_state                                                                 |
| claimed_state                                                                 |
| prepared_state                                                                |
| active_state                                                                  |
| released_state                                                                |
| quiting_state                                                                 |
o-------------------------------------------------------------------------------o
*/


//------------------------------------------------------------------------------
// initial_state(event, port)
//
// Connect to PVSS and report state back to startdaemon
//------------------------------------------------------------------------------
GCFEvent::TResult TBBControl::initial_state(GCFEvent& event,
                                            GCFPortInterface& port)
{
    LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

    GCFEvent::TResult status = GCFEvent::HANDLED;

    switch (event.signal) {
        case F_ENTRY: {
        } break;

        case F_INIT: {
            // Get access to my own propertyset.
            string propSetName( createPropertySetName(PSN_TBB_CONTROL,
                                getName(),
                                itsParameterSet->getString("_DPname")));
            LOG_INFO_STR ("Activating PropertySet " << propSetName);
            itsPropertySet = new RTDBPropertySet(   propSetName,
                                                    PST_TBB_CONTROL,
                                                    PSAT_RW,
                                                    this);
            LOG_INFO_STR ("Activating " << propSetName << " Done");
            // Wait for timer that is set on DP_CREATED event
        } break;

        case DP_CREATED: {
            // NOTE: this function may be called DURING the construction of the PropertySet.
            // Always exit this event in a way that GCF can end the construction.
            DPCreatedEvent  dpEvent(event);
            LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
            itsTimerPort->cancelAllTimers();
            itsTimerPort->setTimer(0.5);
        } break;

        case F_TIMER: {
            if (!itsPropertySetInitialized) {
                itsPropertySetInitialized = true;

                // Instruct codeloggingProcessor
                // TODO: get this from a .h file
                LOG_INFO_STR("MACProcessScope: LOFAR.PermSW.TBBControl");

                // first redirect signalHandler to our quiting state to leave PVSS
                // in the right state when we are going down
                thisTBBControl = this;
                signal (SIGINT,  TBBControl::sigintHandler);    // ctrl-c
                signal (SIGTERM, TBBControl::sigintHandler);    // kill

                // update PVSS.
                LOG_TRACE_FLOW ("Updating state to PVSS");
    // TODO TBB
                itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("initial"));
                itsPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
                itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(false));

                itsPropertySet->setValue(PN_TBC_TRIGGER_RCU_NR, GCFPVInteger(0),0.0,false);
                itsPropertySet->setValue(PN_TBC_TRIGGER_SEQUENCE_NR,    GCFPVInteger(0),0.0,false);
                itsPropertySet->setValue(PN_TBC_TRIGGER_TIME,   GCFPVInteger(0),0.0,false);
                itsPropertySet->setValue(PN_TBC_TRIGGER_SAMPLE_NR,  GCFPVInteger(0),0.0,false);
                itsPropertySet->setValue(PN_TBC_TRIGGER_SUM,    GCFPVInteger(0),0.0,false);
                itsPropertySet->setValue(PN_TBC_TRIGGER_NR_SAMPLES, GCFPVInteger(0),0.0,false);
                itsPropertySet->setValue(PN_TBC_TRIGGER_PEAK_VALUE, GCFPVInteger(0),0.0,false);
                itsPropertySet->flush();

                // Start ParentControl task
                LOG_DEBUG ("Enabling ParentControl task and wait for my name");
                itsParentPort = itsParentControl->registerTask(this);
                // results in CONTROL_CONNECT
            }
        } break;

        case F_CONNECTED: {
            LOG_DEBUG_STR("F_CONNECTED event from port " << port.getName());
            //ASSERTSTR (&port == itsParentPort, "F_CONNECTED event from port " << port.getName());
        } break;

        case F_DISCONNECTED:
        case F_EXIT: {
        } break;

        case CONTROL_CONNECT: {
            CONTROLConnectEvent msg(event);
            LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
            setState(CTState::CONNECTED);
            sendControlResult(port, CONTROL_CONNECTED, msg.cntlrName, CT_RESULT_NO_ERROR);

            // let ParentControl watch over the start and stop times for extra safety.
            ptime startTime = time_from_string(itsParameterSet->getString("Observation.startTime"));
            ptime stopTime  = time_from_string(itsParameterSet->getString("Observation.stopTime"));
            itsParentControl->activateObservationTimers(msg.cntlrName, startTime, stopTime);

            LOG_INFO ("Going to started state");
            TRAN(TBBControl::started_state);  // go to next state.
        } break;

        default: {
            status = _defaultEventHandler(event, port);
        } break;
    }

    return (status);
}


//------------------------------------------------------------------------------
// started_state(event, port)
//
// wait for CLAIM event
//------------------------------------------------------------------------------
GCFEvent::TResult TBBControl::started_state(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR ("started:" << eventName(event) << "@" << port.getName());

    GCFEvent::TResult status = GCFEvent::HANDLED;

    switch (event.signal) {
        case F_ENTRY: {
            // update PVSS
    //      itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("started"));
            itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
            itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(false));
        } break;

        case F_CONNECTED: {
            LOG_DEBUG_STR("F_CONNECTED event from port " << port.getName());
            if (&port == itsTBBDriver) {
                LOG_INFO ("Connected with TBBDriver");
            }
        else if (&port == itsRSPDriver) {
                LOG_INFO ("Connected with RSPDriver");
            }
            else if (&port == itsTriggerPort) {
                LOG_INFO ("Connected with RTDB (external triggering)");
            }
            itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(true));
            
            //itsTimerPort->cancelAllTimers();
            if (itsTBBDriver->isConnected() && itsRSPDriver->isConnected() && itsTriggerPort->isConnected()) {
                LOG_INFO ("Connected with TBBDriver, RSPDriver and RTDB, going to claimed state now");
                setState(CTState::CLAIMED);
                sendControlResult(*itsParentPort, CONTROL_CLAIMED, getName(), CT_RESULT_NO_ERROR);
                TRAN(TBBControl::claimed_state);                // go to next state.
            }
        } break;

        case F_DISCONNECTED: {
            LOG_DEBUG_STR("F_DISCONNECTED event from port " << port.getName());
            if (&port == itsTBBDriver) {
                LOG_WARN_STR ("Connection with TBBDriver failed, retry in 2 seconds");
            }
            else if (&port == itsRSPDriver) {
                LOG_WARN_STR ("Connection with RSPDriver failed, retry in 2 seconds");
            }
            else if (&port == itsTriggerPort) {
                LOG_WARN_STR ("Connection with RTDB failed, retry in 2 seconds");
            }
            port.close();
            itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(false));
            itsTimerPort->setTimer(2.0);
        } break;

        case F_TIMER: {
            if (!itsTBBDriver->isConnected()) {
                LOG_DEBUG ("Trying to reconnect to TBBDriver");
                itsTBBDriver->open();       // will result in F_CONN or F_DISCONN
            }
            else if (!itsRSPDriver->isConnected()) {
                LOG_DEBUG ("Trying to reconnect to RSPDriver");
                itsRSPDriver->open();       // will result in F_CONN or F_DISCONN
            }
            else if (!itsTriggerPort->isConnected()) {
                LOG_DEBUG ("Trying to reconnect to RTDB");
                itsTriggerPort->open();       // will result in F_CONN or F_DISCONN
            }
        } break;

        // -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
        case CONTROL_CLAIM: {
            CONTROLClaimEvent msg(event);
            LOG_DEBUG_STR("Received CLAIM(" << msg.cntlrName << ")");
            setState(CTState::CLAIM);
            LOG_DEBUG ("Trying to connect to TBBDriver");
            itsTBBDriver->open();  // will result in F_CONN or F_DISCONN
            LOG_DEBUG ("Trying to reconnect to RSPDriver");
            itsRSPDriver->open();  // will result in F_CONN or F_DISCONN
            LOG_DEBUG ("Trying to reconnect to RTDB");
            itsTriggerPort->open();  // will result in F_CONN or F_DISCONN
        } break;

        default: {
            status = _defaultEventHandler(event, port);
        } break;
    }

    return (status);
}


//------------------------------------------------------------------------------
// claimed_state(event, port)
//
// wait for PREPARE event.
//------------------------------------------------------------------------------
GCFEvent::TResult TBBControl::claimed_state(GCFEvent& event, GCFPortInterface& port)
{
    int cmdStatus = SUCCESS;
    
    LOG_DEBUG_STR ("claimed:" << eventName(event) << "@" << port.getName());

    GCFEvent::TResult status = GCFEvent::HANDLED;

    switch (event.signal) {
        case F_ENTRY: {
            // update PVSS
            // itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("claimed"));
            itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
        } break;

        // -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
        case CONTROL_PREPARE: {
            CONTROLPrepareEvent msg(event);
            LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
            setState(CTState::PREPARE);
            if (itsObs->isLoaded()) {
                // start setting up the observation
                sendRspModeCmd();
            } else {
                // no observations to setup, go direct prepared state
                sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_NO_ERROR);
                setState(CTState::PREPARED);
                TRAN(TBBControl::prepared_state);  // go to prepared state.
            }
        } break;
        
        // -------------------- EVENTS RECEIVED FROM TBBDriver --------------------
        // response from sendRspMode()
        case RSP_SETTBBACK: {
            cmdStatus = handleRspModeAck(event);
            if (cmdStatus == SUCCESS) {
                //sendStorageCmd();
                sendFreeCmd();
            }
        } break;
        
        // response from sendStorageCmd()
        case TBB_CEP_STORAGE_ACK: {
            cmdStatus = handleStorageAck(event);
            if (cmdStatus == SUCCESS) {
                sendFreeCmd();
            }
        } break;
        
        // response from sendFreeCmd()
        case TBB_FREE_ACK: {
            cmdStatus = handleFreeAck(event);
            if (cmdStatus == SUCCESS) {
                sendAllocCmd();
            }
        } break;
        
        // response from sendAllocCmd()
        case TBB_ALLOC_ACK: {
            cmdStatus = handleAllocAck(event);
            if (cmdStatus == SUCCESS) {
                sendRcuInfoCmd();
            }
        } break;
        
        // response from sendRcuInfoCmd()
        case TBB_RCU_INFO_ACK: {
            cmdStatus = handleRcuInfoAck(event);
            if (cmdStatus == SUCCESS) {
                sendTrigSetupCmd();
            }
        } break;
        
        // response from sendTrigSetupCmd()
        case TBB_TRIG_SETUP_ACK: {
            cmdStatus = handleTrigSetupAck(event);
            if (cmdStatus == SUCCESS) {
                sendTrigCoefCmd();
            }
        } break;
        
        // response from sendTrigCoefCmd()
        case TBB_TRIG_COEF_ACK: {
            cmdStatus = handleTrigCoefAck(event);
            if (cmdStatus == SUCCESS) {
                sendRecordCmd(itsObs->allRCUset);
            }
        } break;
        
        // response from setTbbTrigCoef()
        case TBB_RECORD_ACK: {
            cmdStatus = handleRecordAck(event);
            if (cmdStatus == SUCCESS) {
                sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_NO_ERROR);
                setState(CTState::PREPARED);
                TRAN(TBBControl::prepared_state);  // go to prepared state.
            }
            else {
                sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_RECORD_FAILED);
            }
        } break;
        
        
        default: {
            status = _defaultEventHandler(event, port);
        } break;
    }
    if (cmdStatus == FAILURE) {
        setState(CTState::CLAIMED);
        TRAN(TBBControl::claimed_state);  // go to claimed_state state.
    }
    return (status);
}


//------------------------------------------------------------------------------
// prepared_state(event, port)
//
// wait for RESUME event.
//------------------------------------------------------------------------------
GCFEvent::TResult TBBControl::prepared_state(GCFEvent& event, GCFPortInterface& port)
{
    int cmdStatus = SUCCESS;
    LOG_DEBUG_STR ("prepared:" << eventName(event) << "@" << port.getName());

    GCFEvent::TResult status = GCFEvent::HANDLED;
    
    switch (event.signal) {
        case F_ENTRY: {
            // update PVSS
            // itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("claimed"));
            itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
        } break;

        // -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
        case CONTROL_RESUME: {  // nothing to do, just send answer
            CONTROLResumeEvent msg(event);
            LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
            sendControlResult(port, CONTROL_RESUMED, msg.cntlrName, CT_RESULT_NO_ERROR);
            setState(CTState::RESUME);
            sendSubscribeCmd();
        } break;

        // response from sendSubscribeCmd()
        case TBB_SUBSCRIBE_ACK: {
            cmdStatus = handleSubscribeAck(event);
            if (cmdStatus == SUCCESS) {
                sendReleaseCmd(itsObs->allRCUset);
            }
        } break;

        case TBB_TRIG_RELEASE_ACK: {
            if (handleReleaseAck(event) == SUCCESS) {
                setState(CTState::RESUMED);
                TRAN(TBBControl::active_state);
            }
            else {
                setState(CTState::PREPARED);
            }
        } break;
        
        default: {
            status = _defaultEventHandler(event, port);
        } break;
    }

    return (status);
}

//------------------------------------------------------------------------------
// active_state(event, port)
//
// Normal operation state.
//------------------------------------------------------------------------------
GCFEvent::TResult TBBControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
    int cmdStatus = SUCCESS;
    LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

    GCFEvent::TResult status = GCFEvent::HANDLED;
    

    switch (event.signal) {
        case F_ENTRY: {
            // update PVSS
            // itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("active"));
            itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
            // start timer to call VHECR task
            itsVHECRtimer->setTimer(VHECR_INTERVAL);
        } break;

        case F_TIMER: {
            if (&port == itsVHECRtimer) {
                clock_t nexttime = clock()  + (long)(VHECR_INTERVAL * CLOCKS_PER_SEC); 
                vector<TBBReadCmd>  readCommandVector;
                if (itsObs->vhecrTaskEnabled) {
                    itsVHECRTask->getReadCmd(readCommandVector);
                }
                
                if (!readCommandVector.empty()) {
                    readCmdToRequests(readCommandVector);
                    itsStopTimer->setTimer(0.0);
                }
                else {
                    if (clock() < nexttime) {
                        itsVHECRtimer->setTimer((nexttime - clock()) / (double)CLOCKS_PER_SEC);
                    }
                    else {
                        itsVHECRtimer->setTimer(0.0);
                    }
                }
            }
            else if (&port == itsStopTimer) {
                sendStopCmd();
            }
            else if (&port == itsCepTimer) {
                sendCepStatusCmd();
            }
        } break;

        // -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------

        case CONTROL_SCHEDULE: {
            CONTROLScheduledEvent msg(event);
            LOG_DEBUG_STR("Received SCHEDULE(" << msg.cntlrName << ")");
            // TODO: do something usefull with this information!
        } break;

        case CONTROL_SUSPEND: {  // nothing to do, just send answer
            CONTROLSuspendEvent msg(event);
            LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
            sendControlResult(port, CONTROL_SUSPENDED, msg.cntlrName, CT_RESULT_NO_ERROR);
            setState(CTState::SUSPEND);
            itsVHECRtimer->cancelAllTimers();
            sendUnsubscribeCmd();
        } break;
        
        case CONTROL_RESUME: {  // nothing to do, just send answer
            CONTROLResumeEvent msg(event);
            LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
            sendControlResult(port, CONTROL_RESUMED, msg.cntlrName, CT_RESULT_NO_ERROR);
            setState(CTState::RESUMED);
            sendSubscribeCmd();
            itsVHECRtimer->setTimer(VHECR_INTERVAL);
        } break;
        
        // -------------------- EVENTS RECEIVED FROM TBBDRIVER --------------------
    // TODO TBB
        case TBB_TRIGGER:{
            if (itsObs->vhecrTaskEnabled) {
                status = handleTriggerEvent(event);
            }
        } break;

        case TBB_TRIG_RELEASE_ACK: {
            cmdStatus = handleReleaseAck(event);
        } break;
        
        case TBB_STOP_ACK: {
            cmdStatus = handleStopAck(event);
            if (cmdStatus == SUCCESS) {
                sendRcuInfoCmd();
            }
        } break;
        
        case TBB_RECORD_ACK: {
            cmdStatus = handleRecordAck(event);
            if (cmdStatus == SUCCESS) {
                sendRcuInfoCmd();
                itsVHECRtimer->setTimer(VHECR_INTERVAL);
            }
        } break;
        
        case TBB_READ_ACK: {
            cmdStatus = handleReadAck(event);
        } break;
        
        case TBB_RCU_INFO_ACK: {
            cmdStatus = handleRcuInfoAck(event);
            if (cmdStatus == SUCCESS) {
                if (!itsReadRequests.empty()) {
                    itsCepTimer->setTimer(0.0);
                }
            }
        } break;
        
        case TBB_CEP_STATUS_ACK: {
            cmdStatus = handleCepStatusAck(event); // handleCepStatusAck checks active datapaths
            
            int rcuNr;
            int boardNr;
            double readTime = 0;
            
            vector<ReadRequest> requests(itsReadRequests);
            itsReadRequests.clear();
            
            ReadRequest request;
            while (!requests.empty()) {
                request = requests.back();
                requests.pop_back();
                
                rcuNr = request.rcuNr;
                boardNr = itsRcuInfo.at(rcuNr).boardNr;
                
                if ((request.readActive) && // check if rcu is now sending data
                     (itsBoardInfo.at(boardNr).cepActive == false)) // and cep is not active anymore
                {
                    // read done, next request
                    continue;
                }
                
                if ((itsActiveCepDatapaths == itsMaxCepDatapaths) || // check if max number of datapaths reached
                     (itsBoardInfo.at(boardNr).cepActive) || // check if CEP port is already active
                     (itsRcuInfo.at(rcuNr).rcuState != 'S')) // chech if rcu is stopped
                {
                    // new read not possible now
                    itsReadRequests.push_back(request); // add request back to itsReadRequests
                    continue;
                }
                
                // send new read request
                if (readTime == 0) {
                    readTime = (double)(request.timeBefore + request.timeAfter) * 4.0;
                }
                request.readActive = true;
                sendReadCmd(request);
                itsReadRequests.push_back(request); // add request back to itsReadRequests
                itsActiveCepDatapaths++;
                itsBoardInfo.at(boardNr).cepActive = true;
            }
            
            if (itsActiveCepDatapaths > 0) { // dumping not ready
                if (readTime == 0) {
                    readTime = 0.01;
                }
                itsCepTimer->setTimer(readTime);
            }
            else {
                if (itsAutoRecord) {
                    // all dumping done, set rcu's in recording state
                    usleep(500); // if pages left == 0, TBB can be busy sending last page
                    sendRecordCmd(itsObs->allRCUset);
                }
            }
        } break;
        
        case TBB_CEP_DELAY_ACK: {
            handleCepDelayAck(event);
        } break;
        
        case TBB_STOP_CEP_ACK: {
            handleStopCepAck(event);
        } break;
                
        // response from sendSubscribeCmd()
        case TBB_SUBSCRIBE_ACK: {
            handleSubscribeAck(event);
        } break;
        
        case TBB_UNSUBSCRIBE_ACK: {
            handleUnsubscribeAck(event);
            //TRAN(TBBControl::released_state);
        } break;
            
        
        // ------------ EVENTS RECEIVED FROM RTDB external trigger port -----------
        case CR_STOP:{
            // stop recording for selected rcu's on given time
            CRStopEvent e(event);
            CRStopAckEvent ack;
            
            ack.triggerID = e.triggerID;
            ack.result = CR_NO_ERR;
            
            StopRequest stopRequest;
            vector<CRstopRequest>::iterator iter;
            for (iter = e.stopVector.requests.begin(); iter != e.stopVector.requests.end(); iter++) {
                // check if this station is in stationlist, if empty list select all
                string station_list(toUpper((*iter).stationList));
                // empty station_list looks like "[]"
                if ((station_list.length() == 2) || (station_list.find(PVSSDatabaseName(""), 0) != string::npos)) {
                    LOG_INFO_STR("CR_Trig. Stop recording");
                    stopRequest.rcuSet = strToBitset((*iter).rcuList);
                    stopRequest.stopTime = (*iter).stopTime;
                    
                    itsStopRequests.push_back(stopRequest);
                    LOG_INFO_STR("CR_Trig. rcuSet=" << stopRequest.rcuSet);
                    LOG_INFO_STR("CR_Trig. stopTime=" << stopRequest.stopTime);
                }
            }
            
            if (!itsStopRequests.empty()) {
                itsVHECRtimer->cancelAllTimers();
                itsStopTimer->setTimer(0.0);
            }
            
            // send ack to triggerbox
            itsTriggerPort->send(ack);
        } break;
        
        case CR_READ:{
            itsAutoRecord = false;
            // send read commamd for given rcu's
            CRReadEvent e(event);
            CRReadAckEvent ack;

            ReadRequest readRequest;
            RCUset_t rcuSet;
            rcuSet.reset();

            ack.triggerID = e.triggerID;
            ack.result = CR_NO_ERR;
            
            vector<CRreadRequest>::iterator iter;
            for (iter = e.readVector.requests.begin(); iter != e.readVector.requests.end(); iter++) {
                // check if this station is in stationlist
                string station_list(toUpper((*iter).stationList));
                // empty station_list looks like "[]"
                if ((station_list.length() == 2) || (station_list.find(PVSSDatabaseName(""), 0) != string::npos)) {
                    LOG_INFO_STR("CR_Trig. Start dumping CEP data");
                    readRequest.readTime = (*iter).readTime;
                    readRequest.timeBefore = (*iter).timeBefore;
                    readRequest.timeAfter = (*iter).timeAfter;
                    
                    rcuSet = strToBitset((*iter).rcuList);
                    LOG_INFO_STR("CR_Trig. rcuSet=" << rcuSet);
                    for (int rcu = 0; rcu < itsNrRCUs; rcu++) {
                        if (rcuSet.test(rcu) == true) {
                            readRequest.rcuNr = rcu;
                            itsReadRequests.push_back(readRequest);
                        }
                    }
                }
            }
            
            if (!itsReadRequests.empty()) {
                sendRcuInfoCmd();
            }
            
            // send acknowledge to triggerbox
            itsTriggerPort->send(ack);
        } break;
        
        case CR_RECORD:{
            
            CRRecordEvent e(event);
            CRRecordAckEvent ack;
            RCUset_t rcuSet;
            rcuSet.reset();
            
            ack.triggerID = e.triggerID;
            ack.result = CR_NO_ERR;
            
            
            vector<CRrecordRequest>::iterator iter;
            for (iter = e.recordVector.requests.begin(); iter != e.recordVector.requests.end(); iter++) {
                // check if this station is in stationlist
                string station_list(toUpper((*iter).stationList));
                // empty station_list looks like "[]"
                if ((station_list.length() == 2) || (station_list.find(PVSSDatabaseName(""), 0) != string::npos)) {
                    rcuSet |= strToBitset((*iter).rcuList);
                }
            }
            
            if (rcuSet.count() > 0) {
                // turn on recording, check first if dumping is done
                if (itsReadRequests.empty()) { 
                    LOG_INFO_STR("CR_Trig. Start recording");
                    LOG_INFO_STR("CR_Trig. rcuSet=" << rcuSet);
                    sendRecordCmd(rcuSet);
                }
                else {
                    // busy dumping, start recording when ready 
                    LOG_INFO_STR("CR_Trig. Auto recording turned on");
                    itsAutoRecord = true;
                }
            }
            itsTriggerPort->send(ack);
        } break;
        
        case CR_CEP_SPEED:{
            CRCepSpeedEvent e(event);
            CRCepSpeedAckEvent ack;

            ack.triggerID = e.triggerID;
            ack.result = CR_NO_ERR;
            
            string station_list(toUpper(e.stationList));
            // empty station_list looks like "[]"
            if ((station_list.length() == 2) || (station_list.find(PVSSDatabaseName(""), 0) != string::npos)) {
                itsCepDelay = e.cepDelay;
                itsMaxCepDatapaths = e.cepDatapaths;
                LOG_INFO_STR(formatString("CR_Trig. CEP speed changed: delay= %d nsec, number of datapaths= %d", itsCepDelay*5, itsMaxCepDatapaths));
                sendCepDelayCmd();
            }
            
            itsTriggerPort->send(ack);
        } break;
        
        case CR_STOP_DUMPS:{
            CRStopDumpsEvent e(event);
            CRStopDumpsAckEvent ack;

            ack.triggerID = e.triggerID;
            ack.result = CR_NO_ERR;
            
            string station_list(toUpper(e.stationList));
            // empty station_list looks like "[]"
            if ((station_list.length() == 2) || (station_list.find(PVSSDatabaseName(""), 0) != string::npos)) {
                LOG_INFO_STR("CR_Trig. Stop dumping CEP data");
                // stop dumping cep data, first clear all pending requests
                itsReadRequests.clear();
                sendStopCepCmd();
            }
            
            itsTriggerPort->send(ack);
        } break;
        
        case CR_VHECR_STATE:{
            CRVhecrStateEvent e(event);
            CRVhecrStateAckEvent ack;

            ack.triggerID = e.triggerID;
            ack.result = CR_NO_ERR;
            
            string station_list(toUpper(e.stationList));
            // empty station_list looks like "[]"
            if ((station_list.length() == 2) || (station_list.find(PVSSDatabaseName(""), 0) != string::npos)) {
                LOG_INFO_STR(formatString("CR_Trig. VHECR state=%u", e.state));
                if (e.state == 1) {
                    itsObs->vhecrTaskEnabled = true;
                }
                else {
                    itsObs->vhecrTaskEnabled = false;
                }
            }
            
            itsTriggerPort->send(ack);
        } break;
        
        
        default: {
            status = _defaultEventHandler(event, port);
        } break;
    }
    
    /*
    if (!itsReadRequests.empty()) {
        sendCepStatusCmd();
    }
    */
        
    return (status);
}


//------------------------------------------------------------------------------
// released_state(event, port)
//
// wait for PREPARE event.
//------------------------------------------------------------------------------
GCFEvent::TResult TBBControl::released_state(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR ("released:" << eventName(event) << "@" << port.getName());

    GCFEvent::TResult status = GCFEvent::HANDLED;

    switch (event.signal) {
        case F_ENTRY: {
            // update PVSS
            // itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("claimed"));
            itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
        } break;

        // -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------

        default: {
            status = _defaultEventHandler(event, port);
        } break;
    }
    return (status);
}


//------------------------------------------------------------------------------
// quiting_state(event, port)
//
// Quiting: send QUITED, wait for answer max 5 seconds, stop
//------------------------------------------------------------------------------
GCFEvent::TResult TBBControl::quiting_state(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR ("quiting:" << eventName(event) << "@" << port.getName());

    GCFEvent::TResult status = GCFEvent::HANDLED;

    switch (event.signal) {
        case F_ENTRY: {
            // update PVSS
            setState(CTState::QUIT);
            // tell Parent task we like to go down.
            itsParentControl->nowInState(getName(), CTState::QUIT);

            // itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("quiting"));
            itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
            // disconnect from TBBDriver
            itsTBBDriver->close();
            LOG_INFO("Connection with TBBDriver down, sending QUITED to parent");
            if (itsParentPort->isConnected()) {
                CONTROLQuitedEvent request;
                request.cntlrName = getName();
                request.result    = CT_RESULT_NO_ERROR;
                itsParentPort->send(request);
                itsTimerPort->setTimer(1.0);  // wait 1 second to let message go away
            }
            else {
                GCFScheduler::instance()->stop();
            }
        } break;

        case F_DISCONNECTED: {      
            LOG_DEBUG_STR("F_DISCONNECTED event from port " << port.getName());
        } break;

        case F_TIMER: {
            GCFScheduler::instance()->stop();
        } break;

        default: {
            status = _defaultEventHandler(event, port);
        } break;
    }

    return (status);
}


/*
o- RUN FUNCTIONS --------------------------------------------------------------o
|                                                                              |
|  functions used in active_state                                              |
o------------------------------------------------------------------------------o
*/

//------------------------------------------------------------------------------
// handleTriggerEvent(event)
//------------------------------------------------------------------------------
GCFEvent::TResult TBBControl::handleTriggerEvent(GCFEvent& event)
{
    GCFEvent::TResult   result(GCFEvent::NOT_HANDLED);

    // if not in active_state, return.
    if (itsState != CTState::RESUMED) return (result);

    TBBTriggerEvent msg(event);
    uint32 time = msg.nstimestamp.sec();
    uint32 sample_nr = (uint32)(msg.nstimestamp.nsec() * itsObs->NSEC2SAMPLE);

    TBBTrigger trigger( static_cast<uint32>(msg.rcu),
                                            time,
                                            sample_nr,
                                            msg.nstimestamp,
                                            msg.trigger_sum,
                                            msg.trigger_samples,
                                            msg.peak_value,
                                            msg.missed);
    itsVHECRTask->addTrigger(trigger);
    LOG_DEBUG_STR("Sending Trigger to VHECR task:" << endl << trigger);
    /*
    LOG_TRACE_FLOW ("Sending trigger to PVSS");

    itsPropertySet->setValue(PN_TBC_TRIGGER_RCU_NR,      GCFPVInteger(msg.rcu),            0.0, false);
    itsPropertySet->setValue(PN_TBC_TRIGGER_SEQUENCE_NR, GCFPVInteger(msg.sequence_nr),    0.0, false);
    itsPropertySet->setValue(PN_TBC_TRIGGER_TIME,        GCFPVInteger(msg.time),           0.0, false);
    itsPropertySet->setValue(PN_TBC_TRIGGER_SAMPLE_NR,   GCFPVInteger(msg.sample_nr),      0.0, false);
    itsPropertySet->setValue(PN_TBC_TRIGGER_SUM,         GCFPVInteger(msg.trigger_sum),    0.0, false);
    itsPropertySet->setValue(PN_TBC_TRIGGER_NR_SAMPLES,  GCFPVInteger(msg.trigger_samples),0.0, false);
    itsPropertySet->setValue(PN_TBC_TRIGGER_PEAK_VALUE,  GCFPVInteger(msg.peak_value),     0.0, false);
    // The Navigator also needs all values combined into one field
    string collection(formatString("%d|%d|%d|%d|%d|%d|%d|0", msg.rcu, msg.sequence_nr, msg.time,
                            msg.sample_nr, msg.trigger_sum, msg.trigger_samples, msg.peak_value));
    itsPropertySet->setValue(PN_TBC_TRIGGER_TABLE,       GCFPVString (collection),         0.0, false);
    itsPropertySet->flush();
    */
    
    // release trigger system if mode is one shot (bit0 = 0)
    if((itsRcuInfo.at(msg.rcu).triggerMode & 1) == 0) {
        RCUset_t RCUset;
        RCUset.reset();
        RCUset.set(msg.rcu);
        sendReleaseCmd(RCUset);
    }
    return (result);
}

//------------------------------------------------------------------------------
// sendRecordCmd()
//
// send TBB_RECORD cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendRecordCmd(RCUset_t RCUset)
{
    itsAutoRecord = true;
    TBBRecordEvent cmd;
    cmd.rcu_mask = RCUset;
    LOG_DEBUG_STR("send TBB_RECORD cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleRecordAck(event)
//
// handle TBB_RECORD_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleRecordAck(GCFEvent& event)
{
    TBBRecordAckEvent ack(event);
    bool status_ok = true;

    for (int b = 0; b < itsNrTBBs; b++) {
        if (isBoardUsed(b) == false) { continue; }
        if (ack.status_mask[b] != TBB_SUCCESS) {
            status_ok = false;  // error
            LOG_DEBUG_STR (formatString("returned status TBB_RECORD_ACK, board-%d status=%u", b, ack.status_mask[b])); 
        }
    }

    if (status_ok) {
        return (SUCCESS);
    }
    LOG_ERROR_STR ("Failed to start recording for the selected rcus");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("record error"));
    return (FAILURE);
}

//------------------------------------------------------------------------------
// sendReleaseCmd()
//
// send TBB_TRIG_RELEASE cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendReleaseCmd(RCUset_t RCUset)
{
    TBBTrigReleaseEvent cmd;

    cmd.rcu_stop_mask = RCUset;
    cmd.rcu_start_mask = RCUset;

    LOG_DEBUG_STR("send TBB_RELEASE cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleReleaseAck(event)
//
// handle RSP_TBB_RELEASE_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleReleaseAck(GCFEvent& event)
{

    TBBTrigReleaseAckEvent ack(event);
    bool status_ok = true;

    for (int b = 0; b < itsNrTBBs; b++) {
        if (isBoardUsed(b) == false) { continue; }
        if (ack.status_mask[b] != TBB_SUCCESS) {
            status_ok = false;  // error
            LOG_DEBUG_STR (formatString("returned status TBB_TRIG_RELEASE_ACK, board-%d status=%u", b, ack.status_mask[b])); 
        }
    }

    if (status_ok) {
        return (SUCCESS);
    }

    LOG_ERROR_STR ("Failed to release the trigger system for the selected rcus");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("release error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_RELEASE_FAILED);
    return (FAILURE);
}


//------------------------------------------------------------------------------
// sendStopCmd()
//
// send TBB_STOP cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendStopCmd()
{
    TBBStopEvent cmd;
    // get time now in usec
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double timeNow = tv.tv_sec + (tv.tv_usec / 1000000.);
    double nextStop = timeNow + 3600.0;
    bool sendStop = false;
    
    vector<StopRequest> requests(itsStopRequests);
    itsStopRequests.clear();
    
    StopRequest request;
    while (!requests.empty()) {
        request = requests.back();
        requests.pop_back();
        LOG_INFO_STR(formatString("request stop time = %lf, timenow = %lf" , 
                     (double)request.stopTime, timeNow ));
        
        cmd.rcu_mask.reset();
        for (int i = 0; i < itsNrRCUs; i++) {
            if (request.rcuSet.test(i)) {
                if ((double)request.stopTime == 0.) {
                    cmd.rcu_mask.set(i);
                    sendStop = true;
                }
                else if ((double)request.stopTime <= timeNow) { 
                    if ((double)request.stopTime > (timeNow - itsRcuInfo.at(i).bufferTime)) { 
                        cmd.rcu_mask.set(i);
                        sendStop = true;
                    }
                }
            }
        }
        
        if (cmd.rcu_mask.count() > 0) {
            request.rcuSet.reset();
        }

        if (request.rcuSet.count() > 0) {
            if ((double)request.stopTime < nextStop) {
                nextStop = (double)request.stopTime;
            }
            itsStopRequests.push_back(request);
        }

        if (sendStop) {
            LOG_INFO_STR("send TBB_STOP cmd");
            LOG_INFO_STR("rcuSet=" << cmd.rcu_mask);
            itsTBBDriver->send(cmd);
        }
    }
        
    if (!itsStopRequests.empty()) {
        itsStopTimer->setTimer(nextStop - timeNow);
    }
}

//------------------------------------------------------------------------------
// handleStopAck(event)
//
// handle TBB_STOP_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleStopAck(GCFEvent& event)
{
    TBBStopAckEvent ack(event);
    bool status_ok = true;

    for (int b = 0; b < itsNrTBBs; b++) {
        if (isBoardUsed(b) == false) { continue; }
        if (ack.status_mask[b] != TBB_SUCCESS) {  // if error, check if rcu is used
            status_ok = false;
            LOG_DEBUG_STR(formatString("returned status TBB_STOP_ACK, board=%d", b));
        }
    }

    if (status_ok) {
        return (SUCCESS);
    }
    
    LOG_ERROR_STR ("Failed to stop recording for selected rcus");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("stop error"));
    return (FAILURE);
}

//------------------------------------------------------------------------------
// sendCepStatusCmd()
//
// send TBB_CEP_STATUS cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendCepStatusCmd()
{
    TBBCepStatusEvent cmd;
    for (int b = 0; b < itsNrTBBs; b++) {
        cmd.boardmask += (1 << b);
    }
    LOG_DEBUG_STR("send TBB_CEP_STATUS cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleCepStatusAck(event)
//
// handle TBB_CEP_STATUS_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleCepStatusAck(GCFEvent& event)
{
    TBBCepStatusAckEvent ack(event);
    bool status_ok = true;
    itsActiveCepDatapaths = 0;
    
    for (int b = 0; b < itsNrTBBs; b++) {
        if (isBoardUsed(b) == false) { continue; }
        if (ack.status_mask[b] != TBB_SUCCESS) {
            itsBoardInfo.at(b).cepActive = false;
            status_ok = false;  // error
            LOG_DEBUG_STR (formatString("returned status TBB_CEP_STATUS_ACK, board-%d status=%u", b, ack.status_mask[b])); 
        }
        else {
            // check if boards CEP port is busy
            if (ack.pages_left[b] > 0) {
                itsBoardInfo.at(b).cepActive = true;
                itsActiveCepDatapaths++; 
            }
            else {
                itsBoardInfo.at(b).cepActive = false; 
            }
        }
    }

    if (status_ok) {
        return (SUCCESS);
    }
    LOG_ERROR_STR ("Failed to get cep status");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("info error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_CEPSTATUS_FAILED);
    return (FAILURE);
}

//------------------------------------------------------------------------------
// sendCepDelayCmd()
//
// send TBB_CEP_DELAY cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendCepDelayCmd()
{
    TBBCepDelayEvent cmd;
    for (int b = 0; b < itsNrTBBs; b++) {
        cmd.boardmask += (1 << b);
    }
    cmd.delay = itsCepDelay;
    LOG_DEBUG_STR("send TBB_CEP_DELAY cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleCepDelayAck(event)
//
// handle TBB_CEP_DELAY_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleCepDelayAck(GCFEvent& event)
{
    TBBCepDelayAckEvent ack(event);
    bool status_ok = true;
    
    for (int b = 0; b < itsNrTBBs; b++) {
        if (ack.status_mask[b] != TBB_SUCCESS) {
            status_ok = false;  // error
            LOG_DEBUG_STR (formatString("returned status TBB_CEP_DELAY_ACK, board-%d status=%u", b, ack.status_mask[b])); 
        }
    }

    if (status_ok) {
        return (SUCCESS);
    }
    LOG_ERROR_STR ("Failed to get cep status");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("info error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_CEPSTATUS_FAILED);
    return (FAILURE);
}



//------------------------------------------------------------------------------
// sendStopCepCmd()
//
// send TBB_STOP_CEP cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendStopCepCmd()
{
    TBBStopCepEvent cmd;
    for (int b = 0; b < itsNrTBBs; b++) {
        cmd.boardmask += (1 << b);
    }
    LOG_DEBUG_STR("send TBB_STOP_CEP cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleCepStatusAck(event)
//
// handle TBB_CEP_STATUS_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleStopCepAck(GCFEvent& event)
{
    TBBStopCepAckEvent ack(event);
    bool status_ok = true;
    itsActiveCepDatapaths = 0;
    
    for (int b = 0; b < itsNrTBBs; b++) {
        if (ack.status_mask[b] != TBB_SUCCESS) {
            itsBoardInfo.at(b).cepActive = false;
            status_ok = false;  // error
            LOG_DEBUG_STR (formatString("returned status TBB_STOP_CEP_ACK, board-%d status=%u", b, ack.status_mask[b])); 
        }
        itsBoardInfo.at(b).cepActive = false; 
    }

    if (status_ok) {
        return (SUCCESS);
    }
    LOG_ERROR_STR ("Failed to get cep status");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("info error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_CEPSTATUS_FAILED);
    return (FAILURE);
}




//------------------------------------------------------------------------------
// readTBBdata is called bij VHECR Task
// NOT USED ANYMORE
//------------------------------------------------------------------------------
void TBBControl::readTBBdata(vector<TBBReadCmd> /*readCmd*/)
{
    LOG_DEBUG_STR("Received read cmd from VHECRTask");
    //itsCommandVector.clear();
    //itsStopCommandVector = readCmd;
    //itsTimerPort->setTimer(0.0);
}


//------------------------------------------------------------------------------
// sendReadCmd(int vectorNrToRead)
//
// send TBB_READ cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendReadCmd(ReadRequest read)
{
    TBBReadEvent cmd;
        
    LOG_INFO_STR(formatString("send TBB_READ cmd: rcu=%d, time=%lf, before=%lf, after=%lf",
                    read.rcuNr, (double)read.readTime, (double)read.timeBefore, (double)read.timeAfter));
        
    cmd.rcu          = read.rcuNr;
    cmd.nstimestamp  = read.readTime;
    cmd.nstimebefore = read.timeBefore;
    cmd.nstimeafter  = read.timeAfter;
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleReadAck(event)
//
// handle TBB_READ_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleReadAck(GCFEvent& event)
{
    TBBReadAckEvent ack(event);
    if (ack.status_mask == TBB_SUCCESS) {
        return (SUCCESS);
    }
    return (FAILURE);
}

//------------------------------------------------------------------------------
// readCmdToRequests(vector<TBBReadCmd> read)
//
// convert read command from VHECR Task to stop and read requests
//------------------------------------------------------------------------------
void TBBControl::readCmdToRequests(vector<TBBReadCmd> read)
{
    NsTimestamp nstimestamp;
    double samplesInFrame;
    StopRequest stoprequest;
    ReadRequest readrequest;
    
    vector<TBBReadCmd>::iterator iter;
    for (iter = read.begin(); iter != read.end(); iter++) {
        stoprequest.rcuSet.set((*iter).itsRcuNr);
        
        readrequest.rcuNr = (*iter).itsRcuNr;
        samplesInFrame = 1024.;
        if (itsRcuInfo.at((*iter).itsRcuNr).operatingMode == TBB_MODE_SUBBANDS) {
            samplesInFrame = 487.;
        }
        nstimestamp.set((double)(*iter).itsTime + ((*iter).itsSampleNr * itsObs->sampleTime));
        readrequest.readTime = nstimestamp;
        
        nstimestamp.set((double)(*iter).itsPrePages * samplesInFrame * itsObs->sampleTime);
        readrequest.timeBefore = nstimestamp;
        
        nstimestamp.set((double)(*iter).itsPostPages * samplesInFrame * itsObs->sampleTime);
        readrequest.timeAfter = nstimestamp;
        itsReadRequests.push_back(readrequest);
    }
    
    stoprequest.stopTime.set(0.0);  // stop now
    itsStopRequests.push_back(stoprequest);
                
}


/*
o-BOARD SETUP FUNCTIONS--------------------------------------------------------o
|                                                                              |
| functions used to setup the boards                                           |
o------------------------------------------------------------------------------o
*/

//------------------------------------------------------------------------------
// sendRspMode()
//
// send RSP_TBB_MODE cmd to the RSPDriver
//------------------------------------------------------------------------------
void TBBControl::sendRspModeCmd()
{
    RSPSettbbEvent settbb;

    settbb.timestamp = Timestamp(0,0);
    settbb.rcumask.reset();
    settbb.settings().resize(1);
    settbb.settings()(0).reset();

    vector<TBBObservation::cSettings>::iterator it1;
    for (it1 = itsObs->TbbSettings.begin(); it1 != itsObs->TbbSettings.end(); it1++ ) {
        settbb.rcumask |= (*it1).RCUset;
        if ((*it1).operatingMode == TBB_MODE_SUBBANDS) {
            std::vector<int32>::iterator it2;
            for (it2 = (*it1).SubbandList.begin(); it2 != (*it1).SubbandList.end(); it2++) {
                if ((*it2) >= MAX_SUBBANDS) continue;
                settbb.settings()(0).set(*it2);
            }
        }
    }
    LOG_DEBUG_STR("send RSP_SET_TBB cmd");
    itsRSPDriver->send(settbb);
}

//------------------------------------------------------------------------------
// handleRspModeAck(event)
//
// handle RSP_TBB_MODE_ACK cmd from the RSPDriver
//------------------------------------------------------------------------------
int TBBControl::handleRspModeAck(GCFEvent& event)
{
    RSPSettbbackEvent ack(event);

    if (ack.status == RSP_SUCCESS) {
        return(SUCCESS);
    }
    LOG_DEBUG_STR ("returned status RSP_TBB_MODE_ACK " << ack.status);
    LOG_ERROR_STR ("Failed to set the operating mode for all the rcus");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("operatingMode setup error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_MODESETUP_FAILED);
    return (FAILURE);
}

//------------------------------------------------------------------------------
// sendStorageCmd()
//
// send TBB_STORAGE cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendStorageCmd()
{
    static RCUset_t RCUset;

    TBBCepStorageEvent cmd;
    RCUset.reset();

    //cmd.destination = itsObs->TbbSettings.destination;
    LOG_DEBUG_STR("send TBB_CEP_STORAGE cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleStorageAck(event)
//
// handle TBB_STORAGE_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleStorageAck(GCFEvent& event)
{
    TBBCepStorageAckEvent ack(event);
    bool status_ok = true;

    for (int b = 0; b < itsNrTBBs; b++) {
        if (isBoardUsed(b) == false) { continue; }
        if (ack.status_mask[b] != TBB_SUCCESS) {
            status_ok = false;  // error
            LOG_DEBUG_STR (formatString("returned status TBB_STORAGE_ACK board-%d status=%u", b, ack.status_mask[b])); 
        }
    }

    if (status_ok) {
        return(SUCCESS);
    }
    LOG_ERROR_STR ("Failed to set storage nodes");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("storage setup error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_STORAGESETUP_FAILED);
    return (FAILURE);
}

//------------------------------------------------------------------------------
// sendFreeCmd()
//
// send TBB_FREE cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendFreeCmd()
{
    TBBFreeEvent cmd;

    cmd.rcu_mask.set(); // select all RCUs
    LOG_DEBUG_STR("send TBB_FREE cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleFreeAck(event)
//
// handle TBB_FREE_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleFreeAck(GCFEvent& event)
{
    TBBFreeAckEvent ack(event);
    bool status_ok = true;

    for (int b = 0; b < itsNrTBBs; b++) {
        if (isBoardUsed(b) == false) { continue; }
        if (ack.status_mask[b] != TBB_SUCCESS) {  // if error, check if rcu is used
            status_ok = false;
            LOG_DEBUG_STR (formatString("returned status TBB_FREE_ACK, board-%d status=0x%x", b, ack.status_mask[b])); 
        }
    }

    if (status_ok) {
        return(SUCCESS);
    }
    LOG_ERROR_STR ("Failed to free the memmory for all the rcus");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("free error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_FREE_FAILED);
    return (FAILURE);
}

//------------------------------------------------------------------------------
// sendAllocCmd()
//
// send TBB_ALLOC cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendAllocCmd()
{
    TBBAllocEvent cmd;
    cmd.rcu_mask = itsObs->allRCUset;
    LOG_DEBUG_STR("send TBB_ALLOC cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleAllocAck(event)
//
// handle TBB_ALLOC_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleAllocAck(GCFEvent& event)
{
    TBBAllocAckEvent ack(event);
    bool status_ok = true;

    for (int b = 0; b < itsNrTBBs; b++) {
        if (isBoardUsed(b) == false) { continue; }
        if (ack.status_mask[b] != TBB_SUCCESS) {
            status_ok = false;  // error
            LOG_DEBUG_STR (formatString("returned status TBB_ALLOC_ACK, board-%d status=%u", b, ack.status_mask[b])); 
        }
    }

    if (status_ok) {
        return (SUCCESS);
    }
    LOG_ERROR_STR ("Failed to allocate the memory for the selected rcus");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("alloc error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_ALLOC_FAILED);
    return (FAILURE);
}

//------------------------------------------------------------------------------
// sendRcuInfoCmd()
//
// send TBB_RCU_INFO cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendRcuInfoCmd()
{
    TBBRcuInfoEvent cmd;
    LOG_DEBUG_STR("send TBB_RCU_INFO cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleRcuInfoAck(event)
//
// handle TBB_RCU_INFO_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleRcuInfoAck(GCFEvent& event)
{
    TBBRcuInfoAckEvent ack(event);
    bool status_ok = true;

    if (ack.status_mask != TBB_SUCCESS) {
        status_ok = false;  // error
        LOG_DEBUG_STR (formatString("returned status TBB_RCU_INFO_ACK, status=%u", ack.status_mask)); 
    }

    if (status_ok) {
        // get buffertime
        for (int i = 0; i < itsNrRCUs; i++) {
            if (itsRcuInfo.at(i).rcuNr == i) {
                itsRcuInfo.at(i).boardNr = ack.rcu_on_board[i];
                itsRcuInfo.at(i).bufferTime = (double)ack.rcu_pages[i] * 1024 * itsObs->sampleTime;
                itsRcuInfo.at(i).rcuState = ack.rcu_state[i];
            }
            else {
                LOG_ERROR_STR ("No match for rcuNr and itsRcuInfoNr");
            }
        }
        return (SUCCESS);
    }
    LOG_ERROR_STR ("Failed to receive rcu info for selected rcus");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("rcu info error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_ALLOC_FAILED);
    return (FAILURE);
}
    
//------------------------------------------------------------------------------
// sendTrigSetupCmd()
//
// send TBB_TRIG_SETUP cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendTrigSetupCmd()
{
    static RCUset_t RCUset;

    TBBTrigSetupEvent cmd;
    RCUset.reset();
    vector<TBBObservation::cSettings>::iterator it;
    for (it = itsObs->TbbSettings.begin(); it != itsObs->TbbSettings.end(); it++ ) {
        for (int i = 0; i < itsNrRCUs; i++) {
            if ((*it).RCUset.test(i)) {
                RCUset.set(i);
                cmd.rcu[i].level         = (*it).triggerLevel;
                cmd.rcu[i].start_mode    = static_cast<uint8>((*it).startLevel);
                cmd.rcu[i].stop_mode     = static_cast<uint8>((*it).stopLevel);
                cmd.rcu[i].filter_select = static_cast<uint8>((*it).filter);
                cmd.rcu[i].window        = static_cast<uint8>((*it).detectWindow);
                cmd.rcu[i].trigger_mode  = static_cast<uint8>((*it).triggerMode);
                cmd.rcu[i].operating_mode= static_cast<uint8>((*it).operatingMode);
                itsRcuInfo.at(i).triggerMode = (*it).triggerMode;
            }
        }
    }
    LOG_DEBUG_STR("send TBB_TRIG_SETUP cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleTrigSetupAck(event)
//
// handle TBB_TRIG_SETUP_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleTrigSetupAck(GCFEvent& event)
{
    TBBTrigSetupAckEvent ack(event);
    bool status_ok = true;

    for (int b = 0; b < itsNrTBBs; b++) {
        if (isBoardUsed(b) == false) { continue; }
        if (ack.status_mask[b] != TBB_SUCCESS) {
            status_ok = false;  // error
            LOG_DEBUG_STR (formatString("returned status TBB_TRIG_SETUP_ACK, board-%d status=%u", b, ack.status_mask[b])); 
        }
    }

    if (status_ok) {
        return (SUCCESS);
    }
    LOG_ERROR_STR ("Failed to setup the trigger system for the selected rcus");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("setup error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_TRIGSETUP_FAILED);
    return (FAILURE);
}

//------------------------------------------------------------------------------
// sendTrigCoefCmd()
//
// send TBB_TRIG_COEF cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendTrigCoefCmd()
{
    static RCUset_t RCUset;
    TBBTrigCoefEvent cmd;
    RCUset.reset();
    vector<TBBObservation::cSettings>::iterator it;
    for (it = itsObs->TbbSettings.begin(); it != itsObs->TbbSettings.end(); it++ ) {
        for (int i = 0; i < itsNrRCUs; i++) {
            if ((*it).RCUset.test(i)) {
                RCUset.set(i);
                // each filter has 4 coeffiecients
                for (int c = 0; c < 4; c++) {
                    cmd.rcu[i].filter0[c] = (*it).filter0[c];
                    cmd.rcu[i].filter1[c] = (*it).filter1[c];
                }
            }
        }
    }
    LOG_DEBUG_STR("send TBB_TRIG_COEF cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleTrigCoefAck(event)
//
// handle TBB_TRIG_COEF_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleTrigCoefAck(GCFEvent& event)
{
    TBBTrigCoefAckEvent ack(event);
    bool status_ok = true;

    for (int b = 0; b < itsNrTBBs; b++) {
        if (isBoardUsed(b) == false) { continue; }
        if (ack.status_mask[b] != TBB_SUCCESS) {
            status_ok = false;  // error
            LOG_DEBUG_STR (formatString("returned status TBB_TRIG_ACK, board-%d status=%u", b, ack.status_mask[b])); 
        }
    }

    if (status_ok) {
        return (SUCCESS);
    }
    LOG_ERROR_STR ("Failed to setup the trigger coefficients for the selected rcus");
    itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("setup error"));
    sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_TRIGSETUP_FAILED);
    return (FAILURE);
}

//------------------------------------------------------------------------------
// sendSubscribeCmd()
//
// send TBB_SUBSCRIBE cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendSubscribeCmd()
{
    TBBSubscribeEvent cmd;
    cmd.triggers = true;
    cmd.hardware = false;
    LOG_DEBUG_STR("send TBB_SUBSCRIBE cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleSubscribeAck(event)
//
// handle TBB_SUBSCRIBE_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleSubscribeAck(GCFEvent& /*event*/)
{
    //sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_NO_ERROR);
    return (SUCCESS);
}

//------------------------------------------------------------------------------
// sendUnsubscribeCmd()
//
// send TBB_UNSUBSCRIBE cmd to the TBBDriver
//------------------------------------------------------------------------------
void TBBControl::sendUnsubscribeCmd()
{
    TBBUnsubscribeEvent cmd;
    LOG_DEBUG_STR("send TBB_UNSUBSCRIBE cmd");
    itsTBBDriver->send(cmd);
}

//------------------------------------------------------------------------------
// handleSubscribeAck(event)
//
// handle RSP_TBB_SUBSCRIBE_ACK cmd from the TBBDriver
//------------------------------------------------------------------------------
int TBBControl::handleUnsubscribeAck(GCFEvent& /*event*/)
{
    //sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_NO_ERROR);
    return (SUCCESS);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// _defaultEventHandler(event, port)
//
GCFEvent::TResult TBBControl::_defaultEventHandler(GCFEvent&            event,
                                                    GCFPortInterface&   port)
{
    CTState     cts;
    LOG_DEBUG_STR("Received " << eventName(event) << " in state " << cts.name(itsState)
                  << ". DEFAULT handling.");

    GCFEvent::TResult   result(GCFEvent::HANDLED);

    switch (event.signal) {

        case CONTROL_CONNECT:
        case CONTROL_RESYNC:
        case CONTROL_SCHEDULE:  // TODO: we should do something with this
        case CONTROL_CLAIM:
        case CONTROL_PREPARE:
        case CONTROL_RESUME:
        case CONTROL_SUSPEND: {
            if (!sendControlResult(port, event.signal, getName(), CT_RESULT_NO_ERROR)) {
                result = GCFEvent::NOT_HANDLED;
            }   break;
        }

        case CONTROL_CONNECTED:
        case CONTROL_RESYNCED:
        case CONTROL_SCHEDULED:
        case CONTROL_CLAIMED:
        case CONTROL_PREPARED:
        case CONTROL_RESUMED:
        case CONTROL_SUSPENDED:
        case CONTROL_RELEASED:
        case CONTROL_QUITED: {
        } break;

        case F_DISCONNECTED: {
            if (&port == itsTBBDriver) {
                itsVHECRtimer->cancelAllTimers();
                LOG_DEBUG_STR("Connection with TBBDriver lost, going to started state");
                itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(false));  // [TBB]
                itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("connection lost"));
                TRAN (TBBControl::started_state);
            }
            if (&port == itsParentPort) {
                LOG_WARN_STR("Connection with Parent lost, going to quiting_state");
                TRAN(TBBControl::quiting_state);
            }
            port.close();
        } break;

        case CONTROL_RELEASE: {
            CONTROLReleaseEvent     msg(event);
            LOG_DEBUG_STR("Received RELEASE(" << msg.cntlrName << ")");
            sendControlResult(port, CONTROL_RELEASED, msg.cntlrName, CT_RESULT_NO_ERROR);
            setState(CTState::RELEASE);
            TRAN(TBBControl::released_state);
        } break;

        case CONTROL_QUIT: {
            TRAN(TBBControl::quiting_state);
        } break;

        case DP_CHANGED: {
            LOG_DEBUG_STR("DP " << DPChangedEvent(event).DPname << " was changed");
        } break;

        case DP_SET: {
            LOG_DEBUG_STR("DP " << DPSetEvent(event).DPname << " was set");
        }   break;

        case F_INIT:
        case F_EXIT: {
        }   break;

        default: {
            result = GCFEvent::NOT_HANDLED;
            LOG_WARN_STR("Event " << eventName(event) << " NOT handled in state " << cts.name(itsState));
        }
    }

    return (result);
}

bool TBBControl::isBoardUsed(int board)
{
    int start = board * 16;
    int stop = start + 16;
    for (int i = start; i < stop; i++) {
        if (itsObs->allRCUset.test(i)) { return(true); }
    }
    return(false);
}

//-----------------------------------------------------------------------------
RCUset_t TBBControl::strToBitset(string inputstring)
{
    RCUset_t resultset;
    resultset.reset();
    
    // make inputstring readable
    string input(inputstring);
    LOG_INFO_STR(formatString("selection=%s", input.c_str()));
    input.replace(input.find("["),1,"");
    input.replace(input.find("]"),1,"");
    while (input.find("..") != string::npos) {
        input.replace(input.find(".."),2,":");
    }
    
    // if empty input string select all
    if (input.empty()) {
        resultset.set();
        return(resultset);
    }
    
    //string inputstring(str);
    char* start  = (char*)input.c_str();
    char* end    = 0;
    bool range   = false;
    long prevval = 0;
    
    while(start) {
        long val = strtol(start, &end, 10); // read decimal numbers
        start = (end ? (*end ? end + 1 : 0) : 0); // determine next start
        if (val >= itsNrRCUs || val < 0) {
            // Error: value out of range
            resultset.reset();
            return(resultset);
        }

        if (end) {
            switch (*end) {
                case ',':
                case 0: {
                    if (range) {
                        if (0 == prevval && 0 == val) {
                            val = itsNrRCUs - 1;
                        }
                        if (val < prevval) {
                            // Error: invalid range specification
                            resultset.reset();
                            return(resultset);
                        }
                        for (long i = prevval; i <= val; i++) {
                            resultset.set(i);
                        }
                    } else {
                        resultset.set(val);
                    }
                    range = false;
                } break;

                case ':': {
                    range = true;
                } break;

                default: {
                    cout << formatString("Error: invalid character %c",*end) << endl;
                    resultset.reset();
                    return(resultset);
                } break;
            }
        }
        prevval = val;
    }
    // if empty bitset select all
    if (resultset.none()) {
        resultset.set();
    }
    return(resultset);
}
//
// sigintHandler(signum)
//
void TBBControl::sigintHandler(int signum)
{
    LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

    if (thisTBBControl) {
        thisTBBControl->finish();
    }
}

//
// finish
//
void TBBControl::finish()
{
    TRAN(TBBControl::quiting_state);
}


//
// setState(CTstateNr)
//
void TBBControl::setState(CTState::CTstateNr newState)
{
    itsState = newState;

    if (itsPropertySet) {
        CTState cts;
        itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString(cts.name(newState)));
    }
}

    }  // end off namespace
}  // end off namespace
