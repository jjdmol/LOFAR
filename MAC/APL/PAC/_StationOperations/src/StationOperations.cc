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
#include <boost/shared_ptr.hpp>
#include <Common/lofar_sstream.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
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
  PROPERTYCONFIGLIST_ITEM(PROPERTY_SAMPLING_FREQUENCY, GCF_READABLE_PROP, "160.0")
PROPERTYCONFIGLIST_END

string StationOperations::m_RSPserverName("RSPserver");

// Logical Device version
const string StationOperations::SO_VERSION = string("1.0");

StationOperations::StationOperations(const string& taskName, 
                                     const string& parameterFile, 
                                     GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,SO_VERSION),
  m_RSPclient(*this, m_RSPserverName, GCFPortInterface::SAP, RSP_PROTOCOL)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  m_detailsPropertySet->initProperties(detailsPropertySetConf);
}


StationOperations::~StationOperations()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
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
        double samplingFrequency = (static_cast<const GCFPVDouble*>(pPropAnswer->pValue))->getValue();

        LOG_FATAL("TODO: Send sampling frequency to RSP driver");
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
        boost::shared_ptr<GCFPVDouble> pvSamplingFrequency(static_cast<GCFPVDouble*>(m_detailsPropertySet->getValue(PROPERTY_SAMPLING_FREQUENCY)));
        double samplingFrequency = pvSamplingFrequency->getValue();
          
        LOG_FATAL("TODO: Send sampling frequency to RSP driver");
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

GCFEvent::TResult StationOperations::concrete_initial_state(GCFEvent& event, GCFPortInterface& port, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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
        m_RSPclient.setTimer((long)3);
      }  
      break;
    }
    
    case F_CONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
      if (m_RSPclient.isConnected())
      {
        newState=LOGICALDEVICE_STATE_IDLE;
      }
      break;
    }

    case F_DISCONNECTED:
    {
      port.setTimer((long)3); // try again in 3 seconds
      LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
      port.close();
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
    {
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult StationOperations::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
    
    case LOGICALDEVICE_CLAIMED:
    {
      LOG_TRACE_FLOW("CLAIMED received");
      break;
    }
    
    default:
      break;
  }
  
  return status;
}

GCFEvent::TResult StationOperations::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  return status;
}

GCFEvent::TResult StationOperations::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    // TODO: case RSP_SETSAMPLINGFREQUENCYACK
    // TODO:   ack ok? newState=LOGICALDEVICE_STATE_SUSPENDED;
    // TODO:   nack?   errorCode=?????;
    // TODO:   break;
    default:
      break;
  }
  
  return status;
}

GCFEvent::TResult StationOperations::concrete_active_state(GCFEvent& event, GCFPortInterface& /*p*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
          
    default:
      break;
  }  
  return status;
}

GCFEvent::TResult StationOperations::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  newState=LOGICALDEVICE_STATE_IDLE;
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
  
  LOG_FATAL("TODO: Send sampling frequency to RSP driver");
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
}

void StationOperations::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationOperations::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationOperations::concreteHandleTimers(GCFTimerEvent& /*timerEvent*/, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

}; // namespace VIC
}; // namespace LOFAR

