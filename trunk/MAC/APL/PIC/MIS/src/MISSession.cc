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
#include <RSP_Protocol.ph>
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

#define LOGMSGHDR(in)   \
    LOG_TRACE_FLOW(formatString( \
        "Session %p receives msg with seqnr: %d, timestamp: %d.%06d", \
        this, \
        (in).seqnr, \
        (in).timestamp_sec, \
        (in).timestamp_nsec));

#define RETURN_NOACK_MSG(_eventin_, _eventout_, _response_) \
  { \
    MIS##_eventin_##Event in(e); \
    LOGMSGHDR(in) \
    SEND_RESP_MSG(in, _eventout_, _response_) \
  }

#define SEND_RESP_MSG(_eventin_, _eventout_, _response_) \
  { \
    MIS##_eventout_##Event out; \
    out.seqnr = _curSeqNr++; \
    out.replynr = _eventin_.seqnr; \
    out.response = _response_; \
    setCurrentTime(out.timestamp_sec, out.timestamp_nsec); \
    _missPort.send(out); \
  }
  
MISSession::MISSession(MISDaemon& daemon) :
  GCFTask((State)&MISSession::initial, MISS_TASK_NAME),
  _daemon(daemon),
  _propertyProxy(*this),
  _curSeqNr(1),
  _curReplyNr(0)
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

