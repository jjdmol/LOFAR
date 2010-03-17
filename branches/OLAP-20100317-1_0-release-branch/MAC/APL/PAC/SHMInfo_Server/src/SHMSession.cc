//#  SHMSession.cc: 
//#
//#  Copyright (C) 2002-2008
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
#include <Common/hexdump.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLocators.h>
//#include <Common/StringUtil.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include <APL/RTCCommon/PSAccess.h>

#include "SHMInfoServer.h"
#include "SHMSession.h"
#include "SHMDefines.h"
//#include "SHMSubscription.h"
#include "XCStatistics.h"         //MAXMOD
#include "RspStatus.h"            //MAXMOD

using namespace blitz;


namespace LOFAR {
	using namespace GCF::TM;
	using namespace RTC;
	using namespace APL::RTDBCommon;

	namespace AMI {

#define LOGMSGHDR(_in_)   \
    LOG_TRACE_FLOW(formatString( \
        "Session %p receives msg with seqnr: %llu, timestamp: %llu.%09lu", \
        this, \
        _in_.seqnr,\
        _in_.timestamp_sec,\
        _in_.timestamp_nsec));

#define RETURN_NOACK_MSG(_eventin_, _eventout_, _response_) \
  { \
    SHM##_eventin_##Event in(e); \
    LOGMSGHDR(in) \
    SEND_RESP_MSG(in, _eventout_, _response_) \
  }

#define SEND_RESP_MSG(_eventin_, _eventout_, _response_) \
  { \
    LOG_INFO(formatString( \
        "Response value is: %s.", \
        string(_response_).c_str()));  \
    SHM##_eventout_##Event out; \
    out.seqnr = _curSeqNr++; \
    out.replynr = _eventin_.seqnr; \
    out.response = _response_; \
    setCurrentTime(out.timestamp_sec, out.timestamp_nsec); \
    _missPort.send(out); \
  }
  
#define SEND_RESP_MSG_A(_eventin_, _eventout_, _response_) \
  { \
    LOG_INFO(formatString( \
        "Response value is: %s.", \
        _response_.c_str()));  \
    _eventout_.seqnr = _curSeqNr++; \
    _eventout_.replynr = _eventin_.seqnr; \
    _eventout_.response = _response_; \
    setCurrentTime(out.timestamp_sec, out.timestamp_nsec); \
    _missPort.send(_eventout_); \
  }

