//#  VirtualRoute.cc: Implementation of the VirtualRoute task
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
#include <GCF/GCF_PVString.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/VirtualRoute/VirtualRoute.h>

#include <APL/APLCommon/LogicalDevice_Protocol.ph>

#include "WanLSPropertyProxy.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;

  
namespace AVR
{
INIT_TRACER_CONTEXT(VirtualRoute,LOFARLOGGER_PACKAGE);

// Logical Device version
const string VirtualRoute::VR_VERSION = string("1.0");

VirtualRoute::VirtualRoute(const string& taskName, 
                           const string& parameterFile, 
                           GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,VR_VERSION),
  m_requiredBandwidth(0.0),
  m_logicalSegments(),
  m_lsProps(),
  m_qualityCheckTimerId(0),
  m_lsCapacityCheckTimer(0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
 
  m_requiredBandwidth = m_parameterSet.getDouble("requiredBandwidth");
  m_logicalSegments = m_parameterSet.getStringVector("logicalSegments");
  string wanLcu = m_parameterSet.getString("WANLcu");
  
  for(vector<string>::iterator it=m_logicalSegments.begin();it!=m_logicalSegments.end();++it)
  {
    // create logical segment propertyset
    string psName(wanLcu + string(":") + VR_LOGICALSEGMENT_PROPSET_BASENAME + (*it));
    WanLSPropertyProxyPtr pps(new WanLSPropertyProxy(psName));

    m_lsProps.insert(map<string,WanLSPropertyProxyPtr>::value_type(*it,pps));
  }
}


VirtualRoute::~VirtualRoute()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualRoute::concrete_handlePropertySetAnswer(GCFEvent& answer)
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
      break;
    }
    case F_EXTPS_LOADED:
    {
      //GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
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

GCFEvent::TResult VirtualRoute::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult VirtualRoute::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult VirtualRoute::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // immediately enter claimed state
      newState  = LOGICALDEVICE_STATE_CLAIMED;
      errorCode = LD_RESULT_NO_ERROR;
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualRoute::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  return status;
}

GCFEvent::TResult VirtualRoute::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // start capacity updates
      m_lsCapacityCheckTimer = m_serverPort.setTimer(10L);
      newState=LOGICALDEVICE_STATE_SUSPENDED;
      errorCode = LD_RESULT_NO_ERROR;
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualRoute::concrete_active_state(GCFEvent& event, GCFPortInterface& /*p*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // set allocated value changes
      for(map<string,WanLSPropertyProxyPtr>::iterator it=m_lsProps.begin();it!=m_lsProps.end();++it)
      {
        it->second->changeAllocated(m_requiredBandwidth);
      }
      m_qualityCheckTimerId = m_serverPort.setTimer(10L);
      LOG_DEBUG(formatString("qualityCheckTimerId=%d",m_qualityCheckTimerId));
      break;
    }
    
    case F_EXIT:
    {
      m_serverPort.cancelTimer(m_qualityCheckTimerId);
      m_serverPort.cancelTimer(m_lsCapacityCheckTimer);
      m_qualityCheckTimerId=0;
      m_lsCapacityCheckTimer=0;

      // reset allocated value changes
      for(map<string,WanLSPropertyProxyPtr>::iterator it=m_lsProps.begin();it!=m_lsProps.end();++it)
      {
        it->second->changeAllocated(-1.0 * m_requiredBandwidth);
      }
      break;
    }
          
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualRoute::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_GOINGDOWN;
  
  switch(event.signal)
  {
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }    
  return status;
}

void VirtualRoute::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
}

void VirtualRoute::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualRoute::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualRoute::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

}

void VirtualRoute::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualRoute::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualRoute::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualRoute::concreteHandleTimers(GCFTimerEvent& timerEvent, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  if(timerEvent.id == m_qualityCheckTimerId)
  {
    if(!_checkQualityRequirements())
    {
      LOG_FATAL(formatString("VR(%s): quality too low",getName().c_str()));
      m_serverPort.cancelTimer(m_qualityCheckTimerId);
      m_qualityCheckTimerId=0;
      suspend(LD_RESULT_LOW_QUALITY);
    }
    else
    {
      // keep on polling
      m_qualityCheckTimerId = m_serverPort.setTimer(10L);
    }
  }
  else if(timerEvent.id == m_lsCapacityCheckTimer)
  {
    map<string,WanLSPropertyProxyPtr>::iterator propIt;
    for(propIt=m_lsProps.begin();propIt!=m_lsProps.end();++propIt)
    {
      propIt->second->updateCapacity();
    }
    m_lsCapacityCheckTimer = m_serverPort.setTimer(10L);
  }
}

bool VirtualRoute::_checkQualityRequirements()
{
  bool qualityOk=true;
  map<string,WanLSPropertyProxyPtr>::iterator it = m_lsProps.begin();
  while(qualityOk && it != m_lsProps.end())
  {
    if(it->second->getCapacity() < m_requiredBandwidth)
    {
      LOG_FATAL(formatString("Capacity of Virtual Route %s dropped below required bandwidth (%.4f < %.4f)",getName().c_str(),it->second->getCapacity(),m_requiredBandwidth));
      qualityOk=false;
    }
    ++it;
  }
  return qualityOk;
}

void VirtualRoute::concreteAddExtraKeys(ACC::APS::ParameterSet& /*psSubset*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // // copy samplingFrequency from AO section to child section
  // copyParentValue(psSubset,string("samplingFrequency"));
}

}; // namespace AVR
}; // namespace LOFAR

