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

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTStationBeamformer.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"
#include "PAC/BeamServer/src/ABS_Protocol.ph"

AVTStationBeamformer::AVTStationBeamformer(string& taskName, 
                                           const TPropertySet& primaryPropertySet,
                                           const string& APCName,
                                           const string& APCScope,
                                           string& beamServerPortName) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_beamServer(*this, beamServerPortName, GCFPortInterface::SAP, BEAMSERVER_PROTOCOL),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0),
  m_subbands(),
  m_directionType(0),
  m_directionAngle1(0.0),
  m_directionAngle2(0.0),
  m_beamID(-1)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::AVTStationBeamformer(%s)",getName().c_str()));
}


AVTStationBeamformer::~AVTStationBeamformer()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::~AVTStationBeamformer(%s)",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concreteDisconnected(%s)",getName().c_str()));
  // go to initial state only if the connection with the beamformer is lost.
  if(_isBeamServerPort(port))
  {
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTStationBeamformer::concrete_initial_state(GCFEvent& e, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_initial_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_initial_state, F_INIT_SIG (%s)",getName().c_str()));
      break;

    case F_ENTRY_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_initial_state, F_ENTRY_SIG (%s)",getName().c_str()));
      // open all ports
      m_beamServer.open();
      break;

    case F_CONNECTED_SIG:
    {
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_initial_state, F_CONNECTED_SIG (%s)",getName().c_str()));
      // go to operational only if there is a connection with the beam server.
      if(_isBeamServerPort(port))
      {
        TRAN(AVTLogicalDevice::idle_state);
      }
      break;
    }

    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_initial_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      if(_isBeamServerPort(port))
      {
        m_beamServer.setTimer(1.0); // try again after 1 second
      }
      break;

    case F_TIMER_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_initial_state, F_TIMER_SIG (%s)",getName().c_str()));
      if(_isBeamServerPort(port))
      {
        m_beamServer.open(); // try again
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_initial_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_claiming_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_claiming_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_claiming_state, LOGICALDEVICE_CLAIMED (%s)",getName().c_str()));
      stateFinished=true;
      break;
      
    case BEAMSERVER_ACK: // or something
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_claiming_state, BEAMSERVER_ACK (%s)",getName().c_str()));
      // beam has been created
      if(_isBeamServerPort(port))
      {
        stateFinished=true;
      }
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_claiming_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished, bool& error)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_preparing_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  stateFinished=true;
  error=false;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_preparing_state, LOGICALDEVICE_PREPARED (%s)",getName().c_str()));
      stateFinished=true;
      break;
      
    case ABS_BEAMALLOC_ACK: 
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_preparing_state, ABS_BEAMALLOC_ACK (%s)",getName().c_str()));
      // prepared event is received from the beam server
      if(_isBeamServerPort(port))
      {
        // check the beam ID and status of the ACK message
        boost::shared_ptr<ABSBeamalloc_AckEvent> pAckEvent;
        pAckEvent.reset(static_cast<ABSBeamalloc_AckEvent*>(&event));
        if(pAckEvent.get()!=0)
        {
          if(pAckEvent->beam_index==m_beamID && pAckEvent->status==0)
          {
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
        }
        else
        {
          error=true;
        }
        stateFinished=true;
      }
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_preparing_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_releasing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_releasing_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_RELEASED:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_releasing_state, LOGICALDEVICE_RELEASED (%s)",getName().c_str()));
      stateFinished=true;
      break;
    
    case BEAMSERVER_ACK: // or something like that
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_releasing_state, BEAMSERVER_ACK (%s)",getName().c_str()));
      // released event is received from the beam server
      if(_isBeamServerPort(port))
      {
        stateFinished=true;
      }
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_releasing_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTStationBeamformer::handlePropertySetAnswer(GCFEvent& /*answer*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::handlePropertySetAnswer (%s)",getName().c_str()));
  //todo
}

void AVTStationBeamformer::handleAPCAnswer(GCFEvent& /*answer*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::handleAPCAnswer (%s)",getName().c_str()));
  //todo
}

void AVTStationBeamformer::concreteClaim(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concreteClaim (%s)",getName().c_str()));
  // claim my own resources
  
  // send claim message to BeamServer
  
  // if claiming is an async process, then the end of the claiming state
  // is determined in the concrete_claiming_state() method
  // Otherwise, it is done here by calling dispatch
  GCFEvent event(LOGICALDEVICE_CLAIMED);
  dispatch(event,port);
}

void AVTStationBeamformer::concretePrepare(GCFPortInterface& port,string& parameters)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concretePrepare (%s)",getName().c_str()));
  // prepare my own resources
  vector<string> decodedParameters;
  AVTUtilities::decodeParameters(parameters,decodedParameters);
  // parameters: start time,stop time,frequency,subbands,direction type,angle1,angle2
  m_startTime=atoi(decodedParameters[0].c_str());
  m_stopTime=atoi(decodedParameters[1].c_str());
  m_frequency=atof(decodedParameters[2].c_str());
  AVTUtilities::decodeSubbandsParameter(decodedParameters[3],m_subbands);
  setDirection(decodedParameters[4],atof(decodedParameters[5].c_str),atof(decodedParameters[6].c_str));
  
  m_beamID=0; // TODO
  int spectral_window(0);
  int n_subbands(m_subbands.size());
  int subbandsArray[N_BEAMLETS];
  
  memset(subbandsArray,0,sizeof(subbandsArray[0])*N_BEAMLETS);
  vector<int>::iterator vectorIterator=m_subbands.begin();
  int arrayIndex(0);
  while(arrayIndex<N_BEAMLETS && vectorIterator!=m_subbands.end())
  {
    subbandsArray[arrayIndex++]=m_subbands[vectorIterator++];
  }
  
  ABSBeamallocEvent beamAllocEvent(m_beamID,spectral_window,n_subbands,subbandsArray);
  m_beamServer.send(beamAllocEvent);
}

void AVTStationBeamformer::concreteResume(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concreteResume (%s)",getName().c_str()));
  // resume my own resources
  
  // send resume message to BeamFormer
  
}

void AVTStationBeamformer::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concreteSuspend (%s)",getName().c_str()));
  // suspend my own resources
  
  // send suspend message to BeamFormer
  
}

void AVTStationBeamformer::concreteRelease(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concreteRelease (%s)",getName().c_str()));
  // release my own resources
  
  // send release message to BeamFormer
  
  // if releasing is an async process, then the end of the releasing state
  // is determined in the concrete_releasing_state() method
  // Otherwise, it is done here by calling dispatch
  GCFEvent event(LOGICALDEVICE_RELEASED);
  dispatch(event,port);
}
