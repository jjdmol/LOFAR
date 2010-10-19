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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTDefines.h"
#include "AVTStationReceptorGroup.h"
#include "AVTStationReceptor.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"
#include "AVTResourceManager.h"

using namespace LOFAR;
using namespace AVT;
using namespace std;
using namespace boost;

AVTStationReceptorGroup::TStationReceptorConnection::TStationReceptorConnection( 
      shared_ptr<AVTStationReceptor> _rcu,
      GCFTask&                       _containerTask, 
      string&                        _name, 
      GCFPort::TPortType             _type, 
      int                            _protocol,
      bool                           _connected) :
  rcu(_rcu),
  clientPort(new APLInterTaskPort((GCFTask&)(*_rcu),_containerTask,_name,_type,_protocol)),
  connected(_connected)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("TStationReceptorConnection(0x%x)::TStationReceptorConnection",this));
  rcu->addClientInterTaskPort(clientPort.get());
}

AVTStationReceptorGroup::AVTStationReceptorGroup(string& taskName, 
                                                 const TPropertySet& primaryPropertySet,
                                                 const string& APCName,
                                                 const string& APCScope,
                                                 vector<shared_ptr<AVTStationReceptor> >& rcus) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_stationReceptors(),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::AVTStationReceptorGroup",getName().c_str()));

  vector<shared_ptr<AVTStationReceptor> >::iterator rIt;
  for(rIt=rcus.begin();rIt!=rcus.end();++rIt)
  {
    shared_ptr<AVTStationReceptor> ptr = *rIt;
    
    string portName(ptr->getServerPortName());
    
    TStationReceptorConnection src(ptr,
                                    *this,
                                    portName, 
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

bool AVTStationReceptorGroup::isPrepared(vector<string>& parameters)
{
  // compare all parameters with the parameters provided.
  // if all are equal, then the LD is prepared.
  bool isPrepared=true;
  double frequency=atof(parameters[2].c_str());
  isPrepared = (frequency == m_frequency);

  return isPrepared;
}

bool AVTStationReceptorGroup::checkQualityRequirements(int maxFailedResources)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::%s",getName().c_str(),__func__));
  bool requirementsMet=true;
  
  // quality requirements for this station receptor group:
  // - not more than 1 antenna unavailable
  // - not more than 1 antenna in alarm
  
  int failedResources=0;
  TStationReceptorVectorIter it = m_stationReceptors.begin();
  while(requirementsMet && it!=m_stationReceptors.end())
  {
    if(!(*it).rcu->checkQualityRequirements())
    {
      failedResources++;
    }
    requirementsMet = (failedResources <= maxFailedResources);
    it++;
  }
  
  return requirementsMet;
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s,0x%x)::isStationReceptorClient",getName().c_str(),this));
  TStationReceptorVectorIter it = m_stationReceptors.begin();
  bool found=false;
  while(!found && it!=m_stationReceptors.end())
  {
    found = ((*it).clientPort.get() == &port); // comparing two pointers. yuck?
    it++;
  }
  return found;
}

bool AVTStationReceptorGroup::setReceptorConnected(GCFPortInterface& port, bool connected)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s,0x%x)::%s(0x%x,%s)",getName().c_str(),this,__func__,&port,(connected?"true":"false")));
  TStationReceptorVectorIter it = m_stationReceptors.begin();
  bool found=false;
  while(!found && it!=m_stationReceptors.end())
  {
    if((*it).clientPort.get() == &port) // comparing two pointers. yuck?
    {
      found = true;
      (*it).connected = connected;
    }
    else
    {
      it++;
    }
  }
  return found;
}

bool AVTStationReceptorGroup::allReceptorsConnected()
{
  bool allConnected = true;
  TStationReceptorVectorIter it = m_stationReceptors.begin();
  while(allConnected && it!=m_stationReceptors.end())
  {
    allConnected = (*it).connected;
    it++;
  }
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s,0x%x)::%s: %s",getName().c_str(),this,__func__,(allConnected?"true":"false")));
  return allConnected;
}

bool AVTStationReceptorGroup::allReceptorsInState(TLogicalDeviceState state)
{
  bool allInState = true;
  TStationReceptorVectorIter it = m_stationReceptors.begin();
  while(allInState && it!=m_stationReceptors.end())
  {
    allInState = ((*it).rcu->getLogicalDeviceState()==state);
    it++;
  }
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s,0x%x)::%s(%d): %s",getName().c_str(),this,__func__,state,(allInState?"true":"false")));
  return allInState;
}

