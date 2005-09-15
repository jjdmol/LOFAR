//#  VirtualInstrument.cc: Implementation of the Virtual VirtualInstrument task
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
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>
#include <APLCommon/APLUtilities.h>
#include "VirtualInstrument.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace AVI
{
INIT_TRACER_CONTEXT(VirtualInstrument,LOFARLOGGER_PACKAGE);

// Logical Device version
const string VirtualInstrument::VI_VERSION = string("1.0");

const string VirtualInstrument::VI_PROPNAME_CONNECTEDSTATIONS     = string("connectedStations");
const string VirtualInstrument::VI_PROPNAME_DISCONNECTEDSTATIONS  = string("disconnectedStations");

VirtualInstrument::VirtualInstrument(const string& taskName, 
                                     const string& parameterFile, 
                                     GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,VI_VERSION),
  m_qualityCheckTimerId(0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}


VirtualInstrument::~VirtualInstrument()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concrete_handlePropertySetAnswer(GCFEvent& answer)
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

GCFEvent::TResult VirtualInstrument::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult VirtualInstrument::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult VirtualInstrument::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
    
    case LOGICALDEVICE_CLAIMED:
    {
      LOG_TRACE_FLOW("CLAIMED received");
      // check if all childs are not claiming anymore
      // now only checking VB, because VT's are old style
      if(_childsNotInState(100.0, LDTYPE_VIRTUALBACKEND/*LDTYPE_NO_TYPE*/, LOGICALDEVICE_STATE_CLAIMING))
      {
        LOG_TRACE_FLOW("No childs CLAIMING");
        // ALL virtual backend childs must be claimed
        if(_childsInState(100.0, LDTYPE_VIRTUALBACKEND, LOGICALDEVICE_STATE_CLAIMED))
        {
          LOG_TRACE_FLOW("100% VB's CLAIMED");
          // 50% of the VT's must be claimed
          if(_childsInState(50.0, LDTYPE_VIRTUALTELESCOPE, LOGICALDEVICE_STATE_CLAIMED))
          {
            LOG_TRACE_FLOW("50% VT's CLAIMED");
            // enter claimed state
            newState  = LOGICALDEVICE_STATE_CLAIMED;
            errorCode = LD_RESULT_NO_ERROR;
          }
          else
          {
            newState  = LOGICALDEVICE_STATE_IDLE;
            errorCode = LD_RESULT_LOW_QUALITY;
          }
        }
        else
        {
          newState  = LOGICALDEVICE_STATE_IDLE;
          errorCode = LD_RESULT_LOW_QUALITY;
        }
      }
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
    {
      LOG_TRACE_FLOW("PREPARED received");
      // check if all childs are not preparing anymore
      // now only checking VB, because VT's are old style
      if(_childsNotInState(100.0, LDTYPE_VIRTUALBACKEND/*LDTYPE_NO_TYPE*/, LOGICALDEVICE_STATE_PREPARING))
      {
        LOG_TRACE_FLOW("No childs PREPARING");
        // ALL virtual backend childs must be prepared
        if(_childsInState(100.0, LDTYPE_VIRTUALBACKEND, LOGICALDEVICE_STATE_SUSPENDED))
        {
          LOG_TRACE_FLOW("All VB's SUSPENDED");
          // 00% of the VT's must be prepared
          if(_childsInState(50.0, LDTYPE_VIRTUALTELESCOPE, LOGICALDEVICE_STATE_SUSPENDED))
          {
            LOG_TRACE_FLOW("50% VT's SUSPENDED");
            // enter suspended state
            newState=LOGICALDEVICE_STATE_SUSPENDED;
          }
          else
          {
            newState  = LOGICALDEVICE_STATE_CLAIMED;
            errorCode = LD_RESULT_LOW_QUALITY;
          }
        }
        else
        {
          newState  = LOGICALDEVICE_STATE_CLAIMED;
          errorCode = LD_RESULT_LOW_QUALITY;
        }
      }
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_active_state(GCFEvent& event, GCFPortInterface& /*p*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      m_qualityCheckTimerId = m_serverPort.setTimer(10L);
      LOG_DEBUG(formatString("qualityCheckTimerId=%d",m_qualityCheckTimerId));
      break;
    }
          
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

//  newState=LOGICALDEVICE_STATE_IDLE;
  newState=LOGICALDEVICE_STATE_GOINGDOWN;

  switch(event.signal)
  {
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

void VirtualInstrument::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
}

void VirtualInstrument::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteHandleTimers(GCFTimerEvent& timerEvent, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  if(timerEvent.id == m_qualityCheckTimerId)
  {
    if(!_checkQualityRequirements())
    {
      LOG_FATAL(formatString("VI(%s): quality too low",getName().c_str()));
      m_serverPort.cancelTimer(m_qualityCheckTimerId);
      m_qualityCheckTimerId=0;
      _cancelSchedule(LD_RESULT_LOW_QUALITY);
    }
    else
    {
      // keep on polling
      m_qualityCheckTimerId = m_serverPort.setTimer(5L);
    }
  }
}

bool VirtualInstrument::_checkQualityRequirements()
{
  bool requirementsMet = false;
  
  // ALL virtual backend childs must be active
  if(_childsInState(100.0, LDTYPE_VIRTUALBACKEND, LOGICALDEVICE_STATE_ACTIVE))
  {
    LOG_TRACE_FLOW("All VB's RESUMED");
    // 00% of the VT's must be active
    if(_childsInState(50.0, LDTYPE_VIRTUALTELESCOPE, LOGICALDEVICE_STATE_ACTIVE))
    {
      LOG_TRACE_FLOW("50% VT's RESUMED");
      requirementsMet=true;
    }
  }
  LOG_DEBUG(formatString("checkQualityRequirements returns %s",(requirementsMet?"true":"false"))); 
  return requirementsMet;
}

}; // namespace VIC
}; // namespace LOFAR

