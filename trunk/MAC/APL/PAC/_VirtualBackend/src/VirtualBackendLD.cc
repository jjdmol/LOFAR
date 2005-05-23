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

#include <lofar_config.h>

#include "VirtualBackendLD.h"
#include <APLCommon/LogicalDevice_Protocol.ph>
#include <ACC/KVpair.h>
#include <GCF/GCF_PVChar.h>
#include <GCF/GCF_PVString.h>
#include <GCF/Utils.h>

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

PROPERTYCONFIGLIST_BEGIN(detailsPropertySetConf)
  PROPERTYCONFIGLIST_ITEM("quality", GCF_READABLE_PROP, "d")
  PROPERTYCONFIGLIST_ITEM("__stationCorrelator", GCF_READABLE_PROP, "StationCorrelator=CEP_SCD")
PROPERTYCONFIGLIST_END

const string VirtualBackendLD::VB_VERSION = string("1.0");

VirtualBackendLD::VirtualBackendLD(const string& taskName, 
                                   const string& parameterFile,
                                   GCF::TM::GCFTask* pStartDaemon) :
  LogicalDevice(taskName, parameterFile, pStartDaemon, VB_VERSION),
  _cepApplication(*this, taskName),
  _qualityGuard(*this),
  _rstoID(0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());  
  m_detailsPropertySet->initProperties(detailsPropertySetConf);
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
}

GCFEvent::TResult VirtualBackendLD::concrete_initial_state(
                                              GCFEvent& event, 
                                              GCFPortInterface& /*p*/, 
                                              TLogicalDeviceState& /*newState*/, 
                                              TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s", 
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_idle_state(
                                              GCFEvent& event, 
                                              GCFPortInterface& /*p*/, 
                                              TLogicalDeviceState& /*newState*/, 
                                              TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (event.signal)
  {
    case F_ENTRY:      
      if (_qualityGuard.isQualityLow() || errorCode == LD_RESULT_TIMING_FAILURE)
      {
        time_t rsto(0);
        try 
        {
          rsto = m_parameterSet.getTime("rescheduleTimeOut");
        }
        catch (...)
        {
        }
        if (rsto == 0)
        {
          _qualityGuard.stopMonitoring();
        }
        else
        {
          _rstoID = m_serverPort.setTimer((double) rsto);
        }
      }
      break;
    
    case LOGICALDEVICE_SCHEDULE:
      if (_rstoID > 0)
      {
        m_serverPort.cancelTimer(_rstoID);
        _rstoID = 0;
      }
      break;
      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_claiming_state(
                                              GCFEvent& event, 
                                              GCFPortInterface& /*p*/, 
                                              TLogicalDeviceState& /*newState*/, 
                                              TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_claimed_state(
                                              GCFEvent& event, 
                                              GCFPortInterface& /*p*/, 
                                              TLogicalDeviceState& /*newState*/, 
                                              TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_preparing_state(
                                              GCFEvent& event, 
                                              GCFPortInterface& /*p*/, 
                                              TLogicalDeviceState& /*newState*/, 
                                              TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_active_state(
                                              GCFEvent& event, 
                                              GCFPortInterface& /*p*/, 
                                              TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  return status;
}

GCFEvent::TResult VirtualBackendLD::concrete_releasing_state(
                                              GCFEvent& event, 
                                              GCFPortInterface& /*p*/, 
                                              TLogicalDeviceState& newState, 
                                              TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, formatString("%s - event=%s",
      getName().c_str(),
      evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  
  if (errorCode == LD_RESULT_NO_ERROR)
  {
    _qualityGuard.stopMonitoring();
  }
  else
  {
    newState = LOGICALDEVICE_STATE_IDLE;
  }
  
  return status;
}

void VirtualBackendLD::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  
  _cepAppParams.clear();
  _cepAppParams.adoptFile("CEPAppDefault.param");

  _cepAppParams.replace("AC.application", getName());
  _cepAppParams.replace("AC.resultfile", formatString("./ACC-%s_result.param", getName().c_str()));
  
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
        _neededNodes.insert(nodeName);
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
          _neededNodes.insert(nodeName);
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
        _neededNodes.insert(nodeName);   
      }
      catch (...)
      {
        // skip
      }
    }
  }
 
  _qualityGuard.monitorNodes(_neededNodes);  
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
    system(formatString("writeCEPparamFile %s", getName().c_str()).c_str());
    LOG_WARN("Cannot gurantee all CEP processes are started in time.");
    _doStateTransition(LOGICALDEVICE_STATE_RELEASING, LD_RESULT_TIMING_FAILURE);
  }
  else
  {
    switch (_cepApplication.getLastOkCmd())
    {
      case ACCmdNone:
        _cepApplication.boot(bootTime, paramFileName);
        break;
        
      case ACCmdBoot:
        _cepApplication.define(defineTime);
        break;
        
      case ACCmdDefine:
      case ACCmdInit:
      case ACCmdRun:
        _cepApplication.recover(0, "snapshot-DB");
        break;
              
      default:
        assert(0);
        break;
    }    
    system(formatString("writeCEPparamFile %s", getName().c_str()).c_str());
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
  system(formatString("readCEPResultFile %s", getName().c_str()).c_str());  
  _resultParams.adoptFile(formatString("./ACC-%s_result.param", getName().c_str()));
  _resultParams.replace(KVpair(formatString("%s.quality", getName().c_str()), (int) _qualityGuard.getQuality()));
  if (!_resultParams.isDefined(formatString("%s.faultyNodes", getName().c_str())))
  {
    _resultParams.add(formatString("%s.faultyNodes", getName().c_str()), "");
  }
  _resultParams.writeFile(formatString("%s_result.param", getName().c_str()));
}

void VirtualBackendLD::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
}

void VirtualBackendLD::concreteHandleTimers(GCFTimerEvent& timerEvent, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  if (&port == &m_serverPort && _rstoID == timerEvent.id)
  {
    _rstoID = 0;
    _qualityGuard.stopMonitoring();
    //_cepApplication.quit(0);
  }
}

void VirtualBackendLD::qualityGuardStarted()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  _doStateTransition(LOGICALDEVICE_STATE_CLAIMED);
}

void VirtualBackendLD::qualityGuardStopped()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  _doStateTransition(LOGICALDEVICE_STATE_GOINGDOWN);
}

