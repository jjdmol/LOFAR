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

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTVirtualTelescope.h"
#include "AVTStationBeamformer.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"

AVTVirtualTelescope::AVTVirtualTelescope(string& taskName, 
                                         const TPropertySet& primaryPropertySet,
                                         const string& APCName,
                                         const string& APCScope,
                                         AVTStationBeamformer& sbf) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_stationBeamformer(sbf),
  m_beamFormerClient(*this, sbf.getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::AVTVirtualTelescope(%s)",getName().c_str()));
}


AVTVirtualTelescope::~AVTVirtualTelescope()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::~AVTVirtualTelescope(%s)",getName().c_str()));
}

void AVTVirtualTelescope::setStartTime(const time_t startTime)
{
  // activate timer that triggers the prepare command
  m_startTime=startTime;
}

void AVTVirtualTelescope::setStopTime(const time_t stopTime)
{
  // activate timer that triggers the suspend command
  m_stopTime=stopTime;
}

void AVTVirtualTelescope::setFrequency(const double frequency)
{
  m_frequency=frequency;
}

bool AVTVirtualTelescope::_isBeamFormerClient(GCFPortInterface& port)
{
  return (&port == &m_beamFormerClient); // comparing two pointers. yuck?
}

void AVTVirtualTelescope::concreteDisconnected(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concreteDisconnected (%s)",getName().c_str()));
  // go to initial state only if the connection with the beamformer is lost.
  if(_isBeamFormerClient(port))
  {
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTVirtualTelescope::concrete_initial_state(GCFEvent& e, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_initial_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_initial_state, F_INIT_SIG (%s)",getName().c_str()));
      break;

    case F_ENTRY_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_initial_state, F_ENTRY_SIG (%s)",getName().c_str()));
      // open all ports
      m_beamFormerClient.open();
      break;

    case F_CONNECTED_SIG:
    {
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_initial_state, F_CONNECTED_SIG (%s)",getName().c_str()));
      // go to operational only if there is a connection with the beam former.
      if(_isBeamFormerClient(port))
      {
        TRAN(AVTLogicalDevice::idle_state);
      }
      break;
    }

    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_initial_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      if(_isBeamFormerClient(port))
      {
        m_beamFormerClient.setTimer(1.0); // try again after 1 second
      }
      break;

    case F_TIMER_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_initial_state, F_TIMER_SIG (%s)",getName().c_str()));
      if(_isBeamFormerClient(port))
      {
        m_beamFormerClient.open(); // try again
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_initial_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_claiming_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_claiming_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_claiming_state, LOGICALDEVICE_CLAIMED (%s)",getName().c_str()));
      // claimed event is received from the beam former
      if(_isBeamFormerClient(port))
      {
        stateFinished=true;
      }
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_claiming_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished, bool& error)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_preparing_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  stateFinished=false;
  error=false;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_preparing_state, LOGICALDEVICE_PREPARED (%s)",getName().c_str()));
      // prepared event is received from the beam former
      if(_isBeamFormerClient(port))
      {
        stateFinished=true;
      }
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_preparing_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_releasing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_releasing_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_RELEASED:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_releasing_state, LOGICALDEVICE_RELEASED (%s)",getName().c_str()));
      // released event is received from the beam former
      if(_isBeamFormerClient(port))
      {
        stateFinished=true;
      }
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concrete_releasing_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTVirtualTelescope::handlePropertySetAnswer(GCFEvent& answer)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::handlePropertySetAnswer (%s)",getName().c_str()));

  switch(answer.signal)
  {
    case F_VCHANGEMSG_SIG:
      break;

    default:
      break;
  }  
}

void AVTVirtualTelescope::handleAPCAnswer(GCFEvent& /*answer*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::handleAPCAnswer (%s)",getName().c_str()));
  //todo
}

void AVTVirtualTelescope::concreteClaim(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concreteClaim (%s)",getName().c_str()));
  // claim my own resources
  
  // send claim message to BeamFormer
  GCFEvent event(LOGICALDEVICE_CLAIM);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concretePrepare(GCFPortInterface& /*port*/,string& parameters)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concretePrepare (%s)",getName().c_str()));
  // prepare my own resources
  vector<string> decodedParameters;
  AVTUtilities::decodeParameters(parameters,decodedParameters);
  // parameters: start time,stop time,frequency,subbands,direction type,angle1,angle2
  setStartTime(atoi(decodedParameters[0].c_str()));
  setStopTime(atoi(decodedParameters[1].c_str()));
  setFrequency(atof(decodedParameters[2].c_str()));
  
  // send prepare message to BeamFormer
  // all parameters are forwarded to the beamformer
  char prepareParameters[700];
  strncpy(prepareParameters,parameters.c_str(),700);
  LOGICALDEVICEPrepareEvent prepareEvent(prepareParameters);
  m_beamFormerClient.send(prepareEvent);
}

void AVTVirtualTelescope::concreteResume(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concreteResume (%s)",getName().c_str()));
  // resume my own resources
  
  // send resume message to BeamFormer
  GCFEvent event(LOGICALDEVICE_RESUME);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concreteSuspend (%s)",getName().c_str()));
  // suspend my own resources
  
  // send suspend message to BeamFormer
  GCFEvent event(LOGICALDEVICE_SUSPEND);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concreteRelease(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope::concreteRelease (%s)",getName().c_str()));
  // release my own resources
  
  // send release message to BeamFormer
  GCFEvent event(LOGICALDEVICE_RELEASE);
  m_beamFormerClient.send(event);
}

