//#  AVTStationReceptor.cc: Implementation of the Virtual Telescope task
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
#include <GCF/GCF_PVUnsigned.h>

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTStationReceptor.h"
#include "AVTStationBeamformer.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"
#include "AVTResourceManager.h"

using namespace LOFAR;
using namespace AVT;
using namespace std;
using namespace boost;

AVTStationReceptor::AVTStationReceptor(string& taskName, 
                                         const TPropertySet& primaryPropertySet,
                                         const string& APCName,
                                         const string& APCScope,
                                         const list<string>& requiredResources) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0),
  m_requiredResources(),
  m_requiredResourcesStatus()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::AVTStationReceptor",getName().c_str()));
  
  list<string>::const_iterator it=requiredResources.begin();
  while(it!=requiredResources.end())
  {
    shared_ptr<GCFProperty> pProp(new GCFProperty(*it));
    pProp->setAnswer(&m_propertySetAnswer);
    m_requiredResources[*it] = pProp;
    m_requiredResourcesStatus[*it] = false;
    ++it;
  }
}


AVTStationReceptor::~AVTStationReceptor()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::~AVTStationReceptor",getName().c_str()));
}

bool AVTStationReceptor::checkQualityRequirements()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::%s",getName().c_str(),__func__));
  bool requirementsMet=true;
  
  // quality requirements for this station receptor:
  // - no resources unavailable
  // - no resources in alarm
  map<string,bool>::iterator it=m_requiredResourcesStatus.begin();
  while(requirementsMet && it!=m_requiredResourcesStatus.end())
  {
    requirementsMet = it->second;
    if(!requirementsMet)
    {
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::%s requirement %s not met",getName().c_str(),__func__,it->first.c_str()));
    }
    ++it;
  }
  
  return requirementsMet;
}

void AVTStationReceptor::setStartTime(const time_t startTime)
{
  // activate timer that triggers the prepare command
  m_startTime=startTime;
}

void AVTStationReceptor::setStopTime(const time_t stopTime)
{
  // activate timer that triggers the suspend command
  m_stopTime=stopTime;
}

void AVTStationReceptor::setFrequency(const double frequency)
{
  m_frequency=frequency;
}

void AVTStationReceptor::concreteDisconnected(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concreteDisconnected",getName().c_str()));
}

GCFEvent::TResult AVTStationReceptor::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concrete_initial_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      map<string,shared_ptr<GCFProperty> >::iterator it=m_requiredResources.begin();
      while(it!=m_requiredResources.end())
      {
        it->second->subscribe();
        it->second->requestValue();
        ++it;
      }

      TRAN(AVTLogicalDevice::idle_state);
      break;
    }
    
    case F_CONNECTED:
    {
      break;
    }

    case F_DISCONNECTED:
      break;

    case F_TIMER:
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concrete_initial_state, default (%s)",getName().c_str(),evtstr(event)));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptor::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concrete_claiming_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
      stateFinished=true;
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concrete_claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTStationReceptor::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished, bool& error)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concrete_preparing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  stateFinished=false;
  error=false;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
      stateFinished=true;
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concrete_preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptor::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concrete_releasing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_RELEASED:
      stateFinished=true;
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concrete_releasing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTStationReceptor::handlePropertySetAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_MYPLOADED:
    {
      // property set loaded, now load apc
      m_APC.load(false);
      break;
    }
    
    case F_VGETRESP:
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&answer);
      assert(pPropAnswer);
      
      // is it a required resource status?
      map<string,bool>::iterator it=m_requiredResourcesStatus.find(string(pPropAnswer->pPropName));
      if(it != m_requiredResourcesStatus.end())
      {
        GCFPVUnsigned unsignedValue;
        unsignedValue.copy(*pPropAnswer->pValue);
        it->second = (unsignedValue.getValue() == 0); // true if status is OK
      }
      break;
    }  

    default:
      break;
  }  
}

void AVTStationReceptor::handleAPCAnswer(GCFEvent& answer)
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

void AVTStationReceptor::concreteClaim(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concreteClaim",getName().c_str()));
  // claim my own resources
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  
  map<string,shared_ptr<GCFProperty> >::iterator it;
  for(it=m_requiredResources.begin();it!=m_requiredResources.end();++it)
  {
    resourceManager->requestResource(getName(),it->first);
  }
  
  LOGICALDEVICEClaimedEvent claimedEvent;
  dispatch(claimedEvent,port);
}

void AVTStationReceptor::concretePrepare(GCFPortInterface& port, string& parameters)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concretePrepare",getName().c_str()));
  // prepare my own resources
  vector<string> decodedParameters;
  AVTUtilities::decodeParameters(parameters,decodedParameters);
  // parameters: start time,stop time,frequency,subbands,direction type,angle1,angle2
  setStartTime(atoi(decodedParameters[0].c_str()));
  setStopTime(atoi(decodedParameters[1].c_str()));
  setFrequency(atof(decodedParameters[2].c_str()));
  
  LOGICALDEVICEPreparedEvent preparedEvent;
  dispatch(preparedEvent,port);
}

void AVTStationReceptor::concreteResume(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concreteResume",getName().c_str()));
  // resume my own resources
  
}

void AVTStationReceptor::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concreteSuspend",getName().c_str()));
  // suspend my own resources
  
}

void AVTStationReceptor::concreteRelease(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationReceptor(%s)::concreteRelease",getName().c_str()));
  // release my own resources
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  
  map<string,shared_ptr<GCFProperty> >::iterator it;
  for(it=m_requiredResources.begin();it!=m_requiredResources.end();++it)
  {
    resourceManager->releaseResource(getName(),it->first);
  }
  
  LOGICALDEVICEReleasedEvent releasedEvent;
  dispatch(releasedEvent,port);
}

