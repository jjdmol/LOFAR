//  TriggerControl.cc: Implementation of the TriggerControl task
//
//  Copyright (C) 2006-2008
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  One of the main task of the station controller is the synchronisation between
//  the ClockController, the CalibrationController and the BeamController.
//
//  $Id: TriggerControl.cc 17961 2011-05-04 15:02:32Z overeem $
//
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <Common/ParameterSet.h>
#include <Common/Version.h>
#include <Common/StringUtil.h>
#include <Common/lofar_string.h>
#include <ApplCommon/LofarDirs.h>
#include <ApplCommon/StationInfo.h>
#include <ApplCommon/Observation.h>

#include <MACIO/MACServiceInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <GCF/RTDB/DPservice.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/CR_Protocol/CR_Protocol.ph>
#include <signal.h>
#include <MainCU/Package__Version.h>

#include "TriggerControl.h"
#include "PVSSDatapointDefs.h"


using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::RTDB;
using namespace std;
using namespace LOFAR::StringUtil;

namespace LOFAR {
    using namespace APLCommon;
    namespace MainCU {

// static pointer to this object for signalhandler
static TriggerControl*  thisTriggerControl = 0;

//
// TriggerControl()
//
TriggerControl::TriggerControl(const string&    cntlrName) :
    GCFTask             ((State)&TriggerControl::initial_state,cntlrName),
    itsPropertySet      (0),
    itsDPservice        (0),
    itsSubscriptionID   (0),
    itsPublisher        (0),
    itsListener         (0),
    itsTimerPort        (0)
{
    LOG_TRACE_OBJ_STR (cntlrName << " construction");
    LOG_INFO(Version::getInfo<MainCUVersion>("TriggerControl"));

    // TODO
    LOG_INFO("MACProcessScope: LOFAR.PermSW.TriggerControl");

    // need port for timers.
    itsTimerPort = new GCFTimerPort(*this, "TimerPort");

    // for doing PVSS queries
    itsDPservice = new DPservice(this);
    ASSERTSTR(itsDPservice, "Can't allocate DataPoint Service for PVSS");

    // create listener socket
    itsListener = new GCFTCPPort(*this, MAC_SVCMASK_TRIGGERCTRL, GCFPortInterface::MSPP, CR_PROTOCOL);
    ASSERTSTR(itsListener, "Can't allocated listen-socket for commands");

    // create publish port
    itsPublisher = new GCFRTDBPort(*this, "PublishPort", PSN_CR_TRIGGERPORT);
    ASSERTSTR(itsPublisher, "Can't allocate a RTDB publisher port");

    // for debugging purposes
    registerProtocol (DP_PROTOCOL, DP_PROTOCOL_STRINGS);
    registerProtocol (CR_PROTOCOL, CR_PROTOCOL_STRINGS);
}


//
// ~TriggerControl()
//
TriggerControl::~TriggerControl()
{
    LOG_TRACE_OBJ_STR (getName() << " destruction");

    if (itsDPservice) { delete itsDPservice; }
    if (itsPublisher) { itsPublisher->close(); delete itsPublisher; }
    if (itsListener)  { itsListener->close();  delete itsListener; }
    if (itsTimerPort) { delete itsTimerPort; }
}

//
// sigintHandler(signum)
//
void TriggerControl::sigintHandler(int signum)
{
    LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

    if (thisTriggerControl) {
        thisTriggerControl->finish();
    }
}

//
// finish
//
void TriggerControl::finish()
{
    TRAN(TriggerControl::finishing_state);
}



//
// initial_state(event, port)
//
// Setup connection with PVSS and load Property sets.
//
GCFEvent::TResult TriggerControl::initial_state(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

    switch (event.signal) {
        case F_INIT: {
        } break;

        case F_ENTRY: {
            // Get access to my own propertyset.
            string  myPropSetName(createPropertySetName(PSN_CR_TRIGGER_CONTROL, getName()));
            LOG_DEBUG_STR ("Activating PropertySet " << myPropSetName);
            itsPropertySet = new RTDBPropertySet(myPropSetName, PST_CR_TRIGGER_CONTROL, PSAT_WO, this);
            // Wait for timer that is set in PropertySetAnswer on ENABLED event
        } break;

        case DP_CREATED: {
            // NOTE: thsi function may be called DURING the construction of the PropertySet.
            // Always exit this event in a way that GCF can end the construction.
            DPCreatedEvent  dpEvent(event);
            LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
            itsTimerPort->cancelAllTimers();
            itsTimerPort->setTimer(0.5);    // give RTDB time to get original value.
        } break;

        case F_TIMER: {
            // first redirect signalHandler to our finishing state to leave PVSS
            // in the right state when we are going down
            thisTriggerControl = this;
            signal (SIGINT,  TriggerControl::sigintHandler);    // ctrl-c
            signal (SIGTERM, TriggerControl::sigintHandler);    // kill

            // update PVSS.
            LOG_TRACE_FLOW ("Updating state to PVSS");
            itsPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Initial"));
            itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));

            LOG_DEBUG ("Going to attach to datapoint of MACScheduler");
            TRAN(TriggerControl::attach2MACScheduler);          // go to next state.
        } break;

