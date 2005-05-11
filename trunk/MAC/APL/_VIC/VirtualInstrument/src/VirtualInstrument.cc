//#  VirtualInstrument.cc: Implementation of the Virtual VirtualInstrument task
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
#include <boost/shared_ptr.hpp>
#include <Common/lofar_sstream.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>
#include <APLCommon/APLUtilities.h>
#include "VirtualInstrument.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace AVI
{
INIT_TRACER_CONTEXT(VirtualInstrument,LOFARLOGGER_PACKAGE);

// Logical Device version
const string VirtualInstrument::VI_VERSION = string("1.0");

const string VirtualInstrument::VI_PROPNAME_CONNECTEDSTATIONS     = string("connectedStations");
const string VirtualInstrument::VI_PROPNAME_DISCONNECTEDSTATIONS  = string("disconnectedStations");

VirtualInstrument::VirtualInstrument(const string& taskName, 
                                     const string& parameterFile, 
                                     GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,VI_VERSION),
  m_vtSchedulerPropertySets(),
  m_disconnectedVTSchedulerPropertySets(),
  m_vtPropertySets(),
  m_qualityCheckTimerId(0),
  m_retryPropsetLoadTimerId(0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  try
  {  
    string childs = m_parameterSet.getString("childs");
    vector<string> childsVector;
    APLUtilities::string2Vector(childs,childsVector);
    
    vector<string>::iterator it = childsVector.begin();
    while(it != childsVector.end())
    {
      // filter the Virtual Telescopes from the parameter file: In increment 1, they
      // are old style
      // if <child>.logicalDeviceType == LDTYPE_VIRTUALTELESCOPE
      int ldType = m_parameterSet.getInt((*it) + ".logicalDeviceType");
      if(ldType == LDTYPE_VIRTUALTELESCOPE)
      {
        string remoteSystemName = m_parameterSet.getString((*it) + ".remoteSystem");

        // get <child>.oldStyle* and create that propertyset
        string propsetName = remoteSystemName + string(":") + m_parameterSet.getString((*it) + ".oldStyleSchedulerName");
        string propsetType = m_parameterSet.getString((*it) + ".oldStyleSchedulerType");
        TGCFExtPropertySetPtr vtSchedulerPropset(new GCFExtPropertySet(propsetName.c_str(),propsetType.c_str(),&m_propertySetAnswer));
        m_vtSchedulerPropertySets[(*it)] = vtSchedulerPropset;
        
        propsetName = remoteSystemName + string(":") + m_parameterSet.getString((*it) + ".propertysetBaseName");
        propsetType = m_parameterSet.getString((*it) + ".propertysetDetailsType");
        TGCFExtPropertySetPtr vtPropset(new GCFExtPropertySet(propsetName.c_str(),propsetType.c_str(),&m_propertySetAnswer));
        m_vtPropertySets[(*it)] = vtPropset;
      }
      ++it;
    }
  }
  catch(Exception& e)
  {
    THROW(APLCommon::ParameterNotFoundException,e.message());
  }
  m_retryPropsetLoadTimerId = m_serverPort.setTimer(10L);
}


VirtualInstrument::~VirtualInstrument()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  m_vtSchedulerPropertySets.clear();
  m_disconnectedVTSchedulerPropertySets.clear();
}

