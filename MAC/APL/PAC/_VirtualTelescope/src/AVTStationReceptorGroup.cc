//#  AVTStationReceptorGroup.cc: Implementation of the Virtual Telescope task
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
#include "AVTStationReceptorGroup.h"
#include "AVTStationBeamformer.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"

using namespace AVT;

AVTStationReceptorGroup::AVTStationReceptorGroup(string& taskName, 
                                                 const TPropertySet& primaryPropertySet,
                                                 const string& APCName,
                                                 const string& APCScope,
                                                 vector<AVTStationReceptor&> rcus) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_stationReceptors(),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::AVTStationReceptorGroup",getName().c_str()));

  vector<AVTStationReceptor&>::iterator it;
  for(it=rcus.begin();it!=rcus.end();++it);
  {
    TStationReceptorConnection src( *it,
                                    *this,
                                    it->getServerPortName(), 
                                    GCFPortInterface::SAP, 
                                    LOGICALDEVICE_PROTOCOL,
                                    false);
    m_stationReceptors.push_back(src);
  }
}


AVTStationReceptorGroup::~AVTStationReceptorGroup()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::~AVTStationReceptorGroup",getName().c_str()));
}

void AVTStationReceptorGroup::setStartTime(const time_t startTime)
{
  // activate timer that triggers the prepare command
  m_startTime=startTime;
}

void AVTStationReceptorGroup::setStopTime(const time_t stopTime)
{
  // activate timer that triggers the suspend command
  m_stopTime=stopTime;
}

void AVTStationReceptorGroup::setFrequency(const double frequency)
{
  m_frequency=frequency;
}

bool AVTStationReceptorGroup::isStationReceptorClient(GCFPortInterface& port)
{
  TStationReceptorVector::iterator it = m_stationReceptors.begin();
  bool found=false;
  while(!found && it!=m_stationReceptors.end())
  {
    found = (&(it->clientPort) == &m_beamFormerClient); // comparing two pointers. yuck?
    it++;
  }
  return found;
}

bool AVTStationReceptorGroup::setReceptorConnected(GCFPortInterface& port, bool connected)
{
  TStationReceptorVector::iterator it = m_stationReceptors.begin();
  bool found=false;
  while(!found && it!=m_stationReceptors.end())
  {
    found = (&(it->clientPort) == &m_beamFormerClient); // comparing two pointers. yuck?
    it++;
  }
  if(found)
  {
    it->connected = connected;
  }
  return found;
}

bool AVTStationReceptorGroup::allReceptorsConnected()
{
  bool allConnected = true;
  TStationReceptorVector::iterator it = m_stationReceptors.begin();
  while(allConnected && it!=m_stationReceptors.end())
  {
    allConnected = it->connected;
    it++;
  }
  return allConnected;
}


void AVTStationReceptorGroup::concreteDisconnected(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concreteDisconnected",getName().c_str()));
  // go to initial state only if the connection with the receptor is lost.
  if(isStationReceptorClient(port))
  {
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_initial_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      m_stationBeamformer.start();
      break;

    case F_ENTRY:
      // open all ports
      TStationReceptorVector::iterator it;
      for(it = m_stationReceptors.begin();it!=m_stationReceptors.end();++it)
      {
        it->clientPort.open();
      }
      break;

    case F_CONNECTED:
    {
      // go to operational only if there is a connection with the beam former.
      if(setReceptorConnected(port,true))
      {
        if(allReceptorsConnected())
        {
          if(isAPCLoaded())
          {
            TRAN(AVTLogicalDevice::idle_state);
          }
          else
          {
            port.setTimer(2.0); // try again
          }
        }
      }
      break;
    }

    case F_DISCONNECTED:
      if(setReceptorConnected(port,false))
      {
        port.setTimer(2.0); // try again
      }
      break;

    case F_TIMER:
      if(setReceptorConnected(port,true))
      {
        if(allReceptorsConnected())
        {
          if(isAPCLoaded())
          {
            TRAN(AVTLogicalDevice::idle_state);
          }
          else
          {
            port.setTimer(2.0); // try again
          }
        }
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_initial_state, default (%s)",getName().c_str(),evtstr(event)));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_claiming_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_claiming_state (%s)",getName().c_str(),evtstr(event)));
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished, bool& error)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_preparing_state (%s)",getName().c_str(),evtstr(event)));
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_releasing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_releasing_state (%s)",getName().c_str(),evtstr(event)));
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_releasing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTStationReceptorGroup::handlePropertySetAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_MYPLOADED:
    {
      // property set loaded, now load apc
      m_APC.load(false);
      break;
    }
    
    case F_VCHANGEMSG:
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
            string prepareParameters;
            AVTUtilities::encodeParameters(parameters,prepareParameters);
            
            // send message to myself using a dummyport. VT will send it to SBF and SRG
            GCFDummyPort dummyPort(this,string("VT_command_dummy"),LOGICALDEVICE_PROTOCOL);
            LOGICALDEVICEPrepareEvent prepareEvent;
            prepareEvent.parameters = prepareParameters;
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

void AVTStationReceptorGroup::handleAPCAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_APCLOADED:
    {
      apcLoaded();
      break;
    }
    
    default:
      break;
  }  
}

void AVTStationReceptorGroup::concreteClaim(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concreteClaim",getName().c_str()));
  // claim my own resources
  
  // send claim message to BeamFormer
  GCFEvent event(LOGICALDEVICE_CLAIM);
  m_beamFormerClient.send(event);
}

void AVTStationReceptorGroup::concretePrepare(GCFPortInterface& /*port*/,string& parameters)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concretePrepare",getName().c_str()));
  // prepare my own resources
  vector<string> decodedParameters;
  AVTUtilities::decodeParameters(parameters,decodedParameters);
  // parameters: start time,stop time,frequency,subbands,direction type,angle1,angle2
  setStartTime(atoi(decodedParameters[0].c_str()));
  setStopTime(atoi(decodedParameters[1].c_str()));
  setFrequency(atof(decodedParameters[2].c_str()));
  
  // send prepare message to BeamFormer
  // all parameters are forwarded to the beamformer
  LOGICALDEVICEPrepareEvent prepareEvent;
  prepareEvent.parameters = parameters;
  m_beamFormerClient.send(prepareEvent);
}

void AVTStationReceptorGroup::concreteResume(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concreteResume",getName().c_str()));
  // resume my own resources
  
  // send resume message to BeamFormer
  GCFEvent event(LOGICALDEVICE_RESUME);
  m_beamFormerClient.send(event);
}

void AVTStationReceptorGroup::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concreteSuspend",getName().c_str()));
  // suspend my own resources
  
  // send suspend message to BeamFormer
  GCFEvent event(LOGICALDEVICE_SUSPEND);
  m_beamFormerClient.send(event);
}

void AVTStationReceptorGroup::concreteRelease(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concreteRelease",getName().c_str()));
  // release my own resources
  
  // send release message to BeamFormer
  GCFEvent event(LOGICALDEVICE_RELEASE);
  m_beamFormerClient.send(event);
}