        case DP_CHANGED: {
            _databaseEventHandler(event);
        } break;

        case F_EXIT: {
            itsTimerPort->cancelAllTimers();
        } break;

        default: {
            LOG_DEBUG_STR ("initial, default");
        } break;
    }
    return (GCFEvent::HANDLED);
}

//
// attach2MACScheduler(event, port)
//
// Take a subscribtion on the 'activeObservations' dataelement op the MACScheduler.
//
GCFEvent::TResult TriggerControl::attach2MACScheduler(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR("attach2MACScheduler:" << eventName(event) << "@" << port.getName());

    switch (event.signal) {
        case F_ENTRY: {
            // take subscription on LOFAR.PermSW.MACScheduler.activeObservations
            LOG_DEBUG("Taking subscription on MACScheduler field");
            PVSSresult  result = itsDPservice->query(formatString("'%s.%s'",PSN_MAC_SCHEDULER, PN_MS_ACTIVE_OBSERVATIONS), "");
            ASSERTSTR (result == SA_NO_ERROR, "Taking subscription on MACScheduler DP failed");
            itsTimerPort->setTimer(10.0);       // in case nothing happens
        } break;

        case DP_QUERY_SUBSCRIBED: {         // NOTE: DIT EVENT KOMT EIGENLIJK NOOIT. WAAROM?
            itsTimerPort->cancelAllTimers();
            DPQuerySubscribedEvent  answer(event);
            ASSERTSTR(answer.result == SA_NO_ERROR, "Taking subscription on PVSS-states failed :" << answer.result);

            itsSubscriptionID = answer.QryID;
            LOG_INFO_STR("Subscription on state fields from PVSS successful(" << itsSubscriptionID  << ")");
            LOG_INFO("Going to operational mode");
            TRAN(TriggerControl::openPublisher);        // go to next state.
        } break;

        case DP_QUERY_CHANGED: {
            // don't expect this event here right now, but you never know.
            // in case DP_QUERY_SUBSCRIBED is skipped.
            LOG_WARN("STRANGE ORDER OF EVENTS IN attach2MACScheduler");
            itsTimerPort->cancelAllTimers();
            _handleQueryEvent(event);
            TRAN(TriggerControl::openPublisher);        // go to next state.
        } break;

        case F_TIMER: {
            ASSERTSTR(false, "Taking subscription on datapoint of MACScheduler timed out!");
        } break;

        case F_EXIT: {
            itsTimerPort->cancelAllTimers();
        } break;

        default: {
            LOG_DEBUG_STR ("default");
        } break;
    }
    return (GCFEvent::HANDLED);
}

//
// openPublisher(event, port)
//
// Take a subscribtion on the 'activeObservations' dataelement op the MACScheduler.
//
GCFEvent::TResult TriggerControl::openPublisher(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR("openPublisher:" << eventName(event) << "@" << port.getName());

    switch (event.signal) {
        case F_ENTRY: {
            // take subscription on LOFAR.PermSW.MACScheduler.activeObservations
            LOG_DEBUG_STR("Opening publisher datapoint " << PSN_CR_TRIGGERPORT);
            ASSERTSTR(itsPublisher->open(), "Failed to open the datapoint for publishing trigger-events");
            itsTimerPort->setTimer(10.0);       // in case nothing happens
        } break;

        case F_CONNECTED: {
            LOG_INFO("Publish datapoint is opened, going to start the listener");
            itsTimerPort->cancelAllTimers();
            TRAN(TriggerControl::operational_state);
        } break;

        case F_DISCONNECTED: {
            ASSERTSTR(false, "Opening the publisher datapoint failed");
        } break;

        case F_TIMER: {
            ASSERTSTR(false, "Taking subscription on datapoint of MACScheduler timed out!");
        } break;

        case F_EXIT: {
            itsTimerPort->cancelAllTimers();
        } break;

        default: {
            LOG_DEBUG_STR ("default");
        } break;
    }
    return (GCFEvent::HANDLED);
}

