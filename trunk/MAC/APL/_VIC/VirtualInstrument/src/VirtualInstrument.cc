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
#include <APLCommon/APLUtilities.h>
#include <APLCommon/StartDaemon_Protocol.ph>
#include "VirtualInstrument.h"

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace AVI
{
INIT_TRACER_CONTEXT(VirtualInstrument,LOFARLOGGER_PACKAGE);

VirtualInstrument::VirtualInstrument(const string& taskName, const string& parameterFile) :
  LogicalDevice(taskName,parameterFile),
  m_vtSchedulerPropertySets()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // filter the Virtual Telescopes from the parameter file: In increment 1, they
  // are old style

  try
  {  
    // 1. get childs parameter
    string childs = m_parameterSet.getString("childs");
    vector<string> childsVector;
    APLUtilities::string2Vector(childs,childsVector);
    
    vector<string>::iterator it = childsVector.begin();
    while(it != childsVector.end())
    {
      // 2. if <child>.logicalDeviceType == LDTYPE_VIRTUALTELESCOPE
      int ldType = m_parameterSet.getInt((*it) + ".logicalDeviceType");
      if(ldType == LDTYPE_VIRTUALTELESCOPE)
      {
        // 3.   get <child>.oldStyle* and create that propertyset
        string propsetName = m_parameterSet.getString((*it) + ".oldStyleSchedulerName");
        string propsetType = m_parameterSet.getString((*it) + ".oldStyleSchedulerType");
        
        TGCFExtPropertySetPtr vtPropset(new GCFExtPropertySet(propsetName.c_str(),propsetType.c_str(),&m_propertySetAnswer));
        m_vtSchedulerPropertySets[(*it)] = vtPropset;
      }
      ++it;
    }
  }
  catch(Exception& e)
  {
    THROW(APLCommon::ParameterNotFoundException,e.message());
  }
}


VirtualInstrument::~VirtualInstrument()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concrete_handlePropertySetAnswer(::GCFEvent& answer)
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
      break;
    }
    case F_VGETRESP:
    {
      break;
    }
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        bool scheduleSent=false;
        TString2PropsetMap::iterator it=m_vtSchedulerPropertySets.begin();
        while(!scheduleSent && it!=m_vtSchedulerPropertySets.end())
        {
          if(strstr(pPropAnswer->pScope, it->second->getScope().c_str()) != 0)
          {
            stringstream schedule;
            schedule << "SCHEDULE ";
            // SCHEDULE <sch.nr>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
            //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
            schedule << "1,";
            schedule << "VT,";
            schedule << it->first << ",";
            
            string childs = m_parameterSet.getString(it->first + string(".") + string("childs"));
            vector<string> childsVector;
            APLUtilities::string2Vector(childs,childsVector);
            
            schedule << childsVector[0] << ",";
            schedule << childsVector[1] << ",";
            time_t startTime = _decodeTimeParameter(m_parameterSet.getString(it->first + string(".") + string("startTime")));
            schedule << startTime << ",";
            time_t stopTime = _decodeTimeParameter(m_parameterSet.getString(it->first + string(".") + string("stopTime")));
            schedule << stopTime << ",";
            
            schedule << m_parameterSet.getString(it->first + string(".") + string("frequency")) << ",";
            schedule << m_parameterSet.getString(it->first + string(".") + string("subbands")) << ",";
            schedule << m_parameterSet.getString(it->first + string(".") + string("directionType")) << ",";
            schedule << m_parameterSet.getString(it->first + string(".") + string("angle1")) << ",";
            schedule << m_parameterSet.getString(it->first + string(".") + string("angle2")) << ",";
            
            GCFPVString pvSchedule(schedule.str());
            it->second->setValue("command",pvSchedule);
            scheduleSent = true;
          }
          ++it;
        }
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

::GCFEvent::TResult VirtualInstrument::concrete_initial_state(::GCFEvent& event, ::GCFPortInterface& /*p*/, TLogicalDeviceState& newState)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = GCFEvent::HANDLED;
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

::GCFEvent::TResult VirtualInstrument::concrete_idle_state(::GCFEvent& event, ::GCFPortInterface& /*p*/, TLogicalDeviceState& newState)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = GCFEvent::HANDLED;
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

::GCFEvent::TResult VirtualInstrument::concrete_claiming_state(::GCFEvent& event, ::GCFPortInterface& /*p*/, TLogicalDeviceState& newState)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  
  return status;
}

::GCFEvent::TResult VirtualInstrument::concrete_preparing_state(::GCFEvent& event, ::GCFPortInterface& /*p*/, TLogicalDeviceState& newState)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  
  return status;
}

::GCFEvent::TResult VirtualInstrument::concrete_active_state(::GCFEvent& event, ::GCFPortInterface& /*p*/, TLogicalDeviceState& newState)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  
  return status;
}

::GCFEvent::TResult VirtualInstrument::concrete_releasing_state(::GCFEvent& event, ::GCFPortInterface& /*p*/, TLogicalDeviceState& newState)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  
  return status;
}

void VirtualInstrument::concreteClaim(::GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
}

void VirtualInstrument::concretePrepare(::GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteResume(::GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteSuspend(::GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteRelease(::GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteParentDisconnected(::GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void VirtualInstrument::concreteChildDisconnected(::GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

}; // namespace VIC
}; // namespace LOFAR
