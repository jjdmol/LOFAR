//#  AVITestDriverTask.cc: Implementation of the 
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

#include "APLCommon/APL_Defines.h"
#include "APLCommon/StartDaemon.h"
#include "AVITestDriverTask.h"

#include <stdio.h>
#include <blitz/array.h>
#include <time.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/ParameterSet.h>

using namespace GCF;
using namespace std;

const char SCOPE_VIC_VIStartDaemon[] = "CCU1_VIC_VIStartDaemon";
const char SCOPE_VIC_VI1[] = "VIC_VI1";


namespace LOFAR
{

using namespace APLCommon;

namespace AVI
{

string AVITestDriverTask::m_taskName("AVITestDriver");

AVITestDriverTask::AVITestDriverTask() :
  GCFTask((State)&AVITestDriverTask::initial, m_taskName),
  m_answer(),
  m_extPropSetCCU1VISD(SCOPE_VIC_VIStartDaemon,StartDaemon::PSTYPE_STARTDAEMON.c_str(),&m_answer),
  m_extPropSetVI1(SCOPE_VIC_VI1,"TAplVicVI",&m_answer)
{
  m_answer.setTask(this);

//  ParameterSet::instance()->adoptFile("VirtualInstrument.conf");
}

AVITestDriverTask::~AVITestDriverTask()
{
}

GCFEvent::TResult AVITestDriverTask::initial(GCFEvent& event, GCFPortInterface& port)
{
  LOG_DEBUG(formatString("AVITestDriverTask(%s)::initial (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      m_extPropSetCCU1VISD.load();
      m_extPropSetVI1.load();
      TRAN(AVITestDriverTask::enabled);
      
      break;

    case F_ENTRY:
      break;
    
    case F_CONNECTED:
    {
      LOG_DEBUG(formatString("port '%s' connected", port.getName().c_str()));
      break;
    }
  
    case F_DISCONNECTED:
    {
      port.setTimer((long)3); // try again in 3 seconds
      LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
      port.close();
      break;
    }

    case F_TIMER:
    {
      LOG_INFO(formatString("port '%s' retry of open...", port.getName().c_str()));
      port.open();
      break;
    }

    case F_EXIT:
    {
      break;
    }
      
    default:
      LOG_DEBUG(formatString("AVITestDriverTask(%s)::initial, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVITestDriverTask::enabled(GCFEvent& event, GCFPortInterface& port)
{
  LOG_DEBUG(formatString("AVITestDriverTask(%s)::enabled (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      break;
    }

    case F_TIMER:
    {
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&event);
      assert(pPropAnswer);
      GCFPVUnsigned pvUnsigned;
      GCFPVDouble   pvDouble;
      GCFPVBool     pvBool;
      GCFPVString   pvString;
      switch(pPropAnswer->pValue->getType())
      {
        case LPT_BOOL:
          pvBool.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %s=%d", pPropAnswer->pPropName, pvBool.getValue()));
          break;
        case LPT_UNSIGNED:
          pvUnsigned.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %s=%d", pPropAnswer->pPropName, pvUnsigned.getValue()));
          break;
        case LPT_DOUBLE:
          pvDouble.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %s=%f", pPropAnswer->pPropName, pvDouble.getValue()));
          break;
        case LPT_STRING:
          pvString.copy(*pPropAnswer->pValue);
          LOG_INFO(formatString("property changed: %s=%s", pPropAnswer->pPropName, pvString.getValue().c_str()));
          break;
        case NO_LPT:
        case LPT_CHAR:
        case LPT_INTEGER:
        case LPT_BIT32:
        case LPT_BLOB:
        case LPT_REF:
        case LPT_DATETIME:
        case LPT_DYNARR:
        case LPT_DYNBOOL:
        case LPT_DYNCHAR:
        case LPT_DYNUNSIGNED:
        case LPT_DYNINTEGER:
        case LPT_DYNBIT32:
        case LPT_DYNBLOB:
        case LPT_DYNREF:
        case LPT_DYNDOUBLE:
        case LPT_DYNDATETIME:
        case LPT_DYNSTRING:
        default:
          break;
      }

      break;
    }
     
    case F_DISCONNECTED:
    {
     LOG_DEBUG(formatString("port %s disconnected", port.getName().c_str()));
     port.close();

     TRAN(AVITestDriverTask::initial);
      break;
    }

    case F_EXIT:
    {
      break;
    }

    default:
      LOG_DEBUG(formatString("AVITestDriverTask(%s)::enabled, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

} // namespace AVI


} // namespace LOFAR