void AVTStationReceptorGroup::sendToAllReceptors(GCFEvent& event)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s,0x%x)::sendToAllReceptors",getName().c_str(),this));
  TStationReceptorVectorIter it;
  for(it=m_stationReceptors.begin();it!=m_stationReceptors.end();++it)
  {
    (*it).clientPort->send(event);
  }
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
      break;

    case F_ENTRY:
    {
      // open all ports
      TStationReceptorVectorIter it;
      for(it = m_stationReceptors.begin();it!=m_stationReceptors.end();++it)
      {
        if(!(*it).connected)
        {
          (*it).clientPort->open();
        }
      }
      break;
    }

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
    {
      if(setReceptorConnected(port,false))
      {
        port.setTimer(2.0); // try again
      }
      break;
    }
    
    case F_TIMER:
    {
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

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_initial_state, default (%s)",getName().c_str(),evtstr(event)));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_claiming_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
    {
      // claimed event is received from the station receptor
      if(allReceptorsInState(LOGICALDEVICE_STATE_CLAIMED))
      {
        stateFinished=true;
      }
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished, bool& error)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_preparing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  stateFinished=false;
  error=false;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
    {
      // prepared event is received from the station receptor
      if(allReceptorsInState(LOGICALDEVICE_STATE_SUSPENDED))
      {
        stateFinished=true;
      }
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_active_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  return GCFEvent::NOT_HANDLED;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concrete_releasing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_RELEASED:
    {
      // released event is received from the station receptor
      if(allReceptorsInState(LOGICALDEVICE_STATE_RELEASED))
      {
        stateFinished=true;
      }
      break;
    }
    
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
            GCFDummyPort dummyPort(this,string("SRG_command_dummy"),LOGICALDEVICE_PROTOCOL);
            LOGICALDEVICEPrepareEvent prepareEvent;
            prepareEvent.parameters = prepareParameters;
            dispatch(prepareEvent,dummyPort);
          }
        }
        // SUSPEND
        else if(command==string(LD_COMMAND_SUSPEND))
        {
          // send prepare to myself using a dummyport
          GCFDummyPort dummyPort(this,string("SRG_command_dummy"),LOGICALDEVICE_PROTOCOL);
          LOGICALDEVICESuspendEvent suspendEvent;
          dispatch(suspendEvent,dummyPort); // dummyport
        }
        // RESUME
        else if(command==string(LD_COMMAND_RESUME))
        {
          // send prepare to myself using a dummyport
          GCFDummyPort dummyPort(this,string("SRG_command_dummy"),LOGICALDEVICE_PROTOCOL);
          LOGICALDEVICEResumeEvent resumeEvent;
          dispatch(resumeEvent,dummyPort); // dummyport
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
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  
  TStationReceptorVectorIter rIt;
  for(rIt = m_stationReceptors.begin();rIt != m_stationReceptors.end(); ++rIt)
  {
    resourceManager->requestResource(getName(),(*rIt).rcu->getName());
  }
  
  // send claim message to all receptors
  LOGICALDEVICEClaimEvent claimEvent;
  sendToAllReceptors(claimEvent);
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
  
  // send prepare message to the receptors
  // all parameters are forwarded to the receptors
  LOGICALDEVICEPrepareEvent prepareEvent;
  prepareEvent.parameters = parameters;
  sendToAllReceptors(prepareEvent);
}

void AVTStationReceptorGroup::concreteResume(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concreteResume",getName().c_str()));
  // resume my own resources
  
  // send resume message to receptors
  LOGICALDEVICEResumeEvent resumeEvent;
  sendToAllReceptors(resumeEvent);
}

void AVTStationReceptorGroup::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concreteSuspend",getName().c_str()));
  // suspend my own resources
  
  // send suspend message to receptors
  LOGICALDEVICESuspendEvent suspendEvent;
  sendToAllReceptors(suspendEvent);
}

void AVTStationReceptorGroup::concreteRelease(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptorGroup(%s)::concreteRelease",getName().c_str()));
  // release my own resources
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  
  TStationReceptorVectorIter rIt;
  for(rIt = m_stationReceptors.begin();rIt != m_stationReceptors.end(); ++rIt)
  {
    resourceManager->releaseResource(getName(),(*rIt).rcu->getName());
  }
  
  // send release message to receptors
  LOGICALDEVICEReleaseEvent releaseEvent;
  sendToAllReceptors(releaseEvent);
}

