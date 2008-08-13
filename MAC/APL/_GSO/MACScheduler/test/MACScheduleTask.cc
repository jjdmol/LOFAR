//#  MACScheduleTask.cc: 
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

#include <GCF/GCF_PVString.h>
#include "MACScheduleTask.h"

using namespace LOFAR;
using namespace GSO;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

string MACScheduleTask::m_taskName("MACSchedule");

MACScheduleTask::MACScheduleTask(const string& schedule, const string& updateschedule, const string& cancelschedule) :
  GCFTask((State)&MACScheduleTask::initial, m_taskName),
  m_answer(),
  m_extPropsetMacScheduler("GSO_MACScheduler", "TAplMacScheduler", &m_answer),
  m_scheduleNodeId(schedule),
  m_updateScheduleNodeId(updateschedule),
  m_cancelScheduleNodeId(cancelschedule)
{
  m_answer.setTask(this);
  
  m_extPropsetMacScheduler.load();
}

MACScheduleTask::~MACScheduleTask()
{
}

GCFEvent::TResult MACScheduleTask::initial(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("MACScheduleTask(%s)::initial (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      break;
    
    case F_EXTPS_LOADED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&event);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        m_extPropsetMacScheduler.subscribeProp("command");
      }
      else
      {
        GCFTask::stop();
      }
      break;
    }
    
    case F_SUBSCRIBED:
    {
      TRAN(MACScheduleTask::propertiesLoaded);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("MACScheduleTask(%s)::initial, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult MACScheduleTask::propertiesLoaded(GCFEvent& event, GCFPortInterface& p)
{
  LOG_DEBUG(formatString("MACScheduleTask(%s)::propertiesLoaded (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      string command("");
      if(m_scheduleNodeId.length() > 0)
      {
        command="SCHEDULE ";
        command += m_scheduleNodeId;
      }
      else if(m_updateScheduleNodeId.length() > 0)
      {
        command="UPDATESCHEDULE ";
        command += m_updateScheduleNodeId;
      }
      else if(m_cancelScheduleNodeId.length() > 0)
      {
        command="CANCELSCHEDULE ";
        command += m_cancelScheduleNodeId;
      }
      if(command.length() > 0)
      {
        m_extPropsetMacScheduler.setValue("command",command);
        p.setTimer(1L);
      }
      break;
    }
    
    case F_VCHANGEMSG:
      m_extPropsetMacScheduler.unsubscribeProp("command");
      m_extPropsetMacScheduler.unload();
      break;
      
    case F_EXTPS_UNLOADED:
      GCFTask::stop();
      break;
    
    default:
      LOG_DEBUG(formatString("MACScheduleTask(%s)::propertiesLoaded, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}
