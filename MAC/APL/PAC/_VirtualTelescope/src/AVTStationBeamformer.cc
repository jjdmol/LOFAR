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
#include "APLCommon/src/BeamServer_Protocol.ph"

AVTStationBeamformer::AVTStationBeamformer(string& taskName, 
                                           const TPropertySet& primaryPropertySet,
                                           const string& APCName,
                                           const string& APCScope,
                                           string& beamServerPortName) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_beamServer(*this, beamServerPortName, GCFPortInterface::SAP, BEAMSERVER_PROTOCOL)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::AVTStationBeamformer(%s)",getName().c_str()));
}


AVTStationBeamformer::~AVTStationBeamformer()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::~AVTStationBeamformer(%s)",getName().c_str()));
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

GCFEvent::TResult AVTStationBeamformer::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_preparing_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_preparing_state, LOGICALDEVICE_PREPARED (%s)",getName().c_str()));
      stateFinished=true;
      break;
      
    case BEAMSERVER_ACK: // or something like that
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concrete_preparing_state, BEAMSERVER_ACK (%s)",getName().c_str()));
      // prepared event is received from the beam server
      if(_isBeamServerPort(port))
      {
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
  
  // send claim message to BeamFormer
  
  // if claiming is an async process, then the end of the claiming state
  // is determined in the concrete_claiming_state() method
  // Otherwise, it is done here by calling dispatch
  GCFEvent event(LOGICALDEVICE_CLAIMED);
  dispatch(event,port);
}

void AVTStationBeamformer::concretePrepare(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer::concretePrepare (%s)",getName().c_str()));
  // prepare my own resources
  
  // send prepare message to BeamFormer
  
  // if preparing is an async process, then the end of the preparing state
  // is determined in the concrete_preparing_state() method
  // Otherwise, it is done here by calling dispatch
  GCFEvent event(LOGICALDEVICE_PREPARED);
  dispatch(event,port);
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
