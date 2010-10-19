//#  MISSession.cc: 
//#
//#  Copyright (C) 2002-2003
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

#include "MISSession.h"
#include "MISDaemon.h"
#include "MISDefines.h"
#include "MISSubscription.h"
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/PAL/GCF_Answer.h>
#include <APLCommon/APL_Defines.h>
#include <GCF/GCF_PVInteger.h>
#include <PSAccess.h>
#include <GCF/LogSys/GCF_KeyValueLogger.h>

using namespace blitz;

namespace LOFAR 
{
using namespace GCF::Common;
using namespace GCF::TM;
using namespace GCF::PAL;
using namespace RTC;
 namespace AMI
 {

#define LOGMSGHDR(_in_)   \
    LOG_TRACE_FLOW(formatString( \
        "Session %p receives msg with seqnr: %llu, timestamp: %llu.%09lu", \
        this, \
        _in_.seqnr,\
        _in_.timestamp_sec,\
        _in_.timestamp_nsec));

#define RETURN_NOACK_MSG(_eventin_, _eventout_, _response_) \
  { \
    MIS##_eventin_##Event in(e); \
    LOGMSGHDR(in) \
    SEND_RESP_MSG(in, _eventout_, _response_) \
  }

#define SEND_RESP_MSG(_eventin_, _eventout_, _response_) \
  { \
    LOG_INFO(formatString( \
        "Response value is: %s.", \
        string(_response_).c_str()));  \
    MIS##_eventout_##Event out; \
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

MISSession::MISSession(MISDaemon& daemon) :
  GCFTask((State)&MISSession::initial_state, MISS_TASK_NAME),
  _daemon(daemon),
  _propertyProxy(*this),
  _curSeqNr(1),
  _curReplyNr(0),
  _pRememberedEvent(0),
  _nrOfRCUs(0)
{
  _missPort.init(*this, MISS_PORT_NAME, GCFPortInterface::SPP, MIS_PROTOCOL);
  _rspDriverPort.init(*this, MIS_RSP_PORT_NAME, GCFPortInterface::SAP, RSP_PROTOCOL);
  _daemon.getPortProvider().accept(_missPort);
}

MISSession::~MISSession () 
{
  for (TSubscriptions::iterator iter = _subscriptions.begin();
       iter != _subscriptions.end(); ++iter)
  {
    delete iter->second;
  }
}

GCFEvent::TResult MISSession::initial_state(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      break;

    case F_CONNECTED:
      TRAN(MISSession::waiting_state);
      break;

    case F_DISCONNECTED:
      TRAN(MISSession::closing_state);
      break;
      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::waiting_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned long garbageTimerID = 0;

  switch (e.signal)
  {
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
      
    case F_TIMER:
    {
      GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(e);
      
      if (timerEvent.id == garbageTimerID)
      {
        // cleanup the garbage of obsolete subscriptions
        MISSubscription* pSubs;
        for (list<MISSubscription*>::iterator iter = _subscriptionsGarbage.begin();
             iter != _subscriptionsGarbage.end(); ++iter)
        {
          pSubs = *iter;
          delete pSubs;
        }
        _subscriptionsGarbage.clear();
      }
    
      break;
    }      
    case F_DISCONNECTED:
      if (&_missPort == &p)
      {
        LOG_INFO("Connection lost to a MIS client.");
        TRAN(MISSession::closing_state);
      }
      break;
      
    case MIS_GENERIC_PINGPONG:
      genericPingpong(e);
      break;
      
    case MIS_GENERIC_IDENTIFY_REQUEST:
      getGenericIdentity(e);
      break;
    
    case MIS_DIAGNOSIS_NOTIFICATION:
      setDiagnosis(e);
      break;
    
    case MIS_RECONFIGURATION_REQUEST:
      TRAN(MISSession::reconfigure_state);
      dispatch(e, p);
      break;
    
    case MIS_LOFAR_STRUCTURE_REQUEST:
      TRAN(MISSession::getPICStructure_state);
      dispatch(e, p);
      break;
    
    case MIS_PVSS_DP_SUBSCRIPTION_REQUEST:
      subscribe(e);
      break;
    
    case MIS_SUBBAND_STATISTICS_REQUEST:
      getSubbandStatistics(e);
      break;
    
    case MIS_ANTENNA_CORRELATION_MATRIX_REQUEST:
      TRAN(MISSession::getAntennaCorrelation_state);
      dispatch(e, p);
      break;
    
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

void MISSession::genericPingpong(GCFEvent& e)
{
  MISGenericPingpongEvent in(e);
  LOGMSGHDR(in);
       
  MISGenericPingpongEvent out;
  out.seqnr = _curSeqNr++;
  out.replynr = in.seqnr;
  setCurrentTime(out.timestamp_sec, out.timestamp_nsec);
  out.ttl = (in.ttl > 0 ? in.ttl - 1 : 0);
  _missPort.send(out);
}

void MISSession::getGenericIdentity(GCFEvent& e)
{    
  MISGenericIdentifyRequestEvent in(e);
  LOGMSGHDR(in);
  MISGenericIdentifyResponseEvent out;
  out.seqnr = _curSeqNr++;
  out.replynr = in.seqnr;
  out.response = (_busy ? "BUSY" : "ACK");
  char hostName[200];
  gethostname(hostName, 200);
  out.node_id = hostName;
  out.sw_version = formatString("%d.%d.%d (%s, %s)", 
                                MIS_MAJOR_VER, 
                                MIS_MIDOR_VER, 
                                MIS_MINOR_VER, 
                                __DATE__, 
                                __TIME__);
  setCurrentTime(out.timestamp_sec, out.timestamp_nsec);
  _missPort.send(out);
}

void MISSession::setDiagnosis(GCFEvent& e)
{
  MISDiagnosisNotificationEvent* pIn(0);
  static string resourceStatusPropName = "";

  assert(_pRememberedEvent == 0);
    
  pIn = new MISDiagnosisNotificationEvent(e);
  LOGMSGHDR((*pIn));
  resourceStatusPropName = pIn->component;

  // first try to get the current status value of the component
  // for this purpose it is important that the component name contains ".status"
  if (resourceStatusPropName.find(".status") == string::npos)
  {
    resourceStatusPropName += ".status";
  }
  if (GCFPVSSInfo::propExists(resourceStatusPropName))
  {
    if (_propertyProxy.requestPropValue(resourceStatusPropName) != GCF_NO_ERROR)
    {
      SEND_RESP_MSG((*pIn), DiagnosisResponse, "NAK (Error while requesting the current component status!)");
    }
    else
    {
      _pRememberedEvent = pIn;      
      TRAN(MISSession::setDiagnosis_state);
    }
  }
  else
  {
    SEND_RESP_MSG((*pIn), DiagnosisResponse, "NAK (Component has no status or does not exist!)");
  }
  if (_pRememberedEvent == 0)
  {
    delete pIn;
  }
}

GCFEvent::TResult MISSession::setDiagnosis_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static string resourceStatusPropName = "";
  
  switch (e.signal)
  {
    case F_VGETRESP:
    {
      assert (_pRememberedEvent);
      MISDiagnosisNotificationEvent* pIn = (MISDiagnosisNotificationEvent*)_pRememberedEvent;
      resourceStatusPropName = pIn->component;
      // first try to get the current status value of the component
      // for this purpose it is important that the component name contains ".status"
      if (resourceStatusPropName.find(".status") == string::npos)
      {
        resourceStatusPropName += ".status";
      }

      GCFPVInteger resourceState(RS_IDLE);
      GCFPropValueEvent* pE = (GCFPropValueEvent*)(&e);
      resourceState.copy(*pE->pValue); 
      string response  = _daemon.getPolicyHandler().checkDiagnose(*pIn, resourceState);
      if (response == "ACK")
      {
        _propertyProxy.setPropValue(resourceStatusPropName, resourceState);
        if (resourceStatusPropName.find(":") == string::npos)
        {
          resourceStatusPropName = GCFPVSSInfo::getLocalSystemName() + string(":") + resourceStatusPropName;
        }
        timeval ts = {pIn->timestamp_sec, pIn->timestamp_nsec / 1000};
        string descr(formatString (
            "%s(cl:%d, url:%d)",
            pIn->diagnosis.c_str(),
            pIn->confidence,
            pIn->diagnosis_id.c_str()));
        
        LOG_KEYVALUE_TSD(resourceStatusPropName, resourceState, 
                         KVL_ORIGIN_SHM, ts, descr);
      }
      SEND_RESP_MSG((*pIn), DiagnosisResponse, response);
      TRAN(MISSession::waiting_state);      
      break;
    }  
      
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}


GCFEvent::TResult MISSession::reconfigure_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case MIS_RECONFIGURATION_REQUEST:
    {
      RETURN_NOACK_MSG(ReconfigurationRequest, ReconfigurationResponse, "NAK (not supported yet)");
      TRAN(MISSession::waiting_state);
      break;
    }       
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::getPICStructure_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
      
    case MIS_LOFAR_STRUCTURE_REQUEST:
    {
      RETURN_NOACK_MSG(LofarStructureRequest, LofarStructureResponse, "NAK (not supported yet)");
      TRAN(MISSession::waiting_state);
      break;
    }       
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

void MISSession::subscribe(GCFEvent& e)
{
  MISPvssDpSubscriptionRequestEvent in(e);
  LOGMSGHDR(in);
  string response = "ACK";
  
  LOG_DEBUG(formatString(
      "Subscription request (%s)",
      in.request.c_str()));
      
  if (in.request == "UNSUBSCRIBE")
  {
    TSubscriptions::iterator iter = _subscriptions.find(in.dpname);
    if (iter != _subscriptions.end())
    {
      iter->second->unsubscribe(in.seqnr);
    }
    else
    {
      response = "NAK (not subscribed; ignored)";
    }
  }
  else if (in.request == "SUBSCRIBE" || in.request == "SINGLE-SHOT")
  {
    TSubscriptions::iterator iter = _subscriptions.find(in.dpname);
    if (iter != _subscriptions.end())
    {
      response = "NAK (subscription already made; ignored)";
    }
    else
    {
      MISSubscription* pNewSubscription = new MISSubscription(*this, 
                                                            in.dpname, 
                                                            in.seqnr, 
                                                            (in.request == "SINGLE-SHOT"));
      _subscriptions[in.dpname] = pNewSubscription;
      TRAN(MISSession::subscribe_state);
      pNewSubscription->subscribe();
    }        
  }
  
  if (response != "ACK")
  {
    SEND_RESP_MSG(in, PvssDpSubscriptionResponse, response);
  }
}

GCFEvent::TResult MISSession::subscribe_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

void MISSession::getSubbandStatistics(GCFEvent& e)
{
  assert(_pRememberedEvent == 0);
  MISSubbandStatisticsRequestEvent* pIn = new MISSubbandStatisticsRequestEvent(e);
  LOGMSGHDR((*pIn));

  if (_nrOfRCUs == 0)
  {
    try
    {
      _nrOfRCUs = GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL;
      LOG_DEBUG(formatString (
          "NrOfRCUs %d",
          _nrOfRCUs));
      _allRCUSMask.reset(); // init all bits to false value
      _allRCUSMask.flip(); // flips all bits to the true value
      // if nrOfRCUs is less than MAX_N_RCUS the not used bits must be unset
      for (int i = _nrOfRCUs; i < MAX_N_RCUS; i++)
      {
        _allRCUSMask.set(i, false);
      }
    }
    catch (...)
    {
      SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (no RSP configuration available)");
      delete pIn;
      return;
    }    
  }

  if (_rspDriverPort.isConnected())
  {
    
    RSPGetstatusEvent getstatus;
    getstatus.timestamp = Timestamp(0, 0);
    getstatus.cache = true;
    
    getstatus.rcumask = _allRCUSMask;
    _rspDriverPort.send(getstatus);
  }
  else    
  {
    _rspDriverPort.open();
  }
  _pRememberedEvent = pIn;
  TRAN(MISSession::getSubbandStatistics_state);


}

GCFEvent::TResult MISSession::getSubbandStatistics_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static MISSubbandStatisticsResponseEvent ackout;
  static MISSubbandStatisticsRequestEvent* pIn(0);

  switch (e.signal)
  {
    case F_ENTRY:
      if (ackout.rcu_settings == 0)
      {
        ackout.rcu_settingsNOE = _nrOfRCUs;
        ackout.rcu_settings = new uint8[_nrOfRCUs];
        ackout.invalidNOE = _nrOfRCUs;
        ackout.invalid = new uint8[_nrOfRCUs];
        ackout.dataNOE = _nrOfRCUs * 512;
        ackout.data = new double[_nrOfRCUs * 512];
      }
      assert(_pRememberedEvent);
      pIn = (MISSubbandStatisticsRequestEvent*) _pRememberedEvent;
      break;
         
    case F_CONNECTED:
    case F_DISCONNECTED:
      if (&_rspDriverPort == &p)
      {
        RSPGetstatusEvent getstatus;
        getstatus.timestamp = Timestamp(0, 0);
        getstatus.cache = true;
      
        getstatus.rcumask = _allRCUSMask;
        
        if (!_rspDriverPort.send(getstatus))
        {
          SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (connection to rsp driver could not be established or is lost)");
          if (e.signal == F_DISCONNECTED)
          {
            p.close();
          }
          TRAN(MISSession::waiting_state);
        }
      }
      else
      {
        status = defaultHandling(e, p);
      }  
      break;
      
    case RSP_GETSTATUSACK:
    {
      RSPGetstatusackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (error in ack of rspdriver)");
        TRAN(MISSession::waiting_state);      
        break;
      }

      //memcpy(ackout.invalid, ack.sysstatus.rcu().data(), nrOfRCUs * sizeof(ackout.invalid[0]));
     
      RSPGetrcuEvent getrcu;
      
      getrcu.timestamp = Timestamp(0, 0);
      getrcu.rcumask = _allRCUSMask;
      getrcu.cache = true;
      if (!_rspDriverPort.send(getrcu))
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (lost connection to rsp driver)");
        TRAN(MISSession::waiting_state);      
      }
      break;
    }
    case RSP_GETRCUACK:
    {
      RSPGetrcuackEvent ack(e);
      
      if (SUCCESS != ack.status)
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (error in ack of rspdriver)");
        TRAN(MISSession::waiting_state);      
        break;
      }

      memcpy(ackout.rcu_settings, ack.settings().data(), _nrOfRCUs * sizeof(ackout.rcu_settings[0]));

      // subscribe to statistics updates
      RSPGetstatsEvent getstats;

      getstats.timestamp = Timestamp(0,0);

      getstats.rcumask = _allRCUSMask;
      getstats.cache = true;
      getstats.type = Statistics::SUBBAND_POWER;
      
      if (!_rspDriverPort.send(getstats))
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (lost connection to rsp driver)");
        TRAN(MISSession::waiting_state);      
      }
      break;
    }  
    case RSP_GETSTATSACK:
    {
      RSPGetstatsackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (error in ack of rspdriver)");
        TRAN(MISSession::waiting_state);      
        break;
      }