   //MAXMOD
   unsigned int i, j;
   
//
// SHMSession(SHMInfoServer)
//
SHMSession::SHMSession(SHMInfoServer& daemon) :
	GCFTask((State)&SHMSession::initial_state, "SHMSession"),
	_daemon			 	 (daemon),
	_curSeqNr			 (1),
	_curReplyNr			 (0),
	_pRememberedEvent	 (0),
	_nrOfRCUs			 (0)
{
	_missPort.init(*this, MAC_SVCMASK_SHMSESSION, GCFPortInterface::SPP, SHM_PROTOCOL);
	_rspDriverPort.init(*this, MAC_SVCMASK_RSPDRIVER, GCFPortInterface::SAP, RSP_PROTOCOL);
	_daemon.getPortProvider().accept(_missPort);
}

//
// ~SHMSession()
//
SHMSession::~SHMSession () 
{
#if 0
  for (TSubscriptions::iterator iter = _subscriptions.begin();
       iter != _subscriptions.end(); ++iter) {
    delete iter->second;
  }
#endif
}

//
// initial_state(event, port)
//
GCFEvent::TResult SHMSession::initial_state(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal) {
    case F_INIT: 
      break;

    case F_ENTRY:
      break;

    case F_CONNECTED:
      TRAN(SHMSession::waiting_state);
      break;

    case F_DISCONNECTED:
      TRAN(SHMSession::closing_state);
      break;
      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

//
// waiting_state(event, port)
//
GCFEvent::TResult SHMSession::waiting_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned long garbageTimerID = 0;

  switch (e.signal) {
    case F_ENTRY:
      _busy = false;
      garbageTimerID = _missPort.setTimer(5.0, 5.0);
      if (_pRememberedEvent) delete _pRememberedEvent;
		  _pRememberedEvent = 0;
      break;

    case F_EXIT:
      _busy = true;
      _missPort.cancelTimer(garbageTimerID);
      break;
      
    case F_TIMER: {
#if 0
      GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(e);
      
      if (timerEvent.id == garbageTimerID) {
        // cleanup the garbage of obsolete subscriptions
        SHMSubscription* pSubs;
        for (list<SHMSubscription*>::iterator iter = _subscriptionsGarbage.begin();
             iter != _subscriptionsGarbage.end(); ++iter) {
          pSubs = *iter;
          delete pSubs;
        }
        _subscriptionsGarbage.clear();
      }
#endif    
      break;
    }      

    case F_DISCONNECTED:
      if (&_missPort == &p) {
        LOG_INFO("Connection lost to a SHM client.");
        TRAN(SHMSession::closing_state);
      }
      break;
      
    case SHM_GENERIC_PINGPONG:
      genericPingpong(e);
      break;
      
    case SHM_GENERIC_IDENTIFY_REQUEST:
      getGenericIdentity(e);
      break;
    
    case SHM_DIAGNOSIS_NOTIFICATION:
      setDiagnosis(e);
      break;
    
    case SHM_RECONFIGURATION_REQUEST:
      TRAN(SHMSession::reconfigure_state);
//      dispatch(e, p);
	  GCFScheduler::instance()->queueEvent(this, e, &p);
      break;
    
    case SHM_LOFAR_STRUCTURE_REQUEST:
      TRAN(SHMSession::getPICStructure_state);
//      dispatch(e, p);
	  GCFScheduler::instance()->queueEvent(this, e, &p);
      break;
    
//    case SHM_PVSS_DP_SUBSCRIPTION_REQUEST:
//      subscribe(e);
//      break;
    
    case SHM_SUBBAND_STATISTICS_REQUEST:
      getSubbandStatistics(e);
      break;
    
    case SHM_ANTENNA_CORRELATION_MATRIX_REQUEST:
      //MAXMOD
      getAntennaCorrelation(e);
      //      TRAN(SHMSession::getAntennaCorrelation_state);
      //dispatch(e, p);
      break;

    case SHM_RSP_STATUS_REQUEST:
      //MAXMOD
      getRspStatus(e);
      //      TRAN(SHMSession::getAntennaCorrelation_state);
      //dispatch(e, p);
      break;
    
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

//
// genericPingPong(event)
//
void SHMSession::genericPingpong(GCFEvent& e)
{
  SHMGenericPingpongEvent in(e);
  LOGMSGHDR(in);
       
  //cout << "MAXMOD: in genericPingping, e.signal = " << e.signal << endl;
  SHMGenericPingpongEvent out;
  out.seqnr = _curSeqNr++;
  out.replynr = in.seqnr;
  setCurrentTime(out.timestamp_sec, out.timestamp_nsec);
  out.ttl = (in.ttl > 0 ? in.ttl - 1 : 0);
  _missPort.send(out);
}

//
// getGenericIdentity(event)
//
void SHMSession::getGenericIdentity(GCFEvent& e)
{    
  SHMGenericIdentifyRequestEvent in(e);
  LOGMSGHDR(in);
  SHMGenericIdentifyResponseEvent out;
  out.seqnr = _curSeqNr++;
  out.replynr = in.seqnr;
  out.response = (_busy ? "BUSY" : "ACK");
  char hostName[200];
  gethostname(hostName, 200);
  out.node_id = hostName;
  out.sw_version = formatString("%d.%d.%d (%s, %s)", 
                                SHM_MAJOR_VER, 
                                SHM_MIDOR_VER, 
                                SHM_MINOR_VER, 
                                __DATE__, 
                                __TIME__);
  setCurrentTime(out.timestamp_sec, out.timestamp_nsec);
  _missPort.send(out);
}

//
// setDiagnosis(event)
//
void SHMSession::setDiagnosis(GCFEvent& e)
{
	SHMDiagnosisNotificationEvent	diagnose(e);
	SHMDiagnosisResponseEvent       ackout(e);
	
	ssize_t maxsend;

	//MAXMOD debug
	//cout << "MAXMOD: in setDiagnosis " << endl;
	//cout << "MAXMOD: e.signal = " << e.signal << endl;
	//cout << "MAXMOD: new_state = " << diagnose.new_state << endl;
	//cout << "MAXMOD: component = " << diagnose.component << endl;
	//cout << "MAXMOD: diagnosis_id = " << diagnose.diagnosis_id << endl;
	LOG_INFO(formatString("MAXMOD: in setDiagnosis"));
	LOG_INFO(formatString("MAXMOD: component = %s", diagnose.component.c_str()));
	LOG_INFO(formatString("MAXMOD: diagnosis_id = %s", diagnose.diagnosis_id.c_str()));
	
	setObjectState("SHM:" + diagnose.diagnosis_id, diagnose.component, diagnose.new_state);
	
	// respond to SHM 
	ackout.seqnr   =  _curSeqNr++;
	ackout.replynr =  diagnose.seqnr;
	ackout.response = "ACK";
	setCurrentTime(ackout.timestamp_sec, ackout.timestamp_nsec);
	maxsend = _missPort.send(ackout);

	LOG_INFO(formatString(
			       "MAXMOD: setDiagnosisResponse Timestamp: %lu.%06lu",
			       ackout.timestamp_sec,
			       ackout.timestamp_nsec));
	LOG_INFO(formatString("MAXMOD: setDiagnosis %d response of _missPort.send(ackout).size",maxsend));
	//TRAN(SHMSession::waiting_state);
}


GCFEvent::TResult SHMSession::reconfigure_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
    case SHM_RECONFIGURATION_REQUEST:
    {
      RETURN_NOACK_MSG(ReconfigurationRequest, ReconfigurationResponse, "NAK (not supported yet)");
      TRAN(SHMSession::waiting_state);
      break;
    }       
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult SHMSession::getPICStructure_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
      
    case SHM_LOFAR_STRUCTURE_REQUEST:
    {
      RETURN_NOACK_MSG(LofarStructureRequest, LofarStructureResponse, "NAK (not supported yet)");
      TRAN(SHMSession::waiting_state);
      break;
    }       
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

void SHMSession::subscribe(GCFEvent& e)
{
#if 0
  SHMPvssDpSubscriptionRequestEvent in(e);
  LOGMSGHDR(in);
  string response = "ACK";
  
  LOG_DEBUG(formatString(
      "Subscription request (%s)",
      in.request.c_str()));
      
  if (in.request == "UNSUBSCRIBE") {
    TSubscriptions::iterator iter = _subscriptions.find(in.dpname);
    if (iter != _subscriptions.end()) {
      iter->second->unsubscribe(in.seqnr);
    }
    else {
      response = "NAK (not subscribed; ignored)";
    }
  }
  else if (in.request == "SUBSCRIBE" || in.request == "SINGLE-SHOT") {
    TSubscriptions::iterator iter = _subscriptions.find(in.dpname);
    if (iter != _subscriptions.end()) {
      response = "NAK (subscription already made; ignored)";
    }
    else {
      SHMSubscription* pNewSubscription = new SHMSubscription(*this, 
                                                            in.dpname, 
                                                            in.seqnr, 
                                                            (in.request == "SINGLE-SHOT"));
      _subscriptions[in.dpname] = pNewSubscription;
      TRAN(SHMSession::subscribe_state);
      pNewSubscription->subscribe();
    }        
  }
  
  if (response != "ACK") {
    SEND_RESP_MSG(in, PvssDpSubscriptionResponse, response);
  }
#endif
}

GCFEvent::TResult SHMSession::subscribe_state(GCFEvent& e, GCFPortInterface& p)
{
  return(defaultHandling(e, p));
}

//
// getRspStatus(event)
//
void SHMSession::getRspStatus(GCFEvent& e)
{
	assert(_pRememberedEvent == 0);
	SHMRspStatusRequestEvent* pIn = new SHMRspStatusRequestEvent(e);
	LOGMSGHDR((*pIn));

	if (_nrOfRCUs == 0) {
		try {
		  //MAXMOD
		  //_nrOfRCUs = GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL;
		  _nrOfRCUs = GET_CONFIG("RS.N_RSPBOARDS", i) * 4 * N_POL;
		  LOG_DEBUG(formatString ("NrOfRCUs %d", _nrOfRCUs));
		  _allRCUSMask.reset(); // init all bits to false value
		  _allRCUSMask.flip(); // flips all bits to the true value
		  // if nrOfRCUs is less than MAX_N_RCUS the not used bits must be unset
		  //for (int i = _nrOfRCUs; i < MAX_RCUS; i++) {
		  for (int i = _nrOfRCUs; i < MEPHeader::MAX_N_RCUS; i++) {
		    _allRCUSMask.set(i, false);
		  }
		  // idem for RSP mask [reo]
		  _allRSPSMask.reset();
		  _allRSPSMask.flip();
		  for (int b = GET_CONFIG("RS.N_RSPBOARDS",i); b < MAX_N_RSPBOARDS; b++) {
		    _allRSPSMask.set(b,false);
		  }
		}
		catch (...) {
			SEND_RESP_MSG((*pIn), RspStatusResponse, "NAK (no RSP configuration available)");
			delete pIn;
			return;
		}    
	}

	if (_rspDriverPort.isConnected()) {
		RSPGetstatusEvent 		getstatus;
		getstatus.timestamp = Timestamp(0, 0);
		getstatus.cache 	= true;
		getstatus.rspmask 	= _allRSPSMask;
		_rspDriverPort.send(getstatus);
	}
	else    {
		_rspDriverPort.open();
	}

	_pRememberedEvent = pIn;
	TRAN(SHMSession::getRspStatus_state);
}

GCFEvent::TResult SHMSession::getRspStatus_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static SHMRspStatusResponseEvent ackout;
  static SHMRspStatusRequestEvent* pIn(0);
  //MAXMOD
  ssize_t maxsend;

  switch (e.signal) {
    case F_ENTRY:
      /*
	if (ackout.rcu_settings == 0) {
        ackout.rcu_settingsNOE = _nrOfRCUs;
	ackout.rcu_settings = new uint32[_nrOfRCUs];
        ackout.dataNOE = _nrOfRCUs * 512;
        ackout.data = new double[_nrOfRCUs * 512];
	}
      */
      assert(_pRememberedEvent);
      pIn = (SHMRspStatusRequestEvent*) _pRememberedEvent;
      break;

    case F_CONNECTED:
    case F_DISCONNECTED:
      if (&_rspDriverPort == &p) {
        RSPGetstatusEvent 		getstatus;
        getstatus.timestamp = Timestamp(0, 0);
        getstatus.cache 	= true;
        getstatus.rspmask 	= _allRSPSMask;
        
        if (!_rspDriverPort.send(getstatus)) {
          SEND_RESP_MSG((*pIn), RspStatusResponse, "NAK (connection to rsp driver could not be established or is lost)");
          if (e.signal == F_DISCONNECTED) {
	      p.close();
	    }
          TRAN(SHMSession::waiting_state);
        }
      }
      else {
        status = defaultHandling(e, p);
      }  
      break;
      
    case RSP_GETSTATUSACK: {
      RSPGetstatusackEvent ack(e);
      
      if (RSP_SUCCESS != ack.status) {
        SEND_RESP_MSG((*pIn), RspStatusResponse, "NAK (error in ack of rspdriver)");
        TRAN(SHMSession::waiting_state);      
        break;
      }
      
      //have to resize blitz array before access
      ackout.statusdata.board().resize(ack.sysstatus.board().shape());
      //blitz arrays don't like memcpy
      ackout.statusdata.board() = ack.sysstatus.board();
      LOG_DEBUG(formatString("MAXMOD %d response of ack.sysstatus.board().size() ",ack.sysstatus.board().size()));
      LOG_DEBUG(formatString("MAXMOD %d response of ackout.statusdata.board().size() ",ackout.statusdata.board().size()));

      ackout.seqnr = _curSeqNr++;
      ackout.replynr = pIn->seqnr;

      ackout.response = "ACK";
      LOG_DEBUG(formatString("RSP Timestamp: %lu.%06lu", ack.timestamp.sec(), ack.timestamp.usec()));
      ackout.payload_timestamp_sec = ack.timestamp.sec();
      ackout.payload_timestamp_nsec = ack.timestamp.usec() * 1000;
      setCurrentTime(ackout.timestamp_sec, ackout.timestamp_nsec);
      maxsend = _missPort.send(ackout);
      LOG_DEBUG(formatString("MAXMOD %d response of _missPort.send(ackout).size",maxsend));
      TRAN(SHMSession::waiting_state);
      break;
    }

    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}


//
// getSubbandStatistics(event)
//
void SHMSession::getSubbandStatistics(GCFEvent& e)
{
	assert(_pRememberedEvent == 0);
	SHMSubbandStatisticsRequestEvent* pIn = new SHMSubbandStatisticsRequestEvent(e);
	LOGMSGHDR((*pIn));

	if (_nrOfRCUs == 0) {
		try {
		  //MAXMOD
		  //_nrOfRCUs = GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL;
		  _nrOfRCUs = GET_CONFIG("RS.N_RSPBOARDS", i) * 4 * N_POL;
			LOG_DEBUG(formatString ("NrOfRCUs %d", _nrOfRCUs));
			_allRCUSMask.reset(); // init all bits to false value
			_allRCUSMask.flip(); // flips all bits to the true value
			// if nrOfRCUs is less than MAX_N_RCUS the not used bits must be unset
			//for (int i = _nrOfRCUs; i < MAX_RCUS; i++) {
			for (int i = _nrOfRCUs; i < MEPHeader::MAX_N_RCUS; i++) {
				_allRCUSMask.set(i, false);
				LOG_DEBUG(formatString("MAXMOD: in _allRCUSMask loop, i = %d",i));
			}
			// idem for RSP mask [reo]
			_allRSPSMask.reset();
			_allRSPSMask.flip();
			for (int b = GET_CONFIG("RS.N_RSPBOARDS",i); b < MAX_N_RSPBOARDS; b++) {
			        _allRSPSMask.set(b,false);
			}
			LOG_DEBUG(formatString ("MAXMOD: NrOfRCUs %d, _allRCUSMask.count() = %d", _nrOfRCUs, _allRCUSMask.count()));
			LOG_DEBUG(formatString ("MAXMOD: MAX_RCUS = %d,", MAX_RCUS));
			LOG_DEBUG(formatString ("MAXMOD: MEPHeader::MAX_N_RCUS = %d,", MEPHeader::MAX_N_RCUS));
		}
		catch (...) {
			SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (no RSP configuration available)");
			delete pIn;
			return;
		}    
	}

	if (_rspDriverPort.isConnected()) {
		RSPGetstatusEvent 		getstatus;
		getstatus.timestamp = Timestamp(0, 0);
		getstatus.cache 	= true;
		getstatus.rspmask 	= _allRSPSMask;
		_rspDriverPort.send(getstatus);
	}
	else    {
		_rspDriverPort.open();
	}

	_pRememberedEvent = pIn;
	TRAN(SHMSession::getSubbandStatistics_state);
}


GCFEvent::TResult SHMSession::getSubbandStatistics_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static SHMSubbandStatisticsResponseEvent ackout;
  static SHMSubbandStatisticsRequestEvent* pIn(0);
  //MAXMOD
  ssize_t maxsend;

  switch (e.signal)
  {
    case F_ENTRY:
      if (ackout.rcu_settings == 0) {
        ackout.rcu_settingsNOE = _nrOfRCUs;
        //ackout.rcu_settings = new uint8[_nrOfRCUs];
	ackout.rcu_settings = new uint32[_nrOfRCUs];
        ackout.invalidNOE = _nrOfRCUs;
        //ackout.invalid = new uint8[_nrOfRCUs];
	ackout.invalid = new uint32[_nrOfRCUs];
        ackout.dataNOE = _nrOfRCUs * 512;
        ackout.data = new double[_nrOfRCUs * 512];
      }
      assert(_pRememberedEvent);
      pIn = (SHMSubbandStatisticsRequestEvent*) _pRememberedEvent;
      break;
         
    case F_CONNECTED:
    case F_DISCONNECTED:
      if (&_rspDriverPort == &p) {
        RSPGetstatusEvent 		getstatus;
        getstatus.timestamp = Timestamp(0, 0);
        getstatus.cache 	= true;
        getstatus.rspmask 	= _allRSPSMask;
        
        if (!_rspDriverPort.send(getstatus)) {
          SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (connection to rsp driver could not be established or is lost)");
          if (e.signal == F_DISCONNECTED) {
            p.close();
          }
          TRAN(SHMSession::waiting_state);
        }
      }
      else {
        status = defaultHandling(e, p);
      }  
      break;
      
    case RSP_GETSTATUSACK: {
      RSPGetstatusackEvent ack(e);

      if (RSP_SUCCESS != ack.status) {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (error in ack of rspdriver)");
        TRAN(SHMSession::waiting_state);      
        break;
      }

      //memcpy(ackout.invalid, ack.sysstatus.rcu().data(), nrOfRCUs * sizeof(ackout.invalid[0]));
      // There is no member systatus.rcu (anymore). There is a systatus.board (for RSP boards)
      //  which I am using in get_RSPStatus above.
      // I should remove the "invalid" elements but will require restructuring SHM database too.
      // For the time being, set to 0.
      for (int rcuout = 0; rcuout < _nrOfRCUs; rcuout++)
        {
	  ackout.invalid[rcuout] = 0;
	}
      

      RSPGetrcuEvent getrcu;
      
      getrcu.timestamp = Timestamp(0, 0);
      getrcu.rcumask = _allRCUSMask;
      getrcu.cache = true;
      if (!_rspDriverPort.send(getrcu)) {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (lost connection to rsp driver)");
        TRAN(SHMSession::waiting_state);      
      }
      break;
    }

    case RSP_GETRCUACK: {
      RSPGetrcuackEvent ack(e);
      
      if (RSP_SUCCESS != ack.status) {
        LOG_DEBUG(formatString("RSP_GETRCUACK: ack.status = %d",ack.status));
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (error in ack of rspdriver)");
        TRAN(SHMSession::waiting_state);      
        break;
      }

      //memcpy(ackout.rcu_settings, ack.settings().data(), _nrOfRCUs * sizeof(ackout.rcu_settings[0]));
      //MAXMOD do as rspctl does
      for (int rcuout = 0; rcuout < _nrOfRCUs; rcuout++) {
	  ackout.rcu_settings[rcuout] = ack.settings()(rcuout).getRaw();
	}

      //
      // subscribe to statistics updates
      RSPGetstatsEvent getstats;

      getstats.timestamp = Timestamp(0,0);

      getstats.rcumask = _allRCUSMask;
      getstats.cache = true;
      getstats.type = Statistics::SUBBAND_POWER;
      
      if (!_rspDriverPort.send(getstats)) {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (lost connection to rsp driver)");
        TRAN(SHMSession::waiting_state);      
      }
      break;
    }  

    case RSP_GETSTATSACK: {
      RSPGetstatsackEvent ack(e);

      if (RSP_SUCCESS != ack.status) {
	//MAXMOD debug
	cout << "RSP ack.status: " <<   ack.status << endl;
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (error in ack of rspdriver)");


        TRAN(SHMSession::waiting_state);      
        break;
      }

      ackout.seqnr = _curSeqNr++;
      ackout.replynr = pIn->seqnr;
      memcpy(ackout.data, ack.stats().data(), _nrOfRCUs * 512 * sizeof(double));

      ackout.response = "ACK";
      LOG_DEBUG(formatString("RSP Timestamp: %lu.%06lu", ack.timestamp.sec(), ack.timestamp.usec()));
      ackout.payload_timestamp_sec = ack.timestamp.sec();
      ackout.payload_timestamp_nsec = ack.timestamp.usec() * 1000;
      setCurrentTime(ackout.timestamp_sec, ackout.timestamp_nsec);
      maxsend = _missPort.send(ackout);
      LOG_DEBUG(formatString("MAXMOD %d response of _missPort.send(ackout).size",maxsend));
      TRAN(SHMSession::waiting_state);
      break;
    }
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

// code copied from getSubbandStatistics
// types defined in:
///home/avruch/LOFAR/MAC/APL/PIC/SHM/build/gnu_debug/src/SHM_Protocol.ph


void SHMSession::getAntennaCorrelation(GCFEvent& e)
{
	assert(_pRememberedEvent == 0);
	//MAXMOD see SHM_Protocol.cc for typedefs
	SHMAntennaCorrelationMatrixRequestEvent* pIn = new SHMAntennaCorrelationMatrixRequestEvent(e);
	LOGMSGHDR((*pIn));

	// MAXMOD select the subband for XLETs 
	// OK from SHM this works
	LOG_DEBUG(formatString("MAXMOD: Requested Subband for XLETs is %d",pIn->subband_selector));

	int subband = pIn->subband_selector;

	if (subband < 0 || subband >= MAX_SUBBANDS) {
	    LOG_DEBUG(formatString("MAXMOD: Error: argument to --xcsubband out of range, value must be >= 0 and < %d",MAX_SUBBANDS));
	    //exit(EXIT_FAILURE);
	    TRAN(SHMSession::waiting_state);
	  }

	//MAXMOD I'm not sure the following is really nec.
	if (_nrOfRCUs == 0) {
	  try {
	    //MAXMOD
	    _nrOfRCUs = GET_CONFIG("RS.N_RSPBOARDS", i) * 4 * N_POL;
	    LOG_DEBUG(formatString ("NrOfRCUs %d", _nrOfRCUs));
	    _allRCUSMask.reset(); // init all bits to false value
	    _allRCUSMask.flip(); // flips all bits to the true value
	    // if nrOfRCUs is less than MAX_N_RCUS the not used bits must be unset
	    //for (int i = _nrOfRCUs; i < MAX_RCUS; i++) {
	    for (int i = _nrOfRCUs; i < MEPHeader::MAX_N_RCUS; i++) {
	      _allRCUSMask.set(i, false);
	    }
	    // idem for RSP mask [reo]
	    _allRSPSMask.reset();
	    _allRSPSMask.flip();
	    for (int b = GET_CONFIG("RS.N_RSPBOARDS",i); b < MAX_N_RSPBOARDS; b++) {
	      _allRSPSMask.set(b,false);
	    }
	    LOG_DEBUG(formatString ("MAXMOD: NrOfRCUs %d, _allRCUSMask.count() = %d", _nrOfRCUs, _allRCUSMask.count()));
	    LOG_DEBUG(formatString ("MAXMOD: MAX_RCUS = %d,", MAX_RCUS));
	    LOG_DEBUG(formatString ("MAXMOD: MEPHeader::MAX_N_RCUS = %d,", MEPHeader::MAX_N_RCUS));
	  }
	  catch (...) {
	    SEND_RESP_MSG((*pIn), AntennaCorrelationMatrixResponse, "NAK (no RSP configuration available)");
	    delete pIn;
	    return;
	  }    
	}
	

	if (_rspDriverPort.isConnected()) {
		RSPGetxcstatsEvent 	getstats;
		getstats.timestamp = Timestamp(0, 0);
		getstats.cache 	= true;
		_rspDriverPort.send(getstats);
	}
	else    {
		_rspDriverPort.open();
	}

	_pRememberedEvent = pIn;
	TRAN(SHMSession::getAntennaCorrelation_state);
}

GCFEvent::TResult SHMSession::getAntennaCorrelation_state(GCFEvent& e, GCFPortInterface& p)
{
  //MAXMOD copied from getSubbandStatistics

  ////MAXMOD here you need to add code to handle it.
  //case SHM_ANTENNA_CORRELATION_MATRIX_REQUEST:
  //{
  //  RETURN_NOACK_MSG(AntennaCorrelationMatrixRequest, AntennaCorrelationMatrixResponse, "NAK (not supported yet)");
  //  TRAN(SHMSession::waiting_state);
  //  break;
  //}       
  //default:
  //  status = defaultHandling(e, p);
  //  break;
  //}

  GCFEvent::TResult status = GCFEvent::HANDLED;
  static SHMAntennaCorrelationMatrixResponseEvent ackout;
  static SHMAntennaCorrelationMatrixRequestEvent* pIn(0);
  //MAXMOD
  ssize_t maxsend;
  static int N_HBA = 0;
  static int N_LBA = 0;
  static int N_LBA_INNER = 0;
  static int N_LBA_OUTER = 0;

  switch (e.signal)
    {
    case F_ENTRY:
      if (ackout.rcu_settings == 0) {
        ackout.rcu_settingsNOE = _nrOfRCUs;
	ackout.rcu_settings = new uint32[_nrOfRCUs];
        ackout.invalidNOE = _nrOfRCUs;
	ackout.invalid = new uint32[_nrOfRCUs];
        ackout.acmdataNOE = _nrOfRCUs * _nrOfRCUs;
	ackout.geoposNOE = 3;
	ackout.geopos = new double[3];  //lat,lon,h
	ackout.antcoordsNOE = _nrOfRCUs * 3 ;
	ackout.antcoords = new double[_nrOfRCUs * 3];  //xyz Xpol, xyz Ypol
      }
      assert(_pRememberedEvent);
      pIn = (SHMAntennaCorrelationMatrixRequestEvent*) _pRememberedEvent;
      break;

    //MAXMOD cases CONNECTED & DISCONNECTED copied from SubbandStatistics example
    case F_CONNECTED:
    case F_DISCONNECTED:
      if (&_rspDriverPort == &p) {
        RSPGetstatusEvent 		getstatus;
        getstatus.timestamp = Timestamp(0, 0);
        getstatus.cache 	= true;
        getstatus.rspmask 	= _allRSPSMask;
        
        if (!_rspDriverPort.send(getstatus)) {
          SEND_RESP_MSG((*pIn), AntennaCorrelationMatrixResponse, "NAK (connection to rsp driver could not be established or is lost)");
          if (e.signal == F_DISCONNECTED) {
            p.close();
          }
          TRAN(SHMSession::waiting_state);
        }
      }
      else {
        status = defaultHandling(e, p);
      }  
      break;
      
    case RSP_GETSTATUSACK: {       //MAXMOD Now get rcu_settings
      // BTW RSPGetstatusEvent is what I want to add for getting voltages and temps
      RSPGetstatusackEvent ack(e);

      if (RSP_SUCCESS != ack.status) {
        SEND_RESP_MSG((*pIn), AntennaCorrelationMatrixResponse, "NAK (error in ack of rspdriver)");
        TRAN(SHMSession::waiting_state);      
        break;
      }

      for (int rcuout = 0; rcuout < _nrOfRCUs; rcuout++) {
	  ackout.invalid[rcuout] = 0;
	}
      
      RSPGetrcuEvent getrcu;
      
      getrcu.timestamp = Timestamp(0, 0);
      getrcu.rcumask = _allRCUSMask;
      getrcu.cache = true;
      if (!_rspDriverPort.send(getrcu)) {
        SEND_RESP_MSG((*pIn), AntennaCorrelationMatrixResponse, "NAK (lost connection to rsp driver)");
        TRAN(SHMSession::waiting_state);      
      }
      break;
    }

    case RSP_GETRCUACK: {  //MAXMOD save the rcu settings, then set xcsubband choice
      RSPGetrcuackEvent ack(e);
      
      if (RSP_SUCCESS != ack.status) {
        SEND_RESP_MSG((*pIn), AntennaCorrelationMatrixResponse, "NAK (error in ack of rspdriver)");
        TRAN(SHMSession::waiting_state);      
        break;
      }
      
      //MAXMOD guess observing freq (Low or High)
      N_HBA = 0; N_LBA = 0;N_LBA_INNER = 0; N_LBA_OUTER = 0;

      // Inetrnational stations have no lba_inner or lba_outer, only lba
      // so we need an additional counter as well.
      // Note that int. stations have 192 rcus, which we can use
      // to distinguish NL from int stations..
      for (int rcuout = 0; rcuout < _nrOfRCUs; rcuout++) {
	  ackout.rcu_settings[rcuout] = ack.settings()(rcuout).getRaw();
	  //determine RCU mode and hence array 'LBA' or 'HBA' 
	  //Ignore possibility of subarrays, for now.
	  //see RCUSettings.h
	  //This might be insufficient -- perhaps not all ON RCUs are being used. should check somehow.

	  int rcumode = ack.settings()(rcuout).getMode();
	  if ((rcumode == 1) || (rcumode == 2)) {
	    N_LBA_OUTER++ ;
	    N_LBA++;
	  }
	  if ((rcumode == 3) || (rcumode == 4)) {
	    N_LBA_INNER++ ;
	    N_LBA++;
	  }
	  
	  if ((rcumode >= 5) && (rcumode <= 7)) {
	    N_HBA++ ;
	  }
	  // rcumode = -1 => indeterminate RCU mode
	  // rcumode =  0 => RCU off
	}
      
      LOG_DEBUG(formatString("MAXMOD N_LBA = %d, N_LBA_INNER = %d, N_LBA_OUTER = %d, N_HBA = %d", N_LBA, N_LBA_INNER, N_LBA_OUTER, N_HBA));

      //MAXMOD do as rspctl does
      // SET subbands for XC
      RSPSetsubbandsEvent  setsubbands;

      setsubbands.timestamp = Timestamp(0,0);
      setsubbands.rcumask = _allRCUSMask;

      //MAXMOD the constant SubbandSelection::XLET is defined in LOFAR/MAC/APL/PIC/RSP_Protocol/include/APL/RSP_Protocol/SubbandSelection.h
      //LOG_DEBUG(formatString("MAXMOD SubbandSelection::XLET is %d",SubbandSelection::XLET));
      setsubbands.subbands.setType(SubbandSelection::XLET);

      setsubbands.subbands().resize(1,1);
      list<int> subbandlist;
      for (int rcu = 0; rcu < _nrOfRCUs / N_POL; rcu++){
	//for (int rcu = 0; rcu < _nrOfRCUs ; rcu++){
	subbandlist.push_back(pIn->subband_selector);
	LOG_DEBUG(formatString("MAXMOD rcu = %d", rcu));
      }
      LOG_DEBUG(formatString("MAXMOD subbands - type  %d ",setsubbands.subbands.getType()));
      LOG_DEBUG(formatString("MAXMOD subbands - first dim %d ",setsubbands.subbands().extent(firstDim)));
      LOG_DEBUG(formatString("MAXMOD subbands - second dim %d ",setsubbands.subbands().extent(secondDim)));
      LOG_DEBUG_STR("itsRCU:" << string(setsubbands.rcumask.to_string<char,char_traits<char>,allocator<char> >()));

      std::list<int>::iterator it = subbandlist.begin();
      setsubbands.subbands() = (*it);
      
      if (!_rspDriverPort.send(setsubbands)) {
	SEND_RESP_MSG((*pIn), AntennaCorrelationMatrixResponse, "NAK (lost connection to rsp driver)");
	TRAN(SHMSession::waiting_state);      
      }
      break;
    }
      
    case RSP_SETSUBBANDSACK: {
      RSPGetxcstatsEvent 	getxcstats;

      RSPSetsubbandsackEvent ack(e);
      if (RSP_SUCCESS != ack.status) {
        SEND_RESP_MSG((*pIn), AntennaCorrelationMatrixResponse, "NAK (error in ack of rspdriver)");
        TRAN(SHMSession::waiting_state);      
        break;
      }
      getxcstats.timestamp = Timestamp(0, 0);
      getxcstats.cache 	= true;
      if (!_rspDriverPort.send(getxcstats)) {
	SEND_RESP_MSG((*pIn), AntennaCorrelationMatrixResponse, "NAK (lost connection to rsp driver)");
	TRAN(SHMSession::waiting_state);      
      }
      break;
    }

    case RSP_GETXCSTATSACK: {
      RSPGetxcstatsackEvent ack(e);

      if (RSP_SUCCESS != ack.status) {
        SEND_RESP_MSG((*pIn), AntennaCorrelationMatrixResponse, "NAK (error in ack of rspdriver)");
        TRAN(SHMSession::waiting_state);      
        break;
      }
      
      //MAXMOD declare it here or above?
      //SHMAntennaCorrelationMatrixResponseEvent ackout;
      ackout.seqnr = _curSeqNr++;
      ackout.replynr = pIn->seqnr;

      //memcpy(ackout.acmdata(), ack.stats(), ack.stats().size());
      //Number of complex elements is nRCUs^2
      ackout.acmdata().resize(ack.stats().shape());
      ackout.acmdata() = ack.stats();
      ackout.subband_selection = pIn->subband_selector;
      ackout.acmdataNOE = _nrOfRCUs * _nrOfRCUs * 2;
      //MAXMOD
      LOG_DEBUG(formatString("MAXMOD acm -  first dim %d ",ackout.acmdata().extent(firstDim)));
      LOG_DEBUG(formatString("MAXMOD acm - second dim %d ",ackout.acmdata().extent(secondDim)));
      LOG_DEBUG(formatString("MAXMOD acm -  third dim %d ",ackout.acmdata().extent(thirdDim)));
      LOG_DEBUG(formatString("MAXMOD acm - fourth dim %d ",ackout.acmdata().extent(fourthDim)));
      //LOG_DEBUG(formatString("MAXMOD acm - fourth dim %d ",ack.stats().extent(fourthDim)));
      //cout << "MAXMOD ackout.acmdata().shape() = " << ackout.acmdata().shape() << endl;
      LOG_DEBUG(formatString("MAXMOD %d response of ack.stats().size",ack.stats().size()));
      LOG_DEBUG(formatString("MAXMOD %d response of ackout.acmdata().size",ackout.acmdata().size()));
      //cout << "MAXMOD ack.stats = " << ack.stats()(1,1,Range::all(),Range::all()) << endl;

      ///defn of _missPort see home/avruch/LOFAR/MAC/GCF/TM/include/GCF/TM/GCF_PortInterface.h 
      ackout.response = "ACK";
      LOG_DEBUG(formatString(
          "RSP Timestamp: %lu.%06lu",
          ack.timestamp.sec(),
          ack.timestamp.usec()));
      //ackout.timestamp_sec = ack.timestamp.sec();
      //ackout.timestamp_nsec = ack.timestamp.usec()*1000;
      ackout.payload_timestamp_sec = ack.timestamp.sec();
      ackout.payload_timestamp_nsec = ack.timestamp.usec() * 1000;
      setCurrentTime(ackout.timestamp_sec, ackout.timestamp_nsec);

      //hand back the antenna coords
      //arrayname from obs frequency (Low or High)
      // I have been assured that working stations will either be "LBA" or "HBA"
      //How do I tell which array ('HBA' or 'LBA') to hand back? from ackout.rcu_settings.
      //Ignore possibility of subarrays, for now.
      string targetarrayname;
      if ((N_LBA == 0) && (N_HBA == 0)){ //all RCUs off
	targetarrayname = "LBA";       //doesnt matter
      }
      if (N_LBA > N_HBA) {
	if (N_LBA_INNER > N_LBA_OUTER) {
	  targetarrayname = "LBA_INNER";
	}
	if (N_LBA_OUTER > N_LBA_INNER) {
	  targetarrayname = "LBA_OUTER";
	}
	// International stations only have LBA
	if (_nrOfRCUs > 96) {
	  targetarrayname = "LBA";
	}
      }
      if (N_LBA < N_HBA) {
	targetarrayname = "HBA";
      }
      if (N_LBA == N_HBA) {   //confused
	targetarrayname = "LBA";
      }

      // search for AntennaArray with that name;
      // otherwise, return zeros
      char hostName[200];
      gethostname(hostName, 200);
      LOG_DEBUG(formatString("MAXMOD gethostname = %s",hostName));

      const CAL::AntennaArray * targetarray;
      targetarray = _daemon.m_arrays.getByName(targetarrayname);
      if (targetarray != NULL){
	LOG_DEBUG(formatString("MAXMOD N_LBA=%d, N_HBA=%d, targetarray %s",N_LBA,N_HBA,targetarray->getName().c_str()));
      }
      else{
	LOG_DEBUG(formatString("MAXMOD N_LBA=%d, N_HBA=%d, targetarray is NULL",N_LBA, N_HBA));
      }
      
      //unsigned int ndimfoo = targetarray->getGeoLoc().dimensions();
      //LOG_DEBUG(formatString("MAXMOD rank GeoLoc = %u",ndimfoo));
      //for (unsigned int i = 0; i < ndimfoo; i++){
      //LOG_DEBUG(formatString("MAXMOD size of dim %u of GeoLoc = %u",i, targetarray->getGeoLoc().extent(i)));
      //}
      //ndimfoo = targetarray->getAntennaPos().dimensions();
      //LOG_DEBUG(formatString("MAXMOD rank AntennaPos = %u",ndimfoo));
      //for (unsigned int i = 0; i < ndimfoo; i++){
      //	LOG_DEBUG(formatString("MAXMOD size of dim %u of AntennaPos = %u",i, targetarray->getAntennaPos().extent(i)));
      //}

      if (targetarray != NULL){
	// reset possible wrong setting of number of elements (in case 
	// previously no valid array name was found this is set to 1 ).
	if (ackout.geoposNOE == 1) {
	  ackout.geoposNOE = 3;
	  ackout.antcoordsNOE = _nrOfRCUs * 3 ;
	}
	
	for (int i = 0; i < targetarray->getGeoLoc().extent(firstDim); i++){
	  //ackout.geopos[i] = *(const_cast<double *>(targetarray->getGeoLoc()(i).data()));
	  ackout.geopos[i] = targetarray->getGeoLoc()(i);
	  LOG_DEBUG(formatString("MAXMOD ackout.geopos[%d]=%f",i,ackout.geopos[i]));
	}

	//MAXMOD DEBUG
	/**
	LOG_DEBUG_STR("MAXMOD targetarray->getAntennaPos : " << targetarray->getAntennaPos());
	for (int i = 0; i < targetarray->getAntennaPos().extent(firstDim) ; i ++){
	  for (int j = 0; j < targetarray->getAntennaPos().extent(secondDim) ; j ++){
	    for (int k = 0; k < targetarray->getAntennaPos().extent(thirdDim) ; k ++){
	      LOG_DEBUG(formatString("MAXMOD targetarray->getAntennaPos(%d,%d,%d) =  %f", i,j,k,targetarray->getAntennaPos()(i,j,k)));
	    }
	  }
	}
	**/
 
	for (int i = 0; i < targetarray->getAntennaPos().extent(firstDim) ; i ++){
	  //ackout.antcoords[i*6]   = *(const_cast<double *>(targetarray->getAntennaPos()(i,0,0).data()));
	  ackout.antcoords[i*6]   = targetarray->getAntennaPos()(i,0,0);
	  ackout.antcoords[i*6+1] = targetarray->getAntennaPos()(i,0,1);
	  ackout.antcoords[i*6+2] = targetarray->getAntennaPos()(i,0,2);
	  ackout.antcoords[i*6+3] = targetarray->getAntennaPos()(i,1,0);
	  ackout.antcoords[i*6+4] = targetarray->getAntennaPos()(i,1,1);
	  ackout.antcoords[i*6+5] = targetarray->getAntennaPos()(i,1,2);
	  LOG_DEBUG(formatString("MAXMOD AntennaPos row %d",i));
	  //LOG_DEBUG(formatString("MAXMOD (Xpol-x1) ackout.antcoords[%d]=%f",i*6,ackout.antcoords[i*6]));
	}
      }
      else {
	//No matching array found in config file -- return obviously wrong values
	LOG_DEBUG(formatString("MAXMOD No Matching AntennaArray, returning [1],[1]"));
	ackout.geoposNOE = 1;
	//ackout.geopos = new double[1];
	ackout.geopos[0] = 1;
	ackout.antcoordsNOE = 1;
	//ackout.antcoords = new double[1];
	ackout.antcoords[0] = 1;
      }
	
      //vector<string> ArrayNames = _daemon.m_arrays.getNameList();
      //vector<string>::iterator	iter = ArrayNames.begin();
      //vector<string>::iterator	end  = ArrayNames.end();
      //while (iter != end) {
	//cout << "name          :" << iter->first << endl;
	//cout << "spectralwindow:" << iter->second->getSPW().getName() << endl;
	//cout << "RCUmask       :" << iter->second->getRCUMask() << endl;
	//const CAL::AntennaArray * somearray = _daemon.m_arrays.getByName(*iter);
	//cout << "SHMSESSION: iter  :" << *iter << endl;
	//cout << "SHMSESSION: somearray.getName() :" << somearray->getName() << endl;
	//LOG_DEBUG(formatString("MAXMOD Array %s",(*iter).c_str()));
	//LOG_DEBUG(formatString("MAXMOD somearray %s",somearray->getName().c_str()));
	//LOG_DEBUG_STR("MAXMOD getGeoLoc: :" << somearray->getGeoLoc());
	//iter++;
      //}


      maxsend = _missPort.send(ackout);
      LOG_DEBUG(formatString("MAXMOD %d response of _missPort.send(ackout).size",maxsend));
      TRAN(SHMSession::waiting_state);
      break;
      
    }

    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;

}

GCFEvent::TResult SHMSession::closing_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  p.close();
  switch (e.signal) {
    case F_ENTRY:
      if (&p == &_missPort) {
        LOG_INFO("Client gone. Stop all subsessions.");
      }
      else {
        _daemon.clientClosed(*this);
      }
      break;

    case F_CLOSED:
      _daemon.clientClosed(*this);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult SHMSession::defaultHandling(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
    case F_DISCONNECTED:
      if (&p == &_missPort) {
        LOG_INFO("Connection lost to a SHM client.");
        TRAN(SHMSession::closing_state);
      }
      break;
      
    case SHM_GENERIC_PINGPONG:
      genericPingpong(e);
      break;
      
    case SHM_GENERIC_IDENTIFY_REQUEST:
      getGenericIdentity(e);
      break;
    
    case SHM_DIAGNOSIS_NOTIFICATION:
//      RETURN_NOACK_MSG(DiagnosisNotification, DiagnosisResponse, "BUSY");     
      setDiagnosis(e);
      break;

    case SHM_RECONFIGURATION_REQUEST:
      RETURN_NOACK_MSG(ReconfigurationRequest, ReconfigurationResponse, "BUSY");
      break;

    case SHM_LOFAR_STRUCTURE_REQUEST:
      RETURN_NOACK_MSG(LofarStructureRequest, LofarStructureResponse, "BUSY");
      break;

//    case SHM_PVSS_DP_SUBSCRIPTION_REQUEST:
//      RETURN_NOACK_MSG(PvssDpSubscriptionRequest, PvssDpSubscriptionResponse, "BUSY");
//      break;

    case SHM_SUBBAND_STATISTICS_REQUEST:
      RETURN_NOACK_MSG(SubbandStatisticsRequest, SubbandStatisticsResponse, "BUSY");
      break;
    
    case SHM_ANTENNA_CORRELATION_MATRIX_REQUEST:
      RETURN_NOACK_MSG(AntennaCorrelationMatrixRequest, AntennaCorrelationMatrixResponse, "BUSY");
      break;
    
    case SHM_RSP_STATUS_REQUEST:
      RETURN_NOACK_MSG(RspStatusRequest, RspStatusResponse, "BUSY");
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

#if 0
void SHMSession::subscribed(SHMPvssDpSubscriptionResponseEvent& e)
{
  e.seqnr = _curSeqNr++;
  _missPort.send(e);
  TRAN(SHMSession::waiting_state);
}

void SHMSession::valueChanged(SHMPvssDpSubscriptionValueChangedAsyncEvent& e)
{
  e.seqnr = _curSeqNr++;
  _missPort.send(e);
}
#endif 

void SHMSession::mayDelete(const string& propName)
{
#if 0
  TSubscriptions::iterator iter = _subscriptions.find(propName);
  ASSERTSTR(iter != _subscriptions.end(), "Subscription should still exist here!");
  SHMSubscription* pSubs = iter->second;
  _subscriptions.erase(propName);
  _subscriptionsGarbage.push_back(pSubs);
#endif
}

void SHMSession::setCurrentTime(int64& sec, uint32& nsec)
{
  struct timeval tv;
  (void)gettimeofday(&tv, 0);
  sec = tv.tv_sec;
  nsec = tv.tv_usec * 1000;
}
 } // namespace AMI
} // namespace LOFAR
