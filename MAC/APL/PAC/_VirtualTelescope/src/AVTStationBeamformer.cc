//#  AVTStationBeamformer.cc: Implementation of the Virtual Telescope task
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

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>
#include <boost/shared_ptr.hpp>
#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTStationBeamformer.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"
#define DECLARE_SIGNAL_NAMES
#include "ABS_Protocol.ph"

AVTStationBeamformer::AVTStationBeamformer(string& taskName, 
                                           const TPropertySet& primaryPropertySet,
                                           const string& APCName,
                                           const string& APCScope,
                                           string& beamServerPortName) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_beamServer(*this, beamServerPortName, GCFPortInterface::SAP, ABS_PROTOCOL),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0),
  m_subbands(),
  m_directionType(0),
  m_directionAngle1(0.0),
  m_directionAngle2(0.0),
  m_beamID(-1)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::AVTStationBeamformer",getName().c_str()));
}


AVTStationBeamformer::~AVTStationBeamformer()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::~AVTStationBeamformer",getName().c_str()));
}

void AVTStationBeamformer::setDirection(const string type,const double angle1, const double angle2)
{
  if(type==string("J2000"))
  {
    m_directionType=1;
  }
  else if(type==string("LMN"))
  {
    m_directionType=3;
  }
  else
  {
    m_directionType=2;
  }
  m_directionAngle1=angle1;
  m_directionAngle2=angle2;
}

bool AVTStationBeamformer::_isBeamServerPort(GCFPortInterface& port)
{
  return (&port == &m_beamServer); // comparing two pointers. yuck?
}

void AVTStationBeamformer::concreteDisconnected(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteDisconnected",getName().c_str()));
  // go to initial state only if the connection with the beamformer is lost.
  if(_isBeamServerPort(port))
  {
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTStationBeamformer::concrete_initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_initial_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      // open all ports
      m_beamServer.open();
      break;

    case F_CONNECTED_SIG:
    {
      // go to operational only if there is a connection with the beam server.
      if(_isBeamServerPort(port))
      {
        TRAN(AVTLogicalDevice::idle_state);
      }
      break;
    }

    case F_DISCONNECTED_SIG:
      if(_isBeamServerPort(port))
      {
        m_beamServer.setTimer(10.0); // try again after 10 second
      }
      break;

    case F_TIMER_SIG:
      if(_isBeamServerPort(port))
      {
        m_beamServer.open(); // try again
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_initial_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_claiming_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
      stateFinished=true;
      break;
      
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished, bool& error)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_preparing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  stateFinished=true;
  error=false;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
      stateFinished=true;
      break;
      
    case ABS_BEAMALLOC_ACK: 
      // prepared event is received from the beam server
      if(_isBeamServerPort(port))
      {
        // check the beam ID and status of the ACK message
        ABSBeamalloc_AckEvent& ackEvent=static_cast<ABSBeamalloc_AckEvent&>(event);
        if(ackEvent.status==0)
        {
          m_beamID=ackEvent.handle;
          // point the new beam
          struct timeval timeArg;
          timeArg.tv_sec=m_startTime;
          timeArg.tv_usec=0;
          ABSBeampointtoEvent beamPointToEvent(m_beamID,timeArg,m_directionType,m_directionAngle1,m_directionAngle2);
          m_beamServer.send(beamPointToEvent);
        }
        else
        {
          error=true;
        }
        stateFinished=true;
      }
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_releasing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_RELEASED:
      stateFinished=true;
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_releasing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTStationBeamformer::handlePropertySetAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_MYPLOADED_SIG:
    {
      // property set loaded, now load apc
      m_APC.load(true);
      break;
    }
    
    case F_VCHANGEMSG_SIG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&answer);
      assert(pPropAnswer);
      if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, "_command") != 0))
      {
        // command received
      }
      break;
    }  

    default:
      break;
  }  
}

void AVTStationBeamformer::handleAPCAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_APCLOADED_SIG:
    {
      break;
    }
    
    default:
      break;
  }  
}

void AVTStationBeamformer::concreteClaim(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteClaim",getName().c_str()));
  // claim my own resources
  
  // send claim message to BeamServer
  
  // if claiming is an async process, then the end of the claiming state
  // is determined in the concrete_claiming_state() method
  // Otherwise, it is done here by calling dispatch
  GCFEvent event(LOGICALDEVICE_CLAIMED);
  dispatch(event,port);
}

void AVTStationBeamformer::concretePrepare(GCFPortInterface& /*port*/,string& parameters)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concretePrepare",getName().c_str()));
  // prepare my own resources
  vector<string> decodedParameters;
  AVTUtilities::decodeParameters(parameters,decodedParameters);
  // parameters: start time,stop time,frequency,subbands,direction type,angle1,angle2
  m_startTime=atoi(decodedParameters[0].c_str());
  m_stopTime=atoi(decodedParameters[1].c_str());
  m_frequency=atof(decodedParameters[2].c_str());
  AVTUtilities::decodeSubbandsParameter(decodedParameters[3],m_subbands);
  setDirection(decodedParameters[4],atof(decodedParameters[5].c_str()),atof(decodedParameters[6].c_str()));
  
  m_beamID=0; // TODO
  int spectral_window(0);
  int n_subbands(m_subbands.size());
  int subbandsArray[N_BEAMLETS];
  
  memset(subbandsArray,0,sizeof(subbandsArray[0])*N_BEAMLETS);
  vector<int>::iterator vectorIterator=m_subbands.begin();
  int arrayIndex(0);
  while(arrayIndex<N_BEAMLETS && vectorIterator!=m_subbands.end())
  {
    subbandsArray[arrayIndex++]=*vectorIterator;
    ++vectorIterator;
  }
  
  ABSBeamallocEvent beamAllocEvent(spectral_window,n_subbands,subbandsArray);
  m_beamServer.send(beamAllocEvent);
}

void AVTStationBeamformer::concreteResume(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteResume",getName().c_str()));
  // resume my own resources
  
  // send resume message to BeamFormer
  
}

void AVTStationBeamformer::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteSuspend",getName().c_str()));
  // suspend my own resources
  
  // send suspend message to BeamFormer
  
}

void AVTStationBeamformer::concreteRelease(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteRelease",getName().c_str()));
  // release my own resources
  
  // send release message to BeamFormer
  
  // if releasing is an async process, then the end of the releasing state
  // is determined in the concrete_releasing_state() method
  // Otherwise, it is done here by calling dispatch
  GCFEvent event(LOGICALDEVICE_RELEASED);
  dispatch(event,port);
}
