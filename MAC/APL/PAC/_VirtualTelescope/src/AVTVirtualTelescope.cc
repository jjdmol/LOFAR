//#  AVTVirtualTelescope.cc: Implementation of the Virtual Telescope task
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

#include "AVTVirtualTelescope.h"
#include "AVTStationBeamformer.h"
#include "LogicalDevice_Protocol.ph"

AVTVirtualTelescope::AVTVirtualTelescope(string& taskName, 
                                         const TPropertySet& primaryPropertySet,
                                         const string& APCName,
                                         const string& APCScope,
                                         AVTStationBeamformer& sbf) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_stationBeamformer(sbf),
  m_beamFormerClient(*this, sbf.getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL)
{
}


AVTVirtualTelescope::~AVTVirtualTelescope()
{
}

bool AVTVirtualTelescope::_isBeamFormerClient(GCFPortInterface& port)
{
  return (&port == &m_beamFormerClient); // comparing two pointers. yuck?
}

void AVTVirtualTelescope::concreteDisconnected(GCFPortInterface& port)
{
  // go to initial state only if the connection with the beamformer is lost.
  if(_isBeamFormerClient(port))
  {
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTVirtualTelescope::concrete_initial_state(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      // open all ports
      m_beamFormerClient.open();
      break;

    case F_CONNECTED_SIG:
    {
      // go to operational only if there is a connection with the beam former.
      if(_isBeamFormerClient(port))
      {
        TRAN(AVTLogicalDevice::idle_state);
      }
      break;
    }

    case F_DISCONNECTED_SIG:
      if(_isBeamFormerClient(port))
      {
        m_beamFormerClient.setTimer(1.0); // try again after 1 second
      }
      break;

    case F_TIMER_SIG:
      if(_isBeamFormerClient(port))
      {
        m_beamFormerClient.open(); // try again
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_claiming_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
      // claimed event is received from the beam former
      if(_isBeamFormerClient(port))
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

GCFEvent::TResult AVTVirtualTelescope::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
      // prepared event is received from the beam former
      if(_isBeamFormerClient(port))
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

GCFEvent::TResult AVTVirtualTelescope::concrete_releasing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_RELEASED:
      // released event is received from the beam former
      if(_isBeamFormerClient(port))
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

void AVTVirtualTelescope::handlePropertySetAnswer(GCFEvent& /*answer*/)
{
  //todo
}

void AVTVirtualTelescope::handleAPCAnswer(GCFEvent& /*answer*/)
{
  //todo
}

void AVTVirtualTelescope::concreteClaim(GCFPortInterface& /*port*/)
{
  // claim my own resources
  
  // send claim message to BeamFormer
  GCFEvent event(LOGICALDEVICE_CLAIM);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concretePrepare(GCFPortInterface& /*port*/)
{
  // prepare my own resources
  
  // send prepare message to BeamFormer
  GCFEvent event(LOGICALDEVICE_PREPARE);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concreteResume(GCFPortInterface& /*port*/)
{
  // resume my own resources
  
  // send resume message to BeamFormer
  GCFEvent event(LOGICALDEVICE_RESUME);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concreteSuspend(GCFPortInterface& /*port*/)
{
  // suspend my own resources
  
  // send suspend message to BeamFormer
  GCFEvent event(LOGICALDEVICE_SUSPEND);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concreteRelease(GCFPortInterface& /*port*/)
{
  // release my own resources
  
  // send release message to BeamFormer
  GCFEvent event(LOGICALDEVICE_RELEASE);
  m_beamFormerClient.send(event);
}

