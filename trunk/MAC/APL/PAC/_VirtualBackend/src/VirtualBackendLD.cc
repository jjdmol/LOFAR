//#  VirtualBackendLD.cc: Implementation of the Virtual VirtualBackendLD task
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

#include "VirtualBackendLD.h"
#include <boost/shared_ptr.hpp>
#include <Common/lofar_sstream.h>
#include <GCF/GCF_PVString.h>
#include <APLCommon/StartDaemon_Protocol.ph>
#include <ACC/KVpair.h>
#include <set>

using std::set;

namespace LOFAR
{
using namespace ACC;
using namespace APLCommon;
using namespace GCF::Common;
using namespace GCF::TM;
using namespace GCF::PAL;
using namespace ANM;
  
  namespace AVB
  {

INIT_TRACER_CONTEXT(VirtualBackendLD, LOFARLOGGER_PACKAGE);

VirtualBackendLD::VirtualBackendLD(const string& taskName, const string& parameterFile) :
  LogicalDevice(taskName, parameterFile),
  _nodeManager(*this),
  _cepApplication(*this, taskName)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());  
}


VirtualBackendLD::~VirtualBackendLD()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::concrete_handlePropertySetAnswer(GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(answer)).c_str());
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

GCFEvent::TResult VirtualBackendLD::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s", 
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState = LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState = LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:      
      break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_preparing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& /*newState*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_active_state(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  newState = LOGICALDEVICE_STATE_RELEASED;
  return status;
}

void VirtualBackendLD::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  
  _cepAppParams.clear();
  _cepAppParams.adoptFile("CEPAppDefault.param");

  _cepAppParams.replace("AC.application", getName());
  _cepAppParams.replace("AC.resultfile", formatString("./%s_result.log", getName().c_str()));
  
  string processScope("AC.process");
  uint32 nrProcs = m_parameterSet.getInt(formatString("%s[0].count", processScope.c_str()));

  _cepAppParams.replace(KVpair("AC.process[0].count", (int32) nrProcs));
  
  _neededNodes.clear();
  set<string> processInfoCopied;
  string procName, newProcName, nodeName;
  string ldName(getName().c_str());
  
  for (uint32 i = 1; i <= nrProcs; i++) 
  {
    procName = m_parameterSet.getString(formatString("%s[%d].ID", processScope.c_str(), i));
    
    _cepAppParams.add(formatString("AC.process[%d].ID", i), procName);
    
    procName = formatString("AC.%s", procName.c_str());
    uint32 nrOfProcs = indexValue(procName, "()");
    if (nrOfProcs == 0) 
    {
      try
      {
        nodeName = m_parameterSet.getString(formatString("%s.node", procName.c_str()));
        _neededNodes.push_back(nodeName);
      }
      catch (...)
      {
        // skip
      }
      try
      {
        rtrim(procName, "[123456789]");
        if (processInfoCopied.find(procName) == processInfoCopied.end())
        {
          newProcName = procName;
          newProcName.replace(0, strlen("AC"), ldName.c_str(), ldName.length());
          _cepAppParams.adoptCollection(m_parameterSet.makeSubset(procName, newProcName));
          processInfoCopied.insert(procName);
          nodeName = m_parameterSet.getString(formatString("%s[0].node", procName.c_str()));
          _neededNodes.push_back(nodeName);
        }
      }
      catch (...)
      {
        // skip
      }
      continue;
    }

    rtrim(procName, "(0123456789)");
    for (uint32 j = 0; j <= nrOfProcs; j++)
    { 
      try
      {
        newProcName = procName;
        newProcName.replace(0, strlen("AC"), ldName.c_str(), ldName.length());
        _cepAppParams.adoptCollection(m_parameterSet.makeSubset(procName, newProcName));
        nodeName = m_parameterSet.getString(formatString("%s[%d].node", procName.c_str(), j));
        _neededNodes.push_back(nodeName);   
      }
      catch (...)
      {
        // skip
      }
    }
  }
  _neededNodes.unique();
  _nodeManager.claimNodes(_neededNodes);  
}

void VirtualBackendLD::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  

  string paramFileName(formatString("ACC-%s.param", getName().c_str()));
  _cepAppParams.writeFile(paramFileName);
  
  // schedule all ACC commands
  time_t startTime  = getStartTime();
  time_t initTime   = startTime  - _cepAppParams.getTime("AC.timeout.init");
  time_t defineTime = initTime   - _cepAppParams.getTime("AC.timeout.define") - 
                                   _cepAppParams.getTime("AC.timeout.startup");
  time_t bootTime   = defineTime - _cepAppParams.getTime("AC.timeout.createsubsets");
  time_t now = time(0);
  time_t stopTime = getStopTime();
  LOG_DEBUG(formatString(
      "%d boot %s",
      bootTime, ctime(&bootTime)));
      
  LOG_DEBUG(formatString(
      "%d define %s",
      defineTime, ctime(&defineTime)));

  LOG_DEBUG(formatString(
      "%d init %s",
      initTime, ctime(&initTime)));

  LOG_DEBUG(formatString(
      "%d start %s",
      startTime, ctime(&startTime)));

  LOG_DEBUG(formatString(
      "%d now %s time %d",
      now, ctime(&now), time(0)));

  LOG_DEBUG(formatString(
      "%d stop %s",
      stopTime, ctime(&stopTime)));

  if (now > bootTime)
  {
    LOG_WARN("Cannot gurantee all CEP processes are started in time.");
    _doStateTransition(LOGICALDEVICE_STATE_IDLE);
  }
  else
  {
    _cepApplication.boot(bootTime, paramFileName);
  }
}

void VirtualBackendLD::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());  
}

void VirtualBackendLD::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  _nodeManager.releaseNodes(_neededNodes);
}

void VirtualBackendLD::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::concreteHandleTimers(GCFTimerEvent& /*timerEvent*/, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::nodesClaimed(TNodeList& newClaimedNodes, 
                                    TNodeList& releasedNodes,
                                    TNodeList& faultyNodes)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  _doStateTransition(LOGICALDEVICE_STATE_CLAIMED);
}

