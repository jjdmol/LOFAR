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
#include <GCF/ParameterSet.h>

#include <APLCommon/APL_Defines.h>
#include "AVTDefines.h"
#include "AVTPropertyDefines.h"
#include "AVTStationReceptor.h"
#include "AVTStationBeamformer.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"
#include "AVTResourceManager.h"

using namespace LOFAR;
using namespace LOFAR::AVT;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;
using namespace boost;

INIT_TRACER_CONTEXT(AVTStationReceptor,LOFARLOGGER_PACKAGE);

AVTStationReceptor::AVTStationReceptor(string& taskName, 
                                         const string& scope,
                                         const string& APCName,
                                         const list<TPropertyInfo>& requiredResources) :
  AVTLogicalDevice(taskName,scope,TYPE_LCU_PAC_SR,APCName),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0),
  m_requiredResources(),
  m_requiredResourcesStatus()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  list<TPropertyInfo>::const_iterator it=requiredResources.begin();
  while(it!=requiredResources.end())
  {
    shared_ptr<GCFExtProperty> pProp(new GCFExtProperty(*it));
    pProp->setAnswer(&m_propertySetAnswer);
    m_requiredResources[it->propName] = pProp;
    m_requiredResourcesStatus[it->propName] = false;
    ++it;
  }
}


AVTStationReceptor::~AVTStationReceptor()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

bool AVTStationReceptor::checkQualityRequirements()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  bool requirementsMet=true;
  int  unavailableCounter = 0;
  int maxUnavailable = ParameterSet::instance()->getInt(PARAM_MAX_SR_RESOURCES_UNAVAILABLE);
  // quality requirements for this station receptor:
  // - no resources unavailable
  // - no resources in alarm
  map<string,bool>::iterator it=m_requiredResourcesStatus.begin();
  while(requirementsMet && it!=m_requiredResourcesStatus.end())
  {
    if(!it->second)
    {
      LOG_WARN(formatString("AVTStationReceptor(%s)::%s %s unavailable",getName().c_str(),__func__,it->first.c_str()));
      unavailableCounter++;
      requirementsMet = (unavailableCounter <= maxUnavailable);
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
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

GCFEvent::TResult AVTStationReceptor::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      TExtPropertyMap::iterator it=m_requiredResources.begin();
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
      LOG_DEBUG(formatString("AVTStationReceptor(%s)::concrete_initial_state, default (%s)",getName().c_str(),evtstr(event)));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptor::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
      stateFinished=true;
      break;
    
    default:
      LOG_DEBUG(formatString("AVTStationReceptor(%s)::concrete_claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTStationReceptor::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished, bool& error)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  stateFinished=false;
  error=false;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
      stateFinished=true;
      break;
    
    default:
      LOG_DEBUG(formatString("AVTStationReceptor(%s)::concrete_preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationReceptor::concrete_active_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  return GCFEvent::NOT_HANDLED;
}

GCFEvent::TResult AVTStationReceptor::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_RELEASED:
      stateFinished=true;
      break;
    
    default:
      LOG_DEBUG(formatString("AVTStationReceptor(%s)::concrete_releasing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTStationReceptor::handlePropertySetAnswer(GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  switch(answer.signal)
  {
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        // property set loaded, now load apc
//        m_properties.configure(m_APC);
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
        LOG_DEBUG(formatString("%s : apc %s Loaded",getName().c_str(),pConfAnswer->pApcName));
        apcLoaded();
      }
      else
      {
        LOG_WARN(formatString("%s : apc %s NOT LOADED",getName().c_str(),pConfAnswer->pApcName));
      }
      break;
    }
    
    case F_VGETRESP:
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      
      // is it a required resource status?
      string propName(pPropAnswer->pPropName);
      bool found=false;
      map<string,bool>::iterator it=m_requiredResourcesStatus.begin();
      while(!found && it != m_requiredResourcesStatus.end())
      {
        string resName(it->first);
        found = (string::npos != propName.find(resName));// propName may include something like System1:
        if(found)
        {
          GCFPVUnsigned unsignedValue;
          unsignedValue.copy(*pPropAnswer->pValue);
          it->second = (unsignedValue.getValue() == 0); // true if status is OK
          if(it->second)
          {
            LOG_INFO(formatString("%s : Required resource %s available (%d)",getName().c_str(),resName.c_str(),unsignedValue.getValue()));
          }
          else
          {
            LOG_WARN(formatString("%s : Required resource %s not available (%d)",getName().c_str(),resName.c_str(),unsignedValue.getValue()));
          }
        }
        ++it;
      }
      break;
    }  

    default:
      break;
  }  
}

void AVTStationReceptor::concreteClaim(GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // claim my own resources
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  
  TExtPropertyMap::iterator it;
  for(it=m_requiredResources.begin();it!=m_requiredResources.end();++it)
  {
    resourceManager->requestResource(getName(),it->first);
  }
  
  LOGICALDEVICEClaimedEvent claimedEvent;
  dispatch(claimedEvent,port);
}

void AVTStationReceptor::concretePrepare(GCFPortInterface& port, string& parameters)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
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
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // resume my own resources
  
}

void AVTStationReceptor::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // suspend my own resources
  
}

void AVTStationReceptor::concreteRelease(GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // release my own resources
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  
  TExtPropertyMap::iterator it;
  for(it=m_requiredResources.begin();it!=m_requiredResources.end();++it)
  {
    resourceManager->releaseResource(getName(),it->first);
  }
  
  LOGICALDEVICEReleasedEvent releasedEvent;
  dispatch(releasedEvent,port);
}