//
// operational_state(event, port)
//
// Normal operation state, wait for events from parent task.
//
GCFEvent::TResult TriggerControl::operational_state(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR ("operational:" << eventName(event) << "@" << port.getName());
    
    switch (event.signal) {
        case F_ENTRY: {
            itsTimerPort->setTimer(5.0);
            itsListener->open();

            // update PVSS
            itsPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Active"));
            itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
        } break;

        case F_ACCEPT_REQ: {
            GCFTCPPort* itsClient = new GCFTCPPort();
            itsClient->init(*this, "client", GCFPortInterface::SPP, CR_PROTOCOL);
            itsListener->accept(*itsClient);
            LOG_INFO_STR("NEW CLIENT CONNECTED");
        } break;
        
        case F_CONNECTED: {
            if (&port == itsListener) {
                LOG_INFO("TCP Listener started successful");
                itsTimerPort->cancelAllTimers();
            }
        } break;

        case F_DISCONNECTED: {
            // TODO: CHECK FOR OUR OWN PORTS
            port.close();
            if (&port == itsClient) {
                LOG_INFO_STR("CLIENT DISCONNECTED");
            }
            else if (&port == itsListener) {
                LOG_FATAL("Failed to  start TCP Listener");
            }
        } break;

        case F_TIMER: {
            ASSERTSTR(false, "Failed to open the listener for receiving commands");
        } break;

        case F_EXIT: {
            itsTimerPort->cancelAllTimers();
        } break;

        case DP_CHANGED: {
            _databaseEventHandler(event);
        } break;

        case DP_QUERY_CHANGED: {
            _handleQueryEvent(event);
        } break;

        // -------------------- EVENTS RECEIVED FROM PARENT CONTROL ----------------

        // A new observationcontroller has connected
        case CONTROL_CONNECT: {
            CONTROLConnectEvent msg(event);
            _addObservation(getObservationNr(msg.cntlrName));
        } break;


        // -------------------- EXTERNAL COMMANDS --------------------
        case CR_STOP: {
            _CRstopHandler(event, port);
        } break;

        case CR_READ: {
            _CRreadHandler(event, port);
        } break;

        case CR_RECORD: {
            _CRrecordHandler(event, port);
        } break;

        case CR_STOP_DUMPS: {
            _CRstopDumpsHandler(event, port);
        } break;

        case CR_CEP_SPEED: {
            _CRcepSpeedHandler(event, port);
        } break;

        case CR_STOP_ACK:
        case CR_READ_ACK:
        case CR_RECORD_ACK:
        case CR_CEP_SPEED_ACK:
        case CR_STOP_DUMPS_ACK: {
            // TODO
            LOG_DEBUG_STR("Received CR_..._ACK");
        }

        default: {
            LOG_DEBUG("operational_state, default");
        } break;
    } // switch

    return (GCFEvent::HANDLED);
}


//
// finishing_state(event, port)
//
// Write controller state to PVSS, wait for 1 second (using a timer) to let
// GCF handle the property change and close the controller
//
GCFEvent::TResult TriggerControl::finishing_state(GCFEvent& event, GCFPortInterface& port)
{
    LOG_DEBUG_STR ("finishing_state:" << eventName(event) << "@" << port.getName());

    switch (event.signal) {
        case F_ENTRY: {
            // update PVSS
            itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Finished"));
            itsPropertySet->setValue(PN_FSM_ERROR,    GCFPVString(""));

            // cancel active queries
            if  (itsSubscriptionID) {
                itsDPservice->cancelQuery(itsSubscriptionID);
                itsSubscriptionID = 0;
            }
            itsTimerPort->setTimer(1L);
        } break;

        case DP_QUERY_UNSUBSCRIBED: {
        } break;

        case F_TIMER: {
            GCFScheduler::instance()->stop();
        } break;

        default: {
            LOG_DEBUG("finishing_state, default");
        }break;
    }
    return (GCFEvent::HANDLED);
}