void VirtualBackendLD::nodesReleased()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  _cepApplication.quit(0);
}

void VirtualBackendLD::appBooted(uint16 result)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  
  {
    time_t startTime  = getStartTime();
    time_t initTime   = startTime  - _cepAppParams.getTime("AC.timeout.init");
    time_t defineTime = initTime   - _cepAppParams.getTime("AC.timeout.define") - 
                                     _cepAppParams.getTime("AC.timeout.startup");
    _cepApplication.define(defineTime);
  }
}

void VirtualBackendLD::appDefined(uint16 result)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))
  {
    time_t startTime  = getStartTime();
    time_t initTime   = startTime  - _cepAppParams.getTime("AC.timeout.init");
  
    _cepApplication.init(initTime);
  }
}

void VirtualBackendLD::appInitialized(uint16 result)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  if (result == AcCmdMaskOk)
  {    
    _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED);
  }
  else if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  
  {
    _cepApplication.run(getStartTime());
  }
}

void VirtualBackendLD::appRunDone(uint16 result)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());  
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))
  {      
    _cepApplication.quit(getStopTime());
  }
}

void VirtualBackendLD::appPaused(uint16 /*result*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::appQuitDone(uint16 result)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  if (result == AcCmdMaskOk)
  {  
    _doStateTransition(LOGICALDEVICE_STATE_RELEASED);  
  }
}

void VirtualBackendLD::appSnapshotDone(uint16 /*result*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::appRecovered(uint16 /*result*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::appReinitialized(uint16 /*result*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::appReplaced(uint16 /*result*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

string VirtualBackendLD::appSupplyInfo(const string& keyList)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  string ret(keyList);
  return ret;
}

void VirtualBackendLD::appSupplyInfoAnswer(const string& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  LOG_INFO_STR("Answer: " << answer);
}

void VirtualBackendLD::myPSAnswer()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::myPSvalueChanged(const string& /*propName*/, const GCF::Common::GCFPValue& /*value*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

  } // namespace AVB
} // namespace LOFAR
