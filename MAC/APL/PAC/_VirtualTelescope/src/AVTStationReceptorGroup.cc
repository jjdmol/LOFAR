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
#include <GCF/ParameterSet.h>

#include <APLCommon/APL_Defines.h>
#include "AVTDefines.h"
#include "AVTPropertyDefines.h"
#include "AVTStationReceptorGroup.h"
#include "AVTStationReceptor.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"
#include "AVTResourceManager.h"

using namespace LOFAR;
using namespace GCF;
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
  LOG_TRACE_FLOW(formatString("TStationReceptorConnection(0x%x)::TStationReceptorConnection",this));
  rcu->addClientInterTaskPort(clientPort.get());
}

AVTStationReceptorGroup::AVTStationReceptorGroup(string& taskName, 
                                                 const string& scope,
                                                 const string& APCName,
                                                 vector<shared_ptr<AVTStationReceptor> >& rcus) :
  AVTLogicalDevice(taskName,scope,TYPE_LCU_PAC_SRG,APCName),
  m_stationReceptors(),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::AVTStationReceptorGroup",getName().c_str()));

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
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::~AVTStationReceptorGroup",getName().c_str()));
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

bool AVTStationReceptorGroup::checkQualityRequirements()
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::%s",getName().c_str(),__func__));
  bool requirementsMet=true;
  int  unavailableCounter = 0;
  int maxUnavailable = ParameterSet::instance()->getInt(PARAM_MAX_SRG_RESOURCES_UNAVAILABLE);
  
  // quality requirements for this station receptor group:
  // - not more than 1 antenna unavailable
  // - not more than 1 antenna in alarm
  
  TStationReceptorVectorIter it = m_stationReceptors.begin();
  while(requirementsMet && it!=m_stationReceptors.end())
  {
    if(!(*it).rcu->checkQualityRequirements())
    {
      LOG_WARN(formatString("AVTStationReceptorGroup(%s)::%s %s unavailable",getName().c_str(),__func__,it->clientPort->getName().c_str()));
      unavailableCounter++;
      requirementsMet = (unavailableCounter <= maxUnavailable);
    }
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
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s,0x%x)::isStationReceptorClient",getName().c_str(),this));
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
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s,0x%x)::%s(0x%x,%s)",getName().c_str(),this,__func__,&port,(connected?"true":"false")));
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
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s,0x%x)::%s: %s",getName().c_str(),this,__func__,(allConnected?"true":"false")));
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
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s,0x%x)::%s(%d): %s",getName().c_str(),this,__func__,state,(allInState?"true":"false")));
  return allInState;
}

void AVTStationReceptorGroup::sendToAllReceptors(GCFEvent& event)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s,0x%x)::sendToAllReceptors",getName().c_str(),this));
  TStationReceptorVectorIter it;
  for(it=m_stationReceptors.begin();it!=m_stationReceptors.end();++it)
  {
    (*it).clientPort->send(event);
  }
}

void AVTStationReceptorGroup::concreteDisconnected(GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concreteDisconnected",getName().c_str()));
  // go to initial state only if the connection with the receptor is lost.
  if(isStationReceptorClient(port))
  {
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concrete_initial_state (%s)",getName().c_str(),evtstr(event)));
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
      LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concrete_initial_state, default (%s)",getName().c_str(),evtstr(event)));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concrete_claiming_state (%s)",getName().c_str(),evtstr(event)));
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
      LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concrete_claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished, bool& error)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concrete_preparing_state (%s)",getName().c_str(),evtstr(event)));
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
      LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concrete_preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_active_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::%s (%s)",getName().c_str(),__func__,evtstr(event)));
  return GCFEvent::NOT_HANDLED;
}

GCFEvent::TResult AVTStationReceptorGroup::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concrete_releasing_state (%s)",getName().c_str(),evtstr(event)));
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
      LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concrete_releasing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTStationReceptorGroup::handlePropertySetAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        // property set loaded, now load apc
        m_properties.configure(m_APC);
      }
      else
      {
        LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",getName().c_str(),pPropAnswer->pScope));
      }
      break;
    }
    
    case F_PS_CONFIGURED:
    {
      GCFConfAnswerEvent* pConfAnswer=static_cast<GCFConfAnswerEvent*>(&answer);
      if(pConfAnswer->result == GCF_NO_ERROR)
      {
        LOG_TRACE_FLOW(formatString("%s : apc %s Loaded",getName().c_str(),pConfAnswer->pApcName));
        apcLoaded();
      }
      else
      {
        LOG_ERROR(formatString("%s : apc %s NOT LOADED",getName().c_str(),pConfAnswer->pApcName));
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      if ((pPropAnswer->pValue->getType() == LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, PROPNAME_COMMAND) != 0))
      {
        // command received
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        vector<string> parameters;
        string command;
        AVTUtilities::decodeCommand(commandString,command,parameters);
        
        // PREPARE <starttime>,<stoptime>,<frequency>,<subbands>,<directiontype>,<directionangle1>,<directionangle2>
        if(command==string(LD_COMMAND_PREPARE))
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

void AVTStationReceptorGroup::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concreteClaim",getName().c_str()));
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
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concretePrepare",getName().c_str()));
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
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concreteResume",getName().c_str()));
  // resume my own resources
  
  // send resume message to receptors
  LOGICALDEVICEResumeEvent resumeEvent;
  sendToAllReceptors(resumeEvent);
}

void AVTStationReceptorGroup::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concreteSuspend",getName().c_str()));
  // suspend my own resources
  
  // send suspend message to receptors
  LOGICALDEVICESuspendEvent suspendEvent;
  sendToAllReceptors(suspendEvent);
}

void AVTStationReceptorGroup::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_FLOW(formatString("AVTStationReceptorGroup(%s)::concreteRelease",getName().c_str()));
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