//
// _databaseEventHandler(event)
//
void TriggerControl::_databaseEventHandler(GCFEvent& event)
{
    LOG_TRACE_FLOW_STR ("_databaseEventHandler:" << eventName(event));

    switch(event.signal) {
        case DP_CHANGED:  {
            DPChangedEvent      dpEvent(event);
    //      if (strstr(dpEvent.DPname.c_str(), PN_CLC_REQUESTED_CLOCK) != 0) {
    //          itsClock = ((GCFPVInteger*)(dpEvent.value._pValue))->getValue();
    //          LOG_DEBUG_STR("Received (requested)clock change from PVSS, clock is now " << itsClock);
    //          break;
    //      }

            // don't watch state and error fields.
            if ((strstr(dpEvent.DPname.c_str(), PN_OBJ_STATE) != 0) ||
                (strstr(dpEvent.DPname.c_str(), PN_FSM_ERROR) != 0) ||
                (strstr(dpEvent.DPname.c_str(), PN_FSM_CURRENT_ACTION) != 0) ||
                (strstr(dpEvent.DPname.c_str(), PN_FSM_LOG_MSG) != 0)) {
                return;
            }
        } break;

        default: {
        } break;
    }
}

//
// _handleQueryEvent(event)
//
void TriggerControl::_handleQueryEvent(GCFEvent& event)
{
    LOG_TRACE_FLOW_STR ("_handleQueryEvent:" << eventName(event));

    DPQueryChangedEvent     DPevent(event);
    if (DPevent.result != SA_NO_ERROR) {
        LOG_ERROR_STR("PVSS reported error " << DPevent.result << " for the 'active observation' query " <<
                ", cannot determine actual state of the observations!.");
        return;
    }

    if (!itsSubscriptionID) {
        itsSubscriptionID = DPevent.QryID;
    }

    _presetAdministration();    // clear all 'updated' flags.

    // The selected datapoints are delivered with full PVSS names, like:
    // CS001:LOFAR_PermSW_MACScheduler.activeObservations
    // Each event may contain more than one DP.
    GCFPVDynArr*    DPnames  = (GCFPVDynArr*)(DPevent.DPnames._pValue);
    GCFPVDynArr*    DPDynVal = (GCFPVDynArr*)(DPevent.DPvalues._pValue);
    GCFPVDynArr*    DPtimes  = (GCFPVDynArr*)(DPevent.DPtimes._pValue);
    int             nrDPs    = DPnames->getValue().size();
    int             nrValues = DPDynVal->getValue().size();
    int             nrTimes  = DPtimes->getValue().size();
    LOG_DEBUG_STR("nrDPs = " << nrDPs << ", nrValues = " << nrValues << ", nrTimes = " << nrTimes);
    for (int    idx = 0; idx < nrDPs; ++idx) {
        string  nameStr(DPnames->getValue()[idx]->getValueAsString());              // DP name
        LOG_DEBUG_STR("QryUpdate: DP=" << nameStr << ", " << nrValues << " values");
        for (int    val = 0; val < nrValues; ++val) {
//          string  newState(((GCFPVString*)(DPvalues->getValue()[idx]))->getValue());  // value
            LOG_DEBUG_STR("TYPE  = " << DPDynVal->getValue()[val]->getType());  // value
            LOG_DEBUG_STR("VALUE = " << DPDynVal->getValue()[val]->getValueAsString()); // valueAsString
            GCFPVDynArr*    DPvalues = (GCFPVDynArr*) DPDynVal->getValue()[0];
            LOG_DEBUG_STR("DPvalues = " << DPvalues->getValue()[0]->getValueAsString());

//      ...
        }
    } // for

    _cleanupAdministration();       // remove old observations.
}

//
// _presetAdministration()
//
void TriggerControl::_presetAdministration()
{
    map <int, ObsInfo>::iterator    iter = itsObservations.begin();
    map <int, ObsInfo>::iterator    end  = itsObservations.end();
    while (iter != end) {
        iter->second.updated = false;
        iter++;
    }
}

