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
#include <lofar_config.h>

#include <boost/shared_ptr.hpp>
#include <Common/lofar_sstream.h>
#include <Common/LofarLogger.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>
#include <APLCommon/APLUtilities.h>
#include <ArrayReceptorGroup/ArrayReceptorGroup.h>

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
  LogicalDevice(taskName,parameterFile,pStartDaemon,SRG_VERSION)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}


ArrayReceptorGroup::~ArrayReceptorGroup()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
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
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      break;
    }
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
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
          if(_childsInState(00.0, LDTYPE_VIRTUALTELESCOPE, LOGICALDEVICE_STATE_CLAIMED))
          {
            LOG_TRACE_FLOW("00% VT's CLAIMED");
            
            LOG_TRACE_FLOW("need to check old style VT's");
            
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
      break;
  }
  
  return status;
}

GCFEvent::TResult ArrayReceptorGroup::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  return status;
}

GCFEvent::TResult ArrayReceptorGroup::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

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
          if(_childsInState(00.0, LDTYPE_VIRTUALTELESCOPE, LOGICALDEVICE_STATE_SUSPENDED))
          {
            LOG_TRACE_FLOW("00% VT's SUSPENDED");
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
      break;
  }
  
  return status;
}

GCFEvent::TResult ArrayReceptorGroup::concrete_active_state(GCFEvent& event, GCFPortInterface& /*p*/, TLDResult& /*errorCode*/)
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

GCFEvent::TResult ArrayReceptorGroup::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  newState=LOGICALDEVICE_STATE_IDLE;
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
}

}; // namespace VIC
}; // namespace LOFAR

