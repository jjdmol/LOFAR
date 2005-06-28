//#  VirtualTelescope.cc: Implementation of the VirtualTelescope task
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
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>
#include <GCF/ParameterSet.h>
#include <APLCommon/APLUtilities.h>
#include <VirtualTelescope/VirtualTelescope.h>
#include "ABS_Protocol.ph"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace AVT
{
INIT_TRACER_CONTEXT(VirtualTelescope,LOFARLOGGER_PACKAGE);

// Logical Device version
const string VirtualTelescope::VT_VERSION = string("1.0");
const string VirtualTelescope::PARAM_BEAMSERVERPORT = string("mac.apl.avt.BEAMSERVERPORT");
const string VirtualTelescope::DIRECTIONTYPE_J2000 = string("J2000");
const string VirtualTelescope::DIRECTIONTYPE_AZEL = string("AZEL");
const string VirtualTelescope::DIRECTIONTYPE_LMN = string("LMN");


VirtualTelescope::VirtualTelescope(const string& taskName, 
                                     const string& parameterFile, 
                                     GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,VT_VERSION),
  m_beamServer(),
  m_beamID(0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);
}


VirtualTelescope::~VirtualTelescope()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

}

bool VirtualTelescope::_isBeamServerPort(GCFPortInterface& port) const
{
  return (&port == &m_beamServer); // comparing two pointers. yuck?
}

int VirtualTelescope::_convertDirection(const string type) const
{
  int direction=2; // DIRECTIONTYPE_AZEL
  if(type==string(DIRECTIONTYPE_J2000))
  {
    direction=1;
  }
  else if(type==string(DIRECTIONTYPE_LMN))
  {
    direction=3;
  }
  return direction;
}

void VirtualTelescope::concrete_handlePropertySetAnswer(GCFEvent& answer)
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

GCFEvent::TResult VirtualTelescope::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:
    {
      string bsName = GCF::ParameterSet::instance()->getString(PARAM_BEAMSERVERPORT);
      m_beamServer.init(*this, bsName, GCFPortInterface::SAP, ABS_PROTOCOL);
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualTelescope::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult VirtualTelescope::concrete_claiming_state(GCFEvent& event, GCFPortInterface& port, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
    
    case F_CONNECTED:
    {
      if(_isBeamServerPort(port))
      {
        newState = LOGICALDEVICE_STATE_CLAIMED;
      }
      break;
    }

    case F_DISCONNECTED:
      if(_isBeamServerPort(port))
      {
        m_beamServer.setTimer(2.0); // try again
      }
      break;

    case F_TIMER:
      if(_isBeamServerPort(port))
      {
        m_beamServer.open(); // try again
      }
      break;

    default:
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualTelescope::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  return status;
}

GCFEvent::TResult VirtualTelescope::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case ABS_BEAMALLOC_ACK: 
      // prepared event is received from the beam server
      if(_isBeamServerPort(port))
      {
        // check the beam ID and status of the ACK message
        ABSBeamallocAckEvent ackEvent(event);
        if(ackEvent.status==0)
        {
          m_beamID=ackEvent.handle;
          
          int directionType=_convertDirection(m_parameterSet.getString(string("directionType")));
          double directionAngle1=m_parameterSet.getDouble(string("angle1"));
          double directionAngle2=m_parameterSet.getDouble(string("angle2"));
  
          // point the new beam
          time_t time_arg(0); // now
          ABSBeampointtoEvent beamPointToEvent;
          beamPointToEvent.handle = m_beamID;
          beamPointToEvent.time   = time_arg;
          beamPointToEvent.type   = directionType;
          beamPointToEvent.angle[0] = directionAngle1;
          beamPointToEvent.angle[1] = directionAngle2;
          m_beamServer.send(beamPointToEvent);
          newState = LOGICALDEVICE_STATE_SUSPENDED;
        }
        else
        {
          newState = LOGICALDEVICE_STATE_CLAIMED;
          errorCode = LD_RESULT_BEAMALLOC_ERROR;
        }
      }
      break;
    
    default:
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualTelescope::concrete_active_state(GCFEvent& event, GCFPortInterface& /*p*/, TLDResult& /*errorCode*/)
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

GCFEvent::TResult VirtualTelescope::concrete_releasing_state(GCFEvent& event, GCFPortInterface& port, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case ABS_BEAMFREE_ACK: 
      if(_isBeamServerPort(port))
      {
        // check the beam ID and status of the ACK message
        ABSBeamfreeAckEvent ackEvent(event);
        if(ackEvent.status==0 && ackEvent.handle == m_beamID)
        {
        }
        else
        {
          errorCode = LD_RESULT_BEAMFREE_ERROR;
        }
        m_beamServer.close();
        newState=LOGICALDEVICE_STATE_GOINGDOWN;
      }
      break;
    
    default:
      break;
  }
  
  return status;
}

void VirtualTelescope::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  m_beamServer.open();
}

void VirtualTelescope::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  vector<int> subbandsVector;
  APLUtilities::string2Vector(m_parameterSet.getString(string("subbands")),subbandsVector);
  
  int spectral_window = m_parameterSet.getInt(string("spectralWindow"));
  int n_subbands(subbandsVector.size());
  
  int subbandsArray[MEPHeader::N_BEAMLETS];
  char tempLogStr[1000];
  tempLogStr[0]=0;
  
  memset(subbandsArray,0,sizeof(subbandsArray[0])*MEPHeader::N_BEAMLETS);
  vector<int>::iterator vectorIterator=subbandsVector.begin();
  int arrayIndex(0);
  while(arrayIndex<MEPHeader::N_BEAMLETS && vectorIterator!=subbandsVector.end())
  {
    subbandsArray[arrayIndex]=*vectorIterator;
    char tempStr[10];
    sprintf(tempStr,"%d ",subbandsArray[arrayIndex]);
    strcat(tempLogStr,tempStr);
    ++arrayIndex;
    ++vectorIterator;
  }
  
  
  LOG_DEBUG(formatString("VirtualTelescope(%s)::allocate %d subbands: %s",getName().c_str(),n_subbands,tempLogStr));
  ABSBeamallocEvent beamAllocEvent;
  beamAllocEvent.spectral_window = spectral_window;
  beamAllocEvent.n_subbands = n_subbands;
  memcpy(beamAllocEvent.subbands,subbandsArray,sizeof(int)*MEPHeader::N_BEAMLETS);
  m_beamServer.send(beamAllocEvent);
}

void VirtualTelescope::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualTelescope::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualTelescope::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  ABSBeamfreeEvent beamFreeEvent;
  beamFreeEvent.handle=m_beamID;
  m_beamServer.send(beamFreeEvent);
}

void VirtualTelescope::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualTelescope::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualTelescope::concreteHandleTimers(GCFTimerEvent& /*timerEvent*/, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}


}; // namespace AVT
}; // namespace LOFAR

