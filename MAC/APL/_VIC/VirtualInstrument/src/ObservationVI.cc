//#  ObservationVI.cc: Implementation of the Observation VirtualInstrument task
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
#include <APLCommon/APLUtilities.h>
#include "ObservationVI.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace AVI
{
INIT_TRACER_CONTEXT(ObservationVI,LOFARLOGGER_PACKAGE);

// Logical Device version
const string ObservationVI::VI_VERSION = string("1.0");

ObservationVI::ObservationVI(const string& taskName, 
                                     const string& parameterFile, 
                                     GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,VI_VERSION)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}


ObservationVI::~ObservationVI()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ObservationVI::concrete_handlePropertySetAnswer(GCFEvent& answer)
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

GCFEvent::TResult ObservationVI::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult ObservationVI::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult ObservationVI::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // enter claimed state
      newState  = LOGICALDEVICE_STATE_CLAIMED;
      errorCode = LD_RESULT_NO_ERROR;
      break;
    }
    
    default:
      break;
  }
  
  return status;
}

GCFEvent::TResult ObservationVI::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  return status;
}

GCFEvent::TResult ObservationVI::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // preparing is not necessary
      // enter suspended state
      newState=LOGICALDEVICE_STATE_SUSPENDED;
      errorCode = LD_RESULT_NO_ERROR;
      break;
    }
    
    default:
      break;
  }
  
  return status;
}

GCFEvent::TResult ObservationVI::concrete_active_state(GCFEvent& event, GCFPortInterface& /*p*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  return status;
}

GCFEvent::TResult ObservationVI::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  newState=LOGICALDEVICE_STATE_GOINGDOWN;
  return status;
}

void ObservationVI::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ObservationVI::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ObservationVI::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ObservationVI::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ObservationVI::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ObservationVI::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ObservationVI::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ObservationVI::concreteHandleTimers(GCFTimerEvent& /*timerEvent*/, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

}

}; // namespace VIC
}; // namespace LOFAR