//
// _cleanupAdministration()
//
void TriggerControl::_cleanupAdministration()
{
    map <int, ObsInfo>::iterator    iter = itsObservations.begin();
    map <int, ObsInfo>::iterator    end  = itsObservations.end();
    while (iter != end && !iter->second.updated) {
        map <int, ObsInfo>::iterator    delIter = iter;
        iter++;
        LOG_INFO_STR("Removing observation " << delIter->second.obsID);
        itsObservations.erase(delIter);
    }
}


//
// _addObservation(name)
//
bool TriggerControl::_addObservation(int    obsID)
{
    // Already in admin? Return error.
    if (itsObservations.find(obsID) != itsObservations.end()) {
        return (true);
    }

    // find and read parameterset of this observation
    ParameterSet    theObsPS;
    Observation     theObs;
    string          filename(formatString("%s/Observation%d", LOFAR_SHARE_LOCATION, obsID));
    LOG_DEBUG_STR("Trying to readfile " << filename);
    try {
        theObsPS.adoptFile(filename);
        theObs = Observation(&theObsPS, false);
        LOG_DEBUG_STR("theOBS=" << theObs);
    }
    catch (Exception&   ex) {
        LOG_ERROR_STR("Error occured while reading the parameterset: " << ex.what());
        return (false);
    }

    ObsInfo     oi;
    oi.obsID       = obsID;
    oi.updated     = true;
    oi.startTime   = time_from_string(theObsPS.getString("Observation.startTime"));
    oi.stopTime    = time_from_string(theObsPS.getString("Observation.stopTime"));
    std::sort(theObs.stations.begin(), theObs.stations.end());
    oi.stationList = theObs.stations;

    LOG_INFO_STR("Adding observation" << obsID);
    itsObservations[obsID] = oi;

    return (true);
}


#if 0
//
// _updateObsListInPVSS()
//
// Refresh the contents of the activeObservations datafield in PVSS.
//
void TriggerControl::_updateObsListInPVSS()
{
    map <int, ObsInfo>::iterator iter = itsObservations.begin();
    map <int, ObsInfo>::iterator end  = itsObservations.end();

    GCFPValueArray      obsArr;
    while (iter != end) {
        obsArr.push_back(new GCFPVString(iter->first));
        ++iter;
    }
    itsPropertySet->setValue(PN_SC_ACTIVE_OBSERVATIONS, GCFPVDynArr(LPT_STRING, obsArr));
}
#endif

//
// _disconnectedHandler(port)
//
void TriggerControl::_disconnectedHandler(GCFPortInterface& port)
{
    port.close();
}

//================================================================================================
// CR event handlers

void TriggerControl::_CRstopHandler(GCFEvent& event, GCFPortInterface& port)
{
    CRStopEvent e(event);
    CRStopAckEvent ack;
    ack.triggerID = e.triggerID;
    ack.result = CR_OBSERVATION_ERR;

    LOG_DEBUG_STR("handle CRStopEvent");

    // set iter to observation
    map <int, ObsInfo>::iterator obsIter = itsObservations.find(e.observationID);

    // check if observation exists
    if (obsIter != itsObservations.end()) {
        ack.result = CR_NO_ERR;
        // check if station and rcu list is empty
        // check if valid stoptime
        vector<CRstopRequest>::iterator requestIter = e.stopVector.requests.begin();
        vector<CRstopRequest>::iterator requestEnd  = e.stopVector.requests.end();
        while (requestIter != requestEnd) {
            // expand selection string to full string
            (*requestIter).stationList = expandArrayString((*requestIter).stationList);
            (*requestIter).rcuList = expandArrayString((*requestIter).rcuList);
                
            // if not in time window send back time_error
            if (((*requestIter).stopTime.sec() < to_time_t(obsIter->second.startTime)) ||
                ((*requestIter).stopTime.sec() > to_time_t(obsIter->second.stopTime))) {

                ack.result = CR_TIME_ERR;
            }
            ++requestIter;
        }
    }
    // if no error send to RTDB
    if (ack.result ==  CR_NO_ERR) {
        LOG_DEBUG_STR("Valid Event, send to RTDB");
        itsPublisher->send(e);
    }
    port.send(ack);
}