void VirtualBackendLD::lowQuality(TNodeList& faultyNodes)
{  
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  string faultyNodeList;
  Utils::convSetToString(faultyNodeList, faultyNodes);
  _resultParams.replace(KVpair(formatString("%s.quality", getName().c_str()), (int) _qualityGuard.getQuality()));
  _resultParams.replace(formatString("%s.faultyNodes", getName().c_str()), faultyNodeList);
  switch (getLogicalDeviceState())
  {
    case LOGICALDEVICE_STATE_ACTIVE:
    case LOGICALDEVICE_STATE_PREPARING:
    case LOGICALDEVICE_STATE_SUSPENDED:
      //_cepApplication.cancelCmdQueue(); // not in this increment
      //_cepApplication.snapshot(0, "snapshot-DB"); // not in this increment
      _cepApplication.quit(0);
      break;

    default:
      break;  
  }
  _doStateTransition(LOGICALDEVICE_STATE_RELEASING, LD_RESULT_LOW_QUALITY);
}

void VirtualBackendLD::qualityChanged()
{
  m_detailsPropertySet->setValue("quality", GCFPVChar(_qualityGuard.getQuality()));
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
    //_qualityGuard.stopMonitoring(); // not in this increment
  }
}

void VirtualBackendLD::appSnapshotDone(uint16 /*result*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  time_t rsto(0);
  try 
  {
    rsto = m_parameterSet.getTime("rescheduleTimeOut");
  }
  catch (...) {}
  _cepApplication.pause(0, rsto, "condition");
}

void VirtualBackendLD::appRecovered(uint16 /*result*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, getName().c_str());
  
  time_t startTime  = getStartTime();
  time_t reinitTime = startTime  - _cepAppParams.getTime("AC.timeout.reinit");
  
  string paramFileName(formatString("ACC-%s.param", getName().c_str()));
  
  _cepApplication.reinit(reinitTime, paramFileName);
}

void VirtualBackendLD::appReinitialized(uint16 result)
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

  } // namespace AVB
} // namespace LOFAR
