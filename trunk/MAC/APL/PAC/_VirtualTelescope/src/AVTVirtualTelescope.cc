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
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>

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
//  m_beamFormerClient(*this, sbf.getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL),
// use process-internal-inter-task-port 
  m_beamFormerClient(sbf,*this, sbf.getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL),
  m_beamFormerConnected(false),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::AVTVirtualTelescope",getName().c_str()));
  m_stationBeamformer.setClientInterTaskPort(&m_beamFormerClient);
}


AVTVirtualTelescope::~AVTVirtualTelescope()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::~AVTVirtualTelescope",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concreteDisconnected",getName().c_str()));
  // go to initial state only if the connection with the beamformer is lost.
  if(_isBeamFormerClient(port))
  {
    m_beamFormerConnected=false;
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTVirtualTelescope::concrete_initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concrete_initial_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT_SIG:
      m_stationBeamformer.start();
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
        m_beamFormerConnected=true;
        if(isAPCLoaded())
        {
          TRAN(AVTLogicalDevice::idle_state);
        }
        else
        {
          m_beamFormerClient.setTimer(2.0); // try again
        }
      }
      break;
    }

    case F_DISCONNECTED_SIG:
      if(_isBeamFormerClient(port))
      {
        m_beamFormerClient.setTimer(2.0); // try again
      }
      break;

    case F_TIMER_SIG:
      if(_isBeamFormerClient(port))
      {
        if(m_beamFormerConnected)
        {
          if(isAPCLoaded())
          {
            TRAN(AVTLogicalDevice::idle_state);
          }
          else
          {
            m_beamFormerClient.setTimer(2.0); // try again
          }
        }
        else
        {
          m_beamFormerClient.open(); // try again
        }      
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concrete_initial_state, default (%s)",getName().c_str(),evtstr(event)));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_claiming_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concrete_claiming_state (%s)",getName().c_str(),evtstr(event)));
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concrete_claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished, bool& error)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concrete_preparing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  stateFinished=false;
  error=false;

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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concrete_preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_releasing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concrete_releasing_state (%s)",getName().c_str(),evtstr(event)));
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concrete_releasing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTVirtualTelescope::handlePropertySetAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_MYPLOADED_SIG:
    {
      // property set loaded, now load apc
      m_APC.load(false);
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
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        vector<string> parameters;
        string command;
        AVTUtilities::decodeCommand(commandString,command,parameters);
        
        // PREPARE <starttime>,<stoptime>,<frequency>,<subbands>,<directiontype>,<directionangle1>,<directionangle2>
        if(command==string("PREPARE"))
        {
          if(parameters.size()==7)
          {
            // send prepare message:
            char prepareParameters[700];
            AVTUtilities::encodeParameters(parameters,prepareParameters,700);
            
            // send message to myself using a dummyport. VT will send it to SBF and SRG
            GCFDummyPort dummyPort(this,string("VT_command_dummy"),LOGICALDEVICE_PROTOCOL);
            LOGICALDEVICEPrepareEvent prepareEvent(prepareParameters);
            dispatch(prepareEvent,dummyPort);
          }
        }
        // SUSPEND
        else if(command==string("SUSPEND"))
        {
          // send message to myself using a dummyport. VT will send it to SBF and SRG
          GCFDummyPort dummyPort(this,string("VT_command_dummy"),LOGICALDEVICE_PROTOCOL);
          GCFEvent e(LOGICALDEVICE_SUSPEND);
          dispatch(e,dummyPort); 
        }
        // RESUME
        else if(command==string("RESUME"))
        {
          // send message to myself using a dummyport. VT will send it to SBF and SRG
          GCFDummyPort dummyPort(this,string("VT_command_dummy"),LOGICALDEVICE_PROTOCOL);
          GCFEvent e(LOGICALDEVICE_RESUME);
          dispatch(e,dummyPort);
        }
      }
      break;
    }  

    default:
      break;
  }  
}

void AVTVirtualTelescope::handleAPCAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_APCLOADED_SIG:
    {
      apcLoaded();
      break;
    }
    
    default:
      break;
  }  
}

void AVTVirtualTelescope::concreteClaim(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concreteClaim",getName().c_str()));
  // claim my own resources
  
  // send claim message to BeamFormer
  GCFEvent event(LOGICALDEVICE_CLAIM);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concretePrepare(GCFPortInterface& /*port*/,string& parameters)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concretePrepare",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concreteResume",getName().c_str()));
  // resume my own resources
  
  // send resume message to BeamFormer
  GCFEvent event(LOGICALDEVICE_RESUME);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concreteSuspend",getName().c_str()));
  // suspend my own resources
  
  // send suspend message to BeamFormer
  GCFEvent event(LOGICALDEVICE_SUSPEND);
  m_beamFormerClient.send(event);
}

void AVTVirtualTelescope::concreteRelease(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTVirtualTelescope(%s)::concreteRelease",getName().c_str()));
  // release my own resources
  
  // send release message to BeamFormer
  GCFEvent event(LOGICALDEVICE_RELEASE);
  m_beamFormerClient.send(event);
}