      ackout.seqnr = _curSeqNr++;
      ackout.replynr = pIn->seqnr;
      memcpy(ackout.data, ack.stats().data(), _nrOfRCUs * 512 * sizeof(double));

      ackout.response = "ACK";
      LOG_DEBUG(formatString(
          "RSP Timestamp: %lu.%06lu",
          ack.timestamp.sec(),
          ack.timestamp.usec()));
      ackout.payload_timestamp_sec = ack.timestamp.sec();
      ackout.payload_timestamp_nsec = ack.timestamp.usec() * 1000;
      setCurrentTime(ackout.timestamp_sec, ackout.timestamp_nsec);
      _missPort.send(ackout);
      TRAN(MISSession::waiting_state);
      break;
    }
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::getAntennaCorrelation_state(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
      
    case MIS_ANTENNA_CORRELATION_MATRIX_REQUEST:
    {
      RETURN_NOACK_MSG(AntennaCorrelationMatrixRequest, AntennaCorrelationMatrixResponse, "NAK (not supported yet)");
      TRAN(MISSession::waiting_state);
      break;
    }       
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::closing_state(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_ENTRY:
      if (!_missPort.isConnected())
      {
        LOG_INFO("Client gone. Stop all subsessions.");
        _missPort.close();
      }
      else
      {
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

GCFEvent::TResult MISSession::defaultHandling(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_DISCONNECTED:
      if (&p == &_missPort)
      {
        LOG_INFO("Connection lost to a MIS client.");
        TRAN(MISSession::closing_state);
      }
      break;
      
    case MIS_GENERIC_PINGPONG:
      genericPingpong(e);
      break;
      
    case MIS_GENERIC_IDENTIFY_REQUEST:
      getGenericIdentity(e);
      break;
    
    case MIS_DIAGNOSIS_NOTIFICATION:
      RETURN_NOACK_MSG(DiagnosisNotification, DiagnosisResponse, "BUSY");     
      break;

    case MIS_RECONFIGURATION_REQUEST:
      RETURN_NOACK_MSG(ReconfigurationRequest, ReconfigurationResponse, "BUSY");
      break;

    case MIS_LOFAR_STRUCTURE_REQUEST:
      RETURN_NOACK_MSG(LofarStructureRequest, LofarStructureResponse, "BUSY");
      break;

    case MIS_PVSS_DP_SUBSCRIPTION_REQUEST:
      RETURN_NOACK_MSG(PvssDpSubscriptionRequest, PvssDpSubscriptionResponse, "BUSY");
      break;

    case MIS_SUBBAND_STATISTICS_REQUEST:
      RETURN_NOACK_MSG(SubbandStatisticsRequest, SubbandStatisticsResponse, "BUSY");
      break;
    
    case MIS_ANTENNA_CORRELATION_MATRIX_REQUEST:
      RETURN_NOACK_MSG(AntennaCorrelationMatrixRequest, AntennaCorrelationMatrixResponse, "BUSY");
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void MISSession::subscribed(MISPvssDpSubscriptionResponseEvent& e)
{
  e.seqnr = _curSeqNr++;
  _missPort.send(e);
  TRAN(MISSession::waiting_state);
}

void MISSession::valueChanged(MISPvssDpSubscriptionValueChangedAsyncEvent& e)
{
  e.seqnr = _curSeqNr++;
  _missPort.send(e);
}

void MISSession::mayDelete(const string& propName)
{
  TSubscriptions::iterator iter = _subscriptions.find(propName);
  ASSERTSTR(iter != _subscriptions.end(), "Subscription should still exist here!");
  MISSubscription* pSubs = iter->second;
  _subscriptions.erase(propName);
  _subscriptionsGarbage.push_back(pSubs);
}

void MISSession::setCurrentTime(int64& sec, uint32& nsec)
{
  struct timeval tv;
  (void)gettimeofday(&tv, 0);
  sec = tv.tv_sec;
  nsec = tv.tv_usec * 1000;
}
 } // namespace AMI
} // namespace LOFAR