GCFEvent::TResult MISSession::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static bool retryConnect = false;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      if (!_rspDriverPort.isConnected())
      {       
        _rspDriverPort.open();
      }
      break;

    case F_CONNECTED:
      TRAN(MISSession::waiting);
      break;

    case F_DISCONNECTED:
      if (&_rspDriverPort == &p && !retryConnect)
      {
        retryConnect = true;
        p.setTimer(5.0);
      }
      else
      {
        TRAN(MISSession::closing);
      }
      break;
      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::waiting(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static unsigned long garbageTimerID = 0;

  switch (e.signal)
  {
    case F_ENTRY:
      _busy = false;
      garbageTimerID = _missPort.setTimer(5.0, 5.0);
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
      LOG_INFO("Connection lost to a MIS client.");
      TRAN(MISSession::closing);
      break;
      
    case MIS_GENERIC_PINGPONG:
      genericPingpong(e);
      break;
      
    case MIS_GENERIC_IDENTIFY_REQUEST:
      getGenericIdentity(e);
      break;
    
    case MIS_DIAGNOSIS_NOTIFICATION:
      TRAN(MISSession::setDiagnosis);
      dispatch(e, p);
      break;
    
    case MIS_RECONFIGURATION_REQUEST:
      TRAN(MISSession::reconfigure);
      dispatch(e, p);
      break;
    
    case MIS_LOFAR_STRUCTURE_REQUEST:
      TRAN(MISSession::getPICStructure);
      dispatch(e, p);
      break;
    
    case MIS_PVSS_DP_SUBSCRIPTION_REQUEST:
      TRAN(MISSession::subscribe);
      dispatch(e, p);
      break;
    
    case MIS_SUBBAND_STATISTICS_REQUEST:
      TRAN(MISSession::getSubbandStatistics);
      dispatch(e, p);
      break;
    
    case MIS_ANTENNA_CORRELATION_MATRIX_REQUEST:
      TRAN(MISSession::getAntennaCorrelation);
      dispatch(e, p);
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
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

GCFEvent::TResult MISSession::setDiagnosis(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  static MISDiagnosisNotificationEvent* pIn = 0;
  static string resourceStatusPropName = "";
  
  switch (e.signal)
  {
    case MIS_DIAGNOSIS_NOTIFICATION:
    {
      if (pIn) delete pIn;
      pIn = new MISDiagnosisNotificationEvent(e);
      LOGMSGHDR(*pIn);
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
      }
      else
      {
        SEND_RESP_MSG((*pIn), DiagnosisResponse, "NAK (Component has no status or does not exists!)");
      }
      break;
    }
    case F_VGETRESP:
    {
      GCFPVInteger resourceState(RS_IDLE);
      GCFPropValueEvent* pE = (GCFPropValueEvent*)(&e);
      resourceState.copy(*pE->pValue); 
      string response  = _daemon.getPolicyHandler().checkDiagnose(*pIn, resourceState);
      if (response == "ACK")
      {
        _propertyProxy.setPropValue(resourceStatusPropName, resourceState);
        if (resourceStatusPropName.find(":") == string::npos)
        {
          resourceStatusPropName = GCFPVSSInfo::getLocalSystemName() + resourceStatusPropName;
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
      TRAN(MISSession::waiting);      
      break;
    }  
    case F_EXIT:
      if (pIn) delete pIn;
      pIn = 0;
      break;
      
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}


GCFEvent::TResult MISSession::reconfigure(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case MIS_RECONFIGURATION_REQUEST:
    {
      RETURN_NOACK_MSG(ReconfigurationRequest, ReconfigurationResponse, "NAK (not supported yet)");
      TRAN(MISSession::waiting);
      break;
    }       
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::getPICStructure(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
      
    case MIS_LOFAR_STRUCTURE_REQUEST:
    {
      RETURN_NOACK_MSG(LofarStructureRequest, LofarStructureResponse, "NAK (not supported yet)");
      TRAN(MISSession::waiting);
      break;
    }       
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::subscribe(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case MIS_PVSS_DP_SUBSCRIPTION_REQUEST:
    {
      MISPvssDpSubscriptionRequestEvent in(e);
      LOGMSGHDR(in);
      string response = "ACK";
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
          pNewSubscription->subscribe();
        }        
      }
      
      if (response != "ACK")
      {
        SEND_RESP_MSG(in, PvssDpSubscriptionResponse, response);
        TRAN(MISSession::waiting);
      }
      break;
    }  
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::getSubbandStatistics(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static std::bitset<MAX_N_RCUS> allRCUSMask;
  static MISSubbandStatisticsResponseEvent ackout;
  static MISSubbandStatisticsRequestEvent* pIn = 0;
  static uint16 nrOfRCUs = 0;

  switch (e.signal)
  {
    case F_ENTRY:
      if (nrOfRCUs == 0)
      {
        nrOfRCUs = (GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i));
        ackout.rcu_settingsNOE = nrOfRCUs;
        ackout.rcu_settings = new uint8[nrOfRCUs];
        ackout.invalidNOE = 0;// TODO: nrOfRCUs;
        ackout.invalid = new uint8[nrOfRCUs];
        ackout.dataNOE = nrOfRCUs * 512;
        ackout.data = new double[nrOfRCUs * 512];
        allRCUSMask.reset(); // init all bits to false value
        allRCUSMask.flip(); // flips all bits to the true value
        // if nrOfRCUs is less than MAX_N_RCUS the not used bits must be unset
        for (int i = nrOfRCUs; i < MAX_N_RCUS; i++)
          allRCUSMask.set(nrOfRCUs, false);
      }
      
      if (pIn) delete pIn;
      pIn = 0;
      break;
         
    case MIS_SUBBAND_STATISTICS_REQUEST:
    {
      pIn = new MISSubbandStatisticsRequestEvent(e);
      LOGMSGHDR((*pIn));

      RSPGetstatusEvent getstatus;
      getstatus.timestamp = Timestamp(0, 0);
      getstatus.cache = true;
      
      getstatus.rcumask = allRCUSMask;
      
      if (!_rspDriverPort.send(getstatus))
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (lost connection to rsp driver)");
        _rspDriverPort.open();
        TRAN(MISSession::waiting);      
      }
      break;
    }
    case RSP_GETSTATUSACK:
    {
      RSPGetstatusackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (error in ack of rspdriver)");
        TRAN(MISSession::waiting);      
        break;
      }

      //memcpy(ackout.invalid, ack.sysstatus.rcu().data(), nrOfRCUs * sizeof(ackout.invalid[0]));
     
      RSPGetrcuEvent getrcu;
      
      getrcu.timestamp = Timestamp(0, 0);
      getrcu.rcumask = allRCUSMask;
      getrcu.cache = true;
      if (!_rspDriverPort.send(getrcu))
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (lost connection to rsp driver)");
        _rspDriverPort.open();
        TRAN(MISSession::waiting);      
      }
      break;
    }
    case RSP_GETRCUACK:
    {
      RSPGetrcuackEvent ack(e);
      
      if (SUCCESS != ack.status)
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (error in ack of rspdriver)");
        TRAN(MISSession::waiting);      
        break;
      }

      memcpy(ackout.rcu_settings, ack.settings().data(), nrOfRCUs * sizeof(ackout.rcu_settings[0]));

      // subscribe to statistics updates
      RSPGetstatsEvent getstats;

      getstats.timestamp = Timestamp(0,0);

      getstats.rcumask = allRCUSMask;
      getstats.cache = true;
      getstats.type = Statistics::SUBBAND_POWER;
      
      if (!_rspDriverPort.send(getstats))
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (lost connection to rsp driver)");
        _rspDriverPort.open();
        TRAN(MISSession::waiting);      
      }
      break;
    }  
    case RSP_GETSTATSACK:
    {
      RSPGetstatsackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        SEND_RESP_MSG((*pIn), SubbandStatisticsResponse, "NAK (error in ack of rspdriver)");
        TRAN(MISSession::waiting);      
        break;
      }

      ackout.seqnr = _curSeqNr++;
      ackout.replynr = _curReplyNr;
      memcpy(ackout.data, ack.stats().data(), nrOfRCUs * sizeof(ackout.data[0]) * sizeof(double));

      ackout.response = "ACK";
      ackout.payload_timestamp_sec = ack.timestamp.sec();
      ackout.payload_timestamp_nsec = ack.timestamp.usec() * 1000;
      setCurrentTime(ackout.timestamp_sec, ackout.timestamp_nsec);
      _missPort.send(ackout);
      TRAN(MISSession::waiting);
      break;
    }
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::getAntennaCorrelation(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
      
    case MIS_ANTENNA_CORRELATION_MATRIX_REQUEST:
    {
      RETURN_NOACK_MSG(AntennaCorrelationMatrixRequest, AntennaCorrelationMatrixResponse, "NAK (not supported yet)");
      TRAN(MISSession::waiting);
      break;
    }       
    default:
      status = defaultHandling(e, p);
      break;
  }

  return status;
}

GCFEvent::TResult MISSession::closing(GCFEvent& e, GCFPortInterface& /*p*/)
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
        TRAN(MISSession::closing);
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
  TRAN(MISSession::waiting);
}

void MISSession::valueChanged(MISPvssDpSubscriptionValueChangedAsyncEvent& e)
{
  e.seqnr = _curSeqNr++;
  _missPort.send(e);
}

void MISSession::mayDelete(const string& propName)
{
  TSubscriptions::iterator iter = _subscriptions.find(propName);
  ASSERTSTR(iter != _subscriptions.end(), "Subscription should still exists here!");
  MISSubscription* pSubs = iter->second;
  _subscriptions.erase(propName);
  _subscriptionsGarbage.push_back(pSubs);
}

void MISSession::setCurrentTime(int64& sec, uint32& nsec)
{
  Timestamp timestamp;
  timestamp.setNow();
  sec = timestamp.sec();
  nsec = timestamp.usec() * 1000;
}
 } // namespace AMI
} // namespace LOFAR
