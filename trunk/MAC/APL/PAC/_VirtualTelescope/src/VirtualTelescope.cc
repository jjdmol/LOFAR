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
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <boost/shared_ptr.hpp>
#include <Common/lofar_sstream.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>
#include <GCF/ParameterSet.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/VirtualTelescope/VirtualTelescope.h>
#include <APL/BS_Protocol/BS_Protocol.ph>

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
const string VirtualTelescope::PARAM_BEAMSERVER_REMOTETASK = string("mac.apl.sd.remoteservice.task.BeamServer");
const string VirtualTelescope::PARAM_BEAMSERVER_REMOTEPORT = string("mac.apl.sd.remoteservice.port.BeamServer");
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
  registerProtocol(BS_PROTOCOL, BS_PROTOCOL_signalnames);
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
  if(type==string(DIRECTIONTYPE_AZEL))
  {
    direction=2;
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
      string bsRemoteTask = GCF::ParameterSet::instance()->getString(PARAM_BEAMSERVER_REMOTETASK);
      string bsRemotePort = GCF::ParameterSet::instance()->getString(PARAM_BEAMSERVER_REMOTEPORT);
      m_beamServer.init(*this, bsName, GCFPortInterface::SAP, BS_PROTOCOL);
      TPeerAddr peerAddr;
      peerAddr.taskname = bsRemoteTask;
      peerAddr.portname = bsRemotePort;
      m_beamServer.setAddr(peerAddr);
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
  GCFEvent::TResult status = GCFEvent::HANDLED;

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
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualTelescope::concrete_claimed_state(GCFEvent& event, GCFPortInterface& port, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    case F_CONNECTED:
      break;

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
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult VirtualTelescope::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case BS_BEAMALLOCACK: 
      // prepared event is received from the beam server
      if(_isBeamServerPort(port))
      {
        // check the beam ID and status of the ACK message
        BSBeamallocackEvent ackEvent(event);
        if(ackEvent.status==0)
        {
          m_beamID=ackEvent.handle;
          
          double directionAngle1(0.0);
          double directionAngle2(0.0);
          vector<string> angleTimes;
          vector<double> angles1;
          vector<double> angles2;
          
          try
          {
            angleTimes = m_parameterSet.getStringVector(string("angleTimes"));
            angles1 = m_parameterSet.getDoubleVector(string("angle1"));
            angles2 = m_parameterSet.getDoubleVector(string("angle2"));
          }
          catch(Exception &e)
          {
          }
          
          // point the new beam
          BSBeampointtoEvent beamPointToEvent;
          beamPointToEvent.handle = m_beamID;
          beamPointToEvent.pointing.setType(static_cast<Pointing::Type>(_convertDirection(m_parameterSet.getString(string("directionType")))));
          
          if(angleTimes.size() == 0 || angleTimes.size() != angles1.size() || angleTimes.size() != angles2.size())
          {
            // key angleTimes not found: use one fixed angle
            directionAngle1=m_parameterSet.getDouble(string("angle1"));
            directionAngle2=m_parameterSet.getDouble(string("angle2"));

            beamPointToEvent.pointing.setTime(RTC::Timestamp()); // asap
            beamPointToEvent.pointing.setDirection(directionAngle1,directionAngle2);
            m_beamServer.send(beamPointToEvent);
          }
          else
          {
            vector<double>::iterator angle1It = angles1.begin();
            vector<double>::iterator angle2It = angles2.begin();
            for(vector<string>::iterator timesIt=angleTimes.begin();timesIt!=angleTimes.end();++timesIt)
            {
              beamPointToEvent.pointing.setTime(RTC::Timestamp(APLUtilities::decodeTimeString(*timesIt),0));
              beamPointToEvent.pointing.setDirection(*angle1It++,*angle2It++);
              m_beamServer.send(beamPointToEvent);
            }
          }
          
          newState = LOGICALDEVICE_STATE_SUSPENDED;
        }
        else
        {
          newState = LOGICALDEVICE_STATE_CLAIMED;
          errorCode = LD_RESULT_BEAMALLOC_ERROR;
          LOG_FATAL(formatString("Beamlet allocation failed with errorcode: %d",ackEvent.status));
        }
      }
      break;
    
    case F_CONNECTED:
      break;

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
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualTelescope::concrete_active_state(GCFEvent& event, GCFPortInterface& port, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
      break;
          
    case F_CONNECTED:
      break;

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
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualTelescope::concrete_releasing_state(GCFEvent& event, GCFPortInterface& port, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case BS_BEAMFREEACK: 
      if(_isBeamServerPort(port))
      {
        // check the beam ID and status of the ACK message
        BSBeamfreeackEvent ackEvent(event);
        if(ackEvent.status==0 && ackEvent.handle == m_beamID)
        {
        }
        else
        {
          errorCode = LD_RESULT_BEAMFREE_ERROR;
          LOG_FATAL(formatString("Beamlet de-allocation failed with errorcode: %d",ackEvent.status));
        }
        m_beamServer.close();
        newState=LOGICALDEVICE_STATE_GOINGDOWN;
      }
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
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
  
  try
  {
    vector<int> subbandsVector;
    vector<int> beamletsVector;
    APLUtilities::string2Vector(m_parameterSet.getString(string("subbands")),subbandsVector);
    APLUtilities::string2Vector(m_parameterSet.getString(string("beamlets")),beamletsVector);
    
    BSBeamallocEvent beamAllocEvent;
    beamAllocEvent.name = getName();
    beamAllocEvent.subarrayname = m_parameterSet.getString(string("subarrayName"));
    vector<int>::iterator beamletIt=beamletsVector.begin();
    vector<int>::iterator subbandIt=subbandsVector.begin();
    while(beamletIt!=beamletsVector.end() && subbandIt!=subbandsVector.end())
    {
      beamAllocEvent.allocation()[*beamletIt++] = *subbandIt++;
    }
    m_beamServer.send(beamAllocEvent);
  }
  catch(Exception& e)
  {
    LOG_FATAL(formatString("Error preparing VT: %s",e.message().c_str()));
  }
  
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
  BSBeamfreeEvent beamFreeEvent;
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

void VirtualTelescope::concreteHandleTimers(GCFTimerEvent& timerEvent, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualTelescope::concreteAddExtraKeys(ACC::APS::ParameterSet& psSubset)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // copy samplingFrequency from AO section to child section
  copyParentValue(psSubset,string("directionType"));
  copyParentValue(psSubset,string("angleTimes"));
  copyParentValue(psSubset,string("angle1"));
  copyParentValue(psSubset,string("angle2"));
}

}; // namespace AVT
}; // namespace LOFAR