void VirtualInstrument::concrete_handlePropertySetAnswer(GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(answer)).c_str());
  switch(answer.signal)
  {
    case F_SUBSCRIBED:
    {
      break;
    }
    case F_UNSUBSCRIBED:
    {
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      
      bool found=false;
      TString2PropsetMap::iterator it=m_vtPropertySets.begin();
      while(!found && it!=m_vtPropertySets.end())
      {
        if(strstr(pPropAnswer->pPropName,it->second->getFullScope().c_str()) != 0)
        {
          found=true;
          if(strstr(pPropAnswer->pPropName, ".status") != 0)
          {
            GCFPVString* pvString=(GCFPVString*)pPropAnswer->pValue;
            LOG_DEBUG(formatString("VI(%s) - %s=%s",getName().c_str(),pPropAnswer->pPropName,pvString->getValue().c_str()));
            if(pvString->getValue() != string("Active"))
            {
              LOG_FATAL(formatString("VI(%s): quality too low",getName().c_str()));
              // any VT not running results in low quality
              m_serverPort.cancelTimer(m_qualityCheckTimerId);
              m_qualityCheckTimerId = 0;
              _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED,LD_RESULT_LOW_QUALITY);
            }
          }
        }
        ++it;
      }
      break;
    }
    case F_VGETRESP:
    {
      break;
    }
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(strstr(pPropAnswer->pScope, "Scheduler") != 0) // if it is a scheduler propertyset
      {
        if(pPropAnswer->result == GCF_NO_ERROR)
        {
          GCFPValueArray stationsVector;
  
          TString2PropsetMap::iterator it=m_vtSchedulerPropertySets.begin();
          while(it!=m_vtSchedulerPropertySets.end())
          {
            if(strstr(pPropAnswer->pScope, it->second->getScope().c_str()) != 0)
            {
	            it->second->subscribeProp(string("status"));
              _writeScheduleCommand(it->first,it->second);
            }
            GCFPVString* newItem=new GCFPVString(it->first);
            stationsVector.push_back(newItem);
            ++it;
          }
          GCFPVDynArr newStations(LPT_STRING,stationsVector);
          m_detailsPropertySet->setValue(VI_PROPNAME_CONNECTEDSTATIONS,newStations);
          
          // free memory. bah
          for(GCFPValueArray::iterator pvaIt = stationsVector.begin();pvaIt != stationsVector.end();++pvaIt)
          {
            delete *pvaIt;
          }
        }
        else
        {
          LOG_WARN(formatString("failed to load propertyset %s",pPropAnswer->pScope));
  
          bool propsetFound=false;
          TString2PropsetMap::iterator it=m_vtSchedulerPropertySets.begin();
          while(!propsetFound && it!=m_vtSchedulerPropertySets.end())
          {
            if(strstr(pPropAnswer->pScope, it->second->getScope().c_str()) != 0)
            {
              m_disconnectedVTSchedulerPropertySets[it->first] = it->second;
              propsetFound=true;
            }
            ++it;
          }
  
          GCFPValueArray stationsVector;
          it = m_disconnectedVTSchedulerPropertySets.begin();
          while(it != m_disconnectedVTSchedulerPropertySets.end())
          {
            GCFPVString* newItem=new GCFPVString(it->first);
            stationsVector.push_back(newItem);
            ++it;
          }
          GCFPVDynArr newStations(LPT_STRING,stationsVector);
          m_detailsPropertySet->setValue(VI_PROPNAME_DISCONNECTEDSTATIONS,newStations);
          // free memory. bah
          for(GCFPValueArray::iterator pvaIt = stationsVector.begin();pvaIt != stationsVector.end();++pvaIt)
          {
            delete *pvaIt;
          }
        }
      }
      else // it is a VT propertyset
      {
      }
      break;
    }
    case F_EXTPS_UNLOADED:
    {
      break;
    }
    case F_PS_CONFIGURED:
    {
      break;
    }
    case F_MYPS_ENABLED:
    {
      break;
    }
    case F_MYPS_DISABLED:
    {
      break;
    }
    case F_SERVER_GONE:
    {
      break;
    }
    default:
      break;
  }
}