void TriggerControl::_CRreadHandler(GCFEvent& event, GCFPortInterface& port)
{
    CRReadEvent e(event);
    CRReadAckEvent ack;
    ack.triggerID = e.triggerID;
    ack.result = CR_OBSERVATION_ERR;

    LOG_DEBUG_STR("handle CRReadEvent");

    // set iter to observation
    map <int, ObsInfo>::iterator obsIter = itsObservations.find(e.observationID);

    // if observation excist
    if (obsIter != itsObservations.end()) {
        ack.result = CR_NO_ERR;
        vector<CRreadRequest>::iterator requestIter = e.readVector.requests.begin();
        vector<CRreadRequest>::iterator requestEnd  = e.readVector.requests.end();
        while (requestIter != requestEnd) {
            // expand selection string to full string
            (*requestIter).stationList = expandArrayString((*requestIter).stationList);
            (*requestIter).rcuList = expandArrayString((*requestIter).rcuList);
                
            // if not in time window send back time_error
            if (((*requestIter).readTime.sec() < to_time_t(obsIter->second.startTime)) ||
                ((*requestIter).readTime.sec() > to_time_t(obsIter->second.stopTime))) {

                ack.result = CR_TIME_ERR;
            }
            ++requestIter;
        }
        // if no error send to RTDB
        if (ack.result ==  CR_NO_ERR) {
            LOG_DEBUG_STR("Valid Event, send to RTDB");
            itsPublisher->send(e);
        }
    }
    port.send(ack);
}

void TriggerControl::_CRrecordHandler(GCFEvent& event, GCFPortInterface& port)
{
    CRRecordEvent e(event);
    CRRecordAckEvent ack;
    ack.triggerID = e.triggerID;
    ack.result = CR_OBSERVATION_ERR;

    LOG_DEBUG_STR("handle CRRecordEvent");

    // set iter to observation
    map <int, ObsInfo>::iterator obsIter = itsObservations.find(e.observationID);

    // if observation excist
    if (obsIter != itsObservations.end()) {
        ack.result = CR_NO_ERR;
        vector<CRrecordRequest>::iterator requestIter = e.recordVector.requests.begin();
        vector<CRrecordRequest>::iterator requestEnd  = e.recordVector.requests.end();
        while (requestIter != requestEnd) {
            // expand selection string to full string
            (*requestIter).stationList = expandArrayString((*requestIter).stationList);
            (*requestIter).rcuList = expandArrayString((*requestIter).rcuList);
            ++requestIter;
        }
        LOG_DEBUG_STR("Valid Event, send to RTDB");
        itsPublisher->send(e);
    }
    port.send(ack);
}

void TriggerControl::_CRstopDumpsHandler(GCFEvent& event, GCFPortInterface& port)
{
    CRStopDumpsEvent e(event);
    CRStopDumpsAckEvent ack;
    ack.triggerID = e.triggerID;
    ack.result = CR_OBSERVATION_ERR;

    LOG_DEBUG_STR("handle CRStopDumpsEvent");

    // set iter to observation
    map <int, ObsInfo>::iterator obsIter = itsObservations.find(e.observationID);

    // if observation excist
    if (obsIter != itsObservations.end()) {
        ack.result = CR_NO_ERR;
        e.stationList = expandArrayString(e.stationList);
        LOG_DEBUG_STR("Valid Event, send to RTDB");
        itsPublisher->send(e);
    }
    port.send(ack);
}

void TriggerControl::_CRcepSpeedHandler(GCFEvent& event, GCFPortInterface& port)
{
    CRCepSpeedEvent e(event);
    CRCepSpeedAckEvent ack;
    ack.triggerID = e.triggerID;
    ack.result = CR_OBSERVATION_ERR;

    LOG_DEBUG_STR("handle CRCepSpeedEvent");

    // set iter to observation
    map <int, ObsInfo>::iterator obsIter = itsObservations.find(e.observationID);

    // if observation excist
    if (obsIter != itsObservations.end()) {
        ack.result = CR_NO_ERR;
        e.stationList = expandArrayString(e.stationList);
        LOG_DEBUG_STR("Valid Event, send to RTDB");
        itsPublisher->send(e);
    }
    port.send(ack);
}

}; // StationCU
}; // LOFAR
