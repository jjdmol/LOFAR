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

#include "AVTStationBeamformer.h"
#include "LogicalDevice_Protocol.ph"
#include "BeamServer_Protocol.ph"

AVTStationBeamformer::AVTStationBeamformer(const string& taskName, 
                                           const TPropertySet& primaryPropertySet,
                                           const string& APCName,
                                           const string& APCScope,
                                           const string& beamServerPortName) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_beamServer(*this, beamServerPortName, GCFPortInterface::SAP, BEAMSERVER_PROTOCOL)
{
}


AVTStationBeamformer::~AVTStationBeamformer()
{
}

bool AVTStationBeamformer::_isBeamServerPort(GCFPortInterface& port)
{
  return (&port == &m_beamServer) // comparing two pointers. yuck?
}

void AVTStationBeamformer::concreteDisconnected(GCFPortInterface& port)
{
  // go to initial state only if the connection with the beamformer is lost.
  if(_isBeamServerPort(port))
  {
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTStationBeamformer::concrete_initial_state(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
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
        m_beamServer.setTimer(1.0); // try again after 1 second
      }
      break;

    case F_TIMER_SIG:
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

GCFEvent::TResult AVTStationBeamformer::concrete_claiming_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
      stateFinished=true;
      break;
      
    case BEAMSERVER_ACK: // or something
      // beam has been created
      if(_isBeamServerPort(port)
      {
        stateFinished=true;
      }
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
      stateFinished=true;
      break;
      
    case BEAMSERVER_ACK: // or something like that
      // prepaired event is received from the beam server
      if(_isBeamServerPort(port)
      {
        stateFinished=true;
      }
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_releasing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_RELEASED:
      stateFinished=true;
      break;
    
    case BEAMSERVER_ACK: // or something like that
      // released event is received from the beam server
      if(_isBeamServerPort(port)
      {
        stateFinished=true;
      }
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTStationBeamformer::concreteClaim(GCFPortInterface& port)
{
  // claim my own resources
  
  // send claim message to BeamFormer
  
  // if claiming is an async process, then the end of the claiming state
  // is determined in the concrete_claiming_state() method
  // Otherwise, it is done here by calling dispatch(port,LOGICALDEVICE_CLAIMED);

  dispatch(port,LOGICALDEVICE_CLAIMED);
}

void AVTStationBeamformer::concretePrepare(GCFPortInterface& port)
{
  // prepare my own resources
  
  // send prepare message to BeamFormer
  
  // if preparing is an async process, then the end of the preparing state
  // is determined in the concrete_prepairing_state() method
  // Otherwise, it is done here by calling dispatch(port,LOGICALDEVICE_PREPARED);

  dispatch(port,LOGICALDEVICE_PREPARED);
}

void AVTStationBeamformer::concreteResume(GCFPortInterface& port)
{
  // resume my own resources
  
  // send resume message to BeamFormer
  
}

void AVTStationBeamformer::concreteSuspend(GCFPortInterface& port)
{
  // suspend my own resources
  
  // send suspend message to BeamFormer
  
}

void AVTStationBeamformer::concreteRelease(GCFPortInterface& port)
{
  // release my own resources
  
  // send release message to BeamFormer
  
  // if releasing is an async process, then the end of the releasing state
  // is determined in the concrete_releasing_state() method
  // Otherwise, it is done here by calling dispatch(port,LOGICALDEVICE_RELEASED);

  dispatch(port,LOGICALDEVICE_RELEASED);
}

void AVTStationBeamformer::concreteDisconnected(GCFPortInterface& port)
{
}