GCFEvent::TResult VirtualInstrument::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:
    {
      TString2PropsetMap::iterator it;
      for(it=m_vtSchedulerPropertySets.begin();it!=m_vtSchedulerPropertySets.end();++it)
      {
        it->second->load();
      }
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // load property sets of the scheduled VT's
      TString2PropsetMap::iterator it;
      for(it=m_vtPropertySets.begin();it!=m_vtPropertySets.end();++it)
      {
        it->second->load();
      }
      break;
    }
    
    case LOGICALDEVICE_CLAIMED:
    {
      LOG_TRACE_FLOW("CLAIMED received");
      // check if all childs are not claiming anymore
      // now only checking VB, because VT's are old style
      if(_childsNotInState(100.0, LDTYPE_VIRTUALBACKEND/*LDTYPE_NO_TYPE*/, LOGICALDEVICE_STATE_CLAIMING))
      {
        LOG_TRACE_FLOW("No childs CLAIMING");
        // ALL virtual backend childs must be claimed
        if(_childsInState(100.0, LDTYPE_VIRTUALBACKEND, LOGICALDEVICE_STATE_CLAIMED))
        {
          LOG_TRACE_FLOW("100% VB's CLAIMED");
          // 50% of the VT's must be claimed
          if(_childsInState(00.0, LDTYPE_VIRTUALTELESCOPE, LOGICALDEVICE_STATE_CLAIMED))
          {
            LOG_TRACE_FLOW("00% VT's CLAIMED");
            
            LOG_TRACE_FLOW("need to check old style VT's");
            
            // enter claimed state
            newState  = LOGICALDEVICE_STATE_CLAIMED;
            errorCode = LD_RESULT_NO_ERROR;
          }
          else
          {
            newState  = LOGICALDEVICE_STATE_IDLE;
            errorCode = LD_RESULT_LOW_QUALITY;
          }
        }
        else
        {
          newState  = LOGICALDEVICE_STATE_IDLE;
          errorCode = LD_RESULT_LOW_QUALITY;
        }
      }
      break;
    }
    
    default:
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
    {
      LOG_TRACE_FLOW("PREPARED received");
      // check if all childs are not preparing anymore
      // now only checking VB, because VT's are old style
      if(_childsNotInState(100.0, LDTYPE_VIRTUALBACKEND/*LDTYPE_NO_TYPE*/, LOGICALDEVICE_STATE_PREPARING))
      {
        LOG_TRACE_FLOW("No childs PREPARING");
        // ALL virtual backend childs must be prepared
        if(_childsInState(100.0, LDTYPE_VIRTUALBACKEND, LOGICALDEVICE_STATE_SUSPENDED))
        {
          LOG_TRACE_FLOW("All VB's SUSPENDED");
          // 00% of the VT's must be prepared
          if(_childsInState(00.0, LDTYPE_VIRTUALTELESCOPE, LOGICALDEVICE_STATE_SUSPENDED))
          {
            LOG_TRACE_FLOW("00% VT's SUSPENDED");
            // enter suspended state
            newState=LOGICALDEVICE_STATE_SUSPENDED;
          }
          else
          {
            newState  = LOGICALDEVICE_STATE_CLAIMED;
            errorCode = LD_RESULT_LOW_QUALITY;
          }
        }
        else
        {
          newState  = LOGICALDEVICE_STATE_CLAIMED;
          errorCode = LD_RESULT_LOW_QUALITY;
        }
      }
      break;
    }

    default:
      break;
  }
  
  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_active_state(GCFEvent& event, GCFPortInterface& /*p*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      m_qualityCheckTimerId = m_serverPort.setTimer(5L);
      break;
    }
          
    default:
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualInstrument::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

//  newState=LOGICALDEVICE_STATE_IDLE;
  newState=LOGICALDEVICE_STATE_GOINGDOWN;
  return status;
}

void VirtualInstrument::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
}

void VirtualInstrument::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // workaround for old style Virtual Telescope: set command property
  TString2PropsetMap::iterator it;
  for(it=m_vtSchedulerPropertySets.begin();it!=m_vtSchedulerPropertySets.end();++it)
  {
    try
    {
      string cancelMessage("CANCEL 1");      
      GCFPVString pvCancel(cancelMessage);
      it->second->setValue("command",pvCancel);
    }
    catch(Exception& e)
    {
      LOG_FATAL(e.message().c_str());
    }
  }  
}

void VirtualInstrument::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteHandleTimers(GCFTimerEvent& timerEvent, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  if(timerEvent.id == m_retryPropsetLoadTimerId)
  {
    // loop through the buffered events and try to send each one.
    TString2PropsetMap::iterator it = m_disconnectedVTSchedulerPropertySets.begin();
    while(it != m_disconnectedVTSchedulerPropertySets.end())
    {
      _writeScheduleCommand(it->first,it->second);
      // remove the event from the map. If the propset cannot be loaded, it will 
      // be added again later on.
      m_disconnectedVTSchedulerPropertySets.erase(it);
      it = m_disconnectedVTSchedulerPropertySets.begin();
    }

    // keep on polling
    m_retryPropsetLoadTimerId = m_serverPort.setTimer(5L);
  }
  else if(timerEvent.id == m_qualityCheckTimerId)
  {
    if(!_checkQualityRequirements())
    {
      LOG_FATAL(formatString("VI(%s): quality too low",getName().c_str()));
      _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED,LD_RESULT_LOW_QUALITY);
    }
    else
    {
      // keep on polling
      m_qualityCheckTimerId = m_serverPort.setTimer(5L);
    }
  }
}

bool VirtualInstrument::_writeScheduleCommand(const string& name, TGCFExtPropertySetPtr& propset)
{
  bool scheduleSent(false);
  try
  {
    stringstream schedule;
    schedule << "SCHEDULE ";
    // SCHEDULE <sch.nr>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
    //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
    schedule << "1,";
    schedule << "VT,";
    schedule << name << ",";
    
    string childs = m_parameterSet.getString(name + string(".") + string("childs"));
    vector<string> childsVector;
    APLUtilities::string2Vector(childs,childsVector);
    
    schedule << childsVector[0] << ",";
    schedule << childsVector[1] << ",";
    time_t startTime = _decodeTimeParameter(m_parameterSet.getString(name + string(".") + string("startTime")));
    schedule << startTime << ",";
    time_t stopTime = _decodeTimeParameter(m_parameterSet.getString(name + string(".") + string("stopTime")));
    schedule << stopTime << ",";
    
    schedule << m_parameterSet.getString(name + string(".") + string("frequency")) << ",";
    schedule << m_parameterSet.getString(name + string(".") + string("subbands")) << ",";
    schedule << m_parameterSet.getString(name + string(".") + string("directionType")) << ",";
    schedule << m_parameterSet.getString(name + string(".") + string("angle1")) << ",";
    schedule << m_parameterSet.getString(name + string(".") + string("angle2")) << ",";
    
    GCFPVString pvSchedule(schedule.str());
    propset->setValue("command",pvSchedule);
    scheduleSent = true;
  }
  catch(Exception& e)
  {
    LOG_FATAL(e.message().c_str());
  }
  return scheduleSent;
}

bool VirtualInstrument::_checkQualityRequirements()
{
  bool requirementsMet = false;
  
  // ALL virtual backend childs must be active
  if(_childsInState(100.0, LDTYPE_VIRTUALBACKEND, LOGICALDEVICE_STATE_ACTIVE))
  {
    LOG_TRACE_FLOW("All VB's RESUMED");
    // 00% of the VT's must be active
    if(_childsInState(00.0, LDTYPE_VIRTUALTELESCOPE, LOGICALDEVICE_STATE_ACTIVE))
    {
      LOG_TRACE_FLOW("00% VT's RESUMED");
      requirementsMet=true;
      
      // additonal check because of old style VT's: get status from the property sets
      TString2PropsetMap::iterator it;
      for(it=m_vtPropertySets.begin();it!=m_vtPropertySets.end();++it)
      {
        it->second->requestValue("status");
      }
    }
  }
  
  return requirementsMet;
}

}; // namespace VIC
}; // namespace LOFAR

