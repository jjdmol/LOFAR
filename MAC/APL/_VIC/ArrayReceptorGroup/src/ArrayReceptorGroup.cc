//#  ArrayReceptorGroup.cc: Implementation of the ArrayReceptorGroup task
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
#include <APL/APLCommon/APLUtilities.h>
#include <APL/ArrayReceptorGroup/ArrayReceptorGroup.h>

#include <APL/APLCommon/LogicalDevice_Protocol.ph>

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace AAR
{
INIT_TRACER_CONTEXT(ArrayReceptorGroup,LOFARLOGGER_PACKAGE);

// Logical Device version
const string ArrayReceptorGroup::SRG_VERSION = string("1.0");

ArrayReceptorGroup::ArrayReceptorGroup(const string& taskName, 
                                     const string& parameterFile, 
                                     GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,SRG_VERSION),
  m_delayedQualityCheckTimer(0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}


ArrayReceptorGroup::~ArrayReceptorGroup()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

bool ArrayReceptorGroup::_checkQuality()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  //todo: implement quality rule: how many srg's may be suspended before the ARG is suspended
  return (_childsInState(100.0, LDTYPE_NO_TYPE, LOGICALDEVICE_STATE_ACTIVE));
}

void ArrayReceptorGroup::concrete_handlePropertySetAnswer(GCFEvent& answer)
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
      //      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      break;
    }
    case F_EXTPS_LOADED:
    {
      //      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
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

GCFEvent::TResult ArrayReceptorGroup::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult ArrayReceptorGroup::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult ArrayReceptorGroup::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
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
      // check if all childs are claimed
      if(_childsInState(100.0, LDTYPE_NO_TYPE, LOGICALDEVICE_STATE_CLAIMED))
      {
        // enter claimed state
        newState  = LOGICALDEVICE_STATE_CLAIMED;
        errorCode = LD_RESULT_NO_ERROR;
      }
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult ArrayReceptorGroup::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
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

GCFEvent::TResult ArrayReceptorGroup::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
    {
      LOG_TRACE_FLOW("PREPARED received");
      // check if all childs are prepared
      if(_childsInState(100.0, LDTYPE_NO_TYPE, LOGICALDEVICE_STATE_SUSPENDED))
      {
        // enter prepared state
        newState  = LOGICALDEVICE_STATE_SUSPENDED;
        errorCode = LD_RESULT_NO_ERROR;
      }
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult ArrayReceptorGroup::concrete_active_state(GCFEvent& event, GCFPortInterface& p, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }

    case LOGICALDEVICE_SUSPENDED:
    {
      m_delayedQualityCheckTimer = p.setTimer(1L);
      break;
    }     

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult ArrayReceptorGroup::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

void ArrayReceptorGroup::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
}

void ArrayReceptorGroup::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ArrayReceptorGroup::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ArrayReceptorGroup::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ArrayReceptorGroup::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ArrayReceptorGroup::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ArrayReceptorGroup::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void ArrayReceptorGroup::concreteHandleTimers(GCFTimerEvent& timerEvent, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  if(timerEvent.id == m_delayedQualityCheckTimer)
  {
    if(!_checkQuality())
    {
      suspend(LD_RESULT_LOW_QUALITY);
    }
  }
}

void ArrayReceptorGroup::concreteAddExtraKeys(ACC::APS::ParameterSet& psSubset)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  copyParentValue(psSubset,string("rcus"));
  copyParentValue(psSubset,string("antennaArray"));
  copyParentValue(psSubset,string("frequencyTimes"));
  copyParentValue(psSubset,string("frequency"));
  copyParentValue(psSubset,string("nyquistZone"));
  copyParentValue(psSubset,string("bandSelection"));
  copyParentValue(psSubset,string("maxRcusDefect"));
}

}; // namespace VIC
}; // namespace LOFAR

