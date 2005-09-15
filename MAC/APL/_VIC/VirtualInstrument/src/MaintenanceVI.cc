//#  MaintenanceVI.cc: Implementation of the Maintenance VirtualInstrument task
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

#include <boost/shared_ptr.hpp>
#include <Common/lofar_sstream.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVDynArr.h>
#include <APLCommon/APLUtilities.h>
#include "MaintenanceVI.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace AVI
{
INIT_TRACER_CONTEXT(MaintenanceVI,LOFARLOGGER_PACKAGE);

// Logical Device version
const string MaintenanceVI::VI_VERSION = string("1.0");

const string MaintenanceVI::VI_PROPNAME_RESOURCES     = string("__resources"); // the '__' turns it into a reference datapoint
const string MaintenanceVI::TYPE_LCU_PIC_MAINTENANCE  = string("TLcuPicMaintenance");
const string MaintenanceVI::PROPNAME_STATUS           = string("status");
const string MaintenanceVI::PROPNAME_FROMTIME         = string("fromTime");
const string MaintenanceVI::PROPNAME_TOTIME           = string("toTime");

MaintenanceVI::MaintenanceVI(const string& taskName, 
                                     const string& parameterFile, 
                                     GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,VI_VERSION),
  m_resources(),
  m_maintenancePropertySets()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  try
  {
    // put resources in a vector
    m_resources = m_parameterSet.getStringVector(string("resources"));
  }
  catch(Exception& e)
  {
    LOG_ERROR("MaintenanceVI: key \"resources\" not found in parameterset");
  }
}


MaintenanceVI::~MaintenanceVI()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void MaintenanceVI::concrete_handlePropertySetAnswer(GCFEvent& answer)
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
    case F_VGETRESP:
    case F_VCHANGEMSG:
    {
      break;
    }
    case F_EXTPS_LOADED:
    {
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

GCFEvent::TResult MaintenanceVI::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
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

GCFEvent::TResult MaintenanceVI::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:
    {
      // set the __resources datapoint element
      vector<boost::shared_ptr<GCFPValue> > sharedPValueArray;
      
      for(vector<string>::iterator it=m_resources.begin();it!=m_resources.end();++it)
      {
        // the resource string can not be used as name of the reference because
        // the navigator will try to interpret ':' and '_'
        // These are replaced by '-'
        string tempName(*it);
        string replaceChars(":_");
        string::size_type pos = tempName.find_first_of(replaceChars);
        while(pos != string::npos)
        {
          tempName[pos]='-';
          pos = tempName.find_first_of(replaceChars,pos);
        }
        boost::shared_ptr<GCFPVString> newRef(new GCFPVString(tempName + string("=") + (*it)));
        sharedPValueArray.push_back(newRef);
      }
      
      GCFPValueArray refsVector;
      for(vector<boost::shared_ptr<GCFPValue> >::iterator it=sharedPValueArray.begin();it!=sharedPValueArray.end();++it)
      {
        refsVector.push_back((*it).get());
      }
      GCFPVDynArr newChildRefs(LPT_STRING,refsVector);
      m_detailsPropertySet->setValue(VI_PROPNAME_RESOURCES,newChildRefs);
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult MaintenanceVI::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // enter claimed state
      newState  = LOGICALDEVICE_STATE_CLAIMED;
      errorCode = LD_RESULT_NO_ERROR;
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult MaintenanceVI::concrete_claimed_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
      
  return status;
}

GCFEvent::TResult MaintenanceVI::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      // preparing is not necessary
      // enter suspended state
      newState=LOGICALDEVICE_STATE_SUSPENDED;
      errorCode = LD_RESULT_NO_ERROR;
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult MaintenanceVI::concrete_active_state(GCFEvent& event, GCFPortInterface& /*p*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }    
  return status;
}

GCFEvent::TResult MaintenanceVI::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  newState=LOGICALDEVICE_STATE_GOINGDOWN;
  
  switch(event.signal)
  {
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

void MaintenanceVI::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // load the maintenance property sets
  for(vector<string>::iterator it=m_resources.begin();it!=m_resources.end();++it)
  {
    string maintenancePropName((*it) + string("_Maintenance"));
    GCFExtPropertySetSharedPtr maintenancePropertySet(new GCFExtPropertySet(
        maintenancePropName.c_str(),
        TYPE_LCU_PIC_MAINTENANCE.c_str(),
        &m_propertySetAnswer));
    maintenancePropertySet->load();
    m_maintenancePropertySets.push_back(maintenancePropertySet);
  }
}

void MaintenanceVI::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  // set the maintenance from and to properties
  for(vector<GCFExtPropertySetSharedPtr>::iterator it=m_maintenancePropertySets.begin();it!=m_maintenancePropertySets.end();++it)
  {
    GCFPVUnsigned fromTime(m_startTime);
    (*it)->setValue(PROPNAME_FROMTIME,fromTime);
    GCFPVUnsigned toTime(m_stopTime);
    (*it)->setValue(PROPNAME_TOTIME,toTime);
  }
}

void MaintenanceVI::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  // set all resources in maintenance. 
  for(vector<GCFExtPropertySetSharedPtr>::iterator it=m_maintenancePropertySets.begin();it!=m_maintenancePropertySets.end();++it)
  {
    GCFPVUnsigned status(MAINTENANCESTATUS_INMAINTENANCE);
    (*it)->setValue(PROPNAME_STATUS,status);
  }
}

void MaintenanceVI::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void MaintenanceVI::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  // reset all resources from maintenance.
  // set all resources to operational.
  for(vector<GCFExtPropertySetSharedPtr>::iterator it=m_maintenancePropertySets.begin();it!=m_maintenancePropertySets.end();++it)
  {
    GCFPVUnsigned status(MAINTENANCESTATUS_OPERATIONAL);
    (*it)->setValue(PROPNAME_STATUS.c_str(),status);
  }
}

void MaintenanceVI::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void MaintenanceVI::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void MaintenanceVI::concreteHandleTimers(GCFTimerEvent& /*timerEvent*/, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

}

}; // namespace VIC
}; // namespace LOFAR

