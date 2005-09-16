//#  StationOperations.cc: Implementation of the StationOperations task
//#
//#  Copyright (C) 2002-2004
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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <boost/shared_ptr.hpp>
#include <Common/lofar_sstream.h>
#include <GCF/GCF_PVDouble.h>
#include <APLCommon/APLUtilities.h>
#include <StationOperations/StationOperations.h>

#include "RSP_Protocol.ph"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace ASO
{
INIT_TRACER_CONTEXT(StationOperations,LOFARLOGGER_PACKAGE);

PROPERTYCONFIGLIST_BEGIN(detailsPropertySetConf)
  PROPERTYCONFIGLIST_ITEM(PROPERTY_SAMPLING_FREQUENCY, GCF_READABLE_PROP, "160000000")
PROPERTYCONFIGLIST_END

string StationOperations::m_RSPDriverName("RSPDriver");

// Logical Device version
const string StationOperations::SO_VERSION = string("1.0");

StationOperations::StationOperations(const string& taskName, 
                                     const string& parameterFile, 
                                     GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,SO_VERSION),
  m_RSPclient(*this, m_RSPDriverName, GCFPortInterface::SAP, RSP_PROTOCOL),
  m_ntdboards(0),
  m_connectTimer(0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  m_detailsPropertySet->initProperties(detailsPropertySetConf);
}


StationOperations::~StationOperations()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationOperations::sendSamplingFrequency(double samplingFrequency)
{
  LOG_INFO("Sending sampling frequency to RSP driver");
  RSPSetclocksEvent setClocksEvent;
  setClocksEvent.timestamp.setNow();
  setClocksEvent.tdmask.reset(); // reset every bit
  for(int16 td=0;td<m_ntdboards;td++)
  {
    setClocksEvent.tdmask.set(td);
  }
  if(m_ntdboards > 0)
  {
    setClocksEvent.clocks().resize(m_ntdboards);
    setClocksEvent.clocks() = static_cast<uint32>(samplingFrequency);
    m_RSPclient.send(setClocksEvent);
  }
}

void StationOperations::concrete_handlePropertySetAnswer(GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(answer)).c_str());
  switch(answer.signal)
  {
    case F_SUBSCRIBED:
    {
      break;
    }
    case F_UNSUBSCRIBED:
    {
      break;
    }
    case F_VGETRESP:
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      if(strstr(pPropAnswer->pPropName, PROPERTY_SAMPLING_FREQUENCY) != 0)
      {
        TLogicalDeviceState curState = getLogicalDeviceState();
        if(curState == LOGICALDEVICE_STATE_SUSPENDED || curState == LOGICALDEVICE_STATE_ACTIVE)
        {
          double samplingFrequency = (static_cast<const GCFPVDouble*>(pPropAnswer->pValue))->getValue();
          sendSamplingFrequency(samplingFrequency);
        }
      }
      break;
    }
    case F_EXTPS_LOADED:
    {
      break;
    }
    case F_EXTPS_UNLOADED:
    {
      break;
    }
    case F_PS_CONFIGURED:
    {
      break;
    }
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        TLogicalDeviceState curState = getLogicalDeviceState();
        if(curState == LOGICALDEVICE_STATE_SUSPENDED || curState == LOGICALDEVICE_STATE_ACTIVE)
        {
          boost::shared_ptr<GCFPVDouble> pvSamplingFrequency(static_cast<GCFPVDouble*>(m_detailsPropertySet->getValue(PROPERTY_SAMPLING_FREQUENCY)));
          double samplingFrequency = pvSamplingFrequency->getValue();
          sendSamplingFrequency(samplingFrequency);
        }
      }
      break;
    }
    case F_MYPS_DISABLED:
    {
      break;
    }
    case F_SERVER_GONE:
    {
      break;
    }
    default:
      break;
  }
}

GCFEvent::TResult StationOperations::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*port*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:
    {
      newState=LOGICALDEVICE_STATE_IDLE;
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult StationOperations::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult StationOperations::concrete_claiming_state(GCFEvent& event, GCFPortInterface& port, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      bool res=m_RSPclient.open(); // need this otherwise GTM_Sockethandler is not called
      LOG_DEBUG(formatString("m_RSPclient.open() returned %s",(res?"true":"false")));
      if(!res)
      {
        m_connectTimer = m_RSPclient.setTimer((long)3);
      }  
      break;
    }

    case F_CONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
      
      // get rsp config
      RSPGetconfigEvent getconfig;
      m_RSPclient.send(getconfig);
      break;
    }
      
    case F_DISCONNECTED:
    {
      m_connectTimer = m_RSPclient.setTimer((long)3); // try again in 3 seconds
      LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
      m_RSPclient.close();
      break;
    }

    case RSP_GETCONFIGACK:
    {
      RSPGetconfigackEvent ack(event);
      m_ntdboards = ack.n_tdboards;
      LOG_INFO(formatString("n_rcus     =%d",ack.n_rcus));
      LOG_INFO(formatString("n_rspboards=%d",ack.n_rspboards));
      LOG_INFO(formatString("n_tdboards =%d",m_ntdboards));
      newState=LOGICALDEVICE_STATE_CLAIMED;
      break;
    }


    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  
  return status;
}

GCFEvent::TResult StationOperations::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*port*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
    
    case F_DISCONNECTED:
    {
      newState=LOGICALDEVICE_STATE_CLAIMING;
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult StationOperations::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*port*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
          
    case RSP_SETCLOCKSACK:
    {
      RSPSetclocksackEvent ack(event);
      if (SUCCESS == ack.status)
      {
        newState=LOGICALDEVICE_STATE_SUSPENDED;
      }
      else
      {
        newState=LOGICALDEVICE_STATE_CLAIMING;
        errorCode = LD_RESULT_SETCLOCKS_ERROR;
      }
      break;
    }

    case F_DISCONNECTED:
    {
      newState=LOGICALDEVICE_STATE_CLAIMING;
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult StationOperations::concrete_active_state(GCFEvent& event, GCFPortInterface& port, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
          
    case RSP_SETCLOCKSACK:
    {
      RSPSetclocksackEvent ack(event);
      if (SUCCESS != ack.status)
      {
        errorCode = LD_RESULT_SETCLOCKS_ERROR;
      }
      break;
    }

    case F_DISCONNECTED:
    {
      LOG_ERROR(formatString("port '%s' disconnected", port.getName().c_str()));
      _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED,LD_RESULT_LOW_QUALITY);
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult StationOperations::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_IDLE;

  switch (event.signal)
  {
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

void StationOperations::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
}

void StationOperations::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  double samplingFrequency = m_parameterSet.getDouble(PROPERTY_SAMPLING_FREQUENCY);
  m_detailsPropertySet->setValue(PROPERTY_SAMPLING_FREQUENCY,GCFPVDouble(samplingFrequency));
  sendSamplingFrequency(samplingFrequency);
}

void StationOperations::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationOperations::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationOperations::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  m_RSPclient.close();
}

void StationOperations::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationOperations::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationOperations::concreteHandleTimers(GCFTimerEvent& timerEvent, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  if(timerEvent.id == m_connectTimer)
  {
    m_RSPclient.open();
  }
}

void StationOperations::concreteAddExtraKeys(ACC::APS::ParameterSet& psSubset)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
}


}; // namespace ASO
}; // namespace LOFAR

